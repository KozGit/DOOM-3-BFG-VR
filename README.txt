    ____                           _____  ____   ______ ______ _    __ ____   
   / __ \ ____   ____   ____ ___  |__  / / __ ) / ____// ____/| |  / // __ \ 
  / / / // __ \ / __ \ / __ `__ \  /_ < / __  |/ /_   / / __  | | / // /_/ /
 / /_/ // /_/ // /_/ // / / / / /___/ // /_/ // __/  / /_/ /  | |/ // _, _/
/_____/ \____/ \____//_/ /_/ /_//____//_____//_/     \____/   |___//_/ |_|

							Fully Possessed
____________________________________________________________________________



DOOM3-BFG VR : Fully Possessed
Includes support for the HTC Vive, Oculus Rift and Touch motion controls.

VR Implementation: /u/Samson-
Major Contributions - Teleportation, Voice Commands, Flicksync, Bink videos, code improvements: Carl Kenner
Holster slots, Crawl Space Head Collision, minor fixes: Leyland
Slow Mo and Tunnel vision motion sickness fixes: jckhng
Originally inspired by: tmek

DOOM-3-BFG-VR Readme - https://github.com/KozGit/DOOM-3-BFG-VR

Please note this readme has not been fully updated for the VR port yet.  See readme in binary download for more information.
Readme will be updated with VR specific documentation shortly.

DOOM-3-BFG-VR was built using an older version of the RBDOOM3-BFG port.
RBDOOM-3-BFG Readme - https://github.com/RobertBeckebans/DOOM-3-BFG


_______________________________________

CONTENTS
_______________________________



This file contains the following sections:

	1) SYSTEM REQUIREMENT

	2) GENERAL NOTES
	
	3) INSTALLATION, GETTING THE GAMEDATA, RUNNING THE GAME
	
	4) INSTRUCTIONS FOR NEW FEATURES
	
	5) CHANGES
	
	6) CONSOLE VARIABLES
	
	7) KNOWN ISSUES
	
	8) GETTING THE SOURCE CODE

	9) COMPILING ON WIN32 WITH VISUAL C++ 2013 EXPRESS EDITION

	10) COMPILING ON GNU/LINUX
	
	11) BUG REPORTS
	
	12) LICENSE
	
	13) CODE LICENSE EXCEPTIONS



___________________________________

1) SYSTEM REQUIREMENTS
__________________________



Minimum system requirements:

	CPU: 3 GHz + Intel compatible
	System Memory: 8 GB
	Graphics card: Geforce GTX 960 or higher, with recent drivers
	OS: Windows 7 64 bit or above?
	VR: SteamVR compatible HMD or Oculus Rift DK2 or above

Recommended system requirements:

	CPU: 3 GHz + Intel compatible
	System Memory: 8 GB
	Graphics card: Geforce GTX 970 or higher, with latest drivers
	OS: Windows 10 64 bit
	VR: HTC Vive or Oculus Rift CV1 with Touch and roomscale sensors




_______________________________

2) GENERAL NOTES
______________________

This release does not contain any game data, the game data is still
covered by the original EULA and must be obeyed as usual.

Only Doom 3 BFG Edition is supported. Regular Doom 3 is not supported yet.

You should patch the game to the latest version. If using Steam, this will happen automatically.

Note that Doom 3 BFG Edition is available from the Steam store at
http://store.steampowered.com/app/208200/


Steam:
------
The Doom 3 BFG Edition GPL Source Code release does not include functionality for integrating with 
Steam.  This includes roaming profiles, achievements, leaderboards, matchmaking, the overlay, or
any other Steam features.


Bink:
-----
The Doom3BFGVR Edition GPL Source Code release includes functionality for rendering Bink Videos through FFmpeg.


Back End Rendering of Stencil Shadows:
--------------------------------------

The Doom 3 BFG Edition GPL Source Code release does not include functionality enabling rendering
of stencil shadows via the "depth fail" method, a functionality commonly known as "Carmack's Reverse".


Mods:
-----

The Doom 3 BFG Edition GPL Source Code release allow mod editing, in order for it to accept any change in your
mod directory, you should first specify your mod directory adding the following command to the launcher:

"+set fs_game_base modDirectoryName"

so it would end up looking like: Doom3BFGVR +set fs_game_base mymod

Regular Doom 3 (non-BFG) mods are generally not supported.

___________________________________________________

3) INSTALLATION, GETTING THE GAMEDATA, RUNNING THE GAME
__________________________________________

Make sure you have Doom 3 BFG installed in Steam.

Set the Doom 3 BFG language in Steam to whatever language you want.

In the unlikely event that you don't have the Visual C++ 2013 redistributable installed, get it here:
	https://www.microsoft.com/en-au/download/details.aspx?id=40784

The installer will automatically remove the old version of our VR mod if present, so don't worry about that.
But if you have any other incompatible mods installed, you should remove them either before or after running the installer.
(The installer doesn't add any files to the base folder, but it will remove files with the same names as our files,
mostly weapons and player models, possibly breaking other mods for the regular game.)

Start the Doom3BFGVR_Alpha020.exe installer.
If you didn't install Doom 3 BFG on your main Steam hard-drive, change the install path.
Select to install both base and Player AAS, if you have enough disk space. Otherwise just install base.
It will create a shortcut on your desktop and in your program list.

If you want to use speech recognition (recommended), you should set it up in Windows and train it a little.

Run the shortcut on your desktop, or run the Doom3BFGVR.exe file in your Doom 3 BFG folder.



The following instructions are primarily intented for Linux users and all hackers on other operating systems.

To play the game, you need the game data from a legal copy of the game, which 
unfortunately requires Steam for Windows - Steam for Linux or OSX won't do, because
(at least currently) the Doom 3 BFG game is only installable on Steam for Windows.
Even the DVD version of Doom 3 BFG only contains encrytped data that is decoded
by Steam on install.

Fortunately, you can run Steam in Wine to install Doom3 BFG and afterwards copy the 
game data somewhere else to use it with native executables.
Winetricks ( http://winetricks.org/ ) makes installing Windows Steam on Linux really easy.

Anyway:

1. Install Doom 3 BFG in Steam (Windows version), make sure it's getting 
   updated/patched.

2. Create your own Doom 3 BFG directory, e.g. /path/to/Doom3BFG/

3. Copy the game-data's base dir from Steam to that directory 
   (e.g. /path/to/Doom3BFG/), it's in
	/your/path/to/Steam/steamapps/common/DOOM 3 BFG Edition/base/

4. Copy the Doom3BFGVR executable and the FFmpeg DLLs to your own 
   Doom 3 BFG directory (/path/to/Doom3BFG). Copy mod assets to the BASE directory
   
   Your own Doom 3 BFG directory now should look like:
	/path/to/Doom3BFG/
	 ->	Doom3BFGVR (or Doom3BFGVR.exe on Windows)
	 -> openvr_api.dll
	 -> avcodec-55.dll
	 -> avdevice-55.dll
	 -> avfilter-4.dll
	 -> avformat-55.dll
	 -> avutil-52.dll
	 -> postproc-52.dll
	 -> swresample-0.dll
	 -> swscale-2.dll
	 ->	base/
		 ->	classicmusic/
		 ->	_common.crc ( and the rest of the .crc and .resources files )
		 ->	def/
		 -> guis/
		 -> maps/
		 -> materials/
		 -> models/
		 -> particles/
		 -> script/
		 -> skins/
		 -> strings/
		 -> video/
		 -> wad
		 -> vr_default.cfg

5. Run the game by executing the Doom3BFGVR executable.

6. Enjoy

7. If you run into bugs, please report them, see 11)

___________________________________________________

4) INSTRUCTIONS FOR NEW FEATURES
__________________________________________

This game is fully room-scale. It is best played standing in a large space.
You can walk around and crouch in real life.
You can walk up or down stairs or walk off ledges.
You can push small shipping containers, and other items with real life walking.
It won't let you walk through solid objects, but you can lean a bit.
Jumping in real life is not supported.

Default motion controls:
------------------------

Your LEFT hand controls moving and the flashlight (after you get it in level 2: Mars City Undeground).
But moving is not recommended because it will make you sick. You should teleport instead when possible.

The left thumb touch-pad or analog stick moves you relative to where your left hand is pointing.
The left grip makes you crouch (but crouching in real life is better).
On Vive, the left trigger makes you jump, but on Touch it makes you run.

Pushing in with your left thumb presses the flashlight button to turn it on or off.
In BFG edition, the flashlight turns itself off after a while as it goes flat.

Y on the left Touch will recenter the view and adjust your height.
If you sit down or stand up, use it to adjust to your new position.

X on the left touch controller or ≡ (menu) on the left Vive, brings up the PDA (after you get it at Reception).
It also skips cutscenes.
You can move the PDA around in your left hand, while using your right finger to touch the screen,
or you can use the sticks/pads and buttons on each hand to select options.
Use Y or grip to put your PDA away again.
If the Oculus tracking glitches, your PDA can end up far away. In that case repeatedly press Y or Grip to get out.

Press ≡ (menu) on the left Touch to bring up the pause menu (which also uses the PDA like above).
There are lots of VR options you can adjust, and you can set keybindings in Controls.

You can also manually grab the PDA from your left hip using motion controls.
When you move the left controller over the right spot, the right controller vibrates.
Use the grip button to pick it up. Your flashlight will be temporarily holstered there while using the PDA.
When you are finished with the PDA, you can put it back the same way and grab your flashlight.
Grabbing the PDA before you collect it from Reception brings up the pause menu instead of the PDA. 

You can also manually place the flashlight on your left shoulder or your head the same way.
And then later retrieve it from those places the same way.

You can swap the two hands by grabbing the holster on your right hip with your left hand.
That will transfer the flashlight to your right side and pick up any holstered weapon in your left.
Swapping hands swaps most of the controls, but not the analog sticks/touchpads.

Your RIGHT hand (unless you swapped them) controls the weapon and teleporting (and Jumping on Touch, menu on Vive).
Teleporting is the recommended way of moving further than you can walk in real life.

Push in the right touchpad, or hold down B on Touch, then move your hand to aim the parabola where you want to go.
Release the stick or touchpad to teleport there. If it's red, you need to duck/crouch first.
If there's no circle, that means you can't teleport there from your current position.

You can teleport anywhere you could reach by walking, ducking, and jumping onto things.
But you can't teleport across gaps you have to jump across, or onto the tightrope in underground.
There is a path length/complexity limit on how far you can telelport in one hop, but it's very far.
If you didn't install the Player AAS files, you will only be able to teleport places a Zombie can go.
There are bugs in the Player AAS files, and some levels are missing, so sometimes you can't teleport
places you should be able to. Especially when entering another room. In that case, walk in real life
or move forward with the stick/pad a little until you can teleport again.

Pull the trigger to fire the weapon (once you collect it).
For grenades, hold the trigger and throw (but be careful of the chaperone).
For fists, pull the trigger to punch.

The right grip reloads the weapon. Also A on Touch.

On Touch, the right stick's up/down switches weapon. Left/right snap turns you 45 degrees.
Push in the right Touch stick to Jump.

On Vive, the right touchpad left/right switches weapons.
The right Vive ≡ (menu) button brings up the pause menu.

You can manually grab the holster on your right hip to holster your weapon.
If there's already a weapon holstered there, it will swap.

You can grab behind your weapon hand's shoulder to switch to the next weapon.
You can grab behind your weapon hand's lower back for previous weapon.

You can also grab the PDA on your left hip with your right hand, if you prefer.
You can then put it back on the same side like normal, or holster PDA on the right to switch hands.

When you see any touchscreen, you can reach out with your right hand to press buttons on it.
Your hand will turn into a finger when it's inside the touchscreen.
Walk backwards to get out of the touchscreen.

You can't use your hands to manually pick up or push objects yet or punch.

If your arms look weird, push the left stick forward to make your body face the correct direction.
If your body isn't attached to your neck properly, try jumping.

Default Voice Controls:
-----------------------

Speak to other people by walking up to them and speaking.
You can say whatever you want. After you finish talking, they will respond.
They briefly look at you when you first start speaking.
Speaking can theoretically wake up nearby monsters, so be careful.
But most monsters in the game won't wake up when you speak.

You can switch to a specific weapon by saying the name of that weapon (English only, so far).
You can also say "Reload".

You can use some Holodeck commands.

You can also say:
- Reload
- Menu, System
- Cancel
- pause game
- resume game, unpause game
- exit game
- PDA, personal data assistant
- flashlight

To stop listening for voice commands, say either:
- stop listening, consentient

To start listening again say either:
- start listening, consecution

For a (long) list of what you can say, say:
- what can I say


Flicksync:
----------

The Flicksync mode is still very buggy and a work in progress.

Flicksync mode is based on the book Ready Player One by Ernest Cline.

The aim is to act out the game's cutscenes by becoming one of the characters.
You must say all of that character's lines exactly at the appropriate points.

If you don't say the line in time, the other character will repeat the cue (with subtitles),
"Final Dialog Warning" will appear, and the game will pause indefinitely waiting for you to say it.

If you can't remember the line, pull the trigger to use a Cue Card Power-Up (if you have one).
The Cue Card will show you what you need to say (phonetically) in white near the top.

If you don't have a Cue Card Power-Up, or the speech recognition can't recognise the way you say it,
you can press the PDA or menu button to give up that line and lose 10 points.

After you give up twice in a row, it will say "Final Warning". A third failure is GAME OVER.

Pressing the PDA or menu button while it's not waiting for a line, will skip the cutscene like normal.

When you say the line correctly, you get 100 points.
If you say the line clearly and at roughly the right time, you get another 50 bonus points.

If you get 7 lines right in a row, you get another Cue Card Power-Up, up to a maximum of 5.

There are no action bonuses yet. And you automatically follow the character's path.

If the cutscene seems to finish, but it won't let you move, it's still waiting for you to say the 
last line of the cutscene (or to skip it).

The Flicksync mode is still very buggy and a work in progress. Sorry.

To start a Flicksync game, choose Campaign from the main menu, then choose Flicksync.
Choose which character you will be.
Choose how many Cue Cards you start with.
Choose whether to play just the Flicksync cutscene parts,
or play the normal Doom 3 game with Flicksync mode for the cutscenes,
(or a non-Flicksync game with no cutscenes).
Then choose Play.
You will start from a different point depending on which character you choose.

You can also watch the cutscenes from the POV of the player character(s) by choosing Player.
Or you can just watch all the cutscenes by setting Character to None and Game Mode to Cutscenes.

The Flicksync settings carry over into a normal game, so change them back to normal when finished.
You will see SCORE: 0 on the menus if you are in Flicksync mode.

Default gamepad controls
------------------------

You can play the game in VR using an XBox 360/One gamepad if you don't have motion controllers.
It will automatically switch to gamepad mode when you start using the gamepad.
You can still use the gamepad for room-scale if you want.

Left stick makes you move. But this will cause motion sickness.
So it's recommended to teleport by holding B, looking where you want to go, and letting go.
Push left stick in = run
Left trigger = flashlight

Right stick aims your weapon, and will turn you if you aim too far.
Push right stick in = crouch
Right trigger fires
Shoulder buttons switch weapons

A = jump
B = teleport
X = reload
Y = use
View / Back = PDA.
≡ / Start = menu.

___________________________________________________

5) CHANGES
__________________________________________

- VR support (Doom 3 only, 1 and 2 use a virtual screen in VR)

- Teleporting

- Speech recognition and voice commands

- Flicksync game mode

- Better looking cola cans and video discs

- Mostly uncensored Doom 1 and 2 (still no blue SS soldiers in secret level)

- Fixed Doom 2 extra secret level MAP33: Betray (accessed by pushing a wall in level 2).

- Win64 support

- OpenAL Soft sound backend primarily developed for Linux but works on Windows as well

- Bink video support through FFmpeg

- Soft shadows using PCF hardware shadow mapping

	The implementation uses sampler2DArrayShadow and PCF which usually
	requires Direct3D 10.1 however it is in the OpenGL 3.2 core so it should
	be widely supported.
	All 3 light types are supported which means parallel lights (sun) use
	scene independent cascaded shadow mapping.
	The implementation is very fast with single taps (400 fps average per
	scene on a GTX 660 ti OC) however I defaulted it to 16 taps so the shadows look
	really good which should give you stable 100 fps on todays hardware (2014).

- Changed light interaction shaders to use Half-Lambert lighting like in Half-Life 2 to 
	make the game less dark. https://developer.valvesoftware.com/wiki/Half_Lambert


___________________________________________________

6) CONSOLE VARIABLES
__________________________________________

// DG: support "com_showFPS 2" for fps-only view like in classic doom3 => make it CVAR_INTEGER
com_showFPS 0 - INTEGER - show frames rendered per second. 0: off 1: default bfg values, 2: only show FPS (classic view)

noclip - Toggle no-clipping mode, in case you accidentally teleport somewhere you can't get out of

vr_showWIP 0 BOOL - show VR walk in place data 
vr_wipScale 1.0 FLOAT - 

vr_comfortRepeat 100 INTEGER - Delay in MS between repeating comfort snap turns.

vr_joyCurves 0 INTEGER - Joy powercurves. 0 = Doom default  1 = Mixed curve with vr_joyCurveSensitivity and vr_joyCurveLin 
vr_joyCurveSensitivity 9 FLOAT - Sensitivity val 0 - 9 
vr_joyCurveLin 4 FLOAT - Linear point for joyCurves  sens < lin = power curve , sens = lin = linear , sens > lin = frac power curve. 

vr_grabberBeamWidth 4 INTEGER - Beam width for grabber in VR. Default 4.

vr_debugGui 0 BOOL - 
vr_guiFocusPitchAdj 7 FLOAT - View pitch adjust to help activate in game touch screens

vr_bx1 -5 FLOAT - 
vr_bx2 5 FLOAT - 
vr_by1 -5 FLOAT - 
vr_by2 5 FLOAT - 
vr_bz1 -5 FLOAT - 
vr_bz2 5 FLOAT - 

// for testing
ftx 1.0 FLOAT - 
fty 0 FLOAT - 
ftz 0 FLOAT - 

vr_slotDebug 0 BOOL - slot debug visualation
vr_slotMag 0.1 FLOAT - slot vibration magnitude (0 is off)
vr_slotDur 18 INTEGER - slot vibration duration in milliseconds
vr_slotDisable 0 BOOL - slot disable

vr_blink 1 FLOAT - Darkens the screen when head bumps walls and objects.

vr_throwPower 4.0 FLOAT - Throw power

vr_guiH 100 INTEGER - 
vr_guiA 100 INTEGER - 

vr_rumbleDiv 1 FLOAT - rumble divisor 
vr_rumbleSkip 1 FLOAT - frames to skip 
vr_rumbleEnable 1 BOOL - Enable VR controller rumble 

vr_openVrStuckPadAxisFix 1 BOOL - Check for openVR controller stuck pad axis.
vr_openVrStuckPadAxisFixThresh 12 INTEGER - # of identical non zero input polls before axis flagged as stuck.

vr_mouseCapture 0 INTEGER - Constrain the mouse to the game window while running.

vr_debugTouchCursor 0 BOOL - Show the cursor position when using touch screen mode.

vr_pixelDensity 1.25 FLOAT - 
vr_vignette 1 INTEGER - unused
vr_enable 1 INTEGER - Enable VR mode. 0 = Disabled 1 = Enabled.
vr_FBOscale 1.0 FLOAT - unused
vr_scale 1.0 FLOAT - unused
vr_useOculusProfile 1 INTEGER - Use Oculus Profile values. 0 = use user defined profile, 1 = use Oculus profile.
vr_manualIPDEnable 0 INTEGER - Override the HMD provided IPD value with value in vr_manualIPD 0 = disable 1= use manual iPD 
vr_manualIPD 64 FLOAT - User defined IPD value in MM
vr_manualHeight 70 FLOAT - User defined player height in inches
vr_minLoadScreenTime 6000 FLOAT - Min time to display load screens in ms. 0.0f, 10000.0f 

vr_clipPositional 1 BOOL - Clip positional tracking movement . 1 = Clip 0 = No clipping. 

vr_armIKenable 1 BOOL - Enable IK on arms when using motion controls and player body is visible.  1 = Enabled 0 = disabled 
vr_weaponHand 0 INTEGER - Which hand holds weapon.  0 = Right hand  1 = Left Hand  0, 1 

// flashlight cvars

vr_flashlightMode 3 INTEGER - Flashlight mount. 0 = Body 1 = Head 2 = Gun 3= Hand ( if motion controls available.)

vr_flashlightBodyPosX 0 FLOAT - Flashlight vertical offset for body mount.
vr_flashlightBodyPosY 0 FLOAT - Flashlight horizontal offset for body mount.
vr_flashlightBodyPosZ 0 FLOAT - Flashlight forward offset for body mount.

vr_flashlightHelmetPosX 6 FLOAT - Flashlight vertical offset for helmet mount.
vr_flashlightHelmetPosY -6 FLOAT - Flashlight horizontal offset for helmet mount.
vr_flashlightHelmetPosZ -20 FLOAT - Flashlight forward offset for helmet mount.

vr_offHandPosX 0 FLOAT - X position for off hand when not using motion controls.
vr_offHandPosY 0 FLOAT - Y position for off hand when not using motion controls.
vr_offHandPosZ 0 FLOAT - Z position for off hand when not using motion controls.

vr_forward_keyhole 11.25 FLOAT - Forward movement keyhole in deg. If view is inside body direction +/- this value, forward movement is in view direction, not body direction

vr_PDAscale 3 FLOAT - unused
vr_PDAfixLocation 0 BOOL - Fix PDA position in space in front of player  instead of holding in hand.

vr_weaponPivotOffsetForward 3 FLOAT - 
vr_weaponPivotOffsetHorizontal 0 FLOAT - 
vr_weaponPivotOffsetVertical 0 FLOAT - 
vr_weaponPivotForearmLength 16 FLOAT - ;

vr_guiScale 1 FLOAT - scale reduction factor for full screen menu/pda scale in VR 0.0001f, 1.0f  //koz allow scaling of full screen guis/pda
vr_guiSeparation .01 FLOAT - Screen separation value for fullscreen guis.

vr_guiMode 2 INTEGER - Gui interaction mode.  0 = Weapon aim as cursor  1 = Look direction as cursor  2 = Touch screen 

vr_hudScale 1.0 FLOAT - Hud scale 0.1f, 2.0f 
vr_hudPosHor 0 FLOAT - Hud Horizontal offset in inches
vr_hudPosVer 7 FLOAT - Hud Vertical offset in inches
vr_hudPosDis 32 FLOAT - Hud Distance from view in inches
vr_hudPosAngle 30 FLOAT - Hud View Angle
vr_hudPosLock 1 INTEGER - Lock Hud to:  0 = Face, 1 = Body


vr_hudType vr_hudType2 INTEGER - VR Hud Type. 0 = Disable. 1 = Full 2=Look Activate 0, 2 
vr_hudRevealAngle vr_hudRevealAngle48 FLOAT - HMD pitch to reveal HUD in look activate mode.
vr_hudTransparency vr_hudTransparency1 FLOATHud transparency. 0.0 = Invisible thru 1.0 = full 0.0, 100.0 
vr_hudOcclusion vr_hudOcclusion1 BOOLHud occlusion. 0 = Objects occlude HUD, 1 = No occlusion 
vr_hudHealth vr_hudHealth1 BOOL - Show Armor/Health in Hud.
vr_hudAmmo vr_hudAmmo1 BOOL - Show Ammo in Hud.
vr_hudPickUps vr_hudPickUps1 BOOL - Show item pick ups in Hud.
vr_hudTips vr_hudTips1 BOOL - Show tips Hud.
vr_hudLocation vr_hudLocation1 BOOL - Show player location in Hud.
vr_hudObjective vr_hudObjective1 BOOL - Show objectives in Hud.
vr_hudStamina vr_hudStamina1 BOOL - Show stamina in Hud.
vr_hudPills vr_hudPills1 BOOL - Show weapon pills in Hud.
vr_hudComs vr_hudComs1 BOOL - Show communications in Hud.
vr_hudWeap vr_hudWeap1 BOOL - Show weapon pickup/change icons in Hud.
vr_hudNewItems vr_hudNewItems1 BOOL - Show new items acquired in Hud.
vr_hudFlashlight vr_hudFlashlight1 BOOL - Show flashlight in Hud.
vr_hudLowHealth vr_hudLowHealth0 INTEGER0 = Disable, otherwise force hud if heath below this value.

vr_voiceCommands vr_voiceCommands2 INTEGER - Enable voice commands. 0 = none, 1 = menus, 2 = menus and weapons
vr_talkWakeMonsters 1 BOOL - Talking wakes monsters. 0 = no, 1 = yes
vr_talkMode 2 INTEGER - Talk to NPC 0 = buttons, 1 = buttons or voice, 2 = voice only, 3 = voice no cursor
vr_tweakTalkCursor 25 FLOAT - Tweak talk cursor y pos in VR. % val 0 to 99 

vr_flicksyncCharacter 0 INTEGER - Flicksync character. 0 = none, 1 = Betruger, 2 = Swan, 3 = Campbell, 4 = DarkStar, 5 = Tower, 6 = Reception, 7 = Kelly, 8 = Brooks, 9 = Mark Ryan, 10 = Ishii, 11 = Roland, 12 = McNeil, 13 = Marine w PDA, 14 = Marine w Torch, 15 = Point, 16 = Bravo Lead, 17 = Player 0, FLICK_PLAYER 

vr_flicksyncCueCards 0 INTEGER - How many Cue Card Power-Ups to start with. Default = 0, max = 5
vr_cutscenesOnly 0 INTEGER - Skip action and only show cutscenes. 0 = normal game, 1 = cutscenes only, 2 = action only

g_stopTime 0 BOOL - Freeze time. This is used by the Flicksync to pause cutscenes. If there's a bug and everything is frozen try setting it to 0.

vr_wristStatMon 1 INTEGER - Use wrist status monitor. 0 = Disable 1 = Right Wrist 2 = Left Wrist 

// koz display windows monitor name in the resolution selection menu, helpful to ID which is the rift if using extended mode
vr_listMonitorName 0 BOOL - List monitor name with resolution.

vr_viewModelArms 1 BOOL - Dont change this, will be removed. Display arms on view models in VR
vr_disableWeaponAnimation 1 BOOL - Disable weapon animations in VR. ( 1 = disabled )
vr_headKick 0 BOOL - Damage can 'kick' the players view. 0 = Disabled in VR.
vr_joystickMenuMapping 1 BOOL - Use alternate joy mapping  in menus/PDA.  0 = D3 Standard  1 = VR Mode. (Both joys can nav menus,  joy r/l to change select area in PDA.


vr_deadzonePitch 90 FLOAT - Vertical Aim Deadzone 0, 180 
vr_deadzoneYaw 30 FLOAT - Horizontal Aim Deadzone 0, 180 
vr_comfortDelta 10 FLOAT - Comfort Mode turning angle  0, 180 

//vr_interactiveCinematic 1 BOOL - Interactive cinematics in VR ( no camera )

vr_headingBeamWidth 12.0 FLOAT - heading beam width // Koz default was 2, IMO too big in VR.
vr_headingBeamLength 96 FLOAT - heading beam length // koz default was 250, but was to short in VR.  Length will be clipped if object is hit, this is max length for the hit trace. 
vr_headingBeamMode 3 INTEGER - 0 = disabled, 1 = solid, 2 = arrows, 3 = scrolling arrows

vr_weaponSight 0 INTEGER - Weapon Sight.  0 = Lasersight  1 = Red dot  2 = Circle dot  3 = Crosshair 
vr_weaponSightToSurface 1 INTEGER - Map sight to surface. 0 = Disabled 1 = Enabled 

vr_motionWeaponPitchAdj 40 FLOAT - Weapon controller pitch adjust
vr_motionFlashPitchAdj 40 FLOAT - Flashlight controller pitch adjust

vr_nodalX -3 FLOAT - Forward offset from eyes to neck
vr_nodalZ -6 FLOAT - Vertical offset from neck to eye height

vr_vcx -3.5 FLOAT - Controller X offset to handle center // these values work for steam
vr_vcy 0 FLOAT - Controller Y offset to handle center
vr_vcz -.5 FLOAT - Controller Z offset to handle center

vr_mountx 0 FLOAT - If motion controller mounted on object, X offset from controller to object handle.  (Eg controller mounted on Topshot)
vr_mounty 0 FLOAT - If motion controller mounted on object, Y offset from controller to object handle.  (Eg controller mounted on Topshot)
vr_mountz 0 FLOAT - If motion controller mounted on object, Z offset from controller to object handle.  (Eg controller mounted on Topshot)

vr_mountedWeaponController 0 BOOL - If physical controller mounted on object (eg topshot), enable this to apply mounting offsets 0=disabled 1 = enabled

vr_3dgui 1 BOOL - 3d effects for in game guis. 0 = disabled 1 = enabled 
vr_shakeAmplitude 1.0 FLOAT - Screen shake amplitude 0.0 = disabled to 1.0 = full  0.0f, 1.0f 


vr_controllerStandard 0 INTEGER - If 1, use standard controller, not motion controllers Restart after changing 

vr_padDeadzone .25 FLOAT - Deadzone for steam pads.  0.0 = no deadzone 1.0 = dead 
vr_padToButtonThreshold .7 FLOAT - Threshold value for pad contact  to register as button press  .1 high sensitiveity thru  .99 low sensitivity
vr_knockBack 0 BOOL - Enable damage knockback in VR. 0 = Disabled, 1 = Enabled
vr_jumpBounce 0 FLOAT - Enable view bounce after jumping. 0 = Disabled, 1 = Full 0.0f, 1.0f  // Carl
vr_stepSmooth 1 FLOAT - Enable smoothing when climbing stairs. 0 = Disabled, 1 = Full 0.0f, 1.0f  // Carl
vr_walkSpeedAdjust -20 FLOAT - Player walk speed adjustment in VR. (slow down default movement)

vr_wipPeriodMin 10.0 FLOAT - 
vr_wipPeriodMax 2000.0 FLOAT - 

vr_wipVelocityMin .05 FLOAT - 
vr_wipVelocityMax 2.0 FLOAT - 

vr_headbbox 10.0 FLOAT - 

vr_pdaPosX 20 FLOAT - 
vr_pdaPosY 0 FLOAT - 
vr_pdaPosZ -11 FLOAT - 

vr_pdaPitch 30 FLOAT - 

vr_movePoint 0 INTEGER - If enabled, move in the direction the off hand is pointing.
vr_moveClick 0 INTEGER0 = Normal movement.  1 = Click and hold to walk, run button to run.  2 = Click to start walking, then touch only. Run btn to run.  3 = Click to start walking, hold click to run.  4 = Click to start walking, then click toggles run 
vr_playerBodyMode 0 INTEGER - Player body mode: 0 = Display full body 1 = Just Hands  2 = Weapons only 
vr_bodyToMove 1 BOOL - Lock body orientaion to movement direction.

vr_crouchMode 0 INTEGER - Crouch Mode:  0 = Full motion crouch (In game matches real life)  1 = Crouch anim triggered by smaller movement.
vr_crouchTriggerDist 10 FLOAT - Distance ( in inches ) player must crouch in real life to toggle crouch 

vr_frameCheck 0 INTEGER - 0 = bypass frame check

vr_forceOculusAudio 1 BOOL - Request openAL to search for Rift headphones instead of default device Fails to default device if rift not found.
vr_stereoMirror 1 BOOL - Render mirror window with stereo views. 0 = Mono , 1 = Stereo Warped

vr_APISelect 0 INTEGER - VR API Select:  0 = Autodetect ( Oculus Native then OpenVR ) ,  1 = Oculus Native Only  2 = OpenVR only 

vr_teleport 2 INTEGER - Player can teleport at will. 0 = disabled, 1 = gun sight, 2 = right hand, 3 = left hand, 4 = head 0, 4 
vr_teleportMaxTravel 950 INTEGER - Maximum teleport path length/complexity/time. About 250 or 500 are good choices, but must be >= about 950 to use tightrope in MC Underground. 150, 5000 
vr_teleportThroughDoors 0 BOOL - Player can teleport somewhere visible even if the path to get there takes them through closed (but not locked) doors.

ai_debugMove 0 BOOL - Debug teleporter and monster navigation by showing teleport path

vr_teleportSkipHandrails 0 INTEGER - Teleport aim ingnores handrails. 1 = true
vr_teleportShowAimAssist 0 INTEGER - Move telepad target to reflect aim assist. 1 = true
vr_teleportSkin 2 INTEGER - Test skins for cylinder 0, 2 
vr_teleportButtonMode 0 BOOL - 0 = Press aim, release teleport. 1 = 1st press aim, 2nd press teleport
vr_teleportHint 0 BOOL - 

vr_teleportVel 650 FLOAT 
vr_teleportDist 60 FLOAT 
vr_teleportMaxPoints 24 FLOAT - 
vr_teleportMaxDrop 360 FLOAT - 

vr_motionSickness 8 INTEGER - Motion sickness prevention aids. 0 = None, 1 = Chaperone, 2 = Reduce FOV, 3 = Black Screen, 4 = Black & Chaperone, 5 = Reduce FOV & Chaperone, 6 = Slow Mo, 7 = Slow Mo & Chaperone, 8 = Slow Mo & Reduce FOV, 9 = Slow Mo, Chaperone, Reduce FOV, 10 = Third Person, 11 = Particles, 12 = Particles & Chaperone 0, 12 
vr_strobeTime 500 INTEGER - Time in ms between flashes when blacking screen. 0 = no strobe

timescale 1.0 FLOAT - game speed multiplier. Used by slow mo motion sickness fix. If there's an issue with it, you may need to set timescale 1

vr_chaperone 2 INTEGER - Chaperone/Guardian mode. 0 = when near, 1 = when throwing, 2 = when melee, 3 = when dodging, 4 = always 0, 4 
//vr_chaperoneColor 0 INTEGER - Chaperone colour. 0 = default, 1 = black, 2 = grey, 3 = white, 4 = red, 5 = green, 6 = blue, 7 = yellow, 8 = cyan, 9 = magenta, 10 = purple 0, 10 

vr_handSwapsAnalogs 0 BOOL - Should swapping the weapon hand affect analog controls (stick or touchpad) or just buttons/triggers? 0 = only swap buttons, 1 = swap all controls
vr_autoSwitchControllers 1 BOOL - Automatically switch to/from gamepad mode when using gamepad/motion controller. Should be true unless you're trying to use both together, or you get false detections. 0 = no, 1 = yes.


r_useShadowMapping 1 - Use soft shadow mapping instead of hard stencil shadows


___________________________________________________

7) KNOWN ISSUES
__________________________________________

Doom 3 wasn't designed to work with shadow maps so:

- Some lights cause shadow acne with shadow mapping
- Some shadows might almost disappear due to the shadow filtering

___________________________________________________

8) BUG REPORTS
__________________________________________

DOOM-3-BFG VR: Fully Possessed is not perfect, it is not bug free as every other software.
For fixing as much problems as possible we need as much bug reports as possible.
We cannot fix anything if we do not know about the problems.

The best way for telling us about a bug is by submitting a bug report at our GitHub bug tracker page:

	https://github.com/KozGit/DOOM-3-BFG-VR/issues

The most important fact about this tracker is that we cannot simply forget to fix the bugs which are posted there. 
It is also a great way to keep track of fixed stuff.

If you want to report an issue with the game, you should make sure that your report includes all information useful to characterize and reproduce the bug.

    * What version you are using, and what mods you have installed.
    * Search on Google
    * Include the computer's hardware and software description ( CPU, RAM, 3D Card, distribution, kernel etc. )
    * If appropriate, send a console log, a screenshot, an strace ..
    * If you are sending a console log, make sure to enable developer output:

              Doom3BFGVR.exe +set developer 1 +set logfile 2

NOTE: We cannot help you with OS-specific issues like configuring OpenGL correctly, configuring ALSA or configuring the network.
	

	
	
_______________________________

9) LICENSE
______________________


See COPYING.txt for the GNU GENERAL PUBLIC LICENSE

ADDITIONAL TERMS:  The Doom 3 BFG Edition GPL Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU GPL which accompanied the Doom 3 BFG Edition GPL Source Code.  If not, please request a copy in writing from id Software at id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.


____________________________________________

10) GETTING THE SOURCE CODE
___________________________________

This project's GitHub.net Git repository can be checked out through Git with the following instruction set: 

	> git clone https://github.com/KozGit/DOOM-3-BFG-VR.git

If you don't want to use git, you can download the source as a zip file at
	https://github.com/KozGit/DOOM-3-BFG-VR/archive/master.zip



___________________________________________________________________

11) COMPILING ON WIN32 WITH VISUAL C++ 2013 EXPRESS EDITION
__________________________________________________________

1. Download and install the Visual C++ 2013 Express Edition.

2. Download and install the DirectX SDK (June 2010) here:
	http://www.microsoft.com/en-us/download/details.aspx?id=6812

3. Download and install the latest CMake, saying YES to adding CMake to your path.

4. Generate the VC13 projects using CMake by doubleclicking a matching configuration .bat file in the neo/ folder.

5. Go to https://developer.oculus.com/downloads/pc/1.9.0/Oculus_SDK_for_Windows/ then download and extract it somewhere. Copy the LibOVR folder to DOOM-3-BFG/neo/libs

6. Clone the OpenVR git repository in another folder: https://github.com/ValveSoftware/openvr
	If it's an incompatible later version, you may need to do a hard reset in git (from the log in TortoiseGit) to the v1.0.5 version
	Copy the openvr folder to DOOM-3-BFG/neo/libs

7. Use the VC13 solution to compile what you need:
	DOOM-3-BFG/build/Doom3BFGVR.sln
	
8. Download ffmpeg-20140405-git-ec8789a-win32-shared.7z from ffmpeg.zeranoe.com/builds/win32/shared/2014

9. Extract the FFmpeg DLLs to your current build directory under DOOM-3-BFG/build/

10. In Visual Studio, right click project Doom3BFGVR, click Properties. Set Configuration to All Configurations. Choose Debugging, set Command Arguments to:
	+set fs_basepath "C:\Program Files (x86)\Steam\steamapps\common\DOOM 3 BFG Edition"
	or wherever you installed Doom 3 BFG edition

11. To create the installer, download and install NSIS. Build the release version in visual studio. Right click on the installer.nsi file and choose Compile NSIS Script.

__________________________________

12) COMPILING ON GNU/LINUX
_________________________

It probably doesn't compile on GNU/LINUX anymore. I don't have it to test. Sorry.

1. You need the following dependencies in order to compile Doom3BFGVR with all features:
 
	On Debian or Ubuntu:

		> apt-get install cmake libsdl2-dev libopenal-dev libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
	
	On Fedora

		// TODO add ffmpeg libs for bink videos
		
		> yum install cmake SDL-devel openal-devel
	
	On openSUSE (tested in 13.1)
	
		> zypper in openal-soft-devel cmake libSDL-devel libffmpeg1-devel
	
		For SDL 2 replace "libSDL-devel" with "libSDL2-devel".
		"libffmpeg1-devel" requires the PackMan repository. If you don't have that repo, and don't want to add it, remove the "libffmpeg1-devel" option and compile without ffmpeg support.
		If you have the repo and compiles with ffmpeg support, make sure you download "libffmpeg1-devel", and not "libffmpeg-devel".
	
	Instead of SDL2 development files you can also use SDL1.2. Install SDL 1.2 and add to the cmake parameters -DSDL2=OFF
	
	SDL2 has better input support (especially in the console) and better 
	support for multiple displays (especially in fullscreen mode).


2. Generate the Makefiles using CMake:

	> cd neo/
	> ./cmake-eclipse-linux-profile.sh
	
3. Compile DOOM-3-BFG targets with

	> cd ../build
	> make

____________________________________________________________________________________

13) CODE LICENSE EXCEPTIONS - The parts that are not covered by the GPL:
_______________________________________________________________________


EXCLUDED CODE:  The code described below and contained in the Doom 3 BFG Edition GPL Source Code release
is not part of the Program covered by the GPL and is expressly excluded from its terms. 
You are solely responsible for obtaining from the copyright holder a license for such code and complying with the applicable license terms.


JPEG library
-----------------------------------------------------------------------------
neo/libs/jpeg-6/*

Copyright (C) 1991-1995, Thomas G. Lane

Permission is hereby granted to use, copy, modify, and distribute this
software (or portions thereof) for any purpose, without fee, subject to these
conditions:
(1) If any part of the source code for this software is distributed, then this
README file must be included, with this copyright and no-warranty notice
unaltered; and any additions, deletions, or changes to the original files
must be clearly indicated in accompanying documentation.
(2) If only executable code is distributed, then the accompanying
documentation must state that "this software is based in part on the work of
the Independent JPEG Group".
(3) Permission for use of this software is granted only if the user accepts
full responsibility for any undesirable consequences; the authors accept
NO LIABILITY for damages of any kind.

These conditions apply to any software derived from or based on the IJG code,
not just to the unmodified library.  If you use our work, you ought to
acknowledge us.

NOTE: unfortunately the README that came with our copy of the library has
been lost, so the one from release 6b is included instead. There are a few
'glue type' modifications to the library to make it easier to use from
the engine, but otherwise the dependency can be easily cleaned up to a
better release of the library.

zlib library
---------------------------------------------------------------------------
neo/libs/zlib/*

Copyright (C) 1995-2012 Jean-loup Gailly and Mark Adler

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Base64 implementation
---------------------------------------------------------------------------
neo/idlib/Base64.cpp

Copyright (c) 1996 Lars Wirzenius.  All rights reserved.

June 14 2003: TTimo <ttimo@idsoftware.com>
	modified + endian bug fixes
	http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=197039

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

IO for (un)compress .zip files using zlib
---------------------------------------------------------------------------
neo/libs/zlib/minizip/*

Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

Modifications of Unzip for Zip64
Copyright (C) 2007-2008 Even Rouault

Modifications for Zip64 support
Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

MD4 Message-Digest Algorithm
-----------------------------------------------------------------------------
neo/idlib/hashing/MD4.cpp
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD4 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD4 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

MD5 Message-Digest Algorithm
-----------------------------------------------------------------------------
neo/idlib/hashing/MD5.cpp
This code implements the MD5 message-digest algorithm.
The algorithm is due to Ron Rivest.  This code was
written by Colin Plumb in 1993, no copyright is claimed.
This code is in the public domain; do with it what you wish.

CRC32 Checksum
-----------------------------------------------------------------------------
neo/idlib/hashing/CRC32.cpp
Copyright (C) 1995-1998 Mark Adler

OpenGL headers
---------------------------------------------------------------------------
neo/renderer/OpenGL/glext.h
neo/renderer/OpenGL/wglext.h

Copyright (c) 2007-2012 The Khronos Group Inc.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and/or associated documentation files (the
"Materials"), to deal in the Materials without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Materials, and to
permit persons to whom the Materials are furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Materials.

THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

Timidity
---------------------------------------------------------------------------
neo/libs/timidity/*

Copyright (c) 1995 Tuukka Toivonen 

From http://www.cgs.fi/~tt/discontinued.html :

If you'd like to continue hacking on TiMidity, feel free. I'm
hereby extending the TiMidity license agreement: you can now 
select the most convenient license for your needs from (1) the
GNU GPL, (2) the GNU LGPL, or (3) the Perl Artistic License.  


