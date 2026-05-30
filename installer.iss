[Setup]
AppName=AFTERGLOW
AppVersion=1.0.0
AppPublisher=PNK NOISE
DefaultDirName={commoncf64}\VST3\AFTERGLOW.vst3
DisableDirPage=yes
DefaultGroupName=AFTERGLOW
DisableProgramGroupPage=yes
OutputBaseFilename=AFTERGLOW_Windows_Installer
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "build\AFTERGLOW_artefacts\Release\VST3\AFTERGLOW.vst3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; Note: JUCE VST3 builds as a folder with .vst3 extension. We copy its contents to {app} which is the .vst3 folder.

[Messages]
FinishedHeadingLabel=Setup has finished installing AFTERGLOW
FinishedLabel=AFTERGLOW has been installed in your VST3 folder.%n%nPlease rescan your plugins in your DAW to use it.
