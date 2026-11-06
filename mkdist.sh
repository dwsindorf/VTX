#!/usr/bin/env bash
#=============================================================================
# mkdist.sh - build a self-contained VTX install bundle (no git clone, no build)
#
# Usage:  ./mkdist.sh [options]          (run from anywhere inside the repo)
#   -e FILE   compiled executable          (default: bin/vtx.exe)
#   -o DIR    output directory             (default: dist)
#   -n        dry run: print the manifest, don't stage or archive
#   -k        keep the staging directory after archiving
#   -D        don't collect DLL dependencies
#   -i        also build a Windows installer (needs Inno Setup 6, installer/vtx.iss)
#   -v        verbose
#   -h        help
#
# Environment:
#   DLL_PATH  extra ':'-separated dirs searched for runtime DLLs
#   ISCC      path to Inno Setup's ISCC.exe (if not in a standard location)
#   SOFFICE   path to LibreOffice soffice(.com/.exe) (if not in a standard location)
#
# Rules (see the "selection" section below):
#   * Only files tracked by git are shipped (git ls-files). Untracked work
#     files (Images/*, scratch Saves, Textures/Processed/*, ...) are skipped
#     automatically, so nothing needs updating when files get committed.
#   * Shaders: only files reachable from GLSLMgr::loadProgram/setShadowProgram
#     string literals in the C++ sources, plus everything they #include.
#     *.debug files and dead shaders are dropped. Computed fresh on each run.
#   * Docs: the files listed in DOCS below (must be tracked). .odt files are
#     converted to PDF with LibreOffice at packaging time; only the PDF ships.
#     The user manual's Part II (reference) is regenerated from the source
#     first (tools/manual/build.sh), and tables of contents are updated.
#   * Resources: everything tracked except compile-time sources (*.xpm etc.
#     are #included into VtxFrame.cpp and are not needed at runtime).
#   * Textures/Maps, Textures/Images: only images some shipped scene, object
#     or the code refers to by name (see TRIM_DIRS); unused ones are skipped.
#   * Objects, Textures, Bitmaps: all tracked files, plus the full directory tree from
#     the working copy (so empty category folders exist at runtime).
#   * bin/vtx.exe plus every non-system DLL it needs (recursive objdump scan).
#
# Layout of the bundle (matches FileUtil::getBaseDirectory(): base = parent
# of the directory holding vtx.exe):
#   VTX/
#     VTX.bat            launcher - sets cwd to bin\ (required, see below)
#     README.txt, MANIFEST.txt
#     bin/vtx.exe, bin/*.dll
#     Docs/ Objects/ Resources/ Saves/ Shaders/ Textures/
#     Bitmaps/ Images/ Movies/   (empty, written to at runtime)
#=============================================================================
set -euo pipefail

EXE=bin/vtx.exe
OUTDIR=dist
DRYRUN=0
KEEP=0
DLLS=1
VERBOSE=0
INSTALLER=0
APPNAME=VTX

# Documentation to ship (space-separated, repo-relative; must be tracked)
DOCS="Docs/VTX-User-Manual.odt Docs/VTX-Theory-of-Operation.odt"   # .odt -> shipped as .pdf
# Manuals whose generated reference section (tools/manual) is refreshed from
# the current source before conversion. The .odt in the repo isn't modified.
MANUALS="Docs/VTX-User-Manual.odt"

# Image folders whose files VTX only ever loads by name, e.g. Texture("mars",..).
# A file here ships only if a shipped scene/object (Saves, Objects, Resources)
# or the C++ code mentions its name. Other texture folders (Plants, Sprites,
# Hmaps) are also picked at random by the generators, so they always ship.
TRIM_DIRS="Textures/Maps Textures/Images"

usage() { sed -n '3,18p' "$0" | sed 's/^# \{0,1\}//'; exit "${1:-0}"; }
while getopts "e:o:nkDivh" opt; do
    case $opt in
        e) EXE=$OPTARG ;;
        o) OUTDIR=$OPTARG ;;
        n) DRYRUN=1 ;;
        k) KEEP=1 ;;
        D) DLLS=0 ;;
        i) INSTALLER=1 ;;
        v) VERBOSE=1 ;;
        h) usage 0 ;;
        *) usage 1 ;;
    esac
done

say()  { echo "### $*"; }
warn() { echo "WARNING: $*" >&2; }
die()  { echo "ERROR: $*" >&2; exit 1; }
vlog() { [ "$VERBOSE" = 1 ] && echo "    $*" || true; }

#-----------------------------------------------------------------------------
# locate repo
#-----------------------------------------------------------------------------
command -v git >/dev/null || die "git not found"
# work from the project dir (where this script lives) - it may be a
# sub-directory of the git repo, and all paths/pathspecs are relative to it
ROOT=$(cd "$(dirname "$0")" && pwd)
cd "$ROOT"
git rev-parse --is-inside-work-tree >/dev/null 2>&1 || die "$ROOT is not inside a git work tree"
[ -f "$EXE" ] || die "executable not found: $ROOT/$EXE (build it first, or use -e)"

if [ "$INSTALLER" = 1 ]; then
    [ -f installer/vtx.iss ] || die "installer/vtx.iss not found"
    if [ -z "${ISCC:-}" ]; then
        for c in "$(command -v ISCC.exe 2>/dev/null || true)" \
                 "/c/Program Files (x86)/Inno Setup 6/ISCC.exe" \
                 "/c/Program Files/Inno Setup 6/ISCC.exe" \
                 "/cygdrive/c/Program Files (x86)/Inno Setup 6/ISCC.exe" \
                 "/cygdrive/c/Program Files/Inno Setup 6/ISCC.exe"; do
            [ -n "$c" ] && [ -f "$c" ] && { ISCC=$c; break; }
        done
        if [ -z "${ISCC:-}" ] && [ -n "${LOCALAPPDATA:-}" ] && command -v cygpath >/dev/null; then
            c="$(cygpath -u "$LOCALAPPDATA")/Programs/Inno Setup 6/ISCC.exe"
            [ -f "$c" ] && ISCC=$c
        fi
    fi
    [ -n "${ISCC:-}" ] && [ -f "$ISCC" ] || \
        die "Inno Setup ISCC.exe not found - install Inno Setup 6 (winget install JRSoftware.InnoSetup) or set ISCC=/path/to/ISCC.exe"
    command -v cygpath >/dev/null || die "cygpath not found (needed to pass paths to ISCC.exe)"
fi

VERSION=$(git describe --tags --always --dirty 2>/dev/null || date +%Y%m%d)
BUNDLE="$APPNAME-win64-$VERSION"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

git ls-files > "$TMP/tracked"
is_tracked() { grep -Fxq -- "$1" "$TMP/tracked"; }

if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
    warn "work tree has uncommitted changes to tracked files - the bundle"
    warn "ships the working-copy versions (version tag: $VERSION)"
fi

: > "$TMP/files"       # repo-relative files to copy
: > "$TMP/dirs"        # directories to create (may be empty)
add_file() {
    if [ ! -f "$1" ]; then      # tracked, but deleted in the work tree
        warn "$1 is tracked but missing on disk (uncommitted delete?) - skipped"
        return 0
    fi
    echo "$1" >> "$TMP/files"; vlog "+ $1"
}
add_dir()  { echo "$1" >> "$TMP/dirs"; }

#=============================================================================
# selection
#=============================================================================

#--- Docs: explicit list (.odt converted to .pdf at staging) -----------------
: > "$TMP/convert"
for f in $DOCS; do
    if is_tracked "$f" && [ -f "$f" ]; then
        case "$f" in
            *.odt) echo "$f" >> "$TMP/convert" ;;
            *)     add_file "$f" ;;
        esac
    elif [ -f "$f" ]; then
        warn "$f is not tracked by git - skipped (git add + commit it)"
    else
        warn "$f not found - skipped"
    fi
done

if [ -s "$TMP/convert" ]; then
    if [ -z "${SOFFICE:-}" ]; then
        for c in "$(command -v soffice.com 2>/dev/null || true)" \
                 "$(command -v soffice 2>/dev/null || true)" \
                 "/c/Program Files/LibreOffice/program/soffice.com" \
                 "/c/Program Files (x86)/LibreOffice/program/soffice.com" \
                 "/cygdrive/c/Program Files/LibreOffice/program/soffice.com" \
                 "/cygdrive/c/Program Files (x86)/LibreOffice/program/soffice.com"; do
            [ -n "$c" ] && [ -f "$c" ] && { SOFFICE=$c; break; }
        done
    fi
    if [ -z "${SOFFICE:-}" ] || [ ! -f "$SOFFICE" ]; then
        # no LibreOffice: fall back to a committed PDF of the same name, if any
        while read -r f; do
            pdf="${f%.odt}.pdf"
            if is_tracked "$pdf" && [ -f "$pdf" ]; then
                warn "LibreOffice not found - shipping committed $pdf (may be out of date)"
                add_file "$pdf"
            else
                warn "LibreOffice not found - cannot convert $f to PDF; skipped (set SOFFICE=...)"
            fi
        done < "$TMP/convert"
        : > "$TMP/convert"
    fi
fi

#--- Saves: tracked *.spx ----------------------------------------------------
git ls-files -- 'Saves/*.spx' | while read -r f; do add_file "$f"; done
add_dir Saves

#--- Textures, Objects: all tracked files + working-copy dir tree ------------
# names referenced anywhere that ships: quoted strings in tracked .spx files
# under Saves/Objects/Resources, plus string literals in the C++ sources
{ git ls-files -z -- 'Saves/*.spx' 'Objects/*.spx' 'Resources/*.spx' | xargs -0 cat 2>/dev/null
  git ls-files -z -- '*.cpp' ':!includes/GL/' | xargs -0 cat 2>/dev/null; } \
  | { grep -ohE '"[^"]{1,200}"' || true; } | tr -d '"' | tr 'A-Z' 'a-z' | sort -u > "$TMP/refs"
: > "$TMP/trimmed"
for d in Textures Objects Bitmaps; do
    git ls-files -- "$d/" | while read -r f; do
        for t in $TRIM_DIRS; do
            case "$f" in "$t"/*)
                base=$(basename "$f"); base=${base%.*}
                if ! grep -Fxq -- "$(echo "$base" | tr 'A-Z' 'a-z')" "$TMP/refs"; then
                    echo "$f" >> "$TMP/trimmed"
                    continue 2
                fi ;;
            esac
        done
        add_file "$f"
    done
    if [ -d "$d" ]; then
        find "$d" -type d ! -name '.*' >> "$TMP/dirs"
    fi
done
add_dir Textures/Processed      # FileUtil::getProcessedDir() won't create it

#--- Resources: tracked, minus compile-time-only sources ---------------------
git ls-files -- 'Resources/' \
  | { grep -vE '\.(xpm|h|hpp|c|cpp|o|rc|ico)$' || true; } \
  | while read -r f; do add_file "$f"; done
for r in Resources/UI/prefs.spx Resources/Sprites; do
    git ls-files --error-unmatch -- "$r" >/dev/null 2>&1 || \
        warn "$r is not tracked - it is read at runtime"
done

#--- Shaders: closure of what the C++ code actually loads --------------------
# 1. every "*.frag|vert|geom|glsl" string literal in tracked C++ sources
#    (line comments stripped, so commented-out loadProgram calls don't count)
git ls-files -z -- '*.cpp' '*.h' ':!Shaders/' ':!includes/GL/' \
  | xargs -0 cat \
  | sed -e 's#^[[:space:]]*//.*##' -e 's#[[:space:]]//.*##' \
  | { grep -oE '"[A-Za-z0-9_.]+\.(frag|vert|geom|glsl)"' || true; } \
  | tr -d '"' | sort -u > "$TMP/shader_roots"

# 2. follow #include "x" recursively (GLSLProgram expands them itself)
: > "$TMP/shaders"
queue=$(cat "$TMP/shader_roots")
while [ -n "$queue" ]; do
    next=""
    for s in $queue; do
        grep -Fxq -- "$s" "$TMP/shaders" && continue
        echo "$s" >> "$TMP/shaders"
        [ -f "Shaders/$s" ] || continue
        inc=$(sed -n 's/^[[:space:]]*#include[[:space:]]*"\([^"]*\)".*/\1/p' "Shaders/$s")
        next="$next $inc"
    done
    queue=$(echo $next)
done

# 3. ship the ones that are tracked; generated files (set_tex.*, textures.h,
#    set_attributes.vert) are rewritten by GLSLMgr::init, so untracked is fine
GENERATED="set_tex.frag set_tex.vert set_attributes.vert textures.h"
nsh=0
while read -r s; do
    if is_tracked "Shaders/$s"; then
        add_file "Shaders/$s"; nsh=$((nsh+1))
    elif echo " $GENERATED " | grep -q " $s "; then
        vlog "  (generated at runtime: $s)"
    elif [ -f "Shaders/$s" ]; then
        warn "shader Shaders/$s is used but not tracked by git - skipped"
    else
        vlog "  (referenced but no such file: $s)"
    fi
done < "$TMP/shaders"
add_dir Shaders                 # must exist & be writable (.debug dumps)
nskip=$(git ls-files -- 'Shaders/' | sed 's#^Shaders/##' | { grep -Fvxf "$TMP/shaders" || true; } | wc -l)
say "shaders: $nsh used, $nskip tracked-but-unused skipped"
if [ "$VERBOSE" = 1 ]; then
    git ls-files -- 'Shaders/' | sed 's#^Shaders/##' | { grep -Fvxf "$TMP/shaders" || true; } \
        | sed 's/^/    - unused: /'
fi

if [ -s "$TMP/trimmed" ]; then
    mb=$(while read -r f; do [ -f "$f" ] && wc -c < "$f"; done < "$TMP/trimmed" | awk '{s+=$1} END {printf "%.0f", s/1048576}')
    say "textures: $(wc -l < "$TMP/trimmed") unused images in $TRIM_DIRS skipped (${mb} MB)"
    [ "$VERBOSE" = 1 ] && sed 's/^/    - unused: /' "$TMP/trimmed"
fi

#--- runtime-created dirs ----------------------------------------------------
add_dir Bitmaps; add_dir Images; add_dir Movies; add_dir bin

#=============================================================================
# DLL dependencies
#=============================================================================
: > "$TMP/dlls"
if [ "$DLLS" = 1 ]; then
    command -v objdump >/dev/null || die "objdump not found (install binutils, or use -D)"
    # search path: DLL_PATH, dir of the exe, mingw sysroots, then PATH
    SEARCH="${DLL_PATH:-}:$(dirname "$EXE")"
    for cc in x86_64-w64-mingw32-g++ g++; do
        p=$(command -v $cc 2>/dev/null) && SEARCH="$SEARCH:$(dirname "$p")"
    done
    SEARCH="$SEARCH:/usr/x86_64-w64-mingw32/sys-root/mingw/bin:/mingw64/bin:/ucrt64/bin:$PATH"

    find_dll() {    # case-insensitive lookup of $1 along $SEARCH
        local IFS=: d hit
        for d in $SEARCH; do
            [ -n "$d" ] && [ -d "$d" ] || continue
            case "$d" in /c/[Ww]indows*|/cygdrive/c/[Ww]indows*|*/[Ss]ystem32*) continue ;; esac
            hit=$(find "$d" -maxdepth 1 -iname "$1" -print -quit 2>/dev/null)
            [ -n "$hit" ] && { echo "$hit"; return 0; }
        done
        return 1
    }
    is_system_dll() {
        case "$(echo "$1" | tr 'A-Z' 'a-z')" in
            api-ms-win-*|ext-ms-*|kernel32.dll|user32.dll|gdi32.dll|advapi32.dll|\
            shell32.dll|ole32.dll|oleaut32.dll|comctl32.dll|comdlg32.dll|\
            winspool.drv|winmm.dll|ws2_32.dll|wsock32.dll|opengl32.dll|glu32.dll|\
            msvcrt.dll|ucrtbase.dll|shlwapi.dll|uxtheme.dll|version.dll|rpcrt4.dll|\
            imm32.dll|oleacc.dll|uuid.dll|gdiplus.dll|msimg32.dll|ntdll.dll|\
            dwmapi.dll|wininet.dll|setupapi.dll|bcrypt.dll|crypt32.dll|secur32.dll|\
            dbghelp.dll|psapi.dll|usp10.dll|d2d1.dll|dwrite.dll|windowscodecs.dll|\
            mpr.dll|netapi32.dll|userenv.dll|iphlpapi.dll|shcore.dll) return 0 ;;
        esac
        return 1
    }

    queue="$EXE"; seen=" "
    while [ -n "$queue" ]; do
        next=""
        for bin in $queue; do
            for dll in $( (objdump -p "$bin" 2>/dev/null || true) | sed -n 's/^[[:space:]]*DLL Name: //p' | tr -d '\r'); do
                key=$(echo "$dll" | tr 'A-Z' 'a-z')
                case "$seen" in *" $key "*) continue ;; esac
                seen="$seen$key "
                is_system_dll "$dll" && continue
                if path=$(find_dll "$dll"); then
                    echo "$path" >> "$TMP/dlls"; next="$next $path"
                    vlog "dll: $dll <- $path"
                else
                    warn "DLL '$dll' not found on search path - add its dir to DLL_PATH"
                fi
            done
        done
        queue=$(echo $next)
    done
    grep -qi 'cygwin1\.dll' "$TMP/dlls" && \
        warn "vtx.exe links cygwin1.dll (built with Cygwin g++, not MinGW); the bundle will include the Cygwin runtime"
    say "dlls: $(wc -l < "$TMP/dlls") collected"
fi

sort -u -o "$TMP/files" "$TMP/files"
sort -u -o "$TMP/dirs"  "$TMP/dirs"

#=============================================================================
# dry run
#=============================================================================
if [ "$DRYRUN" = 1 ]; then
    say "manifest for $BUNDLE (dry run)"
    echo "$EXE -> bin/$(basename "$EXE")"
    sed 's#.*/#bin/#' "$TMP/dlls"
    cat "$TMP/files"
    sed 's#\(.*\)\.odt$#\1.odt -> \1.pdf  (converted)#' "$TMP/convert"
    say "$(wc -l < "$TMP/files") data files, $(wc -l < "$TMP/dlls") dlls"
    exit 0
fi

#=============================================================================
# stage
#=============================================================================
STAGE="$OUTDIR/$APPNAME"
rm -rf "$STAGE"; mkdir -p "$STAGE"
while read -r d; do mkdir -p "$STAGE/$d"; done < "$TMP/dirs"
while read -r f; do
    mkdir -p "$STAGE/$(dirname "$f")"
    cp -p "$f" "$STAGE/$f"
done < "$TMP/files"
cp -p "$EXE" "$STAGE/bin/vtx.exe"

# Docs: .odt -> .pdf with headless LibreOffice. A private profile dir lets
# this run even while LibreOffice is open on the desktop.
if [ -s "$TMP/convert" ]; then
    winpath() { if command -v cygpath >/dev/null; then cygpath "$@"; else echo "${@: -1}"; fi; }
    mkdir -p "$TMP/lo-profile"
    LO_PROFILE="file:///$(winpath -m "$TMP/lo-profile" | sed 's#^/##')"
    while read -r f; do
        out="$STAGE/$(dirname "$f")"; mkdir -p "$out"
        pdf="$out/$(basename "${f%.odt}").pdf"
        if [ -x tools/manual/build.sh ] || [ -f tools/manual/build.sh ]; then
            refresh=""
            case " $MANUALS " in *" $f "*) refresh=--refresh ;; esac
            say "converting $f -> PDF${refresh:+ (reference refreshed from source)}"
            SOFFICE="$SOFFICE" VTX_VERSION="$VERSION" bash tools/manual/build.sh pdf "$ROOT/$f" "$pdf" $refresh </dev/null \
                || die "PDF conversion of $f failed"
        else
            say "converting $f -> PDF"
            "$SOFFICE" --headless --norestore "-env:UserInstallation=$LO_PROFILE" \
                --convert-to pdf --outdir "$(winpath -w "$out")" "$(winpath -w "$ROOT/$f")" >/dev/null 2>&1 || true
        fi
        [ -f "$pdf" ] || die "PDF conversion of $f failed (tried: $SOFFICE)"
    done < "$TMP/convert"
fi
while read -r d; do cp -p "$d" "$STAGE/bin/"; done < "$TMP/dlls"

# Launcher: VtxScene renders with wxSetWorkingDirectory("../Shaders"),
# relative to the *current* directory, so cwd must be bin\ at startup.
# With no arguments, reopen the last session (Saves\last.spx is written on
# quit, so it only exists after the first run).
printf '%s\r\n' \
  '@echo off' \
  'rem VTX launcher - vtx.exe must start with bin\ as its working directory' \
  'cd /d "%~dp0bin"' \
  'if "%~1"=="" if exist "..\Saves\last.spx" (' \
  '    start "" vtx.exe --m last.spx' \
  '    goto :eof' \
  ')' \
  'start "" vtx.exe %*' > "$STAGE/VTX.bat"

cat > "$STAGE/README.txt" <<EOF
VTX $VERSION - Windows bundle

Install
  Unzip this folder anywhere you have write access, e.g.
  C:\\Users\\<you>\\VTX  (NOT under "C:\\Program Files": VTX writes to
  Shaders\\, Saves\\, Resources\\UI\\prefs.spx, Bitmaps\\ and Images\\).

Run
  Double-click VTX.bat, or make a shortcut to bin\\vtx.exe with
  "Start in" set to the bin folder. Optional arguments:
    VTX.bat [--s|--m|--b|--w|--p] [scene]     (scene = name in Saves\\)
  With no arguments VTX reopens the last session (Saves\\last.spx).

Requires an OpenGL driver with GLSL + geometry shader support.

Documentation is in Docs\\.
EOF
printf 's/$/\r/\n' > "$TMP/crlf.sed"; sed -i -f "$TMP/crlf.sed" "$STAGE/README.txt"

( cd "$STAGE" && find . -type f ! -name MANIFEST.txt | sed 's#^\./##' | sort ) > "$STAGE/MANIFEST.txt"

#=============================================================================
# archive (.zip preferred)
#=============================================================================
ABS_OUT=$(cd "$OUTDIR" && pwd)
if command -v zip >/dev/null; then
    ARCHIVE="$ABS_OUT/$BUNDLE.zip"; rm -f "$ARCHIVE"
    ( cd "$OUTDIR" && zip -qr9 "$ARCHIVE" "$APPNAME" )
elif command -v 7z >/dev/null; then
    ARCHIVE="$ABS_OUT/$BUNDLE.zip"; rm -f "$ARCHIVE"
    ( cd "$OUTDIR" && 7z a -tzip -mx=9 "$BUNDLE.zip" "$APPNAME" >/dev/null )
elif WINTAR=$(for t in /c/Windows/System32/tar.exe /cygdrive/c/Windows/System32/tar.exe; do
                 [ -x "$t" ] && { echo "$t"; break; }; done) && [ -n "$WINTAR" ]; then
    # Windows 10+ ships bsdtar as tar.exe; -a picks .zip format from the name
    # (relative names only, so no MSYS/Cygwin path translation is needed)
    ARCHIVE="$ABS_OUT/$BUNDLE.zip"; rm -f "$ARCHIVE"
    ( cd "$OUTDIR" && "$WINTAR" -a -c -f "$BUNDLE.zip" "$APPNAME" )
else
    ARCHIVE="$ABS_OUT/$BUNDLE.tar.gz"
    warn "zip/7z/tar.exe not found - writing $ARCHIVE instead"
    tar -czf "$ARCHIVE" -C "$OUTDIR" "$APPNAME"
fi
if [ "$INSTALLER" = 1 ]; then
    say "building installer with $ISCC"
    # ISCC accepts -X as well as /X switches; use "-" so Git Bash (MSYS) doesn't
    # mistake /Q, /D..., /O... for Unix paths and rewrite them
    MSYS2_ARG_CONV_EXCL='*' MSYS_NO_PATHCONV=1 \
    "$ISCC" -Q "-DSrcDir=$(cygpath -w "$ABS_OUT/$APPNAME")" "-DAppVersion=$VERSION" \
        "-O$(cygpath -w "$ABS_OUT")" "$(cygpath -w "$ROOT/installer/vtx.iss")" \
        || die "ISCC failed"
    [ -f "$ABS_OUT/$BUNDLE-setup.exe" ] || die "ISCC ran but $ABS_OUT/$BUNDLE-setup.exe was not created"
    say "wrote $ABS_OUT/$BUNDLE-setup.exe"
fi
[ "$KEEP" = 1 ] || rm -rf "$STAGE"

say "$(wc -l < "$TMP/files") data files, $(wc -l < "$TMP/dlls") dlls, $(du -h "$ARCHIVE" | cut -f1)"
say "wrote $ARCHIVE"
