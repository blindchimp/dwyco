[Setup]
DisableReadyMemo=True
AppName=phoo
AppVersion=1.0
AppCopyright=Dwyco, Inc.
PrivilegesRequired=lowest
DefaultDirName={userdocs}\Dwyco\Programs\phoo
DisableDirPage=yes
DisableProgramGroupPage=auto
AppPublisher=Dwyco, Inc.
AppPublisherURL=http://www.dwyco.com
AppSupportURL=http://www.dwyco.com
AppContact=cdchelp@dwyco.com
SourceDir=C:\Users\dwight\dwyco\phoo\vcdist
MinVersion=0,6.1
OutputDir=C:\Users\dwight\phooinst
OutputBaseFilename=dwycophoo
VersionInfoVersion=1.0
VersionInfoCompany=Dwyco, Inc.
VersionInfoDescription=Internet video/audio/conferencing
VersionInfoCopyright=Dwyco, Inc.
VersionInfoProductName=phoo
VersionInfoProductVersion=1.0
AlwaysUsePersonalGroup=True
DefaultGroupName=phoo
DisableStartupPrompt=False
AlwaysShowGroupOnReadyPage=True
AlwaysShowDirOnReadyPage=True
DisableReadyPage=True

[Files]
Source: "C:\Users\dwight\dwyco\phoo\vcdist\*"; DestDir: "{app}"; Flags: ignoreversion createallsubdirs recursesubdirs

[Icons]
Name: "{group}\phoo"; Filename: "{app}\phoo.exe"; WorkingDir: "{app}"; Flags: runmaximized
