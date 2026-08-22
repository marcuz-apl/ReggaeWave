; ReggaeWave Windows Modern NSIS Installer Script
; ------------------------------------------------

!define PRODUCT_NAME "ReggaeWave"
!define PRODUCT_PUBLISHER "Alfazen-Inc"
!define PRODUCT_WEB_SITE "https://github.com/marcuz-apl/ReggaeWave"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\ReggaeWave.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

!ifndef PRODUCT_VERSION
  !define PRODUCT_VERSION "1.3.3-2608221"
!endif

!ifndef EXE_SOURCE
  !define EXE_SOURCE "..\..\build\apps\desktop\ReggaeWave_artefacts\Release\ReggaeWave.exe"
!endif

!ifndef OUTPUT_DIR
  !define OUTPUT_DIR "..\..\dist"
!endif

!ifndef ICON_FILE
  !define ICON_FILE "..\assets\reggaewave.ico"
!endif

; MultiUser Configuration for Dual Installation Tracks (All Users / Current User)
!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_DEFAULT_ALLUSERS
!define MULTIUSER_INSTALLMODE_INSTDIR "Alfazen-Inc\ReggaeWave"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGKEY "${PRODUCT_UNINST_KEY}"
!define MULTIUSER_USE_PROGRAMFILES64

!include "MultiUser.nsh"
!include "MUI2.nsh"
!include "x64.nsh"

; Compressor settings
SetCompressor /SOLID lzma
Unicode true

; Interface Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${ICON_FILE}"
!define MUI_UNICON "${ICON_FILE}"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\ReggaeWave.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ReggaeWave Studio"
!insertmacro MUI_PAGE_FINISH

; Uninstaller Pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Languages
!insertmacro MUI_LANGUAGE "English"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${OUTPUT_DIR}\ReggaeWave-Setup.exe"
ShowInstDetails show
ShowUnInstDetails show

Function .onInit
  !insertmacro MULTIUSER_INIT
FunctionEnd

Function un.onInit
  !insertmacro MULTIUSER_UNINIT
FunctionEnd

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer

  File "/oname=ReggaeWave.exe" "${EXE_SOURCE}"

  ; Create Shortcuts in Alfazen-Inc folder
  CreateDirectory "$SMPROGRAMS\Alfazen-Inc\ReggaeWave"
  CreateShortcut "$SMPROGRAMS\Alfazen-Inc\ReggaeWave\ReggaeWave.lnk" "$INSTDIR\ReggaeWave.exe"
  CreateShortcut "$SMPROGRAMS\Alfazen-Inc\ReggaeWave\Uninstall ReggaeWave.lnk" "$INSTDIR\uninstall.exe"
  CreateShortcut "$DESKTOP\ReggaeWave.lnk" "$INSTDIR\ReggaeWave.exe"

  ; Write Uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Registry Keys (SHCTX switches automatically between HKLM for All Users and HKCU for Current User)
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\ReggaeWave.exe"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\ReggaeWave.exe"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Section Uninstall
  Delete "$DESKTOP\ReggaeWave.lnk"
  Delete "$SMPROGRAMS\Alfazen-Inc\ReggaeWave\ReggaeWave.lnk"
  Delete "$SMPROGRAMS\Alfazen-Inc\ReggaeWave\Uninstall ReggaeWave.lnk"
  RMDir "$SMPROGRAMS\Alfazen-Inc\ReggaeWave"
  RMDir "$SMPROGRAMS\Alfazen-Inc"

  Delete "$INSTDIR\ReggaeWave.exe"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"
  RMDir "$INSTDIR\.."

  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY}"
  DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
