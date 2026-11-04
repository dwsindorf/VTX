; vtx.iss - Inno Setup 6 script for the VTX Windows installer
;
; Normally built by:   ./mkdist.sh -i      (or: make installer)
; which stages the bundle and runs
;   ISCC.exe /DSrcDir=<staged VTX dir> /DAppVersion=<git describe> /O<outdir> vtx.iss
;
; Installs per-user (no admin) into %LOCALAPPDATA%\Programs\VTX.
; VTX writes into its own folders (Shaders\*.debug + generated shaders,
; Saves\last.spx, Resources\UI\prefs.spx, Bitmaps\, Images\), so it must
; NOT be installed under Program Files - all-users mode is deliberately off.

#ifndef SrcDir
  #define SrcDir "..\dist\VTX"
#endif
#ifndef AppVersion
  #define AppVersion "dev"
#endif

[Setup]
AppId={{F11760BD-A7F0-4795-875F-90E37688CC99}
AppName=VTX
AppVersion={#AppVersion}
AppVerName=VTX {#AppVersion}
DefaultDirName={autopf}\VTX
DefaultGroupName=VTX
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputBaseFilename=VTX-win64-{#AppVersion}-setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\bin\vtx.exe
UninstallDisplayName=VTX

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; everything except files the user may have changed since the last install
Source: "{#SrcDir}\*"; DestDir: "{app}"; Excludes: "\Saves\*,\Resources\UI\prefs.spx,\VTX.bat"; \
    Flags: ignoreversion recursesubdirs createallsubdirs
; sample scenes and prefs: install once, never overwrite the user's edits on upgrade
Source: "{#SrcDir}\Saves\*"; DestDir: "{app}\Saves"; Flags: onlyifdoesntexist uninsneveruninstall recursesubdirs createallsubdirs
Source: "{#SrcDir}\Resources\UI\prefs.spx"; DestDir: "{app}\Resources\UI"; Flags: onlyifdoesntexist uninsneveruninstall

[Dirs]
Name: "{app}\Saves"
Name: "{app}\Bitmaps"
Name: "{app}\Images"
Name: "{app}\Movies"
Name: "{app}\Textures\Processed"

[Icons]
; WorkingDir must be bin\ - VtxScene uses wxSetWorkingDirectory("../Shaders")
Name: "{autoprograms}\VTX"; Filename: "{app}\bin\vtx.exe"; WorkingDir: "{app}\bin"
Name: "{autoprograms}\VTX Theory of Operation"; Filename: "{app}\Docs\VTX-Theory-of-Operation.pdf"
Name: "{autodesktop}\VTX"; Filename: "{app}\bin\vtx.exe"; WorkingDir: "{app}\bin"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\vtx.exe"; WorkingDir: "{app}\bin"; Description: "{cm:LaunchProgram,VTX}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; files VTX generates at runtime (user content in Saves\, Images\ etc. is kept)
Type: files; Name: "{app}\Shaders\*.debug"
Type: files; Name: "{app}\Saves\last.spx"
