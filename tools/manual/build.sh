#!/usr/bin/env bash
#=============================================================================
# tools/manual/build.sh - refresh the VTX User Manual and/or export PDFs
#
#   build.sh refresh ODT                  regenerate Part II (reference) in place
#   build.sh pdf ODT OUT.pdf [--refresh]  export ODT to PDF, updating the table of
#                                         contents; with --refresh, Part II is
#                                         regenerated in a temporary copy first
#                                         (the ODT itself is not modified)
#
# Uses LibreOffice's own Python (program/python.exe on Windows), so nothing
# else needs installing. Environment overrides:
#   SOFFICE  path to soffice(.com/.exe)     LOPY  path to LibreOffice's python
#   VTX_VERSION  version text for the reference (default: git describe)
#=============================================================================
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)

die()  { echo "ERROR: $*" >&2; exit 1; }
winpath() { if command -v cygpath >/dev/null; then cygpath -w "$1"; else echo "$1"; fi; }

find_soffice() {
    [ -n "${SOFFICE:-}" ] && [ -f "$SOFFICE" ] && return 0
    for c in "$(command -v soffice.com 2>/dev/null || true)" "$(command -v soffice 2>/dev/null || true)" \
             "/c/Program Files/LibreOffice/program/soffice.com" \
             "/c/Program Files (x86)/LibreOffice/program/soffice.com" \
             "/cygdrive/c/Program Files/LibreOffice/program/soffice.com" \
             "/cygdrive/c/Program Files (x86)/LibreOffice/program/soffice.com"; do
        [ -n "$c" ] && [ -f "$c" ] && { SOFFICE=$c; return 0; }
    done
    return 1
}

# LibreOffice's python: next to soffice on Windows; system python3 with uno elsewhere
find_lopy() {
    [ -n "${LOPY:-}" ] && [ -f "$LOPY" ] && return 0
    if find_soffice; then
        local d; d=$(dirname "$SOFFICE")
        for c in "$d/python.exe" "$d/python"; do
            [ -f "$c" ] && { LOPY=$c; return 0; }
        done
    fi
    if command -v python3 >/dev/null && python3 -c 'import uno' 2>/dev/null; then
        LOPY=python3; return 0
    fi
    return 1
}

version() { echo "${VTX_VERSION:-$(cd "$ROOT" && git describe --tags --always --dirty 2>/dev/null || date +%Y-%m-%d)}"; }

refresh() {   # refresh SRC [OUT]
    find_lopy || die "LibreOffice (with its Python) not found - set SOFFICE or LOPY"
    local args=(update "$(winpath "$1")" --root "$(winpath "$ROOT")" --version "$(version)")
    [ -n "${2:-}" ] && args+=(-o "$(winpath "$2")")
    "$LOPY" "$(winpath "$HERE/mkmanual.py")" "${args[@]}"
}

topdf() {     # topdf SRC.odt OUT.pdf
    if find_lopy; then
        "$LOPY" "$(winpath "$HERE/odt2pdf.py")" "$(winpath "$1")" "$(winpath "$2")"
    elif find_soffice; then
        echo "WARNING: LibreOffice python not found - table of contents page numbers won't be refreshed" >&2
        local tmp; tmp=$(mktemp -d)
        "$SOFFICE" --headless --norestore --convert-to pdf --outdir "$(winpath "$tmp")" "$(winpath "$1")" >/dev/null 2>&1 || true
        mv "$tmp/$(basename "${1%.*}").pdf" "$2" 2>/dev/null || die "PDF conversion of $1 failed"
        rm -rf "$tmp"
    else
        die "LibreOffice not found - set SOFFICE"
    fi
    [ -f "$2" ] || die "PDF conversion of $1 failed"
}

case "${1:-}" in
    refresh)
        [ $# -eq 2 ] || die "usage: build.sh refresh ODT"
        refresh "$2" ;;
    pdf)
        [ $# -ge 3 ] || die "usage: build.sh pdf ODT OUT.pdf [--refresh]"
        src=$2; out=$3
        mkdir -p "$(dirname "$out")"
        if [ "${4:-}" = --refresh ]; then
            tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
            refresh "$src" "$tmp/$(basename "$src")"
            src="$tmp/$(basename "$src")"
        fi
        topdf "$src" "$out" ;;
    *)
        sed -n '3,15p' "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
esac
