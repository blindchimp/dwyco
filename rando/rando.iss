[Setup]
DisableReadyMemo=True
AppName=rando
AppVersion=1.0
AppCopyright=Dwyco, Inc.
PrivilegesRequired=lowest
DefaultDirName={userdocs}\Dwyco\Programs\rando
DisableDirPage=yes
DisableProgramGroupPage=auto
AppPublisher=Dwyco, Inc.
AppPublisherURL=http://www.dwyco.com
AppSupportURL=http://www.dwyco.com
AppContact=cdchelp@dwyco.com
SourceDir=C:\Users\dwight\dwyco\rando\vcdist
MinVersion=0,6.1
OutputDir=C:\Users\dwight\randoinst
OutputBaseFilename=dwycorando
VersionInfoVersion=1.0
VersionInfoCompany=Dwyco, Inc.
VersionInfoDescription=Internet video/audio/conferencing
VersionInfoCopyright=Dwyco, Inc.
VersionInfoProductName=rando
VersionInfoProductVersion=1.0
AlwaysUsePersonalGroup=True
DefaultGroupName=rando
DisableStartupPrompt=False
AlwaysShowGroupOnReadyPage=True
AlwaysShowDirOnReadyPage=True
DisableReadyPage=True

[Files]
Source: "C:\Users\dwight\dwyco\rando\vcdist\*"; DestDir: "{app}"; Flags: ignoreversion createallsubdirs recursesubdirs

[Icons]
Name: "{group}\rando"; Filename: "{app}\rando.exe"; WorkingDir: "{app}"; Flags: runmaximized
