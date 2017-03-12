!define BASE_INSTALL_DIR "$PROGRAMFILES32\Steam\steamapps\common\DOOM 3 BFG Edition"
!define PRODUCT_NAME "Doom 3 BFG VR Fully Possessed"
!define PRODUCT_VERSION 0.021

!define BUILD_DIR "Build\Release"

; HM NIS Edit Wizard helper defines
!define PRODUCT_PUBLISHER "Carl Kenner"
!define PRODUCT_WEB_SITE "https://github.com/KozGit/DOOM-3-BFG-VR/pull/110"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\${PRODUCT_NAME}.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

SetCompressor lzma
SetCompressorDictSize 64

!include "FileFunc.nsh"
!include "LogicLib.nsh"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "neo\sys\win32\rc\res\doom.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; License page
!insertmacro MUI_PAGE_LICENSE "COPYING.txt"
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "Afrikaans"
!insertmacro MUI_LANGUAGE "Albanian"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Basque"
!insertmacro MUI_LANGUAGE "Belarusian"
!insertmacro MUI_LANGUAGE "Bosnian"
!insertmacro MUI_LANGUAGE "Breton"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Estonian"
!insertmacro MUI_LANGUAGE "Farsi"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Galician"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Icelandic"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Irish"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Kurdish"
!insertmacro MUI_LANGUAGE "Latvian"
!insertmacro MUI_LANGUAGE "Lithuanian"
!insertmacro MUI_LANGUAGE "Luxembourgish"
!insertmacro MUI_LANGUAGE "Macedonian"
!insertmacro MUI_LANGUAGE "Malay"
!insertmacro MUI_LANGUAGE "Mongolian"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "NorwegianNynorsk"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Thai"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Uzbek"
!insertmacro MUI_LANGUAGE "Welsh"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME}"
!define UN_NAME "Uninstall $(^Name)"
OutFile "Doom3BFGVR_Fully_Possessed_Alpha021.exe"
InstallDir "${BASE_INSTALL_DIR}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

; Push $filenamestring (e.g. 'c:\this\and\that\filename.htm')
; Push "\"
; Call StrSlash
; Pop $R0
; ;Now $R0 contains 'c:/this/and/that/filename.htm'
Function StrSlash
  Exch $R3 ; $R3 = needle ("\" or "/")
  Exch
  Exch $R1 ; $R1 = String to replacement in (haystack)
  Push $R2 ; Replaced haystack
  Push $R4 ; $R4 = not $R3 ("/" or "\")
  Push $R6
  Push $R7 ; Scratch reg
  StrCpy $R2 ""
  StrLen $R6 $R1
  StrCpy $R4 "\"
  StrCmp $R3 "/" loop
  StrCpy $R4 "/"  
loop:
  StrCpy $R7 $R1 1
  StrCpy $R1 $R1 $R6 1
  StrCmp $R7 $R3 found
  StrCpy $R2 "$R2$R7"
  StrCmp $R1 "" done loop
found:
  StrCpy $R2 "$R2$R4"
  StrCmp $R1 "" done loop
done:
  StrCpy $R3 $R2
  Pop $R7
  Pop $R6
  Pop $R4
  Pop $R2
  Pop $R1
  Exch $R3
FunctionEnd

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
  ReadRegStr $R0 HKCU "Software\Valve\Steam" SteamPath
  IfErrors lbl_error 0
  Push $R0
  Push "/"
  Call StrSlash
  Pop $R0
  StrCpy $INSTDIR "$R0\steamapps\common\DOOM 3 BFG Edition"
  Return
  lbl_error:
    StrCpy $INSTDIR "$PROGRAMFILES32\Steam\steamapps\common\DOOM 3 BFG Edition"
    ClearErrors
FunctionEnd

Section "Base" SEC01
  SectionIn RO
  SetShellVarContext all
  SetOutPath "$INSTDIR"
  ; Sometimes we want to overwrite newer files if they're the wrong bitdepth, etc.
  SetOverwrite ifdiff
  ; Note: We don't remove any files used by Leyland's version or regular RBDoom
  ; Remove all current and previous versions of our EXE files
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\Doom3BFGVR_FP_OculusNativeAlpha015.exe"
  Delete "$INSTDIR\Doom3BFGVR_FP_OculusNativeAlpha014.exe"
  Delete "$INSTDIR\Doom3BFGVR_FP_OpenVR_Alpha015.exe"
  Delete "$INSTDIR\Doom3BFGVR_FP_OpenVR_Alpha014.exe"
  Delete "$INSTDIR\Doom3BFGVR_Oculus.exe"
  Delete "$INSTDIR\Doom3BFGVR_OpenVR.exe"
  Delete "$INSTDIR\Doom3BFGVR.exe"
  ; Remove our avcodec DLLS (which are different from Leyland's and RBDoom)
  Delete "$INSTDIR\avcodec-55.dll"
  Delete "$INSTDIR\avdevice-55.dll"
  Delete "$INSTDIR\avfilter-4.dll"
  Delete "$INSTDIR\avformat-55.dll"
  Delete "$INSTDIR\avutil-52.dll"
  Delete "$INSTDIR\postproc-52.dll"
  Delete "$INSTDIR\swscale-2.dll"
  ; swresample-2.dll is shared with other mods so don't delete it if they're present
  IfFileExists "$INSTDIR\avcodec-57.dll" skipLeyland 0
    Delete "$INSTDIR\swresample-2.dll"
  skipLeyland:
  ; Remove our sixense DLLs (which Leyland and RBDoom don't use)
  Delete "$INSTDIR\sixense.dll"
  Delete "$INSTDIR\sixense_utils.dll"
  ; Remove our Readme.txt file (but are we sure it's ours?)
  Delete "$INSTDIR\Readme.txt"
  Delete "$INSTDIR\Readme_install.txt"
  Delete "$INSTDIR\cvars.pdf"
  ; Remove our old defs
  Delete "$INSTDIR\base\def\aas.def"
  Delete "$INSTDIR\base\def\d3le-player.def"
  Delete "$INSTDIR\base\def\d3xp-player.def"
  Delete "$INSTDIR\base\def\headingbeam.def"
  Delete "$INSTDIR\base\def\player.def"
  Delete "$INSTDIR\base\def\weapon_bfg.def"
  Delete "$INSTDIR\base\def\weapon_bloodstone_active.def"
  Delete "$INSTDIR\base\def\weapon_bloodstone_passive.def"
  Delete "$INSTDIR\base\def\weapon_chaingun.def"
  Delete "$INSTDIR\base\def\weapon_chainsaw.def"
  Delete "$INSTDIR\base\def\weapon_fists.def"
  Delete "$INSTDIR\base\def\weapon_flashlight.def"
  Delete "$INSTDIR\base\def\weapon_flashlight_new.def"
  Delete "$INSTDIR\base\def\weapon_grabber.def"
  Delete "$INSTDIR\base\def\weapon_handgrenade.def"
  Delete "$INSTDIR\base\def\weapon_machinegun.def"
  Delete "$INSTDIR\base\def\weapon_pda.def"
  Delete "$INSTDIR\base\def\weapon_pistol.def"
  Delete "$INSTDIR\base\def\weapon_plasmagun.def"
  Delete "$INSTDIR\base\def\weapon_rocketlauncher.def"
  Delete "$INSTDIR\base\def\weapon_shotgun.def"
  Delete "$INSTDIR\base\def\weapon_shotgun_double.def"
  Delete "$INSTDIR\base\def\weapon_soulcube.def"
  RMDir "$INSTDIR\base\def"
  ; Remove our guis
  Delete "$INSTDIR\base\guis\lookforward.tga"
  Delete "$INSTDIR\base\guis\powercellgenerator.gui"
  Delete "$INSTDIR\base\guis\cranegame\crane_control.gui"
  Delete "$INSTDIR\base\guis\weapons\vrstatgui.gui"
  RMDir /r "$INSTDIR\base\guis\assets\statwatch"
  RMDir "$INSTDIR\base\guis\assets"
  RMDir "$INSTDIR\base\guis\cranegame"
  RMDir "$INSTDIR\base\guis\weapons"
  RMDir "$INSTDIR\base\guis"
  Delete "$INSTDIR\base\generated\images\guis\lookforward*.bimage"
  RMDir /r "$INSTDIR\base\generated\images\guis\assets\statwatch"
  RMDir "$INSTDIR\base\generated\images\guis"
  ; Remove our materials
  Delete "$INSTDIR\base\materials\characters.mtr"
  Delete "$INSTDIR\base\materials\patd.mtr"
  Delete "$INSTDIR\base\materials\vr.mtr"
  Delete "$INSTDIR\base\materials\vr_characters.mtr"
  Delete "$INSTDIR\base\materials\weapons.mtr"
  RMDir "$INSTDIR\base\materials"
  ; Remove our character models
  Delete "$INSTDIR\base\models\characters\male_npc\marine\marine_arm2.tga"
  Delete "$INSTDIR\base\models\characters\male_npc\marine\marine_arm2_h.tga"
  Delete "$INSTDIR\base\models\characters\male_npc\marine\marine_arm2_local.tga"
  Delete "$INSTDIR\base\models\characters\male_npc\marine\marine_arm2_s.tga"
  RMDir "$INSTDIR\base\models\characters\male_npc\marine"
  RMDir /r "$INSTDIR\base\generated\images\models\characters\male_npc\marine"
  Delete "$INSTDIR\base\models\characters\male_npc\soldier\soldier_arm2.tga"
  Delete "$INSTDIR\base\models\characters\male_npc\soldier\soldier_arm2_h.tga"
  Delete "$INSTDIR\base\models\characters\male_npc\soldier\soldier_arm2_local.tga"
  Delete "$INSTDIR\base\models\characters\male_npc\soldier\soldier_arm2_s.tga"
  RMDir "$INSTDIR\base\models\characters\male_npc\soldier"
  RMDir "$INSTDIR\base\models\characters\male_npc"
  RMDir /r "$INSTDIR\base\generated\images\models\characters\male_npc\soldier"
  RMDir "$INSTDIR\base\generated\images\models\characters\male_npc"
  Delete "$INSTDIR\base\models\characters\player_character\body\body_arm2_s.tga"
  Delete "$INSTDIR\base\models\characters\player_character\body\body_codpiecefix_arm2_local.tga"
  Delete "$INSTDIR\base\models\characters\player_character\body\body_codpiecefix_local_scaled.tga"
  Delete "$INSTDIR\base\models\characters\player_character\body\body2_arm2.tga"
  RMDir "$INSTDIR\base\models\characters\player_character\body"
  RMDir "$INSTDIR\base\models\characters\player_character"
  RMDir "$INSTDIR\base\models\characters"
  RMDir /r "$INSTDIR\base\generated\images\models\characters\player_character\body"
  RMDir "$INSTDIR\base\generated\images\models\characters\player_character"
  RMDir "$INSTDIR\base\generated\images\models\characters"
  RMDir "$INSTDIR\base\generated\images\models"
  Delete "$INSTDIR\base\models\md5\characters\npcs\playermoves\*_vrik*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\characters\npcs\playermoves\*_vrik*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\characters\npcs\playermoves\*_vrik*.*"
  RMDir "$INSTDIR\base\models\md5\characters\npcs\playermoves"
  RMDir "$INSTDIR\base\models\md5\characters\npcs"
  RMDir "$INSTDIR\base\generated\anim\models\md5\characters\npcs\playermoves"
  RMDir "$INSTDIR\base\generated\anim\models\md5\characters\npcs"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\characters\npcs\playermoves"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\characters\npcs"
  Delete "$INSTDIR\base\models\md5\characters\player\*_vrik*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\characters\player\*_vrik*.*"
  RMDir "$INSTDIR\base\models\md5\characters\player"
  RMDir "$INSTDIR\base\models\md5\characters"
  RMDir "$INSTDIR\base\generated\anim\models\md5\characters\player"
  RMDir "$INSTDIR\base\generated\anim\models\md5\characters"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\characters\player"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\characters"
  Delete "$INSTDIR\base\models\md5\player\fists_idle.md5anim"
  RMDir "$INSTDIR\base\models\md5\player"
  RMDir /r "$INSTDIR\base\generated\images\models\characters\male_npc"
  RMDir /r "$INSTDIR\base\generated\images\models\characters\player_character"
  RMDir /r "$INSTDIR\base\generated\images\addnormals\models\characters\male_npc"
  RMDir /r "$INSTDIR\base\generated\images\addnormals\models\characters\player_character"
  RMDir "$INSTDIR\base\generated\images\models\characters"
  RMDir "$INSTDIR\base\generated\images\models"
  RMDir "$INSTDIR\base\generated\images\addnormals\models\characters"
  RMDir "$INSTDIR\base\generated\images\addnormals\models"
  RMDir "$INSTDIR\base\generated\images\addnormals"
  ; Remove our headingbeam models
  Delete "$INSTDIR\base\models\images\headingbeamarrow.tga"
  Delete "$INSTDIR\base\generated\images\models\images\headingbeamarrow*.bimage"
  Delete "$INSTDIR\base\models\images\headingbeamsolid.tga"
  Delete "$INSTDIR\base\generated\images\models\images\headingbeamsolid*.bimage"
  Delete "$INSTDIR\base\models\images\lasercircledot.tga"
  Delete "$INSTDIR\base\generated\images\models\images\lasercircledot*.bimage"
  Delete "$INSTDIR\base\models\images\lasercrosshair.tga"
  Delete "$INSTDIR\base\generated\images\models\images\lasercrosshair*.bimage"
  Delete "$INSTDIR\base\models\images\laserdot.tga"
  Delete "$INSTDIR\base\generated\images\models\images\laserdot*.bimage"
  Delete "$INSTDIR\base\models\images\laserdot2.tga"
  Delete "$INSTDIR\base\generated\images\models\images\laserdot2*.bimage"
  Delete "$INSTDIR\base\models\images\telepad*.tga"
  Delete "$INSTDIR\base\generated\images\models\images\telepad*.bimage"
  Delete "$INSTDIR\base\models\images\teleportbeam*.tga"
  Delete "$INSTDIR\base\models\images\teleporttarget*.tga"
  Delete "$INSTDIR\base\generated\images\models\images\teleport*.bimage"
  RMDir "$INSTDIR\base\models\images"
  RMDir "$INSTDIR\base\generated\images\models\images"
  RMDir "$INSTDIR\base\generated\images\models"
  RMDir "$INSTDIR\base\generated\images"
  Delete "$INSTDIR\base\models\mapobjects\headingbeam.lwo"
  Delete "$INSTDIR\base\generated\rendermodels\models\mapobjects\headingbeam.*"
  Delete "$INSTDIR\base\models\mapobjects\hud.lwo"
  Delete "$INSTDIR\base\generated\rendermodels\models\mapobjects\hud.lwo"
  Delete "$INSTDIR\base\models\mapobjects\weaponsight.lwo"
  Delete "$INSTDIR\base\generated\rendermodels\models\mapobjects\weaponsight.*"
  Delete "$INSTDIR\base\models\mapobjects\filler\cola1.lwo"
  Delete "$INSTDIR\base\generated\rendermodels\models\mapobjects\filler\cola1.*"
  Delete "$INSTDIR\base\generated\collision\models\mapobjects\filler\cola1.bcmodel"
  Delete "$INSTDIR\base\models\mapobjects\filler\colo1.lwo"
  Delete "$INSTDIR\base\generated\rendermodels\models\mapobjects\filler\colo1.lwo"
  Delete "$INSTDIR\base\generated\collision\models\mapobjects\filler\colo1.bcmodel"
  RMDir "$INSTDIR\base\models\mapobjects\filler"
  RMDir "$INSTDIR\base\generated\rendermodels\models\mapobjects\filler"
  RMDir "$INSTDIR\base\models\mapobjects"
  RMDir "$INSTDIR\base\genderated\rendermodels\models\mapobjects"
  RMDir "$INSTDIR\base\generated\collision\models\mapobjects\filler"
  RMDir "$INSTDIR\base\genderated\collision\models\mapobjects"
  RMDir "$INSTDIR\base\genderated\collision\models"
  RMDir "$INSTDIR\base\genderated\collision"
  ; Remove our item models
  Delete "$INSTDIR\base\models\md5\items\flashlight_world\*_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\items\flashlight_world\*_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\items\flashlight_world\*_vr*.*"
  RMDir "$INSTDIR\base\models\md5\items\flashlight_world"
  RMDir "$INSTDIR\base\generated\anim\models\md5\items\flashlight_world"
  RMDir "$INSTDIR\base\generated\anim\rendermodels\md5\items\flashlight_world"
  Delete "$INSTDIR\base\models\md5\items\pda_view\*_vr_*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\items\pda_view\*_vr_*.*"
  Delete "$INSTDIR\base\generated\anim\rendermodels\md5\items\pda_view\*_vr_*.*"
  RMDir /r "$INSTDIR\base\models\md5\items\pda_view\backup scaled pda"
  RMDir /r "$INSTDIR\base\generated\anim\models\md5\items\pda_view\backup scaled pda"
  RMDir /r "$INSTDIR\base\generated\rendermodels\models\md5\items\pda_view\backup scaled pda"
  RMDir "$INSTDIR\base\models\md5\items\pda_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\items\pda_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\items\pda_view"
  RMDir /r "$INSTDIR\base\models\md5\items\telepad"
  RMDir /r "$INSTDIR\base\generated\anim\models\md5\items\telepad"
  RMDir /r "$INSTDIR\base\generated\rendermodels\models\md5\items\telepad"
  RMDir "$INSTDIR\base\models\md5\items"
  RMDir "$INSTDIR\base\generated\anim\models\md5\items"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\items"
  ; Remove our weapon models
  RMDir /r "$INSTDIR\base\models\md5\old weapons with hand meshes"
  Delete "$INSTDIR\base\models\md5\weapons\bfg_view\viewbfg_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\bfg_view\viewbfg_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\bfg_view\viewbfg_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\bfg_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\bfg_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\bfg_view"
  Delete "$INSTDIR\base\models\md5\weapons\blood_orb_view\new\bloodorb_vr*.*"
  Delete "$INSTDIR\base\models\md5\weapons\blood_orb_view\new\fingersonly.md5anim"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\blood_orb_view\new\bloodorb_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\blood_orb_view\new\fingersonly.md5anim"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\blood_orb_view\new\bloodorb_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\blood_orb_view\new\fingersonly.md5anim"
  RMDir "$INSTDIR\base\models\md5\weapons\blood_orb_view\new"
  RMDir "$INSTDIR\base\models\md5\weapons\blood_orb_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\blood_orb_view\new"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\blood_orb_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\blood_orb_view\new"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\blood_orb_view"
  Delete "$INSTDIR\base\models\md5\weapons\chaingun_view\viewchaingun_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\chaingun_view\viewchaingun_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\chaingun_view\viewchaingun_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\chaingun_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\chaingun_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\chaingun_view"
  Delete "$INSTDIR\base\models\md5\weapons\chainsaw_view\viewchainsaw_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\chainsaw_view\viewchainsaw_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\chainsaw_view\viewchainsaw_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\chainsaw_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\chainsaw_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\chainsaw_view"
  Delete "$INSTDIR\base\models\md5\weapons\doublebarrel_view\new\doublebarrel_view_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\doublebarrel_view\new\doublebarrel_view_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\doublebarrel_view\new\doublebarrel_view_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\doublebarrel_view\new"
  RMDir "$INSTDIR\base\models\md5\weapons\doublebarrel_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\doublebarrel_view\new"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\doublebarrel_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\doublebarrel_view\new"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\doublebarrel_view"
  Delete "$INSTDIR\base\models\md5\weapons\fists_view\fists_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\fists_view\fists_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\fists_view\fists_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\fists_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\fists_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\fists_view"
  Delete "$INSTDIR\base\models\md5\weapons\grabber_view\grabber_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\grabber_view\grabber_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\grabber_view\grabber_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\grabber_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\grabber_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\grabber_view"
  Delete "$INSTDIR\base\models\md5\weapons\grenade_view\viewgrenade_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\grenade_view\viewgrenade_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\grenade_view\viewgrenade_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\grenade_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\grenade_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\grenade_view"
  Delete "$INSTDIR\base\models\md5\weapons\machinegun_view\viewmachinegun_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\machinegun_view\viewmachinegun_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\machinegun_view\viewmachinegun_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\machinegun_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\machinegun_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\machinegun_view"
  Delete "$INSTDIR\base\models\md5\weapons\pistol_view\viewpistol_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\pistol_view\viewpistol_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\pistol_view\viewpistol_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\pistol_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\pistol_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\pistol_view"
  Delete "$INSTDIR\base\models\md5\weapons\plasmagun_view\viewplasmagun_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\plasmagun_view\viewplasmagun_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\plasmagun_view\viewplasmagun_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\plasmagun_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\plasmagun_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\plasmagun_view"
  Delete "$INSTDIR\base\models\md5\weapons\rocketlauncher_view\viewrocketlauncer_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\rocketlauncher_view\viewrocketlauncer_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\rocketlauncher_view\viewrocketlauncer_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\rocketlauncher_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\rocketlauncher_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\rocketlauncher_view"
  Delete "$INSTDIR\base\models\md5\weapons\shotgun_view\viewshotgun_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\shotgun_view\viewshotgun_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\shotgun_view\viewshotgun_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\shotgun_view"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons\shotgun_view"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons\shotgun_view"
  Delete "$INSTDIR\base\models\md5\weapons\soulcube_view\soulcube_vr*.*"
  Delete "$INSTDIR\base\generated\anim\models\md5\weapons\soulcube_view\soulcube_vr*.*"
  Delete "$INSTDIR\base\generated\rendermodels\models\md5\weapons\soulcube_view\soulcube_vr*.*"
  RMDir "$INSTDIR\base\models\md5\weapons\soulcube_view"
  RMDir "$INSTDIR\base\models\md5\weapons"
  RMDir "$INSTDIR\base\models\md5"
  RMDir "$INSTDIR\base\models"
  RMDir "$INSTDIR\base\generated\anim\models\md5\weapons"
  RMDir "$INSTDIR\base\generated\anim\models\md5"
  RMDir "$INSTDIR\base\generated\anim\models"
  RMDir "$INSTDIR\base\generated\anim"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5\weapons"
  RMDir "$INSTDIR\base\generated\rendermodels\models\md5"
  RMDir "$INSTDIR\base\generated\rendermodels\models"
  RMDir "$INSTDIR\base\generated\rendermodels"
  RMDir "$INSTDIR\base\generated"
  ; Remove our map Player AAS files
  Delete "$INSTDIR\base\maps\game\mp\*.aas_player"
  RMDIR "$INSTDIR\base\maps\game\mp"
  Delete "$INSTDIR\base\maps\game\*.aas_player"
  RMDIR "$INSTDIR\base\maps\game"
  RMDIR "$INSTDIR\base\maps"
  ; Remove our scripts
  RMDir /r "$INSTDIR\base\all_in_one_script"
  RMDir /r "$INSTDIR\base\script\all_in_one_script"
  Delete "$INSTDIR\base\script\ai_player.script"
  Delete "$INSTDIR\base\script\doom_defs.script"
  Delete "$INSTDIR\base\script\doom_events.script"
  Delete "$INSTDIR\base\script\weapon_base.script"
  Delete "$INSTDIR\base\script\weapon_bfg.script"
  Delete "$INSTDIR\base\script\weapon_bloodstone_active1.script"
  Delete "$INSTDIR\base\script\weapon_bloodstone_active2.script"
  Delete "$INSTDIR\base\script\weapon_bloodstone_active3.script"
  Delete "$INSTDIR\base\script\weapon_bloodstone_passive.script"
  Delete "$INSTDIR\base\script\weapon_chainsaw.script"
  Delete "$INSTDIR\base\script\weapon_flashlight.script"
  Delete "$INSTDIR\base\script\weapon_grabber.script"
  Delete "$INSTDIR\base\script\weapon_handgrenade.script"
  Delete "$INSTDIR\base\script\weapon_pistol.script"
  Delete "$INSTDIR\base\script\weapon_plasmagun.script"
  Delete "$INSTDIR\base\script\weapon_rocketlauncher.script"
  Delete "$INSTDIR\base\script\weapon_shotgun.script"
  Delete "$INSTDIR\base\script\weapon_soulcube.script"
  RMDir "$INSTDIR\base\script"
  ; Remove our particles
  Delete "$INSTDIR\base\particles\grabber.prt"
  RMDir "$INSTDIR\base\particles"
  ; Remove our skins
  Delete "$INSTDIR\base\skins\skins_characters_player.skin"
  Delete "$INSTDIR\base\skins\skins_models_weapons.skin"
  RMDir "$INSTDIR\base\skins"
  ; We can't really remove our strings\english.lang, because they're probably using it
  ; maybe if we check for other .lang files, and delete english.lang if they're present
  ; Remove our default config files
  Delete "$INSTDIR\base\vr_default.cfg"
  Delete "$INSTDIR\base\vr_oculus_default.cfg"
  Delete "$INSTDIR\base\vr_openvr_default.cfg"
  
  File "${BUILD_DIR}\Doom3BFGVR.exe"
  File "${BUILD_DIR}\..\..\README.txt"
  File "${BUILD_DIR}\..\..\COPYING.txt"
  File "${BUILD_DIR}\avcodec-55.dll"
  File "${BUILD_DIR}\avdevice-55.dll"
  File "${BUILD_DIR}\avfilter-4.dll"
  File "${BUILD_DIR}\avformat-55.dll"
  File "${BUILD_DIR}\avutil-52.dll"
  File "${BUILD_DIR}\postproc-52.dll"
  File "${BUILD_DIR}\swresample-2.dll"
  File "${BUILD_DIR}\swscale-2.dll"
  File "${BUILD_DIR}\OpenAL*.dll"
  File "${BUILD_DIR}\openvr_api.dll"
  File /r /x *.aas_player "${BUILD_DIR}\..\..\vr_assets\Fully Possessed"
  
  ; This needs to be done after Doom3BFGVR.exe is copied
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\Doom3BFGVR.exe"
  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\Doom3BFGVR.exe"
  
  ; Fix language detection by deleting incorrect .lang files
  ${If} ${FileExists} "$INSTDIR\base\_sound_pc_jp.resources"
  ${AndIf} ${FileExists} "$INSTDIR\base\strings\japanese.lang"
    Delete "$INSTDIR\base\strings\german.lang"
    Delete "$INSTDIR\base\strings\french.lang"
    Delete "$INSTDIR\base\strings\italian.lang"
    Delete "$INSTDIR\base\strings\spanish.lang"
    Delete "$INSTDIR\base\strings\english.lang"
  ${EndIf}
  ${If} ${FileExists} "$INSTDIR\base\_sound_pc_fr.resources"
  ${AndIf} ${FileExists} "$INSTDIR\base\strings\french.lang"
    Delete "$INSTDIR\base\strings\german.lang"
    Delete "$INSTDIR\base\strings\english.lang"
    Delete "$INSTDIR\base\strings\italian.lang"
    Delete "$INSTDIR\base\strings\spanish.lang"
    Delete "$INSTDIR\base\strings\japanese.lang"
  ${EndIf}
  ${If} ${FileExists} "$INSTDIR\base\_sound_pc_gr.resources"
  ${AndIf} ${FileExists} "$INSTDIR\base\strings\german.lang"
    Delete "$INSTDIR\base\strings\english.lang"
    Delete "$INSTDIR\base\strings\french.lang"
    Delete "$INSTDIR\base\strings\italian.lang"
    Delete "$INSTDIR\base\strings\spanish.lang"
    Delete "$INSTDIR\base\strings\japanese.lang"
  ${EndIf}
  ${If} ${FileExists} "$INSTDIR\base\_sound_pc_it.resources"
  ${AndIf} ${FileExists} "$INSTDIR\base\strings\italian.lang"
    Delete "$INSTDIR\base\strings\german.lang"
    Delete "$INSTDIR\base\strings\french.lang"
    Delete "$INSTDIR\base\strings\english.lang"
    Delete "$INSTDIR\base\strings\spanish.lang"
    Delete "$INSTDIR\base\strings\japanese.lang"
  ${EndIf}
  ${If} ${FileExists} "$INSTDIR\base\_sound_pc_sp.resources"
  ${AndIf} ${FileExists} "$INSTDIR\base\strings\spanish.lang"
    Delete "$INSTDIR\base\strings\german.lang"
    Delete "$INSTDIR\base\strings\french.lang"
    Delete "$INSTDIR\base\strings\italian.lang"
    Delete "$INSTDIR\base\strings\english.lang"
    Delete "$INSTDIR\base\strings\japanese.lang"
  ${EndIf}
  ${If} ${FileExists} "$INSTDIR\base\_sound_pc_en.resources"
  ${AndIf} ${FileExists} "$INSTDIR\base\strings\english.lang"
    Delete "$INSTDIR\base\strings\german.lang"
    Delete "$INSTDIR\base\strings\french.lang"
    Delete "$INSTDIR\base\strings\italian.lang"
    Delete "$INSTDIR\base\strings\spanish.lang"
    Delete "$INSTDIR\base\strings\japanese.lang"
  ${EndIf}
  
  ; Copy saved games if they don't already exist in our new folder
  CreateDirectory "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\DOOM\*.dsg" skipdoom1 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\DOOM" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed"
  skipdoom1:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\DOOM2\*.dsg" skipdoom2 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\DOOM2" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed"
  skipdoom2:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-autosave\gamedata.save" skipauto 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-autosave" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skipauto:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-quick\gamedata.save" skipquick 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-quick" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skipquick:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_0\gamedata.save" skip0 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_0" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip0:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_1\gamedata.save" skip1 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_1" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip1:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_2\gamedata.save" skip2 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_2" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip2:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_3\gamedata.save" skip3 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_3" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip3:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_4\gamedata.save" skip4 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_4" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip4:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_5\gamedata.save" skip5 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_5" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip5:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_6\gamedata.save" skip6 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_6" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip6:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_7\gamedata.save" skip7 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_7" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip7:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_8\gamedata.save" skip8 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_8" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip8:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_9\gamedata.save" skip9 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_9" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip9:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_10\gamedata.save" skip10 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_10" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip10:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_11\gamedata.save" skip11 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_11" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip11:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_12\gamedata.save" skip12 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_12" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip12:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_13\gamedata.save" skip13 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_13" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip13:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_14\gamedata.save" skip14 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_14" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip14:
  IfFileExists "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame\GAME-doom3_15\gamedata.save" skip15 0
    CopyFiles "$PROFILE\Saved Games\id Software\DOOM 3 BFG VR\base\savegame\GAME-doom3_15" "$PROFILE\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\savegame"
  skip15:
  
  SetOutPath "$TEMP"
  SetOverwrite on
SectionEnd

Section "Player AAS (teleport navigation files)" SEC02
  SetShellVarContext all
  SetOutPath "$INSTDIR\Fully Possessed"
  SetOverwrite ifnewer
  File /r "${BUILD_DIR}\..\..\vr_assets\Fully Possessed\maps"
  
  SetOutPath "$TEMP"
  SetOverwrite on
SectionEnd

Section -AdditionalIcons
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${UN_NAME}.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\Dolphin.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Doom3BFGVR.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  ${GetSize} "$INSTDIR\Fully Possessed" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "EstimatedSize" "$0"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Comments" "Virtual Reality mod for Doom 3 BFG"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Installs all files required to run Doom 3 BFG VR Fully Possessed."
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "Highly recommended. Player AAS navigation files make teleporting much better, but take up 1.2GB. Without them you can only teleport places a Zombie could go."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section Uninstall
  SetShellVarContext all
  ; Only uninstall what we put there
  ; Note: We don't remove any files used by Leyland's version or regular RBDoom
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\Doom3BFGVR.exe"
  ; Remove our avcodec DLLS (which are different from Leyland's and RBDoom)
  Delete "$INSTDIR\avcodec-55.dll"
  Delete "$INSTDIR\avdevice-55.dll"
  Delete "$INSTDIR\avfilter-4.dll"
  Delete "$INSTDIR\avformat-55.dll"
  Delete "$INSTDIR\avutil-52.dll"
  Delete "$INSTDIR\postproc-52.dll"
  Delete "$INSTDIR\swscale-2.dll"
  ; swresample-2.dll is shared with other mods so don't delete it if they're present
  IfFileExists "$INSTDIR\avcodec-57.dll" skipLeyland2 0
    Delete "$INSTDIR\swresample-2.dll"
  skipLeyland2:
  ; Remove our Readme.txt file (but are we sure it's ours?)
  Delete "$INSTDIR\README.txt"
  ; Remove our mod folder
  RMDir /r "$INSTDIR\Fully Possessed"
  ; Don't remove COPYING.txt, OpenAL*.dll, or openvr_api.dll because they're probably used by others
  IfFileExists "$INSTDIR\*.exe" skip3Files 0
    Delete $INSTDIR\COPYING.txt
    Delete $INSTDIR\openvr_api.dll
    Delete $INSTDIR\OpenAL*.dll
  skip3Files:
  
  ; Remove shortcuts
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\${UN_NAME}.lnk"
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"

  RMDir "$SMPROGRAMS\${PRODUCT_NAME}"
  ; If we installed to an empty folder, we can delete the folder when done
  ; (RMDir only deletes EMPTY folders, so this is safe)
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
