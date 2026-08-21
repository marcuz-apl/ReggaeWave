; ReggaeWave Windows Modern NSIS Installer Script
; ------------------------------------------------

!define PRODUCT_NAME "ReggaeWave"
!define PRODUCT_PUBLISHER "Alfazen-Inc"
!define PRODUCT_WEB_SITE "https://github.com/marcuz-apl/ReggaeWave"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\ReggaeWave.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!ifndef PRODUCT_VERSION
  !define PRODUCT_VERSION "1.3.0"
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

; Compressor settings
SetCompressor /SOLID lzma
Unicode true
RequestExecutionLevel admin

; Modern UI
!include "MUI2.nsh"
!include "x64.nsh"

; Interface Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${ICON_FILE}"
!define MUI_UNICON "${ICON_FILE}"

; Pages
!insertmacro MUI_PAGE_WELCOME
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
InstallDir "$PROGRAMFILES64\ReggaeWave"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer

  File "/oname=ReggaeWave.exe" "${EXE_SOURCE}"

  ; Create Shortcuts
  CreateDirectory "$SMPROGRAMS\ReggaeWave"
  CreateShortcut "$SMPROGRAMS\ReggaeWave\ReggaeWave.lnk" "$INSTDIR\ReggaeWave.exe"
  CreateShortcut "$SMPROGRAMS\ReggaeWave\Uninstall ReggaeWave.lnk" "$INSTDIR\uninstall.exe"
  CreateShortcut "$DESKTOP\ReggaeWave.lnk" "$INSTDIR\ReggaeWave.exe"

  ; Write Uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Registry Keys
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\ReggaeWave.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\ReggaeWave.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Section Uninstall
  Delete "$DESKTOP\ReggaeWave.lnk"
  Delete "$SMPROGRAMS\ReggaeWave\ReggaeWave.lnk"
  Delete "$SMPROGRAMS\ReggaeWave\Uninstall ReggaeWave.lnk"
  RMDir "$SMPROGRAMS\ReggaeWave"

  Delete "$INSTDIR\ReggaeWave.exe"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
