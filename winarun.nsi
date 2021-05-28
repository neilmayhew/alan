# WinArun installed script using NSIS
#

Unicode True

OutFile "winarun<BUILD>.<PLATFORM>.setup.exe"

;-------------------------------------------------------------------------------
; Includes
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "WinVer.nsh"
!include "x64.nsh"

;-------------------------------------------------------------------------------
; Constants
!define PRODUCT_NAME "Alan Interpreter for Windows"
!define PRODUCT_DESCRIPTION "Alan V3 Interactive Fiction Interpreter <BUILD>"
!define PRODUCT_VERSION 3.0.0.8 ; Numeric Status (https://en.wikipedia.org/wiki/Software_versioning)
!define COMPANY "AlanIF Adventure Factories"

;-------------------------------------------------------------------------------
; Attributes
Name "${PRODUCT_NAME}"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKCU "Software\AlanIF\${PRODUCT_NAME}" ""
;RequestExecutionLevel user ; user|highest|admin

;-------------------------------------------------------------------------------
; Version Info
VIProductVersion "${PRODUCT_VERSION}"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "FileDescription" "${PRODUCT_DESCRIPTION}"
VIAddVersionKey "LegalCopyright" "${COPYRIGHT}"
VIAddVersionKey "FileVersion" "${SETUP_VERSION}"

; NOTE: All BMP's need to be in older Windows 3.x bitmap format!!!
; One converter that generates that is https://www.coolutils.com/online/Image-Converter
;-------------------------------------------------------------------------------
; Modern UI Appearance
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "images\nsis_setup_header.bmp" ;;"${NSISDIR}\Contrib\Graphics\Header\orange.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "images\nsis_setup_wizard.bmp" ;;"${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp!define MUI_HEADERIMAGE_UNBITMAP "images\nsis_setup_header.bmp" ;;"${NSISDIR}\Contrib\Graphics\Header\orange.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "images\nsis_setup_wizard.bmp" ;;"${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
!define MUI_FINISHPAGE_NOAUTOCLOSE

!define MUI_WELCOMEPAGE_TITLE "${PRODUCT_DESCRIPTION}"

;-------------------------------------------------------------------------------
; Installer Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE COPYING.txt
#!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

;-------------------------------------------------------------------------------
; Uninstaller Pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;-------------------------------------------------------------------------------
; Languages
!insertmacro MUI_LANGUAGE "English"

Section "install"

    SetOutPath "$INSTDIR"

    # Files
    File bin\winarun.exe
    File <WINGLK_ROOT>\Glk.dll
    # File <WINGLK_ROOT>\ScaleGfx.dll -- Was removed in 1.51
    File COPYING.txt
    File regression\saviour.a3c
    File regression\saviour.a3r
    File regression\logo.png
    File games\adventv3\adventv3.a3c

    WriteINIStr "$INSTDIR\alanif.url" "InternetShortcut" "URL" "https://www.alanif.se"

    writeUninstaller "$INSTDIR\uninstall.exe"

    WriteRegStr HKCR ".a3c" "Alan V3 Game File" "AlanV3Game"
    WriteRegStr HKCR "AlanV3Game" "" "Alan V3 Game File"
    WriteRegStr HKCR "AlanV3Game\DefaultIcon" "" "$INSTDIR\winarun.exe,0"
    WriteRegStr HKCR "AlanV3Game\shell\open\command" "" '"$INSTDIR\winarun.exe" "%1"'

SectionEnd

Section "startmenu"

    CreateDirectory "$SMPrograms\${PRODUCT_NAME}"
    CreateShortCut "$SMPrograms\${PRODUCT_NAME}\Alan V3 Interpreter.lnk" "$INSTDIR\winarun.exe"
    CreateShortCut "$SMPrograms\${PRODUCT_NAME}\Alan V3 Interactive Fiction System on the web.lnk" "$INSTDIR\alanif.url"
    CreateShortCut "$SMPrograms\${PRODUCT_NAME}\Saviour - a sample game.lnk" "$INSTDIR\saviour.a3c"
    CreateShortCut "$SMPrograms\${PRODUCT_NAME}\Advent - a crude conversion of ADVENT.lnk" "$INSTDIR\adventV3.a3c"
    CreateShortCut "$SMPrograms\${PRODUCT_NAME}\COPYING.lnk" "$INSTDIR\COPYING.txt"
    CreateShortCut "$SMPrograms\${PRODUCT_NAME}\Uninstall Alan V3 Interpreter.lnk" "$INSTDIR\uninstall.exe"

SectionEnd

Section "uninstall"

    Delete "$INSTDIR\uninstall.exe"
    Delete $INSTDIR\winarun.exe
    Delete $INSTDIR\Glk.dll
    # Delete $INSTDIR\ScaleGfx.dll # Removed in WindowsGlk 1.51
    Delete $INSTDIR\COPYING.txt
    Delete $INSTDIR\saviour.a3c
    Delete $INSTDIR\saviour.a3r
    Delete $INSTDIR\logo.png
    Delete $INSTDIR\adventv3.a3c
    RMDir "$INSTDIR"

    Delete "$SMPrograms\${PRODUCT_NAME}\Alan V3 Interpreter.lnk"
    Delete "$SMPrograms\${PRODUCT_NAME}\Alan V3 Interpreter on the Web.lnk"
    Delete "$SMPrograms\${PRODUCT_NAME}\Saviour - a sample game.lnk"
    Delete "$SMPrograms\${PRODUCT_NAME}\Advent - a crude conversion of ADVENT.lnk"
    Delete "$SMPrograms\${PRODUCT_NAME}\COPYING.lnk"
    Delete "$SMPrograms\${PRODUCT_NAME}\Uninstall Alan V3 Interpreter.lnk"
    RMDir "$SMPrograms\${PRODUCT_NAME}"

SectionEnd
