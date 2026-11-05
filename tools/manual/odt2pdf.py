"""
odt2pdf.py - convert an .odt to PDF with LibreOffice, updating the table of
contents (and all other indexes and fields) first, so page numbers are right.

Must run with LibreOffice's own Python, which includes the UNO bridge:
  Windows:  "C:/Program Files/LibreOffice/program/python.exe" odt2pdf.py IN.odt OUT.pdf
  Linux:    python3 odt2pdf.py IN.odt OUT.pdf   (with python3-uno installed)

Starts a private headless LibreOffice (its own profile, so an open LibreOffice
window is not disturbed), loads the document, updates indexes, exports, quits.
"""
import os
import shutil
import subprocess
import sys
import tempfile
import time
import uuid

import uno
from com.sun.star.beans import PropertyValue


def prop(name, value):
    p = PropertyValue()
    p.Name, p.Value = name, value
    return p


def soffice_path():
    exe = 'soffice.exe' if os.name == 'nt' else 'soffice'
    here = os.path.dirname(sys.executable)      # LibreOffice's program/ folder
    cand = os.path.join(here, exe)
    return cand if os.path.exists(cand) else 'soffice'


def main(src, dst):
    src, dst = os.path.abspath(src), os.path.abspath(dst)
    pipe = 'vtx_' + uuid.uuid4().hex[:8]
    profile = tempfile.mkdtemp(prefix='vtx-lo-')
    # work on a copy: the original may be open (and locked) in LibreOffice
    work = os.path.join(profile, 'doc_' + os.path.basename(src))
    shutil.copyfile(src, work)
    proc = subprocess.Popen([soffice_path(), '--headless', '--norestore', '--nologo',
                             '-env:UserInstallation=' + uno.systemPathToFileUrl(profile),
                             '--accept=pipe,name=%s;urp;' % pipe],
                            stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    desktop = None
    try:
        local = uno.getComponentContext()
        resolver = local.ServiceManager.createInstanceWithContext('com.sun.star.bridge.UnoUrlResolver', local)
        ctx = None
        for _ in range(120):                       # wait up to ~60 s for startup
            try:
                ctx = resolver.resolve('uno:pipe,name=%s;urp;StarOffice.ComponentContext' % pipe)
                break
            except Exception:
                time.sleep(0.5)
        if ctx is None:
            sys.exit('odt2pdf: LibreOffice did not start')
        desktop = ctx.ServiceManager.createInstanceWithContext('com.sun.star.frame.Desktop', ctx)
        doc = desktop.loadComponentFromURL(uno.systemPathToFileUrl(work), '_blank', 0,
                                           (prop('Hidden', True),))
        if doc is None:
            sys.exit('odt2pdf: LibreOffice could not open ' + src)
        # update twice: the first pass can change page breaks the second one sees
        for _ in range(2):
            idx = doc.getDocumentIndexes()
            for i in range(idx.getCount()):
                idx.getByIndex(i).update()
            doc.getTextFields().refresh()
        doc.storeToURL(uno.systemPathToFileUrl(dst), (
            prop('FilterName', 'writer_pdf_Export'),
            prop('FilterData', uno.Any('[]com.sun.star.beans.PropertyValue', (
                prop('ExportBookmarks', True), prop('UseTaggedPDF', True))))))
        doc.close(True)
    finally:
        if desktop is not None:
            try:
                desktop.terminate()
            except Exception:
                pass
        try:
            proc.wait(timeout=30)
        except Exception:
            proc.kill()
        shutil.rmtree(profile, ignore_errors=True)
    if not os.path.exists(dst):
        sys.exit('odt2pdf: export failed')
    print('### wrote', dst)


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2])
