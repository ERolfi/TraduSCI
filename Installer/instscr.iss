[Setup]
AppName=TraduSCI II
AppVerName=TraduSCI 2.4
AppPublisher=Enrico Rolfi
AppPublisherURL=http://erolfi.wordpress.com
AppSupportURL=http://erolfi.wordpress.com
AppUpdatesURL=http://erolfi.wordpress.com
DefaultDirName={pf}\TraduSCI24
DefaultGroupName=TraduSCI 2.4
;LicenseFile=End User License.txt
Compression=lzma
SolidCompression=yes
ChangesAssociations=yes
PrivilegesRequired=poweruser
ShowLanguageDialog=yes

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"; LicenseFile:"English\End User License.txt"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"; LicenseFile:"Italian\Licenza di Utilizzo.txt"
Name: "sp"; MessagesFile: "compiler:Languages\Spanish.isl"; LicenseFile:"Spanish\End User License.txt"



[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "associateV56"; Description: "{cm:AssocFileExtension,TraduSCI,V56}"; GroupDescription: "Associate Filetype:"
Name: "associateP56"; Description: "{cm:AssocFileExtension,TraduSCI,P56}"; GroupDescription: "Associate Filetype:"
Name: "associateMSG"; Description: "{cm:AssocFileExtension,TraduSCI,MSG}"; GroupDescription: "Associate Filetype:"

[Files]
Source: "English\SCIaMano.exe"; DestDir: "{app}"; Flags: ignoreversion; Languages: en
Source: "Italian\SCIaMano.exe"; DestDir: "{app}"; Flags: ignoreversion; Languages: it
Source: "Spanish\SCIaMano.exe"; DestDir: "{app}"; Flags: ignoreversion; Languages: sp
Source: "English\fotoscihop.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: en
Source: "Italian\fotoscihop.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: it
Source: "Spanish\fotoscihop.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: sp
Source: "English\End User License.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: en
Source: "Italian\Licenza di Utilizzo.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: it
Source: "Spanish\End User License.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: sp
Source: "wingraph32.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "English\Sciamano.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: en
Source: "Italian\Sciamano.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: it
Source: "Spanish\Sciamano.txt"; DestDir: "{app}"; Flags: ignoreversion; Languages: sp
Source: "English\SCIdump.dll"; DestDir: "{app}"; Flags: ignoreversion; Languages:  en
Source: "Italian\SCIdump.dll"; DestDir: "{app}"; Flags: ignoreversion; Languages:  it
Source: "Spanish\SCIdump.dll"; DestDir: "{app}"; Flags: ignoreversion; Languages:  sp
Source: "audio.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "Audio.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "English\FotoSCIhop.exe"; DestDir: "{app}"; Flags: ignoreversion; Languages: en
Source: "Italian\FotoSCIhop.exe"; DestDir: "{app}"; Flags: ignoreversion; Languages: it
Source: "Spanish\FotoSCIhop.exe"; DestDir: "{app}"; Flags: ignoreversion; Languages: sp
Source: "SCIaMano.exe.manifest"; DestDir: "{app}"; Flags: ignoreversion
Source: "wingraph32.exe.manifest"; DestDir: "{app}"; Flags: ignoreversion
Source: "FotoSCIhop.exe.manifest"; DestDir: "{app}"; Flags: ignoreversion

Source: "libcrypto-1_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libssl-1_1.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "Samples\*"; DestDir: "{app}\Samples"; Flags: ignoreversion recursesubdirs; Languages:  en sp
Source: "Samples\*"; DestDir: "{app}\Esempi"; Flags: ignoreversion recursesubdirs; Languages:  it
Source: "Fonts\*"; DestDir: "{app}\Fonts"; Flags: ignoreversion recursesubdirs; Languages:  en sp
Source: "Fonts\*"; DestDir: "{app}\Caratteri"; Flags: ignoreversion recursesubdirs; Languages:  it
Source: "English\Manual\*"; DestDir: "{app}\Manual"; Flags: ignoreversion recursesubdirs; Languages: en
Source: "Italian\Manuale\*"; DestDir: "{app}\Manuale"; Flags: ignoreversion recursesubdirs; Languages: it
Source: "Spanish\Manual\*"; DestDir: "{app}\Manual"; Flags: ignoreversion recursesubdirs; Languages: sp
Source: "Tools\*"; DestDir: "{app}\Tools"; Flags: ignoreversion recursesubdirs
Source: "English\Fonts-Read Me.txt"; DestName:"Read Me.txt"; DestDir: "{app}\Fonts"; Flags: ignoreversion recursesubdirs; Languages: en sp
Source: "Italian\Caratteri-note.txt"; DestName:"note.txt"; DestDir: "{app}\Caratteri"; Flags: ignoreversion recursesubdirs; Languages: it
Source: "English\Tools-Read Me.txt"; DestName:"Read Me.txt"; DestDir: "{app}\Tools"; Flags: ignoreversion recursesubdirs; Languages: en sp
Source: "Italian\Tools-note.txt"; DestName:"note.txt"; DestDir: "{app}\Tools"; Flags: ignoreversion recursesubdirs; Languages: it
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKCR; Subkey: ".msg"; ValueType: string; ValueName: ""; ValueData: "MSG_tradusci"; Flags: deletekey uninsdeletevalue; Tasks: associateMSG
Root: HKCR; Subkey: ".v56"; ValueType: string; ValueName: ""; ValueData: "V56_tradusci"; Flags: deletekey uninsdeletevalue; Tasks: associateV56
Root: HKCR; Subkey: ".p56"; ValueType: string; ValueName: ""; ValueData: "P56_tradusci"; Flags: deletekey uninsdeletevalue; Tasks: associateP56

Root: HKCR; Subkey: "MSG_tradusci"; ValueType: string; ValueName: ""; ValueData: "Message"; Flags: deletekey uninsdeletekey; Tasks: associateMSG
Root: HKCR; Subkey: "V56_tradusci"; ValueType: string; ValueName: ""; ValueData: "View 256"; Flags: deletekey uninsdeletekey; Tasks: associateV56
Root: HKCR; Subkey: "P56_tradusci"; ValueType: string; ValueName: ""; ValueData: "Picture 256"; Flags: deletekey uninsdeletekey; Tasks: associateP56

Root: HKCR; Subkey: "MSG_tradusci\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\SCIaMano.exe"" ""%1"""; Tasks: associateMSG
Root: HKCR; Subkey: "V56_tradusci\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\FotoSCIhop.exe"" ""%1"""; Tasks: associateV56
Root: HKCR; Subkey: "P56_tradusci\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\FotoSCIhop.exe"" ""%1"""; Tasks: associateP56


[INI]
Filename: "{app}\TraduSCI.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://erolfi.wordpress.com"
;Filename: "{app}\Pollodigomma.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.iagtg.net"

[Icons]
Name: "{group}\SCIaMano"; Filename: "{app}\SCIaMano.exe"
Name: "{group}\FotoSCIhop"; Filename: "{app}\FotoSCIhop.exe"
Name: "{group}\Tools"; Filename: "{app}\Tools"
Name: "{group}\Fonts"; Filename: "{app}\Fonts"; Languages: en sp
Name: "{group}\Caratteri"; Filename: "{app}\Caratteri"; Languages: it
Name: "{group}\Samples"; Filename: "{app}\Samples"; Languages: en sp
Name: "{group}\Esempi"; Filename: "{app}\Esempi"; Languages: it
Name: "{group}\Manual"; Filename: "{app}\Manual\instructions.html"; Languages: en
Name: "{group}\Manual"; Filename: "{app}\Manual\instrucciones.html"; Languages: sp
Name: "{group}\Manuale"; Filename: "{app}\Manuale\istruzioni.html"; Languages: it
Name: "{group}\{cm:ProgramOnTheWeb,TraduSCI}"; Filename: "{app}\TraduSCI.url"
;Name: "{group}\Pollodigomma IAGTG"; Filename: "{app}\Pollodigomma.url"
Name: "{group}\Read Me (FotoSCIhop)"; Filename: "{app}\fotoscihop.txt"; Languages: en
Name: "{group}\Changes in SCIaMano"; Filename: "{app}\Sciamano.txt"; Languages: en
Name: "{group}\Leerme (FotoSCIhop)"; Filename: "{app}\fotoscihop.txt"; Languages: sp
Name: "{group}\Cambios en SCIaMano"; Filename: "{app}\Sciamano.txt"; Languages: sp
Name: "{group}\Leggimi (FotoSCIhop)"; Filename: "{app}\fotoscihop.txt"; Languages: it
Name: "{group}\Cambiamenti in SCIaMano"; Filename: "{app}\Sciamano.txt"; Languages: it
Name: "{group}\{cm:UninstallProgram,TraduSCI}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\FotoSCIhop"; Filename: "{app}\FotoSCIhop.exe"; Tasks: desktopicon
Name: "{userdesktop}\SCIaMano"; Filename: "{app}\SCIaMano.exe"; Tasks: desktopicon

[UninstallDelete]
Type: files; Name: "{app}\TraduSCI.url"
;Type: files; Name: "{app}\Pollodigomma.url"


[Run]
Filename: "{app}\Manual\instructions.html"; Description: "View the Manual"; Flags: nowait postinstall shellexec skipifsilent; Languages: en
Filename: "{app}\Manual\instrucciones.html"; Description: "Lea el Manual"; Flags: nowait postinstall shellexec skipifsilent; Languages: sp
Filename: "{app}\Manuale\istruzioni.html"; Description: "Consulta il Manuale"; Flags: nowait postinstall shellexec skipifsilent; Languages: it

