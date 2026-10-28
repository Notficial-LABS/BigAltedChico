; SPDX-License-Identifier: MIT
Unicode True
!include "MUI2.nsh"
!include "x64.nsh"
!include "WinVer.nsh"
!include "Sections.nsh"
!ifndef PAYLOAD
 !error "Pass PAYLOAD, OUTPUT, VERSION and RESOURCES to makensis."
!endif
Name "BigAltedChico"
OutFile "${OUTPUT}"
InstallDir "$PROGRAMFILES64\Notficial LABS\BigAltedChico"
InstallDirRegKey HKLM "Software\Notficial LABS\BigAltedChico" "InstallDir"
RequestExecutionLevel admin
SetCompressor /SOLID lzma
AllowSkipFiles off
ShowInstDetails show
ShowUninstDetails show
VIProductVersion "${VERSION}.0"
VIAddVersionKey /LANG=1033 "ProductName" "BigAltedChico"
VIAddVersionKey /LANG=1033 "CompanyName" "Notficial LABS"
VIAddVersionKey /LANG=1033 "FileDescription" "BigAltedChico Windows x64 Test Installer"
VIAddVersionKey /LANG=1033 "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=1033 "LegalCopyright" "Leonardo Marcelo / Notficial LABS"
!define MUI_ABORTWARNING
!define MUI_ICON "${RESOURCES}/../Icons/BAC-windows.ico"
!define MUI_UNICON "${RESOURCES}/../Icons/BAC-windows.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${RESOURCES}/WindowsHeader.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${RESOURCES}/WindowsSidebar.bmp"
!define MUI_WELCOMEPAGE_TITLE "BigAltedChico ${VERSION}"
!define MUI_WELCOMEPAGE_TEXT "Install VST3 and/or the Standalone application.$\r$\n$\r$\nClose your DAW and BigAltedChico before continuing.$\r$\n$\r$\nThis is an unsigned Windows x64 test build. Your presets, custom skins and preferences are preserved."
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${PAYLOAD}/Licenses/Combined-License.txt"
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE CheckSelection
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_TEXT "BigAltedChico has been installed.$\r$\n$\r$\nRescan VST3 plugins in your DAW, or use the Standalone shortcut if selected.$\r$\n$\r$\nNew instances start with Clean. Raise Mix to hear the effect. LIVE: 30 ms; Quality: 150 ms, plus host/device buffering."
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "VST3" SEC_VST3
 SetOutPath "$COMMONFILES64\VST3\BigAltedChico.vst3"
 File /r "${PAYLOAD}/BigAltedChico.vst3/*"
 WriteRegDWORD HKLM "Software\Notficial LABS\BigAltedChico" "VST3" 1
SectionEnd
Section "Standalone" SEC_APP
 SetOutPath "$INSTDIR"
 File "${PAYLOAD}/BigAltedChico.exe"
 CreateDirectory "$SMPROGRAMS\Notficial LABS"
 CreateShortcut "$SMPROGRAMS\Notficial LABS\BigAltedChico.lnk" "$INSTDIR\BigAltedChico.exe"
 WriteRegDWORD HKLM "Software\Notficial LABS\BigAltedChico" "Standalone" 1
SectionEnd
Section -Notices
 SetOutPath "$INSTDIR\Licenses"
 File /r "${PAYLOAD}/Licenses/*"
 WriteUninstaller "$INSTDIR\Uninstall.exe"
 WriteRegStr HKLM "Software\Notficial LABS\BigAltedChico" "InstallDir" "$INSTDIR"
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NotficialLABS.BigAltedChico" "DisplayName" "BigAltedChico"
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NotficialLABS.BigAltedChico" "DisplayVersion" "${VERSION}"
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NotficialLABS.BigAltedChico" "Publisher" "Notficial LABS"
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NotficialLABS.BigAltedChico" "DisplayIcon" '$\"$INSTDIR\Uninstall.exe$\",0'
 WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NotficialLABS.BigAltedChico" "UninstallString" '$\"$INSTDIR\Uninstall.exe$\"'
 WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NotficialLABS.BigAltedChico" "NoModify" 1
 WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NotficialLABS.BigAltedChico" "NoRepair" 1
SectionEnd
Function .onInit
 ${IfNot} ${RunningX64}
  MessageBox MB_OK|MB_ICONSTOP "This installer requires 64-bit Windows."
  Abort
 ${EndIf}
 ${IfNot} ${AtLeastWin10}
  MessageBox MB_OK|MB_ICONSTOP "Windows 10 or newer is required."
  Abort
 ${EndIf}
 SetRegView 64
 SetShellVarContext all
FunctionEnd
Function CheckSelection
 ${IfNot} ${SectionIsSelected} ${SEC_VST3}
 ${AndIfNot} ${SectionIsSelected} ${SEC_APP}
  MessageBox MB_OK|MB_ICONEXCLAMATION "Select at least one component."
  Abort
 ${EndIf}
FunctionEnd
Function un.onInit
 SetRegView 64
 SetShellVarContext all
FunctionEnd
Section "Uninstall"
 ; Remove only our installed program files, never AppData, presets or skins.
 ReadRegDWORD $0 HKLM "Software\Notficial LABS\BigAltedChico" "VST3"
 ${If} $0 == 1
  Delete "$COMMONFILES64\VST3\BigAltedChico.vst3\Contents\x86_64-win\BigAltedChico.vst3"
  Delete "$COMMONFILES64\VST3\BigAltedChico.vst3\Contents\Resources\moduleinfo.json"
  RMDir "$COMMONFILES64\VST3\BigAltedChico.vst3\Contents\x86_64-win"
  RMDir "$COMMONFILES64\VST3\BigAltedChico.vst3\Contents\Resources"
  RMDir "$COMMONFILES64\VST3\BigAltedChico.vst3\Contents"
  RMDir "$COMMONFILES64\VST3\BigAltedChico.vst3"
 ${EndIf}
 Delete "$INSTDIR\BigAltedChico.exe"
 Delete "$SMPROGRAMS\Notficial LABS\BigAltedChico.lnk"
 RMDir "$SMPROGRAMS\Notficial LABS"
 !include "${PAYLOAD}/UninstallLicenses.nsh"
 RMDir "$INSTDIR\Licenses"
 Delete "$INSTDIR\Uninstall.exe"
 RMDir "$INSTDIR"
 DeleteRegKey HKLM "Software\Notficial LABS\BigAltedChico"
 DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NotficialLABS.BigAltedChico"
SectionEnd
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
 !insertmacro MUI_DESCRIPTION_TEXT ${SEC_VST3} "VST3 for Ableton Live and compatible 64-bit hosts. Installed in Common Files\VST3."
 !insertmacro MUI_DESCRIPTION_TEXT ${SEC_APP} "Use BigAltedChico without a DAW. Installed in Program Files with a Start Menu shortcut."
!insertmacro MUI_FUNCTION_DESCRIPTION_END
