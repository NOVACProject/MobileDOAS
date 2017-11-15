; Mobile DOAS installer

[Setup]
AppName=MobileDOAS
AppVersion=6.0
DefaultDirName={pf}\MobileDOAS
DefaultGroupName=MobileDOAS
Compression=lzma2
SolidCompression=yes   
UninstallDisplayIcon={app}\MobileDOAS.exe
; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
ArchitecturesAllowed=x64
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "..\64\Release\MobileDOAS.exe"; DestDir: "{app}" 
Source: "..\doc\MobileDOAS_5_UserManual.pdf"; DestDir: "{app}"

[Icons]
Name: "{group}\MobileDOAS"; Filename: "{app}\MobileDOAS.exe"
