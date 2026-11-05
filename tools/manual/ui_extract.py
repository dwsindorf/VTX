#!/usr/bin/env python3
"""
ui_extract.py - extract the VTX user interface from the wxWidgets sources.

Reads wx/*.cpp (and the on-screen key help in includes/SceneClass.cpp) and
returns a structured model of:
  * the main window menus and toolbar      (wx/VtxFrame.cpp)
  * keyboard commands per F-key mode       (includes/SceneClass.cpp show_mode_info)
  * every dialog -> object type -> tab page -> group -> control
    with label, control kind, range, default value, choices and tooltip
  * each object's right-click (tree) menu

Usage:  python3 ui_extract.py [VTX_ROOT] > ui.json
Stdlib only - no third-party packages.
"""
import json
import os
import re
import sys

# ---------------------------------------------------------------------------
# source helpers
# ---------------------------------------------------------------------------
STR = r'(?:wxT|_T|_)?\(?\s*"((?:[^"\\]|\\.)*)"\s*\)?'   # "x", wxT("x"), _T("x")


def strip_comments(src):
    """Remove // and /* */ comments, keeping string literals intact."""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if c == '"' or c == "'":
            q, j = c, i + 1
            while j < n and src[j] != q:
                j += 2 if src[j] == '\\' else 1
            out.append(src[i:j + 1]); i = j + 1
        elif src.startswith('//', i):
            j = src.find('\n', i); i = n if j < 0 else j
        elif src.startswith('/*', i):
            j = src.find('*/', i + 2); i = n if j < 0 else j + 2
        else:
            out.append(c); i += 1
    return ''.join(out)


def drop_disabled(src):
    """Drop #if 0 ... #endif blocks (commonly used to park old UI code)."""
    lines, out, depth = src.split('\n'), [], 0
    for ln in lines:
        s = ln.strip()
        if depth:
            if re.match(r'#\s*if', s):
                depth += 1
            elif re.match(r'#\s*endif', s):
                depth -= 1
            elif re.match(r'#\s*else', s) and depth == 1:
                depth = 0
            continue
        if re.match(r'#\s*if\s+0\b', s):
            depth = 1
            continue
        out.append(ln)
    return '\n'.join(out)


def unescape(s):
    return s.replace('\\"', '"').replace('\\n', ' ').replace('\\t', '\t').strip()


def body_of(src, start):
    """Return text of the {...} block starting at the first '{' after start."""
    i = src.find('{', start)
    if i < 0:
        return ''
    depth, j = 0, i
    while j < len(src):
        c = src[j]
        if c == '"':
            j += 1
            while j < len(src) and src[j] != '"':
                j += 2 if src[j] == '\\' else 1
        elif c == '{':
            depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                return src[i + 1:j]
        j += 1
    return src[i + 1:]


def functions(src, cls):
    """{name: body} for every Class::name(...) { ... } definition."""
    res = {}
    for m in re.finditer(r'\b%s::(\w+)\s*\([^;{]*\)\s*(?:const\s*)?\{' % re.escape(cls), src):
        res[m.group(1)] = body_of(src, m.end() - 1)
    return res


def statements(body):
    """Split a function body into top-level-ish statements (on ';' outside strings/parens)."""
    out, cur, depth, i = [], [], 0, 0
    while i < len(body):
        c = body[i]
        if c == '"':
            j = i + 1
            while j < len(body) and body[j] != '"':
                j += 2 if body[j] == '\\' else 1
            cur.append(body[i:j + 1]); i = j + 1; continue
        if c in '([':
            depth += 1
        elif c in ')]':
            depth -= 1
        if c == ';' and depth <= 0:
            out.append(''.join(cur).strip()); cur = []; depth = 0
        elif c in '{}' and depth <= 0:
            if ''.join(cur).strip():
                out.append(''.join(cur).strip())
            cur = []
        else:
            cur.append(c)
        i += 1
    if ''.join(cur).strip():
        out.append(''.join(cur).strip())
    return [re.sub(r'\s+', ' ', s) for s in out if s]


def split_args(s):
    """Split a C argument list at top-level commas."""
    args, cur, depth, i = [], [], 0, 0
    while i < len(s):
        c = s[i]
        if c == '"':
            j = i + 1
            while j < len(s) and s[j] != '"':
                j += 2 if s[j] == '\\' else 1
            cur.append(s[i:j + 1]); i = j + 1; continue
        if c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
        if c == ',' and depth == 0:
            args.append(''.join(cur).strip()); cur = []
        else:
            cur.append(c)
        i += 1
    if ''.join(cur).strip():
        args.append(''.join(cur).strip())
    return args


def call_args(stmt, name):
    """Args of the first call `name(...)` in stmt, or None."""
    m = re.search(r'\b' + re.escape(name) + r'\s*\(', stmt)
    if not m:
        return None
    depth, j = 1, m.end()
    while j < len(stmt) and depth:
        if stmt[j] == '"':
            j += 1
            while j < len(stmt) and stmt[j] != '"':
                j += 2 if stmt[j] == '\\' else 1
        elif stmt[j] == '(':
            depth += 1
        elif stmt[j] == ')':
            depth -= 1
        j += 1
    return split_args(stmt[m.end():j - 1])


def lit(arg):
    """String literal value of an argument, or None."""
    if arg is None:
        return None
    m = re.fullmatch(r'\s*(?:wxT|_T|_|wxString)?\s*\(?\s*"((?:[^"\\]|\\.)*)"\s*\)?\s*', arg)
    return unescape(m.group(1)) if m else None


def num(arg):
    if arg is None:
        return None
    a = arg.strip().rstrip('f')
    try:
        v = float(a)
        return int(v) if v == int(v) and 'e' not in a.lower() and '.' not in a else v
    except ValueError:
        return a  # expression / constant name: keep as text


def string_arrays(src):
    """name -> [strings] for `wxString name[] = { "a", "b" }` style arrays."""
    arrays = {}
    for m in re.finditer(r'(?:wxString|const\s+char\s*\*|char\s*\*|wxChar\s*\*)\s+(\w+)\s*\[\s*\w*\s*\]\s*=\s*\{(.*?)\}\s*;', src, re.S):
        vals = [unescape(v) for v in re.findall(r'"((?:[^"\\]|\\.)*)"', m.group(2))]
        arrays[m.group(1)] = vals
    return arrays


# ---------------------------------------------------------------------------
# tab classes
# ---------------------------------------------------------------------------
CONTROL_KINDS = {
    'SliderCtrl': 'slider', 'ExprSliderCtrl': 'slider/expression', 'ColorSlider': 'color slider',
    'ExprTextCtrl': 'expression', 'TextCtrl': 'text', 'StaticTextCtrl': 'readout',
    'wxCheckBox': 'checkbox', 'wxButton': 'button', 'wxChoice': 'choice',
    'wxRadioBox': 'radio', 'wxColourPickerCtrl': 'color picker', 'wxComboBox': 'combo',
}
LABEL_ARG = {   # index of the label argument (after panel, id)
    'SliderCtrl': 2, 'ExprSliderCtrl': 2, 'ColorSlider': 2, 'ExprTextCtrl': 2,
    'TextCtrl': 2, 'StaticTextCtrl': 2, 'wxCheckBox': 2, 'wxButton': 2, 'wxRadioBox': 2,
}
NEW_RE = re.compile(r'(?:([\w\[\]\.>-]+)\s*=\s*)?new\s+(%s)\s*\(' % '|'.join(CONTROL_KINDS))


def parse_page(body, arrays):
    """Parse one Add*Tab(panel) body into groups of controls."""
    arrays = dict(arrays, **string_arrays(body))   # local arrays shadow file-level ones
    groups, cur, by_var, pending_label = [], None, {}, None
    unlabeled = []          # controls waiting for a following wxStaticText label
    box_of = {}             # sizer var -> group (static boxes)
    row_of = {}             # sizer var -> [controls] (plain row/column sizers)
    placed = {}             # id(control) -> group it was Add()-ed into

    def group(label):
        g = {'group': label, 'controls': []}
        groups.append(g)
        return g

    for st in statements(body):
        m = re.search(r'new\s+wxStaticBoxSizer\s*\(', st)
        if m:
            a = call_args(st, 'wxStaticBoxSizer') or []
            label = next((lit(x) for x in a if lit(x) is not None), None)
            cur = group(label or '')
            v = re.match(r'(?:[\w:*\s]*?[\s*])?(\w+)\s*=', st)
            if v:
                box_of[v.group(1)] = cur
            continue
        m = re.match(r'(?:[\w:*\s]*?[\s*])?(\w+)\s*=\s*new\s+(?:wxBoxSizer|wxFlexGridSizer|wxGridSizer)\b', st)
        if m:
            row_of[m.group(1)] = []
            box_of.pop(m.group(1), None)
            continue
        m = re.match(r'(\w+)\s*->\s*Add\s*\(\s*(?:new\s+wxStaticText[^;]*$)?', st)
        if m and (m.group(1) in box_of or m.group(1) in row_of) and 'new wxStaticText' not in st:
            a = call_args(st, 'Add') or []
            child = re.match(r'\s*(\w+)(?:\s*->\s*getSizer\s*\(\s*\))?', a[0]) if a else None
            items = []
            if child:
                ch = child.group(1)
                if ch in by_var:
                    items = [by_var[ch]]
                elif ch in row_of:
                    items = list(row_of[ch])
                elif ch in box_of:
                    items = []
            if m.group(1) in box_of:
                for c in items:
                    placed[id(c)] = box_of[m.group(1)]
            else:
                row_of[m.group(1)].extend(items)
            continue
        m = re.search(r'new\s+wxStaticText\s*\(', st)
        if m:
            a = call_args(st, 'wxStaticText') or []
            label = next((lit(x) for x in a if lit(x)), None)
            if label and unlabeled:
                unlabeled.pop(0)['label'] = label
            else:
                pending_label = label
            continue
        m = NEW_RE.search(st)
        if m:
            var, cls = m.group(1), m.group(2)
            a = call_args(st[m.start():], cls) or []
            c = {'kind': CONTROL_KINDS[cls]}
            if var:
                c['_var'] = var.split('->')[-1].split('.')[-1]
            if cls in LABEL_ARG and len(a) > LABEL_ARG[cls]:
                c['label'] = lit(a[LABEL_ARG[cls]])
            if cls == 'wxChoice':
                c['label'] = pending_label
                arr = next((x for x in reversed(a) if x.strip() in arrays), None)
                if arr:
                    c['choices'] = arrays[arr.strip()]
            if cls == 'wxRadioBox':
                arr = next((x for x in a if x.strip() in arrays), None)
                if arr:
                    c['choices'] = arrays[arr.strip()]
            if cls == 'wxColourPickerCtrl':
                c['label'] = pending_label
            if cls in ('wxChoice', 'wxColourPickerCtrl', 'wxComboBox'):
                pending_label = None
            if cur is None:
                cur = group('')
            if not c.get('label'):
                if cls in ('wxButton',):
                    continue                    # icon/helper button
                if cur['group'] and not any(x.get('label') == cur['group'] for x in cur['controls']):
                    c['label'] = cur['group']   # e.g. Layer > Width: slider named by its box
                elif cls not in ('wxChoice', 'wxColourPickerCtrl'):
                    unlabeled.append(c)
            cur['controls'].append(c)
            if var:
                by_var[var.split('->')[-1].split('.')[-1]] = c
            continue
        # property setters on a known control: X->setRange(a,b) etc.
        m = re.match(r'([\w\[\]]+)\s*(?:->|\.)\s*(\w+)\s*\(', st)
        if m and m.group(1) in by_var:
            c, meth = by_var[m.group(1)], m.group(2)
            a = call_args(st, meth) or []
            if meth == 'setRange' and len(a) >= 2:
                c['range'] = [num(a[0]), num(a[1])]
            elif meth == 'setValue' and a and 'default' not in c:
                v = num(a[0])
                c['default'] = v
            elif meth in ('SetToolTip', 'setToolTip') and a and lit(a[0]):
                c['tooltip'] = lit(a[0])
            elif meth == 'SetSelection' and a and 'choices' in c:
                try:
                    c['default'] = c['choices'][int(a[0])]
                except (ValueError, IndexError):
                    pass
            elif meth == 'SetValue' and a and c['kind'] == 'checkbox':
                c['default'] = 'on' if a[0].strip() == 'true' else 'off'
    # move controls into the static box they were actually Add()-ed to
    allc = [(g, c) for g in groups for c in g['controls']]
    for g in groups:
        g['controls'] = []
    for g, c in allc:
        placed.get(id(c), g)['controls'].append(c)
    for c in unlabeled:             # still unnamed: derive from the variable name
        if not c.get('label') and c.get('_var'):
            words = re.sub(r'^m_', '', c['_var'])
            words = re.sub(r'(Slider|Sldr|Ctrl|Expr|Text)$', '', words)
            words = re.sub(r'(?<=[a-z])(?=[A-Z])|_', ' ', words).strip().lower()
            c['label'] = words.capitalize() if words else None
    for g in groups:
        for c in g['controls']:
            c.pop('_var', None)
        g['controls'] = [c for c in g['controls'] if c.get('label') or c['kind'] in ('choice', 'color picker')]
        for c in g['controls']:
            if not c.get('label'):
                c['label'] = g['group'] or c['kind'].capitalize()
    return [g for g in groups if g['controls']]


def parse_menu(funcs):
    """Right-click menu items from showMenu()."""
    body = funcs.get('showMenu')
    if not body:
        return []
    items = []
    for st in statements(body):
        for meth in ('AppendCheckItem', 'Append', 'AppendSubMenu'):
            a = call_args(st, meth)
            if a is not None and re.search(r'\bmenu\s*\.\s*' + meth, st):
                label = next((lit(x) for x in a if lit(x)), None)
                if label:
                    items.append(label)
                break
        if 'AddFileMenu' in st:
            items += ['Save..', 'Randomize', 'Default', 'Replace with..', 'Remove file..']
    seen, out = set(), []
    for i in items:
        if i not in seen:
            seen.add(i); out.append(i)
    return out


def parse_tabs_file(path):
    cls = os.path.splitext(os.path.basename(path))[0]
    src = drop_disabled(strip_comments(open(path, encoding='latin-1').read()))
    arrays = string_arrays(src)
    funcs = functions(src, cls)
    create = funcs.get('Create', '')
    pages, last_fn, page_vars = [], None, {}
    for st in statements(create):
        m = re.match(r'(\w+)\s*\(\s*(?:page|panel|\w+)\s*\)$', st)
        if m and m.group(1).startswith('Add'):
            last_fn = m.group(1)
            continue
        m = re.match(r'(\w+)\s*=\s*page$', st)
        if m and last_fn:
            page_vars[m.group(1)] = last_fn
            last_fn = None
            continue
        a = call_args(st, 'AddPage')
        if a is not None and len(a) >= 2:
            name = lit(a[1])
            if name and last_fn in funcs:
                pages.append({'tab': name, 'groups': parse_page(funcs[last_fn], arrays)})
            elif name:
                pages.append({'tab': name, 'groups': []})
            last_fn = None
    # pages built in Create but added elsewhere (shown only in some modes)
    for fn_body in funcs.values():
        for st in statements(fn_body):
            a = call_args(st, 'AddPage')
            if a and len(a) >= 2 and a[0].strip() in page_vars:
                name = lit(a[1])
                if name and not any(p['tab'] == name for p in pages):
                    pages.append({'tab': name, 'groups': parse_page(funcs[page_vars[a[0].strip()]], arrays),
                                  'conditional': True})
    if not pages:   # panels built directly in a builder function
        for fn, body in funcs.items():
            if fn == 'buildUI' or (fn.startswith('Add') and fn.endswith(('Tab', 'Panel', 'Controls'))):
                pages.append({'tab': re.sub(r'^Add|Tab$|Panel$|^buildUI$', '', fn) or None,
                              'groups': parse_page(body, arrays)})
    return {'class': cls, 'pages': pages, 'menu': parse_menu(funcs)}


# ---------------------------------------------------------------------------
# dialog structure (which tab class appears where)
# ---------------------------------------------------------------------------
def scene_dialog_objects(wx):
    src = strip_comments(open(os.path.join(wx, 'VtxSceneDialog.cpp'), encoding='latin-1').read())
    objs = []
    for m in re.finditer(r'SET_TABS_OBJECT\s*\(\s*TN_(\w+)\s*,\s*(\w+)\s*\)', src):
        name = m.group(1).replace('PLANT_', '').replace('_', ' ').title()
        name = {'Fchnl': 'Fractal', 'Rocks3D': 'Rocks 3D'}.get(name, name)
        objs.append({'object': name, 'class': m.group(2)})
    return objs


def image_dialog_tabs(wx):
    src = strip_comments(open(os.path.join(wx, 'VtxImageDialog.cpp'), encoding='latin-1').read())
    var_cls = dict(re.findall(r'(m_\w+)\s*=\s*new\s+(Vtx\w+)\s*\(', src))
    tabs = []
    for m in re.finditer(r'AddPage\s*\(\s*(m_\w+)\s*,\s*' + STR, src):
        if m.group(1) in var_cls:
            tabs.append({'tab': unescape(m.group(2)), 'class': var_cls[m.group(1)]})
    return tabs


def function_dialog_tabs(wx):
    src = strip_comments(open(os.path.join(wx, 'VtxFunctDialog.cpp'), encoding='latin-1').read())
    return [{'function': m.group(1).title(), 'class': m.group(2)}
            for m in re.finditer(r'functs\[ID_(\w+)\]\s*=\s*new\s+(Vtx\w+)\s*\(', src)
            if m.group(1) != 'NOEDIT']


# ---------------------------------------------------------------------------
# main window: menus, toolbar
# ---------------------------------------------------------------------------
def main_menus(wx):
    src = strip_comments(open(os.path.join(wx, 'VtxFrame.cpp'), encoding='latin-1').read())
    menus, cur, sub = [], None, None
    for st in statements(src):
        if re.match(r'menu\s*=\s*new\s+wxMenu', st):
            cur = []; continue
        if re.match(r'submenu\s*=\s*new\s+wxMenu', st):
            sub = []; continue
        m = re.match(r'(menu|submenu)\s*->\s*(Append|AppendCheckItem)\s*\(', st)
        if m:
            a = call_args(st, m.group(2))
            label = next((lit(x) for x in a if lit(x) is not None), None)
            if label is None:
                continue
            help_ = None
            lits = [lit(x) for x in a if lit(x) is not None]
            if len(lits) > 1:
                help_ = lits[1]
            text, _, accel = label.partition('\t')
            item = {'label': text.replace('&', '').strip(), 'key': accel.strip() or None}
            if help_:
                item['help'] = help_
            if m.group(1) == 'submenu':
                if sub is not None:
                    sub.append(item)
            elif cur is not None:
                if a and a[-1].strip() == 'submenu':
                    item['items'] = sub = [] if sub is None else sub
                cur.append(item)
            continue
        m = re.match(r'menubar\s*->\s*Append\s*\(\s*menu\s*,', st)
        if m and cur is not None:
            a = call_args(st, 'Append')
            menus.append({'menu': lit(a[1]).replace('&', ''), 'items': cur})
            cur = None
    tools = []
    for m in re.finditer(r'AddTool\s*\(\s*\w+\s*,\s*' + STR + r'\s*,[^,]+,\s*' + STR, src):
        tools.append({'label': unescape(m.group(1)), 'tooltip': unescape(m.group(2))})
    return menus, tools


# ---------------------------------------------------------------------------
# keyboard: on-screen key help (F1..F6 modes)
# ---------------------------------------------------------------------------
def key_help(includes):
    src = strip_comments(open(os.path.join(includes, 'SceneClass.cpp'), encoding='latin-1').read())
    i = src.find('Scene::show_mode_info')
    body = body_of(src, i) if i >= 0 else ''
    modes = []
    for block in re.split(r'\bcase\s+\w+\s*:', body)[1:]:
        lines = re.findall(r'draw_string\s*\(\s*(\w+)\s*,\s*"((?:[^"\\]|\\.)*)"', block)
        title = next((t for c, t in lines if c == 'DATA_COLOR' and t.startswith(('[F', 'F1'))), None)
        if not title:
            continue
        mode = re.search(r'\[(F\d):([^\]]+)\]', title)
        entries = []
        for c, t in lines:
            if c != 'MENU_COLOR':
                continue
            t = unescape(t)
            t = re.sub(r'%s', '', t).replace('[%c]', '').replace('%c', '')
            entries.append(t.rstrip())
        modes.append({'key': mode.group(1), 'mode': mode.group(2), 'lines': entries})
    return modes


def key_table(includes):
    """Every single-character key command defined in KeyIF.h, in file order."""
    src = strip_comments(open(os.path.join(includes, 'KeyIF.h'), encoding='latin-1').read())
    keys = []
    for m in re.finditer(r"\b(KEY_\w+)\s*=\s*('(?:\\.|[^'])'|0x[0-9a-fA-F]+)", src):
        name, val = m.group(1), m.group(2)
        if val.startswith("'"):
            ch = val[1:-1].replace('\\\\', '\\')
            key = {' ': 'Space'}.get(ch, ch)
        else:
            code = int(val, 16)
            key = {0x15: 'Ctrl+u', 0x04: 'Ctrl+d'}.get(code)
            if 0xffbe <= code <= 0xffc7:
                key = 'F%d' % (code - 0xffbe + 1)
            key = key or {0xff51: 'Left arrow', 0xff53: 'Right arrow',
                          0xff52: 'Up arrow', 0xff54: 'Down arrow'}.get(code)
        if key and not any(k['name'] == name for k in keys):
            keys.append({'key': key, 'name': name})
    return keys


# ---------------------------------------------------------------------------
def extract(root):
    wx, inc = os.path.join(root, 'wx'), os.path.join(root, 'includes')
    classes = {}
    for f in sorted(os.listdir(wx)):
        if re.match(r'Vtx\w+(Tabs|Funct)\.cpp$', f):
            classes[f[:-4]] = parse_tabs_file(os.path.join(wx, f))
    menus, tools = main_menus(wx)
    return {
        'menus': menus,
        'toolbar': tools,
        'keys': key_help(inc),
        'key_table': key_table(inc),
        'properties': scene_dialog_objects(wx),
        'image_editor': image_dialog_tabs(wx),
        'functions': function_dialog_tabs(wx),
        'classes': classes,
    }


if __name__ == '__main__':
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), '..', '..')
    json.dump(extract(root), sys.stdout, indent=1)
