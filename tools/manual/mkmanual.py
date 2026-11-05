#!/usr/bin/env python3
"""
mkmanual.py - build or refresh the VTX User Manual (Docs/VTX-User-Manual.odt).

The manual is an ordinary LibreOffice document that you edit by hand, except
for one named section, "VTX_Reference", which holds Part II (menus, keys and
every dialog control). That section is generated from the source code by
ui_extract.py and is replaced each time this script runs - so never edit text
inside it; put control descriptions in control_notes.txt instead.

  python3 mkmanual.py update  Docs/VTX-User-Manual.odt [-o OUT.odt] [--root DIR]
      Regenerate Part II in place (or into OUT.odt). Everything outside the
      VTX_Reference section is left exactly as it is.

  python3 mkmanual.py new  Docs/VTX-User-Manual.odt --guide tools/manual/guide.md
                       [--template Docs/VTX-Theory-of-Operation.odt] [--root DIR]
      Create the manual from scratch: Part I from the guide (Markdown subset),
      page styles and fonts from the template, Part II generated.
      (One-time use - after that, edit the .odt in LibreOffice.)

Stdlib only.
"""
import argparse
import datetime
import os
import re
import sys
import zipfile
from xml.sax.saxutils import escape

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ui_extract  # noqa: E402

SECTION = 'VTX_Reference'
NS = ('xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
      'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
      'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
      'xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" '
      'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
      'xmlns:xlink="http://www.w3.org/1999/xlink" '
      'xmlns:dc="http://purl.org/dc/elements/1.1/" '
      'xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" '
      'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" '
      'xmlns:loext="urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0" '
      'office:version="1.3"')
NAVY, HEADBG, RULE, NOTEBG = '#1f3a5f', '#dde6f0', '#b4c2d3', '#eef3f8'

# ---------------------------------------------------------------------------
# named styles the manual relies on (added to styles.xml when missing)
# ---------------------------------------------------------------------------
NAMED_STYLES = {
    'VTX_20_Note': f'''<style:style style:name="VTX_20_Note" style:display-name="VTX Note" style:family="paragraph" style:parent-style-name="Text_20_body">
  <style:paragraph-properties fo:margin-left="0.15in" fo:margin-right="0.15in" fo:margin-top="0.08in" fo:margin-bottom="0.1in" fo:background-color="{NOTEBG}" fo:padding="0.07in" fo:border-left="0.04in solid {NAVY}" fo:border-right="none" fo:border-top="none" fo:border-bottom="none"/>
</style:style>''',
    'VTX_20_Screenshot': f'''<style:style style:name="VTX_20_Screenshot" style:display-name="VTX Screenshot" style:family="paragraph" style:parent-style-name="Text_20_body">
  <style:paragraph-properties fo:text-align="center" fo:margin-top="0.1in" fo:margin-bottom="0.15in" fo:padding="0.35in" fo:border="0.02in dashed #8899aa"/>
  <style:text-properties fo:color="#5a6b7d" fo:font-style="italic"/>
</style:style>''',
    'VTX_20_Part': f'''<style:style style:name="VTX_20_Part" style:display-name="VTX Part" style:family="paragraph" style:parent-style-name="Heading" style:next-style-name="Text_20_body">
  <style:paragraph-properties fo:break-before="page" fo:margin-top="1.6in" fo:margin-bottom="0.3in"/>
  <style:text-properties fo:color="{NAVY}" fo:font-size="26pt" fo:font-weight="bold"/>
</style:style>''',
    'VTX_20_Ref_20_Text': '''<style:style style:name="VTX_20_Ref_20_Text" style:display-name="VTX Ref Text" style:family="paragraph" style:parent-style-name="Table_20_Contents">
  <style:paragraph-properties fo:margin-top="0.02in" fo:margin-bottom="0.02in"/>
  <style:text-properties fo:font-size="9pt"/>
</style:style>''',
    'VTX_20_Ref_20_Head': '''<style:style style:name="VTX_20_Ref_20_Head" style:display-name="VTX Ref Head" style:family="paragraph" style:parent-style-name="VTX_20_Ref_20_Text">
  <style:text-properties fo:font-weight="bold"/>
</style:style>''',
    'VTX_20_Ref_20_Group': f'''<style:style style:name="VTX_20_Ref_20_Group" style:display-name="VTX Ref Group" style:family="paragraph" style:parent-style-name="VTX_20_Ref_20_Text">
  <style:paragraph-properties fo:margin-top="0.05in"/>
  <style:text-properties fo:color="{NAVY}" fo:font-weight="bold" fo:font-style="italic"/>
</style:style>''',
    'VTX_20_Ref_20_Tab': f'''<style:style style:name="VTX_20_Ref_20_Tab" style:display-name="VTX Ref Tab" style:family="paragraph" style:parent-style-name="Heading" style:next-style-name="Text_20_body" style:default-outline-level="3">
  <style:paragraph-properties fo:margin-top="0.12in" fo:margin-bottom="0.03in" fo:keep-with-next="always"/>
  <style:text-properties fo:color="#33475e" fo:font-size="11pt" fo:font-weight="bold"/>
</style:style>''',
    'VTX_20_Contents_20_Heading': f'''<style:style style:name="VTX_20_Contents_20_Heading" style:display-name="VTX Contents Heading" style:family="paragraph" style:parent-style-name="Heading">
  <style:paragraph-properties fo:break-before="page" fo:margin-bottom="0.15in"/>
  <style:text-properties fo:color="{NAVY}" fo:font-size="18pt" fo:font-weight="bold"/>
</style:style>''',
    'Strong_20_Emphasis': '''<style:style style:name="Strong_20_Emphasis" style:display-name="Strong Emphasis" style:family="text">
  <style:text-properties fo:font-weight="bold"/>
</style:style>''',
    'Emphasis': '''<style:style style:name="Emphasis" style:family="text">
  <style:text-properties fo:font-style="italic"/>
</style:style>''',
    'Source_20_Text': '''<style:style style:name="Source_20_Text" style:display-name="Source Text" style:family="text">
  <style:text-properties style:font-name="Courier New" fo:font-family="'Courier New'" style:font-family-generic="modern" style:font-pitch="fixed" fo:font-size="90%"/>
</style:style>''',
    'Contents_20_1': '''<style:style style:name="Contents_20_1" style:display-name="Contents 1" style:family="paragraph" style:parent-style-name="Standard" style:class="index">
  <style:paragraph-properties fo:margin-top="0.06in">
    <style:tab-stops><style:tab-stop style:position="6.69in" style:type="right" style:leader-style="dotted" style:leader-text="."/></style:tab-stops>
  </style:paragraph-properties>
  <style:text-properties fo:font-weight="bold"/>
</style:style>''',
    'Contents_20_2': '''<style:style style:name="Contents_20_2" style:display-name="Contents 2" style:family="paragraph" style:parent-style-name="Standard" style:class="index">
  <style:paragraph-properties fo:margin-left="0.2in">
    <style:tab-stops><style:tab-stop style:position="6.49in" style:type="right" style:leader-style="dotted" style:leader-text="."/></style:tab-stops>
  </style:paragraph-properties>
</style:style>''',
}


def ensure_named_styles(styles_xml):
    """Add any manual styles missing from styles.xml (keeps the author's edits)."""
    missing = [v for k, v in NAMED_STYLES.items() if f'style:name="{k}"' not in styles_xml]
    if missing:
        styles_xml = styles_xml.replace('</office:styles>', '\n'.join(missing) + '\n</office:styles>', 1)
    return styles_xml


# ---------------------------------------------------------------------------
# automatic styles for generated tables / lists (prefix keeps them findable)
# ---------------------------------------------------------------------------
def auto_styles(prefix, widths_list):
    """Table, column and cell styles for each distinct column-width set."""
    out = [
        f'<style:style style:name="{prefix}hd" style:family="table-cell"><style:table-cell-properties fo:background-color="{HEADBG}" fo:padding="0.03in" fo:border="none"/></style:style>',
        f'<style:style style:name="{prefix}td" style:family="table-cell"><style:table-cell-properties fo:padding="0.03in" fo:border-top="none" fo:border-left="none" fo:border-right="none" fo:border-bottom="0.5pt solid {RULE}"/></style:style>',
        f'<style:style style:name="{prefix}gp" style:family="table-cell"><style:table-cell-properties fo:padding="0.03in" fo:border="none"/></style:style>',
        f'<style:style style:name="{prefix}hr" style:family="table-row"><style:table-row-properties fo:keep-together="always"/></style:style>',
    ]
    for i, widths in enumerate(widths_list):
        total = sum(widths)
        out.append(f'<style:style style:name="{prefix}t{i}" style:family="table"><style:table-properties style:width="6.69in" table:align="margins" fo:margin-top="0.04in" fo:margin-bottom="0.12in"/></style:style>')
        for j, w in enumerate(widths):
            out.append(f'<style:style style:name="{prefix}t{i}.{j}" style:family="table-column"><style:table-column-properties style:column-width="{6.69 * w / total:.3f}in" style:rel-column-width="{int(65535 * w / total)}*"/></style:style>')
    # lists
    out.append(f'''<text:list-style style:name="{prefix}ul">
  <text:list-level-style-bullet text:level="1" text:bullet-char="&#8226;"><style:list-level-properties text:list-level-position-and-space-mode="label-alignment"><style:list-level-label-alignment text:label-followed-by="listtab" text:list-tab-stop-position="0.3in" fo:text-indent="-0.18in" fo:margin-left="0.3in"/></style:list-level-properties></text:list-level-style-bullet>
</text:list-style>''')
    out.append(f'''<text:list-style style:name="{prefix}ol">
  <text:list-level-style-number text:level="1" style:num-suffix="." style:num-format="1"><style:list-level-properties text:list-level-position-and-space-mode="label-alignment"><style:list-level-label-alignment text:label-followed-by="listtab" text:list-tab-stop-position="0.3in" fo:text-indent="-0.22in" fo:margin-left="0.3in"/></style:list-level-properties></text:list-level-style-number>
</text:list-style>''')
    out.append(f'<style:style style:name="{prefix}sect" style:family="section"><style:section-properties text:dont-balance-text-columns="false" style:editable="false"><style:columns fo:column-count="1" fo:column-gap="0in"/></style:section-properties></style:style>')
    return '\n'.join(out)


class Doc:
    """Accumulates ODF body XML; remembers table column sets for auto styles."""

    def __init__(self, prefix):
        self.prefix, self.parts, self.widths = prefix, [], []

    # inline markup: **bold**, *italic*, `code`
    @staticmethod
    def inline(text):
        out, pos = [], 0
        for m in re.finditer(r'\*\*(.+?)\*\*|`(.+?)`|(?<![\w*])\*(?!\s)(.+?)(?<!\s)\*(?!\w)', text):
            out.append(escape(text[pos:m.start()]))
            if m.group(1) is not None:
                out.append(f'<text:span text:style-name="Strong_20_Emphasis">{Doc.inline(m.group(1))}</text:span>')
            elif m.group(2) is not None:
                out.append(f'<text:span text:style-name="Source_20_Text">{escape(m.group(2))}</text:span>')
            else:
                out.append(f'<text:span text:style-name="Emphasis">{Doc.inline(m.group(3))}</text:span>')
            pos = m.end()
        out.append(escape(text[pos:]))
        return ''.join(out).replace('\t', '<text:tab/>')

    def p(self, text, style='Text_20_body', raw=False):
        body = text if raw else self.inline(text)
        self.parts.append(f'<text:p text:style-name="{style}">{body}</text:p>')

    def h(self, level, text, style=None):
        style = style or f'Heading_20_{level}'
        self.parts.append(f'<text:h text:style-name="{style}" text:outline-level="{level}">{self.inline(text)}</text:h>')

    def part(self, text):
        self.parts.append(f'<text:p text:style-name="VTX_20_Part">{self.inline(text)}</text:p>')

    def lst(self, items, ordered=False):
        st = self.prefix + ('ol' if ordered else 'ul')
        li = ''.join(f'<text:list-item><text:p text:style-name="Text_20_body">{self.inline(i)}</text:p></text:list-item>' for i in items)
        self.parts.append(f'<text:list text:style-name="{st}">{li}</text:list>')

    def table(self, header, rows, widths=None, small=False):
        """rows: list of lists, or ('group', label) tuples for a spanning group row."""
        ncol = len(header)
        widths = tuple(widths or [1] * ncol)
        if widths not in self.widths:
            self.widths.append(widths)
        t = f'{self.prefix}t{self.widths.index(widths)}'
        ps = 'VTX_20_Ref_20_Text' if small else 'Table_20_Contents'
        hs = 'VTX_20_Ref_20_Head' if small else 'Table_20_Heading'
        x = [f'<table:table table:name="{t}_{len(self.parts)}" table:style-name="{t}">']
        x += [f'<table:table-column table:style-name="{t}.{j}"/>' for j in range(ncol)]
        x.append(f'<table:table-header-rows><table:table-row table:style-name="{self.prefix}hr">')
        x += [f'<table:table-cell table:style-name="{self.prefix}hd" office:value-type="string"><text:p text:style-name="{hs}">{self.inline(c)}</text:p></table:table-cell>' for c in header]
        x.append('</table:table-row></table:table-header-rows>')
        for r in rows:
            x.append('<table:table-row>')
            if isinstance(r, tuple) and r[0] == 'group':
                x.append(f'<table:table-cell table:style-name="{self.prefix}gp" table:number-columns-spanned="{ncol}" office:value-type="string"><text:p text:style-name="VTX_20_Ref_20_Group">{escape(r[1])}</text:p></table:table-cell>')
                x += ['<table:covered-table-cell/>'] * (ncol - 1)
            else:
                for c in r:
                    x.append(f'<table:table-cell table:style-name="{self.prefix}td" office:value-type="string"><text:p text:style-name="{ps}">{self.inline(str(c)) if c not in (None, "") else ""}</text:p></table:table-cell>')
            x.append('</table:table-row>')
        x.append('</table:table>')
        self.parts.append(''.join(x))

    def xml(self):
        return '\n'.join(self.parts)

    def styles(self):
        return auto_styles(self.prefix, self.widths)


# ---------------------------------------------------------------------------
# Part I: guide (Markdown subset) -> ODF
# ---------------------------------------------------------------------------
def render_guide(md, doc):
    lines = md.split('\n')
    i, para = 0, []

    def flush():
        if para:
            doc.p(' '.join(para)); para.clear()

    while i < len(lines):
        ln = lines[i].rstrip()
        if not ln.strip():
            flush(); i += 1; continue
        m = re.match(r'(#{1,3})\s+(.*)', ln)
        if m:
            flush(); doc.h(len(m.group(1)), m.group(2)); i += 1; continue
        m = re.match(r'\[\[SCREENSHOT:\s*(.*?)\]\]', ln.strip())
        if m:
            flush(); doc.p('Screenshot: ' + m.group(1), 'VTX_20_Screenshot'); i += 1; continue
        if ln.startswith('>'):
            flush()
            note = []
            while i < len(lines) and lines[i].startswith('>'):
                note.append(lines[i].lstrip('> ').rstrip()); i += 1
            doc.p(' '.join(note), 'VTX_20_Note'); continue
        if ln.startswith('|'):
            flush()
            rows = []
            while i < len(lines) and lines[i].startswith('|'):
                cells = [c.strip() for c in lines[i].strip().strip('|').split('|')]
                if not all(re.fullmatch(r':?-+:?', c) for c in cells):
                    rows.append(cells)
                i += 1
            n = len(rows[0])
            widths = [1.4, 3] if n == 2 else [1.2, 2, 2] if n == 3 else None
            doc.table(rows[0], rows[1:], widths)
            continue
        m = re.match(r'(\s*)([-*]|\d+\.)\s+(.*)', ln)
        if m:
            flush()
            ordered = m.group(2)[0].isdigit()
            items = []
            while i < len(lines):
                mm = re.match(r'\s*([-*]|\d+\.)\s+(.*)', lines[i])
                if not mm:
                    if lines[i].startswith('  ') and lines[i].strip() and items:
                        items[-1] += ' ' + lines[i].strip(); i += 1; continue
                    break
                items.append(mm.group(2).rstrip()); i += 1
            doc.lst(items, ordered)
            continue
        para.append(ln.strip()); i += 1
    flush()


# ---------------------------------------------------------------------------
# Part II: reference, from ui_extract
# ---------------------------------------------------------------------------
KIND = {
    'slider': 'slider', 'slider/expression': 'expression slider', 'color slider': 'color slider',
    'expression': 'expression field', 'text': 'text', 'readout': 'display only',
    'checkbox': 'checkbox', 'button': 'button', 'choice': 'list', 'radio': 'option buttons',
    'color picker': 'color box', 'combo': 'list',
}

KEY_DESC = {
    'KEY_F1': 'Key help mode: All (default)', 'KEY_F2': 'Key help mode: View',
    'KEY_F3': 'Key help mode: Movie', 'KEY_F4': 'Key help mode: Stats',
    'KEY_F5': 'Key help mode: Colors', 'KEY_F6': 'Key help mode: Test',
    'KEY_LEFT': 'Turn (head) left', 'KEY_RIGHT': 'Turn (head) right',
    'KEY_FORWARD': 'Move forward', 'KEY_BACKWARD': 'Move back',
    'KEY_HELP': 'Show or hide the on-screen key help', 'KEY_DEFAULT': 'Make the default scene (at startup)',
    'KEY_UP': 'Move up', 'KEY_DOWN': 'Move down', 'KEY_TILT_UP': 'Tilt the view up (also Ctrl+Up)',
    'KEY_TILT_DOWN': 'Tilt the view down (also Ctrl+Down)', 'KEY_MOVE_SELECT': 'Move to the selected object',
    'KEY_REWIND_VIEWS': 'Rewind to the first saved view', 'KEY_DECR_VIEW': 'Previous view in the history',
    'KEY_INCR_VIEW': 'Next view in the history', 'KEY_UNSELECT': 'Clear the selection',
    'KEY_MARK_VIEW': 'Mark (save) the current view', 'KEY_UNDO_MOVE': 'Undo the last move',
    'KEY_CLEAR_VIEWS': 'Clear the view history', 'KEY_RAND_SEED': 'New random seed',
    'KEY_CMND_QUIT': 'Quit', 'KEY_CMND_MV_INC': 'Increase movement speed',
    'KEY_CMND_MV_DEC': 'Decrease movement speed', 'KEY_CMND_TM_INC': 'Increase the time rate',
    'KEY_CMND_TM_DEC': 'Decrease the time rate', 'KEY_CMND_MV_GO': 'Start or stop continuous motion',
    'KEY_CMND_TM_GO': 'Start or stop time', 'KEY_CMND_MV_RST': 'Reset movement speed',
    'KEY_CMND_TM_RST': 'Reset the time rate', 'KEY_MORE_DETAIL': 'More detail', 'KEY_LESS_DETAIL': 'Less detail',
    'KEY_VIEW_SURFACE': 'Surface view', 'KEY_VIEW_ORBIT': 'Orbit view', 'KEY_VIEW_GLOBAL': 'Orbit (global) view',
    'KEY_VIEW_VERT': 'Vertical view', 'KEY_VIEW_HORZ': 'Horizontal view',
    'KEY_RENDER_SHADED': 'Draw shaded (shaders)', 'KEY_RENDER_SOLID': 'Draw solid (OpenGL)',
    'KEY_RENDER_LINES': 'Draw lines', 'KEY_RENDER_POINTS': 'Draw points', 'KEY_RENDER_WATER': 'Water on/off',
    'KEY_RENDER_DEPTH': 'Water depth on/off', 'KEY_RENDER_TEXTURES': 'Textures on/off',
    'KEY_RENDER_SPRITES': 'Sprites on/off', 'KEY_RENDER_SPECULAR': 'Specular light on/off',
    'KEY_RENDER_BUMPS': 'Bump maps on/off', 'KEY_RENDER_REFLECT': 'Water reflections on/off',
    'KEY_RENDER_LIGHTING': 'Lighting on/off', 'KEY_RENDER_SMOOTH': 'Smooth shading on/off',
    'KEY_RENDER_SHADOWS': 'Shadows on/off', 'KEY_RENDER_TWILITE': 'Twilight on/off', 'KEY_RENDER_FOG': 'Fog on/off',
    'KEY_RENDER_AVES': 'Averaged normals on/off', 'KEY_RENDER_AA': 'Anti-aliasing on/off',
    'KEY_RENDER_HDR': 'High dynamic range on/off', 'KEY_RENDER_GEOMETRY': 'Geometry shader terrain on/off',
    'KEY_ADAPT_OVERLAP': 'Occlusion test on/off', 'KEY_ADAPT_LOD': 'Level of detail on/off',
    'KEY_ADAPT_REBUILD': 'Rebuild the scene', 'KEY_ADAPT_BEST': 'Scene quality: Very High',
    'KEY_ADAPT_HIGH': 'Scene quality: High', 'KEY_ADAPT_MED': 'Scene quality: Medium',
    'KEY_ADAPT_LOW': 'Scene quality: Low',
}
DEV_KEYS = re.compile(r'KEY_(TEST\d|CMND_YYDEBUG|RASTER_\w+|RENDER_(MIX|NVIS|FACE)|ADAPT_(UNIFORM|HIDDEN)|F([7-9]|10))$')


def humanize(name):
    return re.sub(r'^KEY_(CMND_|RENDER_|ADAPT_|VIEW_)?', '', name).replace('_', ' ').capitalize()


def load_notes(path):
    """control_notes.txt: 'Object / Tab / Group / Control: text' (Group optional)."""
    notes = {}
    if path and os.path.exists(path):
        for ln in open(path, encoding='utf-8'):
            ln = ln.strip()
            if not ln or ln.startswith('#') or ':' not in ln:
                continue
            key, _, text = ln.partition(':')
            parts = [p.strip().lower() for p in key.split('/')]
            notes[tuple(parts)] = text.strip()
    return notes


def note_for(notes, obj, tab, group, label):
    for k in ((obj, tab, group, label), (obj, tab, label), (obj, label)):
        k = tuple(x.lower() for x in k if x)
        if k in notes:
            return notes[k]
    return ''


def control_rows(pages_groups, notes, obj):
    """Rows for one tab: group header rows + one row per control."""
    rows = []
    tab, groups = pages_groups
    for g in groups:
        if g['group']:
            rows.append(('group', g['group']))
        for c in g['controls']:
            rng = ''
            if c.get('choices'):
                rng = ', '.join(c['choices'])
            elif c.get('range'):
                lo, hi = c['range']
                rng = f'{lo} to {hi}'
            extra = note_for(notes, obj, tab, g['group'], c['label']) or c.get('tooltip', '')
            rows.append([c['label'], KIND.get(c['kind'], c['kind']), rng, extra])
    return rows


def render_reference(ui, doc, notes, version):
    cls = ui['classes']
    doc.part('Part II: Reference')
    doc.p(f'This part is generated from the VTX source code ({escape(version)}), so it matches this version of the program exactly. '
          'Ranges are the ranges of the sliders: you can often type values outside them.')

    # --- menus and toolbar
    doc.h(1, 'Menus and toolbar')
    for m in ui['menus']:
        doc.h(2, m['menu'] + ' menu')
        rows = []
        for it in m['items']:
            rows.append([it['label'], it.get('key') or '', it.get('help', '')])
            for sub in it.get('items', []):
                rows.append([f"{it['label']} > {sub['label']}", sub.get('key') or '', sub.get('help', '')])
        doc.table(['Command', 'Key', 'Notes'], rows, [2.2, 1, 2.6], small=True)
    doc.h(2, 'Toolbar')
    doc.table(['Button', 'Action'], [[t['label'], t['tooltip']] for t in ui['toolbar']], [1.6, 3], small=True)

    # --- keyboard
    doc.h(1, 'Keyboard')
    doc.p('Press **?** to show the on-screen key help, and **F1** to **F6** to switch modes. '
          'The number keys **0** to **9** do different things in each mode. Letter keys are case-sensitive.')
    for k in ui['keys']:
        doc.h(2, f"{k['key']}: {k['mode']} mode")
        rows = []
        for ln in k['lines']:
            s = ln.strip()
            if not s or set(s) <= set('-') or re.match(r'^-+.*-+$', s):
                if s:
                    title = s.strip('- ').strip()
                    if title:
                        rows.append(('group', title.capitalize()))
                continue
            # "x do this   y do that" pairs -> separate rows
            for chunk in re.split(r'\s{3,}', s):
                m = re.match(r'^\s*(\S+?)[:\s]\s*(.+)$', chunk)
                if m:
                    act = m.group(2).strip()
                    rows.append([m.group(1), act[:1].upper() + act[1:]])
                else:
                    rows.append(['', chunk])
        doc.table(['Key', 'Action'], rows, [0.8, 4], small=True)
    doc.h(2, 'All key commands')
    doc.p('Every single-key command, as defined in the program. Commands marked *developer* are for testing and diagnostics.')
    rows = []
    for k in ui['key_table']:
        if re.match(r'KEY_FN_', k['name']):
            continue
        desc = KEY_DESC.get(k['name'], humanize(k['name']))
        if DEV_KEYS.match(k['name']):
            desc = f'{desc} (*developer*)' if k['name'] in KEY_DESC else f'{humanize(k["name"])} (*developer*)'
        rows.append([k['key'], desc])
    doc.table(['Key', 'Command'], rows, [0.9, 4], small=True)

    # --- properties dialog
    doc.h(1, 'Properties dialog')
    doc.p('Each object type in the scene tree has its own tabs. For each type, this chapter lists its right-click menu, '
          'then every tab, grouped as in the dialog.')
    seen = {}
    for o in ui['properties']:
        c = cls.get(o['class'])
        if not c:
            continue
        doc.h(2, o['object'])
        if o['class'] in seen:
            doc.p(f"Uses the same tabs as **{seen[o['class']]}**.")
            continue
        seen[o['class']] = o['object']
        has_controls = any(g['controls'] for p in c['pages'] for g in p['groups'])
        if c['menu']:
            doc.p('**Menu:** ' + ', '.join(c['menu']))
        if not has_controls:
            doc.p('This object has no editable settings in this version.')
            continue
        for p in c['pages']:
            if not p['groups']:
                continue
            title = p['tab'] + (' (shown only for some modes)' if p.get('conditional') else '')
            doc.h(3, title + ' tab', 'VTX_20_Ref_20_Tab')
            doc.table(['Control', 'Type', 'Range or choices', 'Notes'],
                      control_rows((p['tab'], p['groups']), notes, o['object']), [1.5, 1.1, 1.6, 2.6], small=True)

    # --- image editor
    doc.h(1, 'Image Editor')
    done = {}
    for t in ui['image_editor']:
        c = cls.get(t['class'])
        doc.h(2, t['tab'] + ' tab')
        if t['class'] in done:
            doc.p(f"Same controls as the **{done[t['class']]}** tab.")
            continue
        done[t['class']] = t['tab']
        for p in (c or {}).get('pages', []):
            if p['groups']:
                doc.table(['Control', 'Type', 'Range or choices', 'Notes'],
                          control_rows((p['tab'] or t['tab'], p['groups']), notes, 'Image ' + t['tab']), [1.5, 1.1, 1.6, 2.6], small=True)

    # --- function editors
    doc.h(1, 'Function editors')
    doc.p('These dialogs open from an expression field: double-click a function token, then right-click and choose **Edit**.')
    for f in ui['functions']:
        c = cls.get(f['class'])
        name = {'Fchnl': 'Fractal'}.get(f['function'], f['function'])
        doc.h(2, name)
        for p in (c or {}).get('pages', []):
            if p['groups']:
                doc.h(3, p['tab'] + ' tab', 'VTX_20_Ref_20_Tab')
                doc.table(['Control', 'Type', 'Range or choices', 'Notes'],
                          control_rows((p['tab'], p['groups']), notes, name), [1.5, 1.1, 1.6, 2.6], small=True)


def reference_section(ui, notes, version):
    doc = Doc('vtxref_')
    render_reference(ui, doc, notes, version)
    body = (f'<text:section text:style-name="vtxref_sect" text:name="{SECTION}" text:protected="true">'
            f'{doc.xml()}</text:section>')
    return body, doc.styles()


# ---------------------------------------------------------------------------
# table of contents (cached entries; page numbers are filled by odt2pdf.py)
# ---------------------------------------------------------------------------
def toc_xml(headings):
    entries = ''.join(
        f'<text:p text:style-name="Contents_20_{lvl}">{escape(t)}<text:tab/></text:p>'
        for lvl, t in headings if lvl <= 2)
    return f'''<text:table-of-content text:style-name="vtx_sect" text:protected="true" text:name="Table of Contents1">
<text:table-of-content-source text:outline-level="2" text:use-index-marks="false">
<text:index-title-template text:style-name="VTX_20_Contents_20_Heading">Contents</text:index-title-template>
<text:table-of-content-entry-template text:outline-level="1" text:style-name="Contents_20_1"><text:index-entry-link-start text:style-name="Index_20_Link"/><text:index-entry-text/><text:index-entry-tab-stop style:type="right" style:leader-char="."/><text:index-entry-page-number/><text:index-entry-link-end/></text:table-of-content-entry-template>
<text:table-of-content-entry-template text:outline-level="2" text:style-name="Contents_20_2"><text:index-entry-link-start text:style-name="Index_20_Link"/><text:index-entry-text/><text:index-entry-tab-stop style:type="right" style:leader-char="."/><text:index-entry-page-number/><text:index-entry-link-end/></text:table-of-content-entry-template>
</text:table-of-content-source>
<text:index-body><text:index-title text:style-name="vtx_sect" text:name="Table of Contents1_Head"><text:p text:style-name="VTX_20_Contents_20_Heading">Contents</text:p></text:index-title>{entries}</text:index-body>
</text:table-of-content>'''


def headings_in(xml):
    out = []
    for m in re.finditer(r'<text:h [^>]*text:outline-level="(\d)"[^>]*>(.*?)</text:h>', xml, re.S):
        out.append((int(m.group(1)), re.sub(r'<[^>]+>', '', m.group(2)).replace('&amp;', '&').replace('&lt;', '<').replace('&gt;', '>')))
    return out


# ---------------------------------------------------------------------------
# ODT package I/O
# ---------------------------------------------------------------------------
def read_odt(path):
    with zipfile.ZipFile(path) as z:
        return {n: z.read(n) for n in z.namelist()}


def write_odt(path, files):
    tmp = path + '.tmp'
    with zipfile.ZipFile(tmp, 'w') as z:
        z.writestr(zipfile.ZipInfo('mimetype'), files['mimetype'], compress_type=zipfile.ZIP_STORED)
        for n, data in files.items():
            if n != 'mimetype':
                z.writestr(n, data, compress_type=zipfile.ZIP_DEFLATED)
    os.replace(tmp, path)


VERSION = None


def version_string(root):
    if VERSION:
        return VERSION
    try:
        import subprocess
        v = subprocess.run(['git', 'describe', '--tags', '--always', '--dirty'], cwd=root,
                           capture_output=True, text=True, timeout=10).stdout.strip()
    except Exception:
        v = ''
    return v or datetime.date.today().isoformat()


def replace_auto_styles(content, prefix, new_styles):
    """Drop old prefix* automatic styles, add new ones."""
    content = re.sub(r'<style:style style:name="%s[^"]*"[^>]*?(?:/>|>.*?</style:style>)\s*' % prefix, '', content, flags=re.S)
    content = re.sub(r'<text:list-style style:name="%s[^"]*".*?</text:list-style>\s*' % prefix, '', content, flags=re.S)
    if '<office:automatic-styles/>' in content:
        content = content.replace('<office:automatic-styles/>', '<office:automatic-styles></office:automatic-styles>')
    return content.replace('</office:automatic-styles>', new_styles + '</office:automatic-styles>', 1)


def update(path, out, root, notes_path):
    files = read_odt(path)
    content = files['content.xml'].decode('utf-8')
    m = re.search(r'<text:section\b[^>]*text:name="%s"[^>]*>.*?</text:section>' % SECTION, content, re.S)
    if not m:
        sys.exit(f'{path}: no section named "{SECTION}" - create the manual with "mkmanual.py new" first')
    ui = ui_extract.extract(root)
    section, styles = reference_section(ui, load_notes(notes_path), version_string(root))
    content = content[:m.start()] + section + content[m.end():]
    content = replace_auto_styles(content, 'vtxref_', styles)
    files['content.xml'] = content.encode('utf-8')
    files['styles.xml'] = ensure_named_styles(files['styles.xml'].decode('utf-8')).encode('utf-8')
    write_odt(out or path, files)
    n = sum(len(g['controls']) for c in ui['classes'].values() for p in c['pages'] for g in p['groups'])
    print(f'### manual reference refreshed: {len(ui["properties"])} object types, {n} controls -> {out or path}')


def new(path, guide, template, root, notes_path):
    tpl = read_odt(template)
    styles = ensure_named_styles(tpl['styles.xml'].decode('utf-8'))
    # template font declarations (content.xml) so headings/body render as in the template
    tcontent = tpl['content.xml'].decode('utf-8')
    fonts = re.search(r'<office:font-face-decls>.*?</office:font-face-decls>', tcontent, re.S)
    fonts = fonts.group(0) if fonts else ''

    version = version_string(root)
    g = Doc('vtx_')
    g.p('VTX User Manual', 'Title')
    g.p('Exploring and editing a procedural universe with the Virtual Terrain Explorer', 'Subtitle')
    g.p('Dean Sindorf', 'Subtitle')
    g.p(f'Draft, {datetime.date.today():%B %Y}', 'Subtitle')
    g.parts.append('@@TOC@@')
    g.part('Part I: User guide')
    render_guide(open(guide, encoding='utf-8').read(), g)

    ui = ui_extract.extract(root)
    section, ref_styles = reference_section(ui, load_notes(notes_path), version)
    body = g.xml()
    body = body.replace('@@TOC@@', toc_xml(headings_in(body) + headings_in(section)))
    content = f'''<?xml version="1.0" encoding="UTF-8"?>
<office:document-content {NS}>
{fonts}
<office:automatic-styles>
<style:style style:name="vtx_sect" style:family="section"><style:section-properties style:editable="false"><style:columns fo:column-count="1" fo:column-gap="0in"/></style:section-properties></style:style>
{g.styles()}
{ref_styles}
</office:automatic-styles>
<office:body><office:text>
{body}
{section}
</office:text></office:body>
</office:document-content>'''
    meta = f'''<?xml version="1.0" encoding="UTF-8"?>
<office:document-meta {NS}><office:meta><dc:title>VTX User Manual</dc:title><meta:initial-creator>Dean Sindorf</meta:initial-creator><dc:creator>Dean Sindorf</dc:creator><meta:generator>mkmanual.py</meta:generator></office:meta></office:document-meta>'''
    manifest = '''<?xml version="1.0" encoding="UTF-8"?>
<manifest:manifest xmlns:manifest="urn:oasis:names:tc:opendocument:xmlns:manifest:1.0" manifest:version="1.3">
 <manifest:file-entry manifest:full-path="/" manifest:version="1.3" manifest:media-type="application/vnd.oasis.opendocument.text"/>
 <manifest:file-entry manifest:full-path="content.xml" manifest:media-type="text/xml"/>
 <manifest:file-entry manifest:full-path="styles.xml" manifest:media-type="text/xml"/>
 <manifest:file-entry manifest:full-path="meta.xml" manifest:media-type="text/xml"/>
</manifest:manifest>'''
    write_odt(path, {
        'mimetype': b'application/vnd.oasis.opendocument.text',
        'content.xml': content.encode('utf-8'),
        'styles.xml': styles.encode('utf-8'),
        'meta.xml': meta.encode('utf-8'),
        'META-INF/manifest.xml': manifest.encode('utf-8'),
    })
    print(f'### created {path}')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('mode', choices=['update', 'new'])
    ap.add_argument('odt')
    ap.add_argument('-o', '--out', help='write here instead of updating ODT in place (update mode)')
    ap.add_argument('--root', default=os.path.join(HERE, '..', '..'), help='VTX source root (default: ../..)')
    ap.add_argument('--notes', default=os.path.join(HERE, 'control_notes.txt'))
    ap.add_argument('--guide', default=os.path.join(HERE, 'guide.md'))
    ap.add_argument('--template', help='ODT whose page and paragraph styles to use (new mode)')
    ap.add_argument('--version', help='version text for the reference (default: git describe)')
    a = ap.parse_args()
    global VERSION
    VERSION = a.version
    if a.mode == 'update':
        update(a.odt, a.out, a.root, a.notes)
    else:
        template = a.template or os.path.join(a.root, 'Docs', 'VTX-Theory-of-Operation.odt')
        new(a.odt, a.guide, template, a.root, a.notes)


if __name__ == '__main__':
    main()
