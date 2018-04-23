[Setup]
DisableReadyPage=True
DisableReadyMemo=True
AppName=phoo
AppVersion=1.0
AppCopyright=Dwyco, Inc.
PrivilegesRequired=none
DefaultDirName={userdocs}\Dwyco\Programs\phoo
DisableDirPage=yes
DisableProgramGroupPage=auto
AppPublisher=Dwyco, Inc.
AppPublisherURL=http://www.dwyco.com
AppSupportURL=http://www.dwyco.com
AppContact=cdchelp@dwyco.com
SourceDir=C:\Users\dwight\phoo\vcdist
MinVersion=0,6.1
OutputDir=C:\Users\dwight\phoo
OutputBaseFilename=dwycophoo
VersionInfoVersion=1.0
VersionInfoCompany=Dwyco, Inc.
VersionInfoDescription=Internet video/audio/conferencing
VersionInfoCopyright=Dwyco, Inc.
VersionInfoProductName=phoo
VersionInfoProductVersion=1.0
AlwaysUsePersonalGroup=True
DefaultGroupName=phoo

[Files]
Source: "C:\Users\dwight\phoo\vcdist\*"; DestDir: "{app}"; Flags: ignoreversion createallsubdirs recursesubdirs

[Icons]
Name: "{group}\phoo"; Filename: "{app}\phoo.exe"; WorkingDir: "{app}"; Flags: runmaximized
