# Building VTX

This describes how to build VTX from source and how to package it for distribution. To *use* VTX, see the [README](README.md) and the [User Manual](Docs/VTX-User-Manual.odt). For how VTX works internally, see the [Theory of Operation](Docs/VTX-Theory-of-Operation.odt).

## Overview

The Windows build is a cross-compile. You build in a **Cygwin** terminal, but the compiler is Cygwin's MinGW-w64 cross-compiler (`x86_64-w64-mingw32-g++`), so `vtx.exe` is a native Windows program that doesn't need Cygwin to run.

| Component | Version in use | Notes |
|---|---|---|
| Cygwin | 64-bit | Provides make, bison, flex, git and the MinGW-w64 cross-compiler |
| MinGW-w64 g++ | 11.3 | Cygwin package `mingw64-x86_64-gcc-g++` |
| wxWidgets | 3.0.6 | Static, monolithic, ANSI (non-Unicode) build with OpenGL |

> **Build VTX from a Cygwin terminal, not Git Bash.** The wxWidgets tools are configured with Cygwin paths (`/cygdrive/c/...`), which don't exist in Git Bash. Packaging (`make distribution`, `make installer`, `make manual`) works from either.

## 1. Install Cygwin

1. Download and run `setup-x86_64.exe` from <https://www.cygwin.com/>. Keep the default install directory, `C:\cygwin64`.
2. In the package list, install at least:
   - `binutils`, `bison`, `flex`, `gcc-core`, `gcc-g++`, `git`, `make`
   - `mingw64-x86_64-gcc-g++` (the MinGW-w64 C++ cross-compiler, which brings the rest of the MinGW-w64 toolchain with it)
   - `zip` (optional, used by `make distribution`)
3. Open a **Cygwin64 Terminal** for the following steps.

## 2. Build wxWidgets

VTX uses a static, monolithic, **ANSI** (non-Unicode) build of wxWidgets with OpenGL support. The VTX code relies on the ANSI build (for example, `wxString::ToAscii()` returning a `char*`).

> wxWidgets 3.0.6 is the tested version. ANSI builds (`--disable-unicode`) are not available in wxWidgets 3.3 and later, so don't use those versions.

```sh
cd /cygdrive/c/Users/$USER        # or wherever you keep wxWidgets
git clone https://github.com/wxWidgets/wxWidgets
cd wxWidgets
git checkout v3.0.6

mkdir release && cd release
export LDFLAGS='-static-libstdc++ -static-libgcc'
../configure --with-msw --disable-shared --disable-unicode \
             --host=x86_64-w64-mingw32 --build=i686-pc-cygwin \
             --enable-monolithic --with-opengl --disable-precomp-headers
make -j8
```

There's no need for `make install`: VTX uses this build in place, through `wxWidgets/release/wx-config`.

To check the build, compile and run a sample:

```sh
cd samples/minimal && make && ./minimal.exe
```

## 3. Set up the environment

Add these to `~/.bashrc` in Cygwin, adjusting the path to your wxWidgets directory:

```sh
export PLATFORM=WIN
export PATH=/cygdrive/c/Users/$USER/wxWidgets/release:$PATH
```

`PLATFORM` is passed to the compiler as `-D$(PLATFORM)`. Check the setup with:

```sh
wx-config --version          # 3.0.6
wx-config --cxx              # x86_64-w64-mingw32-g++
```

## 4. Build VTX

```sh
cd VTX/Release
make build_wx                # builds Release/vtx.exe
```

`make showenv` prints the flags and paths the build uses. `make clean` removes the build files. The `Debug` directory builds a debug version the same way.

Check that the result is a self-contained Windows program. It should list only Windows system DLLs, and no `cygwin1.dll`:

```sh
objdump -p vtx.exe | grep "DLL Name"
```

Run the new build with:

```sh
cd ../bin
./vtx-last                   # reopens the last session, if there is one
```

## 5. Package a release

These targets run from the top-level directory, in Cygwin or Git Bash:

| Command | Output (in *dist/*) | Needs |
|---|---|---|
| `make distribution` | `VTX-win64-version.zip` | LibreOffice (to convert the manuals to PDF) |
| `make installer` | the zip, plus `VTX-win64-version-setup.exe` | LibreOffice and [Inno Setup 6](https://jrsoftware.org/isinfo.php) |
| `make manual` | `VTX-User-Manual.pdf`, after refreshing the manual's reference section | LibreOffice |

The bundle contains only files tracked by git that VTX actually uses. `./mkdist.sh -h` describes the rules, and `make dist DISTFLAGS="-n -v"` lists what would be included.

Part II of the User Manual is generated from the source code by the scripts in `tools/manual`. Don't edit that part of `Docs/VTX-User-Manual.odt` by hand. Put control descriptions in `tools/manual/control_notes.txt` instead.

To publish a release, tag the commit, run `make installer`, and attach the two files in *dist/* to a new release on GitHub, for example:

```sh
git tag v1.0 && git push --tags
gh release create v1.0 dist/VTX-win64-v1.0.zip dist/VTX-win64-v1.0-setup.exe --title "VTX 1.0"
```

Making movies in VTX also needs FFmpeg (`ffmpeg` and `ffplay` on the PATH), which isn't included in the bundle.

## Using Eclipse (optional)

The repository includes Eclipse CDT project files (`.project`, `.cproject` and `.settings/`). Eclipse runs the same makefiles, so the command-line build above must work first.

1. **File > Import > General > Existing Projects into Workspace**, and select the VTX directory.
2. Under **Project > Properties > C/C++ Build > Tool Chain Editor**, use the **Cygwin GCC** toolchain.
3. Define `PLATFORM=WIN` and add the wxWidgets `release` directory to `PATH`, either as Windows environment variables or in the project's **C/C++ Build > Environment**.
4. Use the make targets in the **Release** and **Debug** folders: **build** (runs `build_wx`), **clean**, **test** and **showenv**.

## Linux

Not verified recently. VTX was last built on Ubuntu 18.04, and `bin/vtx-linux` is a Linux build. The wxWidgets configure line used there was:

```sh
../configure --with-gtk --with-opengl --disable-shared --disable-unicode --disable-infobar
```

`--disable-infobar` is needed because the ANSI build of `InfoBar.cpp` doesn't compile. Set `PLATFORM=LINUX` before running `make build_wx`.

## Troubleshooting

**`wx-config`: Directory '/cygdrive/c/...' no longer exists.** You're in Git Bash or another non-Cygwin shell. Build from a Cygwin terminal.

**`vtx.exe` needs `cygwin1.dll`.** It was built with Cygwin's own compiler instead of the MinGW-w64 cross-compiler. Build with `make build_wx CC=x86_64-w64-mingw32-g++`, and check `wx-config --cxx`.

**Missing DLLs on another PC** (`libstdc++-6.dll`, `libwinpthread-1.dll` and so on). `vtx.exe` should be linked with `-static`, and wxWidgets built with `LDFLAGS='-static-libstdc++ -static-libgcc'`. `make distribution` copies any non-system DLLs it finds into the bundle, so check its "dlls collected" count.
