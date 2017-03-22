    ____                           _____  ____   ______ ______ _    __ ____   
   / __ \ ____   ____   ____ ___  |__  / / __ ) / ____// ____/| |  / // __ \ 
  / / / // __ \ / __ \ / __ `__ \  /_ < / __  |/ /_   / / __  | | / // /_/ /
 / /_/ // /_/ // /_/ // / / / / /___/ // /_/ // __/  / /_/ /  | |/ // _, _/
/_____/ \____/ \____//_/ /_/ /_//____//_____//_/     \____/   |___//_/ |_|

							Fully Possessed
____________________________________________________________________________



DOOM3-BFG VR : Fully Possessed
Includes support for the HTC Vive via OpenVR, and the Oculus Rift and Touch motion controls via the Oculus SDK or OpenVR.

VR Implementation: /u/Samson-
Major Contributions - Teleportation, Voice Commands, Flicksync, Bink videos, Loading other savegames, code improvements: Carl Kenner
Holster slots, Crawl Space Head Collision, minor fixes: Leyland
QuakeCon teleporting, Slow Mo and Tunnel vision motion sickness fixes: jckhng
Originally inspired by: tmek

DOOM-3-BFG-VR Readme - https://github.com/KozGit/DOOM-3-BFG-VR

DOOM-3-BFG-VR was built using an older version of the RBDOOM3-BFG port.
RBDOOM-3-BFG Readme - https://github.com/RobertBeckebans/RBDOOM-3-BFG


_______________________________________

CONTENTS
_______________________________



This file contains the following sections:

	1) SYSTEM REQUIREMENTS

	2) GENERAL NOTES
	
	3) INSTALLATION, GETTING THE GAMEDATA, RUNNING THE GAME
	
	4) NEW FEATURES
	
	5) CHANGES
	
	6) CONTROLS
	
	7) FLICKSYNC
	
	8) VR OPTIONS
	
	9) CONSOLE VARIABLES
	
	10) KNOWN ISSUES
	
	11) GETTING THE SOURCE CODE

	12) COMPILING ON WIN32 WITH VISUAL C++ 2013 EXPRESS EDITION

	13) COMPILING ON GNU/LINUX
	
	14) BUG REPORTS
	
	15) LICENSE
	
	16) CODE LICENSE EXCEPTIONS



___________________________________

1) SYSTEM REQUIREMENTS
__________________________



Minimum system requirements:

	CPU: 3 GHz + Intel compatible
	System Memory: 8 GB
	Graphics card: Nvidia Geforce GTX 960 or higher, with recent drivers
	               or AMD 290 or above
	OS: Windows 7 64 bit or above?
	VR: SteamVR compatible HMD or Oculus Rift DK2 or above
	A microphone or HMD containing a microphone

Recommended system requirements:

	CPU: 3 GHz + Intel compatible
	System Memory: 8 GB
	Graphics card: Nvidia Geforce GTX 970 or higher, with latest drivers
	               or AMD 290 or above
	OS: Windows 10 64 bit
	VR: HTC Vive or Oculus Rift CV1 with Touch and roomscale sensors
	


_______________________________

2) GENERAL NOTES
______________________

This release does not contain any game data, the game data is still
covered by the original EULA and must be obeyed as usual.

Only the Doom 3 BFG Edition is supported. Classic Doom 3 is not supported yet.

You should patch the game to the latest version. If using Steam, this will happen automatically.

Note that Doom 3 BFG Edition is available from the Steam store at
http://store.steampowered.com/app/208200/


Saved games:
------------
The saved game folder is:
%UserProfile%\Saved Games\id Software\DOOM 3 BFG\Fully Possessed

Your settings are also saved there as vr_oculus.cfg and vr_openvr.cfg
You can delte the those setting files to return to default settings.

It will also try to load games (with partial success) from:
%UserProfile%\Saved Games\id Software\DOOM 3 BFG\base
%UserProfile%\Saved Games\id Software\RBDOOM 3 BFG\base

Games loaded from other versions, other mods, or different texture packs
will have some glitches until you complete the level (which may not always be
possible without cheats). You should use the "restartMap" console command after
loading if you are near the start or you get stuck due to a glitch. It will
restart the map with your current inventory but without any glitches. You can
also use the "endLevel" console command to win the level and continue to the
next level (without any glitches) if you get stuck near the end. But that is
sort of cheating.


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

The Doom 3 BFG Edition GPL Source Code release allows mod editing. In order for it to accept any change in your
mod directory, you should first specify your mod directory adding the following command to the launcher:

The fs_game mod directory is used exclusively by DOOM3-BFG VR : Fully Possessed and should not be used by other mods.
Additional mods can my use the fs_game_base directory

"+set fs_game_base modDirectoryName"

so it would end up looking like: Doom3BFGVR +set fs_game_base mymod

This will result in the game searching the Fully Possessed directory, followed by the modDirectoryName directory,
and finally the BASE directory when loading assets.

Binary mods from Classic Doom 3 (non-BFG) mods are not supported.  
It is possible however to utilize some of the assets from Classic Doom mods,
however this is beyond the scope of this document.


___________________________________________________

3) INSTALLATION, GETTING THE GAMEDATA, RUNNING THE GAME
__________________________________________

1. Make sure you have Doom 3 BFG Edition installed in Steam.

2. Set the Doom 3 BFG language in Steam to your desired language. 

3. In the unlikely event that you don't have the Visual C++ 2013 redistributable installed, get it here:
	https://www.microsoft.com/en-au/download/details.aspx?id=40784

4. Old version 0.015 of this mod installed assets directly into the BASE directory of the Doom 3 BFG
   installation.  All assets are now installed into a separate mod folder named 'Fully Possessed'
   located in the Doom 3 BFG directory. An installer is now provided that will automatically remove files from old
   versions of this mod if present, and install the updated mod.  The installer doesn't add any files to the base 
   folder, but it will remove files installed by mods that have the same names as files used by this mod ( mostly 
   weapons and player models ). This will NOT affect the base game, but could possibly break other mods installed.

   ( Note that at the time of this writing, the stock RBDOOM-3-BFGVR mod by Leyland does not install any assets,
   and this installation/removal process should not affect it. HOWEVER, if the hi-def expansion has been installed
   in addition to the RBDOOM-3_BFGVR Mod, the hi-def expansion will need to be reinstalled. )

   * If any other mods have been installed, or if you are unsure what deleting files may mean, it is STRONGLY 
   recommended to make a backup copy of your Doom3 BFG installation before running the installer. *

5. Installing the mod:

   Start the Doom3BFGVR_Fully_Possessed_Alpha022.exe installer.

   If Doom 3 BFG was not originally installed on your main Steam hard-drive, change the install path to reflect
   the Doom 3 BFG installation directory. If you have enough disk space, it is recommended to select to install 
   both Base and Player AAS files. The AAS files may be omitted, but will negatively impact the ability to use 
   teleportation.  It is only recommended to skip the AAS file installation if you do not plan on using teleportation.

   When installation is complete, a shortcut will be created on your desktop and in your program list.

6. If you want to use speech recognition to activate voice commands ( recommended ), it should be enabled in 
   Windows and trained for optimal performance.  Speech recognition settings are available in the Control Panel 
   under 'Speech recognition.' At a minimum run 'Set up microphone', and for optimal performance 'Train your 
   computer to better understand you' 

7. If you have an Oculus Rift: In the Oculus Home window, turn on: (cog), Settings, General, Unknown Sources

8. Run the shortcut on your desktop, or run the Doom3BFGVR.exe file in your Doom 3 BFG folder to launch the game.
   (SteamVR will be automatically activated if needed)

   To play the game, you need the game data from a legal copy of the game, which 
   requires Steam for Windows. Currently, Doom 3 BFG: Fully Possessed is only compatible with Windows.

9. Enjoy

10. If you run into bugs, please report them. ( see section 9 )

___________________________________________________

4) NEW FEATURES
__________________________________________

New in 0.22:
	REALLY fix loading saved games from other versions and mods.
	ATI / AMD graphics card support.
	Scaled ammo, weapons, and keycards to correct sizes.
	Option to use floor height.
	World Scale option.
	vr_chibi head scale console command.
	More laser sight options.
	Head tracking in the main menu.
	Holster slots work better with the PDA and QuickSave.
	Allow path names longer than 260 chars (fixes crash?).
	Slightly improve voice commands.
	Default keyboard / mouse controls.
	Less debug prints for sound effects.

New in 0.21:
	Talking waking monsters is fixed.
	Loading all saved games from 0.020 is fixed (not really).
	You can now load most saved games from RBDoom 3 BFG (with major issues).
	You can now load all saved games from version 0.015 (with major issues).
	Improved performance.
	There's a menu option for Asynchronous SpaceWarp.
	You can now chose to walk in the direction of either hand when walking.
	Language is now detected based on sound files not .lang files.
	Added QuakeCon style teleport option. http://www.gamespot.com/articles/doom-may-have-solved-vrs-traversal-problem/1100-6442439/
	Added vr_hmdPerfHud console variable to debug framerate issues.


DOOM3-BFG VR : Fully Possessed offers many new features:

Native support for the HTC Vive via OpenVR when detected.
Native support for the Oculus Rift and Touch via the Oculus SDK when detected.



- Motion Controls: Full motion controls are supported.
		
	Independent	weapon and flashlight control via motion controllers.
	
	All in game GUIs ( including the in game PDA ) can be interacted with as touch screens.
	
	Game and VR settings can be adjusted in game via a touch screen interface.
	
	A weapon slot system has been implemented.  The player may use motion controls to stash or 
	retrieve a weapon or the PDA from virtual holsters located on the player body.  The player may also
	use motion controls to grab or place the flashlight on the head or the body, and then pick
	it back up in the hand later.  
	
	Full room-scale support.  ( See locomotion ) 
	

	
- Locomotion:  Multiple locomotion options are supported.

	Teleportation support:
	  
	  A parabolic aiming beam identifies areas the player can teleport to, as identified by
	  the game AAS system. Teleporting to a location will cause the player to activate all in game 'triggers'
	  along the path the player would walk to reach the destination.  This includes activating cutscenes, 
	  alerting or activating monsters, triggering any in game scripted events, and taking any environmental 
	  damage that the player would incur along this route. Players may not teleport through closed doors, or
	  to areas the AAS system has marked as unreachable by the player.
	  
	  LIMITATIONS:  Doom 3 was not initially designed with the idea of teleportation in mind. Doom 3 uses an 
	  Area Awareness System (AAS) to determine if a location is reachable by a character in the game, and then
	  generate a path to that point. AAS files have been generated for the player for most of the levels in the 
	  game, but there are limitations. Issues can happen for example if trying to teleport onto a desk -
	  you will teleport to the desk but then be bounced to the floor, just as if you had tried to jump on the desk.
	  The AAS system is also currently unable to identify areas that are only reachable by ladder, so it is not 
	  currently possible to teleport to an area if the only way to reach it is via ladder. Additionally, there are 
	  a few small	areas in the game the AAS system is not aware of, and therefore cannot be teleported to. In these
	  instances, the player will need to use normal locomotion to reach these areas and continue the game.

	Full room-scale support.
	  
	  Your character can walk around and crouch using your real life motion. Walking in real life allows you to 
	  walk around in the game, including walking up and down stairs or off ledges.  You can push small shipping
	  containers and other items with real life motion. Solid objects cannot be walked through, but they can be
	  leaned over to some degree. Jumping via real life motion is not supported.

	Gamepad and Motion Controller locomotion:
	
	  Gamepad or Motion Controller Pads, Sticks, or Buttons can be used for artificial locomotion.
	  Options are included for controller relative movement ( Onward style ), and normal 
	  gamepad style movement controls.  There are a variety of options available to the player, including
	  smooth turning, comfort (snap) turning, and multiple comfort options to reduce the effects of 
	  VR sickness when using artificial movement.  All of the controls can be rebound or disabled 
	  ( for example, turning or strafe can be removed completely from the controllers ), and the various
	  comfort options enabled or disabled.  These changes can be easily made in game via touch screen
	  access to VR specific game menus using the PDA.

	Comfort Options:
	
	  There are various locomotion options available to assist in preventing motion sickness, including comfort 
	  ( snap ) turning, third person movement, movement based FOV reduction, slow motion movement, and using the
	  Chaperone boundaries to provide a static reference.
	  
- Voice Commands:

	DOOM3-BFG VR : Fully Possessed supports voice commands via the Windows speech recognition engine.
	System commands such as pause, resume, and menu are accessible by speaking.  Additionally, the player may 
	'Talk' to NPC's simply by speaking to them when they are in focus.  ( Talking does not imply the NPCs carry out
	an actual conversation - speaking to them prompts them to say their predetermined lines.)
	All weapons can be selected by saying their name, and can also be reloaded by voice command.
	Phrases that activate each voice command are defined in a plaintext file that can be modified 
	by the user to customize behavior.
	

	
- Character and Weapon Options:

	The player may choose to display the full character body while playing.  Basic IK is implemented for the player
	head, arms, and crouch height, so movement in real life is replicated in the game to some degree. Note that
	the IK is extremely basic, and often results in incorrect poses.  
	
	If they player does not wish to see their body while playing ( for example if the incorrect arm IK is found to be
	to distracting, annoying, or just silly ), the player body may be disabled. There are options to instead display
	only the active weapon with hands and a stat watch,	or just the active weapon with no hands.	
	
	The stat watch is a display located on the player wrist that indicates the current ammo status for the active weapon,
	as well as vertical Health and Armor bars to monitor player status.  The Health bar is located on the left side of 
	the statwatch, and the Armor bar is on the right.  The bars will shrink and change color as health and armor are 
	depleted.  
	
	
	
- HUD and GUI options:

	The HUD is now projected into the world as a translucent object.  The HUD can be enabled fully, disabled, or
	can activate when the player looks down past a user defined angle.  Individual elements of the HUD can be 
	hidden if desired. The HUD position can be adjusted by the user, and locked to either the player view
	or the player body.
	
	In game GUIS such as the PDA, interactive computers, card readers, buttons, or keypads can be interacted with 
	simply by touching them with your virtual finger as you would a normal touchscreen.  If desired, touchscreen 
	mode can be deactivated, and gui interaction can be controlled either by aiming your weapon or via gaze aiming.
	
	
	
- Flicksync:

	Flicksync is an experimental mode based on the book Ready Player One by Ernest Cline. 
	In the book, players in a virtual environment participate in and act out the lines of their
	favorite movies, and their virtual performances scored.
	
	In this just for fun mode, the player selects a character to portray in the game. The aim is to act out the game's 
	cutscenes by becoming that character. You must say all of that character's lines exactly at the appropriate
	times. Points are earned by saying your lines correctly, clearly, and at the right time.  
	( See the Flicksync section under Controls for full instructions .)
	The Flicksync mode is still a work in progress, so bugs are to be expected.


___________________________________________________

5) CHANGES
__________________________________________

- VR support (Doom 3 only, 1 and 2 use a virtual screen in VR)

- Motion Controls

- Voice Commands

- Flicksync Mode

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
	
6) CONTROLS
__________________________________________


* IMPORTANT * ALL CONTROLS CAN BE EASILY CUSTOMIZED/REMAPPED BY THE USER *

	Un-Binding or Binding controls:
	
	  Any axis or button can be re-mapped to any movement or control function.
	  Rebinding controls is accomplished through the 'Settings->Controls->Key Bindings' menu.
	  This menu is available from the Main Menu when the game is first launched, or
	  by pressing the System Menu button in game, and selecting the 'Settings->Controls->
	  Key	Bindings' menu on the PDA.
	  
	  Be careful not to speak when binding a control, or you will rebind what talking does.
	  You can't rebind specific voice commands. Edit the dict file instead.
	  The thumb rests on the Touch controllers can be bound, so be careful not to bump them.
	  
	  You can use the right joystick and trigger to highlight and select an action.
	  If you are in game and using the PDA to adjust settings, you may use motion controls 
	  and your virtual finger to access the menu as a touch screen.  Touch an entry once to 
	  highlight, and touch again to select.
	  
	  Use the joystick or touch controls to highlight a command and see its current bindings.
	  If available, button graphics will identify a bound control, otherwise a text descriptor is 
	  used. If the list of bindings is longer than the available space in the menu, the full binding
	  list for the selected action is listed (without	button graphics) at the bottom of the screen.
	  
	  Select the highlighted action with the trigger or by tapping. Once selected, you can unbind a 
	  currently bound control or add a new binding. To unbind a control, simply press the axis
	  or button you would like to unbind and it will be removed from the list. To bind a control,
	  simply press the desired button or axis and it will be added to the binding list.  If the 
	  desired button/axis	is currently bound to another action, you will be asked if you want to 
	  continue and reassign it to this command ( removing the existing binding ), or to cancel.
	  
	  Changing comfort ( snap ) turning:
	    
	    By default, the game is configured to use comfort or snap turning.
	    This can be easily changed via the Settings->VR Options->Comfort Options
	    menu, under the Turning option.  Cycle through the various options to select the turning
	    mode you prefer.  ( Analog mode provides the typical smooth turning function - however this 
	    is one of the most common cause of VR sickness.)
	    
	    Alternative, you may manually rebind Turn Left, Turn Right, Comfort Turn Left, and Comfort Turn
	    Right in the Settings->Controls->Key Bindings menu.
	      
	  
	  
Default controls for the Oculus Touch, HTC Vive, and Gamepad :

*	Read each section carefully. The controls behave differently depending
	on if you are playing the game or using a menu or the PDA. *
	  
	  
	Walking and crouching in real life are reflected in the game. You can
	walk up and down steps, over ledges, or push small objects by walking in
	real life.  Crouching can also be controlled via button, see below.
	
	By default, the flashlight ( or PDA when active ) are held in the left hand,
	and the weapon is held in the right hand.  This can be changed by picking up the 
	weapon or PDA from it's slot with your other hand, or through the VR Options->
	Character Options: Weapon Hand menu entry.
	
	Grenades are thrown via motion controls - press
	the trigger to start the throw, swing your arm and release the trigger to throw.
	
	
  _________________________________________	  
  Default Control Bindings: Oculus Touch

	*IN MENUS OR PDA:
	
	  Use either stick to highlight menu entries.
	  Press either trigger to select.
	  Press Y or Grip to go back/exit.
	  
	  If you are using the PDA, or the System Menus on the PDA in game,
	  you can use your virtual finger to select menu items just like using
	  a touch screen.
	
	*GAMEPLAY CONTROLS.

	  LEFT Controller:
	  
	    By default, the LEFT controller/hand will aim the flashlight, once equipped.
	    
	    Left Stick: 	The left stick controls player movement.  The default configuration is 
	            motion relative to the direction the controller is pointing ( Onward style ).  
	            
	            Artificial motion will induce VR sickness for most people, so teleportation
	            instead of stick control should be considered as a primary form of 
	            movement in these instances.
	            
	            For those who find teleportation immersion breaking or uncomfortable,
	            there are a variety of ( optional ) comfort modes available in the VR menus.
	            These modes include FOV reduction, Slo-Motion, Black Screen and Chaperone 
	            effects to reduce the discomfort caused by mismatched vection. See the 
	            VR MENUS section for more details.
	           
	    Left Grip:  	Crouch.
	              
	    
	    Left Stick 
	    Press: 	    Press the left stick down to toggle the flashlight on and off.
	    
	    
	    Left Trigger:	Hold to run.
	    
	    Left Y:	    Recenter view/reset height.
	            If you stand up or sit down while playing, press this to adjust the
	            game to your new position.
	            
	            If the player body is visible, use this to reset the body to face the
	            current view direction.
	            
	    
	    Left X:	    Activate the PDA if it's in you inventory.
	            Left X also will skip the active cutscene.
	            In a Flicksync, this gives up on the current line.
	                
	        
	    Left ≡ (menu)	Brings up the in game Pause/System menu on the PDA.
	            ( The PDA does not need to be in you inventory to use this function)
	            
	          
	  
	  RIGHT CONTROLLER:

	    By default the RIGHT controller controls the right hand and the weapon 
	    
	    Right Stick
	    Up/Down:	  Next / Previous Weapon
	    
	    Right Stick
	    Left/Right:	  Snap turn 45 degrees left or right.
	    
	    Right Grip:	  Reload
	          
	    Right Stick
	    Press:	    Jump
	    
	    Right Trigger:	Attack / Fire
	            Use Cue Card Power-Up in Flicksync
	    
	    Right A:	  Reload
	    
	    Right B: 	  Teleport	  
	         
  _________________________________________	  
  Default Control Bindings: HTC Vive

	*IN MENUS OR PDA:
	
	  Use either pad to highlight menu entries.
	  Press either trigger to select.
	  Press either grip to go back/exit.
	  
	  If you are using the PDA, or the System Menus on the PDA in game,
	  you can use your virtual finger to select menu items just like using
	  a touch screen.
	
	*GAMEPLAY CONTROLS.

	  LEFT Controller:
	  
	    By default, the LEFT controller/hand will aim the flashlight, once equipped.
	    
	    Left Pad: 	  The left stick controls player movement.  The default configuration is 
	            motion relative to the direction the controller is pointing ( Onward style ).  
	            
	            Artificial motion will induce VR sickness for most people, so teleportation
	            instead of stick control should be considered as a primary form of 
	            movement in these instances.
	            
	            For those who find teleportation immersion breaking or uncomfortable,
	            there are a variety of ( optional ) comfort modes available in the VR menus.
	            These modes include FOV reduction, Slo-Motion, Black Screen and Chaperone 
	            effects to reduce the discomfort caused by mismatched vection. See the 
	            VR MENUS section for more details.
	           
	    Left Grip:  	Crouch.
	              
	    
	    Left Pad 
	    Press: 	    Press the left pad down to toggle the flashlight on and off.
	    
	    
	    Left Trigger:	Jump
	                      
	        
	    Left ≡ (menu)	Activate the PDA if it's in you inventory.
	            Will skip the active cutscene.
	            In a Flicksync, this gives up on the current line.
	            
	          
	  
	  RIGHT CONTROLLER:

	    By default the RIGHT controller controls the right hand and the weapon 
	    
	    Right Pad
	    Up:	      Next Weapon
	    
	    Right Pad
	    Down:	    Previous Weapon
	    
	    Right Pad
	    Left/Right:	  Snap turn 45 degrees left or right.
	    
	    Right Grip:	  Reload
	          
	    Right Pad
	    Press:	    Teleport
	    
	    Right Trigger:	Attack / Fire
	            Use Cue Card Power-Up in Flicksync
	    
	    Right ≡ (menu): Brings up the in game Pause/System menu on the PDA.
	            ( The PDA does not need to be in you inventory to use this function)
	


 _________________________________________	  
  Default Control Bindings: Gamepad


	You can play the game in VR using an XBox 360/One gamepad if you don't have motion controllers.
	The game will automatically switch to gamepad mode when you start using the gamepad.
	You can use the gamepad for room-scale if you want.

	    Left Stick: Movement. This can cause motion sickness, So it's recommended to 
	          teleport by holding B, looking where you want to go, and letting go.
	
	    Push left
	    stick in:	Run
	    
	    Left
	    trigger:	Toggle Flashlight

	
	    Right 
	    stick:	  Aim weapon, and will turn you if you aim past the deadzone threshold.
	    
	    Push right
	    stick in:	Crouch
	    
	    Right
	    trigger: 	Attack / Fire weapon
	    
	    Shoulder
	    buttons:	Switch weapons

	    A = jump
	    B = teleport
	    X = reload
	    Y = use
	    View / Back = PDA / Skip cutscene
	    ≡ / Start = menu.
	    
	  
 _________________________________________
  Default Control Bindings: Keyboard / Mouse


	You can play the game in VR using keyboard and mouse if you don't have motion controllers.
	The game will NOT automatically switch to gamepad mode when you start using the keyboard,
	so you might need to turn on gamepad mode manually in the VR options if your arms are weird.
	You can't use the mouse for room-scale yet because we haven't implemented head aiming.

	    WASD: Movement. This can cause motion sickness, So it's recommended to 
	          teleport by holding RMB, looking where you want to go, and letting go.
	    
	    Z/X:	Comfort turn 45 degrees left or right
	    C:	  	Crouch
	    Space:	Jump
	
	    Shift:	Run

	    E:	Use (not really needed)
	    R:  Reload
	    F:	Toggle Flashlight
	
	    Arrow 
	    Keys:	Aim weapon, and will turn you if you aim past the deadzone threshold.
	    
	    NumPad:	Comfort turn left/right 45 degrees
	    NumPad0:	Teleport
	    
	    LMB: 	Attack / Fire weapon
	    RMB:	Teleport (aim with head, not mouse)
	    MMB:	Reload
	    Mouse
	    Wheel:	Switch weapons
	    
	    ~:		Console
	    Q:		Soul Cube / Artifact
	    1-0:	Switch weapons
	    
	    Tab:	PDA / Skip cutscene
	    Esc:	menu
	    Pause:	Freeze program
	    F5:		Quick Save
	  
	_________________________________________	  
	Using the PDA: 
	  
	  You can move the PDA around in your hand, while using your right finger to touch the screen
	  to select entries or scroll bars. You can also use the sticks/pads and buttons on each hand 
	  to select options.

	  Use Y or grip to put your PDA away again.

	  If the tracking glitches, your PDA can end up far away. If this occurs,  repeatedly press Y or Grip 
	  to close the PDA. ( This is a known issue that will be addressed )
	  
	  You can also manually grab the PDA from your left hip using motion controls.
	  When you move the left controller over the correct spot, the right controller vibrates.
	  Use the grip button to pick up the PDA. Your flashlight will be temporarily holstered on your
	  hip while using the PDA.

	  When you are finished with the PDA, you can place it back the same way, and grab your flashlight.
	  ( Grabbing the PDA before you collect it from Reception brings up the pause menu instead of the PDA. )
	  
	_________________________________________	  
	Using the System/Pause menu:
	
	  Activating the System/Pause menu ( Press ≡ (menu) on the left Touch or Right Vive ) will pause the game
	  and display the system menus on the PDA as above. You can load/save/exit the game, Change the control
	  bindings via System->Controls->Key Bindings, or adjust the various VR options via VR Options. See the 
	  section on VR options for more details.

	  Touch the 'Back' options, or press Y or the Grips to go back/exit.
	  
	  
	_________________________________________	  
	Teleporting:
	
	  You can teleport anywhere you could normally reach by walking, ducking, or jumping onto things.
	  When you teleport, your character will trigger any in game events and take any environmental damage
	  that the player would incur by walking the route to the destination.
	  
	  You can't teleport across gaps you have to jump across, or onto the tightrope in the Mars City 
	  Underground level. There is a path length/complexity limit on how far you can teleport in one hop,
	  but it's fairly far. There are some bugs in the Player AAS files, and some levels are missing, so there are 
	  instances where you can't teleport places you should be able to. In these instances, walk in real life
	  or move forward with the stick/pad a little until you can teleport again. If you didn't install the Player
	  AAS files at install, you will only be able to teleport places a Zombie can go.
	  
	  To teleport, push in the right touchpad, or hold down B on Touch, then move your hand to aim the parabola
	  where you want to go. Release the stick or touchpad to teleport there. Pressing the trigger while the aiming
	  beam is active will cancel the teleport. If the teleport target is red, you will need to duck/crouch before
	  teleporting to the destination.	If there's no circle, that means you can't teleport there from your current
	  location.
	  
	  There are two teleportation modes. "Blink" teleports you instantly. "QuakeCon" slows time and warps you there.
	  
	_________________________________________	  
	Using Slots:
	
	  Slots are locations defined on the player body you can stash or retrieve weapons, the PDA, or 
	  the flashlight by using motion controls to grab/stash them.	When a motion controller is inside
	  of a slot area, the right controller will vibrate.  When the controller vibrates, press the 
	  grip button on the controller in the slot to grab/place/swap the item.
	      
	  You can manually place the flashlight on your left shoulder or your head using the slots, And then 
	  later retrieve it from those places the same way.

	  You can swap your hands by grabbing the holster on your right hip with your left hand. This will
	  transfer the flashlight to your right side and pick up any holstered weapon in your left.
	  Swapping hands will swap most of the controls, but does not swap the analog sticks/touchpads.


	  You can manually grab the holster on your right hip to holster your weapon.
	  If there's already a weapon holstered there, the weapons will swap.

	  You can grab behind your weapon hand's shoulder to switch to the next weapon.
	  You can grab behind your weapon hand's lower back for previous weapon.

	  You can also grab the PDA on your left hip with your right hand, if you prefer.
	  You can then put it back on the same side like normal, or holster PDA on the right to switch hands.
	  
	_________________________________________	
	Interacting with Guis:
	
	  To use a touchscreen/gui, walk up to the screen and look at it.  When you have focused on the gui, 
	  your weapon will lower and your hand will point a finger and raise to the gui. You can now reach out
	  and use the gui like a normal touchscreen.
	  
	  To exit the gui, walk backwards or teleport.


	
	_________________________________________	
	Motion Control Errata:
	
	Currently, you cannot use motion controls to pick up items in the environment, or to punch/melee attack.
	( These features are being worked on)

	If you are displaying the player body, and the arms look strange due to a bad IK pose, use the controller to
	move the character forward. This will make your body face the correct direction.
	If your body isn't attached to your neck properly, try jumping.  
	If things just seem out of alignment, or to switch from seated to standing mode, Recenter view/reset height
	with whichever button is defined (you must say "reset view" on Vive).


	

Voice Controls:

	You can use the built in microphone in the Rift or the Vive to activate voice commands using Windows built
	in Speech Recognition.
	
	If you want to use speech recognition to activate voice commands ( recommended ), it should be enabled in 
	Windows and trained for optimal performance.  Speech recognition settings are available in the Control Panel 
	under 'Speech recognition.' At a minimum run 'Set up microphone', and for optimal performance 'Train your 
	computer to better understand you' 
	
	The phrases recognized by the voice commands are defined in the Fully Possessed/dict/voice.dict file.  This file
	may be edited to change the phrases associated with each action.
	
	Available Voice Commands:
	
	Talk:
	
	    Speak to other people by walking up to them and speaking.
	    You can say whatever you want. After you finish talking, they will respond.
	    They will briefly look at you when you first start speaking.
	    Speaking can wake up nearby monsters, so be careful!
	    
	  Say: Anything
	  
	
	Recenter view:
	
	    This resets the view height to you current view position,
	    and makes the player and the view face your current view direction.
	    Useful for switching between sitting and standing mode of if the 
	    player body becomes disoriented.
	    
	  Say:
	      Reset View
	      Recenter View
	      Reset HMD Orientation
	      Reset HMD View
	      
	Run:
	  
	  Say:	Start Running
	      
	      
	Walk/Stop Running:
	
	  Say:	stop running
	      start walking
	
	Reload:
	
	  Say:	Reload
	  
	Flashlight:
	
	  Say:	Flashlight
	      Torch
	      
	PDA Select:

	  Say:	PDA
	      personal data assistant
	      
	  Say:	Cancel
		  
	System/Pause Menu
	
	  Say:	menu
	      system
	      system menu
	      computer, menu
	      computer, arch		  

	  Say:	Cancel
	      Exit Game
	      Return To Game
	      
	Weapon Selection:
	  
	  You can switch to a specific weapon by saying the name of that weapon.
	  Available Weapons:
	  
	  Fists:
	  
	    Say:	Fist
	        Fists
	        Hands
	        
	  Pistol:

	    Say:	Pistol
	    
	  Chainsaw
	  
	    Say:	Chainsaw
	        Beaver tooth
	        Beaver tooth chainsaw
	        Mixom beaver tooth
	        Mixom beaver tooth chainsaw
	      
	  Shotgun:
	  
	    say:	Shotgun
	        Pump action shotgun
	        Single barrel shotgun
	        
	  SuperShotgun:
	  
	    say:	Super shotgun
	        Double barrel shotgun
	        Combat shotgun
	        
	  Machine Gun:
	  
	    say:	Machine gun
	        Enforcer
	        M G 88
	        M G 88 enforcer
	        M G
	        
	  Chain Gun:
	  
	    say:	Chain gun
	        Mach 2 chain gun
	        UAC weapons division mach 2 chain gun
	        Saw
	        Mini gun
	        Gatling gun
	        
	  Grenade:
	  
	    say:	Grenade
	        Grenades
	        
	  Plasma Gun:
	  
	    Say:	Plasma gun 
	        Plasma rifle
	        
	  Rocket Launcher:
	  
	    Say:	Rocket Launcher
	    
	  BFG:
	  
	    Say:	BFG
	        BFG-9000
	        Bio force gun
	        Big fragging gun
	        Big freaking gun
	        Big fucking gun
	        
	  Soul Cube:
	  
	    Say:	Soul cube
	    
	  Artifact
	  
	    Say:
		    Artifact
	        The artifact
	        Heart of hell
	        Blood stone
	        
	  Grabber:
	  
	    Say:
		    Grabber
	        Ionized plasma levitator
	        IPL unit
	        Gravity gun
	

	    
	Holodeck Commands:

	  Say:
	      Computer, Freeze Program
	      Computer, Resume Program
	      Computer, Play Program
	      Computer, Run Program
	      Computer, Continue Program

	      Pause Game
	      Resume Game
	      Unpause Game
	      
		  Computer, Arch		  
	      Computer, Menu

	      Computer, End Program
	      Computer, Exit
	      Computer, Program Complete
	      Computer, Cancel Program

	      
	Speech Recognition Control:

	  Say:	What Can I Say
	      
	      Start Listening
	      Consecution
	      
	      Stop Listening
	      Consentient
	  
	  



	  
___________________________________________________

7) FLICKSYNC
__________________________________________


Flicksync is an experimental mode based on the book Ready Player One by Ernest Cline. 
In the book, players in a virtual environment participate in and act out the lines of their
favorite movies, and their virtual performances scored. The Flicksync mode is still a work 
in progress, so bugs are to be expected.

How Flicksync is played:
 
	In this just for fun mode, the aim is to act out the game's cutscenes by becoming a character
	in the game. You must say all of that character's lines exactly at the appropriate
	times. Points are earned by saying your lines correctly, clearly, and at the right time.  

	If you don't say the line in time, the other character will repeat the cue (with subtitles),
	"Final Dialog Warning" will appear, and the game will pause indefinitely waiting for you to say it.

	If you can't remember the line, pull the trigger to use a Cue Card Power-Up (if you have one).
	The Cue Card will show you what you need to say (phonetically) in white near the top.

	If you don't have a Cue Card Power-Up, or the speech recognition can't recognize the way you say it,
	you can press the PDA or menu button to give up that line and lose 10 points.

	After you give up twice in a row, it will say "Final Warning". A third failure is GAME OVER.

	Pressing the PDA or menu button while it's not waiting for a line, will skip the cutscene like normal.

	When you say the line correctly, you get 100 points.
	If you say the line clearly and at roughly the right time, you get another 50 bonus points.

	If you get 7 lines right in a row, you get another Cue Card Power-Up, up to a maximum of 5.

	There are no action bonuses yet, and you automatically follow the character's path.

	If the cutscene seems to finish, but it won't let you move, it's still waiting for you to say the 
	last line of the cutscene ( or to skip it ).

	When you have finished all the cutscenes you chose, you win, and it will say "Flicksync Complete".
	If you were playing in Cutscenes Only mode, then all the NPCs and monsters will disappear and you
	will go back to being yourself instead of your character. You can then walk around the empty level
	and use the level's exit to return to the main menu, or use the pause menu to exit.


Starting a Flicksync game:


	To start a Flicksync game, choose Campaign from the main menu, then select Flicksync.

	Choose which character you will be.

	Choose how many Cue Cards you start with.

	Choose whether to play just the Flicksync cutscene parts, or play the normal Doom 3 game
	with Flicksync mode for the cutscenes, or a non-Flicksync game with no cutscenes.
	The "no cutscenes" mode carries over into regular games, so don't leave it set to no cutscenes
	if you want cutscenes.
	
	Choose which scenes you want to play.
	  
	  You can play: 	All the cutscenes
	          Just the cutscenes for your character's expansion
	          All the scenes starting from your character's first scene,
	          All the scenes related to your character's story arc
	          Just the scenes with your character in them.
	  There's currently a bug where you must watch the first cutscene of a map if that
	  cutscene starts automatically on map startup, and you have a scene in a later
	  cutscene of that map.
	          
	Then choose the highest level you have reached in the game if you want to avoid spoilers.

	Choose Play.

	You will start from a different point depending on which character or scenes you choose.

	You can also watch the cutscenes from the POV of the player character(s) by choosing Player,
	or you can just watch all the cutscenes by setting Character to None and Game Mode to
	Cutscenes Only.

Flicksync settings carry over if you start a map from the DEV menu, so you may 
want to change them back to normal first.

Starting a new game from the Campaign / New game menu will clear the flicksync settings,
except for the "no cutscenes" option.
You will see SCORE: 0 on the menus if you are in Flicksync mode.



___________________________________________________

8) VR Options
__________________________________________


VR Related game changes can be made from the Settings->VR Options menu.

	Menu: Character Options:
	
	  Body Mode:  
	  
	    Full Body + Arms:
	      
	        The full player body will be displayed in game, with 
	        ( very ) basic IK for the arms.  While playing,
	        if the arms seem misaligned with the body, move in any direction
	        to align the body direction with the movement direction.  Alternatively,
	        you can press or say the button/command to reset the view. ( Left Y on Touch,
	        or say Reset View)
	        
	    Hands + Weaps:
	    
	        If you do not wish to see the player body ( perhaps the basic arm IK
	        is too basic :) ), select this option to display your hands and weapons only.
	        
	    Weapons Only:
	    
	        Selection this option will display only the weapons in game, no player body
	        or hands.  ( Hands will be displayed if there is currently no weapon selected, if 
	        the fists are selected, or if interacting with a GUI.)
	        
	    Note that Body Mode also affects Flicksync. But in Flicksync you will be fully invisible
	    if you chose Hands + Weaps or Weapons Only.
	  
	  
	  Flashlight Mount:
	  
	        Select where the flashlight will be mounted during the game.
	        
	        Options: 	Hand - 	Hold the flashlight in the non weapon hand.
	                  Flashlight can be aimed with motion controls.
	              
	              Body - 	The flashlight will be mounted to the body,
	                  and shine in the direction the body is facing.
	                
	              Head - 	The flashlight will be mounted to the head
	                  and shine where you are looking.
	                  
	              Weapon-	The flashlight will be mounted to the muzzle of
	                  your weapon.  In the event the weapon has no muzzle
	                  ( grenade, chainsaw, soul cube, etc ) the flashlight
	                  will temporarily move to your head.
	                  
	        You can change this in-game by manually grabbing your flashlight and placing
	        it where you want it (except for on the weapon), if Holster Slots are enabled.
	  
	  Weapon Hand
	      
	        Select the handedness of the game.
	        
	        Options:	Right - The weapon will default to the right hand,
	                  flashlight / PDA will be in the left.
	                  
	              Left - 	The weapon will default to the left hand,
	                  the flashlight / PDA will be in the right.
	                  
	        You can change this in-game by manually grabbing the holster on the opposite
	        side, if Holster Slots are enabled.
	                  
	  Holster Slots
	        
	        Enable or Disable the ability to use the holster slots located on your body.
	        
	        Options:	Enable
	        
	              Disable
	              
	  View Height
	  
	        Change the height of the player view.
	        Use this option if you are taller or shorter than the default view.
	        Measured in game units (approximately inches).
	        The actual height is five more than this.
	        This setting is now saved between maps and sessions.
	
	  Use Floor Height
	        
	        Enable:	The virtual floor will always be at the same height as the real floor.
	                This might make you feel like you're not as tall as a space marine should be.
	
	        Disable:	The player's view height (specified above) will be the same height as your real eyes.
	                    This might make you feel like you're floating above the floor.
	
	Menu: Comfort Options

	  This menu allows the player select options intended to combat motion sickness, either due
	  to locomotion or game effects. There are also options to help prevent damage to your controllers
	  when using motion controls by activating the Guardian or Chaperone grids as needed.
	  
	  
	  Teleport:
	  
	        Change teleport aiming
	        
	        Options:	Right Hand - 	The teleport aiming beam will originate from the 
	                      right hand, or device in the right hand.
	                      
	              Left Hand - 	The teleport aiming beam will originate from the 
	                      left hand, or device in the left hand.
	                      
	              Head - 	    Use you view to aim the teleport beam.
	              
	              Disabled -	  Disable teleport and teleport aiming.
	              
	              Gun Sight - 	Do not use the parabolic teleport aiming beam, instead
	                      use the weapon sight.  The laser reticle will change to 
	                      indicate if you are able to teleport to a location.
	                      You must set the weapon sight to dot mode to use this option.
	                      This option may also hurt frame rate, as the teleport destination
	                      is always being scanned.
	                      
	        If you swap hands using the weapons slots, the teleport aiming will swap as well.
	        
	        
	        
	  Teleport Mode:
	  
	        Choose whether to teleport instantly, or slow time and warp along the path.
	        
	        Options:
	              Blink - 	Aiming is like normal, teleporting is instant and covered by a blink.
	                      
	              QuakeCon - 	Time is slowed while aiming. Teleport by moving at warp speed to the destination.
				             This style was used in a Doom (2016) VR demo at QuakeCon.
							 http://www.gamespot.com/articles/doom-may-have-solved-vrs-traversal-problem/1100-6442439/
	                      
	  Turning:
	      
	        Change the turning mode. Changing the turning menu option will remap your control 
	        bindings for whatever controls you assigned to turning to support the selected mode.
	        If you want to use a combination of Analog and Comfort turning, you can adjust the
	        control bindings yourself in Settings->Controls->Key Bindings.
	        
	        This setting only affects motion controllers. Set Key Bindings for keyboard or gamepad.
	        
	        Options:	Snap ( 10 - 180 ) - Comfort turning will be used. Each turn will move the view/reset
	                      	the selected number of degrees.
	                      	
	              Real Life -	    No artificial turning will be used, the player will need to turn
	                      	in real life to turn in the game. 
	                      	
	              Analog - 	    The analog stick can be used to turn the player smoothly.
	                      	This option is guaranteed to cause the most motion sickness.
	                      	It is not recommended for the regular user, and should only be
	                      	considered by people who absolutely never get VR sickness.
	                      	
	                      	
	              
	  Walk Speed Adjust:
	  
	        Adjust the movement speed of the player.
	        
	        This value can be changed to increase or decrease the maximum walk speed
	        of the player from the normal value of 140. Negative values slow movement,
	        positive values will increase it. This value persists throughout the game.
	        There's an alternative to "Walk Speed Adjust" in the next option, which slows
	        time instead. If you opt to use that, you can set Walk Speed Adjust here to 0.
	        
	  
	  Motion Sickness Aid:

	        If you experience any form of VR sickness, this is one of the most important options
	        to experiment with. It applies an effect whenever you're moving artificially, whether 
	        using the joystick, jumping, crouching, or because you were pushed by something in the
	        game, or you fell off a ledge, or you tried to walk in real life through a solid virtual
	        object. Some effects can work in combination with other effects.
	                  
	        There are a variety of options to ( hopefully ) assist in preventing motion sickness 
	        during player movement. The best way to avoid motion sickness is to play standing up,
	        walking, turning, and crouching in real life, while using teleportation and third person 
	        mode for all artificial movement. You can play this way even if using a gamepad.
	  
	        The most common cause of motion sickness is using the analog stick to turn the view, so 
	        this should be avoided whenever possible if you are susceptible to VR sickness. There
	        are some places where you have to use artificial movement though, such as climbing ladders,
	        jumping across gaps, and some glitchy areas not accessible via teleport, so hopefully 
	        these menu options can help.
	        
	        Options:	Third Person - 	  When artificial motion is used, the player view will remain
	                      	stationary, and the player character will be controlled
	                      	in the third person via analog controls.  When the character
	                      	stops moving, the player view will fade to the new character
	                      	location. This is a very effective way to combat motion 
	                      	sickness.  There is	currently one drawback: Moving backwards
	                      	is not well implemented	(yet).  If you move backwards, you 
	                      	will not be able to see the character you are controlling 
	                      	unless you turn around, so it is recommended to	only move 
	                      	backwards in short hops.
	                      	
	              None - 	      If you are fortunate enough to never experience motion sickness,
	                      	you can select this option to disable all motion sickness aids.
	                      	Be warned, this is only for those who truly have 'VR legs'.
	                      	
	              Chaperone - 	  Turns on the Chaperone ( or Guardian ) grid whenever you are
	                      	moving artificially. Any grid that's solidly locked to the real
	                      	world is supposed to help with motion sickness. The other methods
	                      	may be more effective or less intrusive. Chaperone can be combined
	                      	with other effects ( including "Black Screen" if you don't like the
	                      	empty void).

	              Reduce Fov - 	  Reduce FOV, or "Tunnel vision" will block out your peripheral vision
	                      	when moving, which for some people is where most motion sickness comes
	                      	from. It can be combined with other effects such as Chaperone.
	                      	
	              
	              Black Screen - 	  The only 100% effect mode, it completely blacks out your view while
	                      	moving artificially, so there's no vection and no motion sickness.
	                      	It will strobe a brief frame a couple of times per second. ( The 
	                      	strobe time can be adjusted with the console variable vr_strobeTime
	                      	which is in ms). Obviously blacking out the screen makes it harder
	                      	to navigate and see what you're doing, but if you are not using 
	                      	artificial navigation much ( only when you cant teleport ) it can be 
	                      	useful.
	                      	
	              Black + Chaperone-	Combine the Black Screen and Chaperone aids.
	              
	              FOV + Chaperone- 	Combine FOV Reduction and Chaperone aids.
	              
	              Slo Mo-	      Slows time while you are moving, to make it feel like you are moving
	                      	slower, but without harming your ability to dodge and escape from 
	                      	monsters. The faster you move, the more time is slowed. The downside
	                      	of this mode is that all the sound and voices are slowed down. This
	                      	effect can be combined with Chaperone and/or Reduce FOV.

	              SloMo + Chaperone-	Combine Slo-Mo with Chaperone.
	              
	              Slo Mo + FOV-	  Combine Slo-Mo with FOV Reduction.
	              
	              Slo+FOV+Chaperone-	Combine Slo-Mo, FOV Reduction, and Chaperone effects.
	              
	              
	  [Chaperone:]
	      
		  This setting has moved to the VR Options > Rendering Options menu.
		  
	  Knockback and Head Kick:
	  
	      Knockback and Headkick are Doom 3's effects that push your view. Knockback is when 
	      your entire player body is moved due to impact. Head Kick is when your view 'Kicks'
	      due to an impulse such as firing a weapon or being struck. They don't work well in VR,
	      and can contribute to motion sickness, so they are disabled by default. They can be 
	      enabled if desired.
	      
	        Options:	Knockback Only-	  The player body will be knocked back due to impacts.
	        
	              Headkick Only-	  The player view can kick due to impulses.
	              
	              Enabled-	    Both Knockback and Headkick will be enabled.
	              
	              Disabled-	    Neither Knockback or Headkick are enabled.
	              
	  Step Smooth and Jump Bounce:
	  
	      Affect the player view when going up and down steps, or after landing from a jump or fall.
	      
	        Options:
	        
	              Step Smooth Only-	Step Smooth makes climbing stairs smooth instead of jarring.
	                      	Smooth motion increases motion sickness, but in this case can 
	                      	be jarring without it. Enabled by default, but can be turned 
	                      	off if you prefer. This option enables smooth steps and 
	                      	disables jump bouncing.
	                      	
	              Jump Bounce Only-	Jump Bounce is when your view bounces when landing after jumping.
	                      	It can be uncomfortable, so it's disabled by default, but this option
	                      	enables the jump bounce but not smooth stepping.
	                      	
	              Enabled-	    Steps will be smooth and the view will bounce when landing.
	              
	              Disabled-	    Steps will be instant, and the view will not bounce when landing.
	              
	  Shake Amplitude:
	  
	      Doom 3 has screen shaking effects in some areas of the game. You can adjust this amplitude of these
	      effects. Moving the slider all the way to the left disables the effect completely, and moving the slider
	      to the right will increase the effect until it reaches full power.
	      
	      This effect can be very uncomfortable, so it is disabled by default.
	      
	      
	      
	              
	Menu: Control Options:
	
	
	  Options for how the player controls the game.
	  
	  Controller Type:
	  
	      Select the default controller type.
	      
	        Options:
	        
	            Motion Controllers-	  Motion controls will be used if present
	            
	            Standard Controller-	A gamepad (or keyboard and mouse) will be the default control device.
	            
	      I recommend leaving it set on Motion Controllers, because in that mode it will switch
	      to gamepad automatically when you start using it, and switch back if you use motion.
	      
	  Move Mode:
	  
	      Select the default movement mode:
	      
	        Options:
	        
	            Standard Stick Move-	The joysticks or pads will control the character
	                      	like a standard gamepad.  Pressing forward will move
	                      	the player forward in relation to the body.
	                      	
	            Off Hand = Forward-	  Forward motion is relative to the direction the off hand
	                      	controller is pointing. ( 'Onward' style movement. )
	                      	
	            Look = Forward-	    Forward motion is relative to the direction the player is 
	                      	looking.
	                      	
	  Crouch Mode:
	  
	      Select the default crouch mode.  Note that at any time crouching may be accomplished by using
	      the button assigned to crouch.
	      
	        Options:
	        
	            Full Motion Crouching-  The player can crouch in real life to crawl through ducts
	                      	or narrow passages.
	                      	
	            Crouch Trigger-	    If the player in real life crouches a distance greater than
	                      	the number of inches defined by 'Crouch Trigger', it will act
	                      	as if the crouch button has been pressed.
	                      	
	  Crouch Trigger Dist:	The number of inches they player must crouch in real life to/from
	              trigger a full in game crouch when using 'Crouch Trigger' mode.
	              
	  Weapon Pitch:	    You can change this value to adjust the default pitch of the 
	              weapon to suit your preference.
	              
	  Flashlight Pitch:	  You can change this value to adjust the default pitch of the 
	              flashlight to suit your preference.
	  
	  Talk Mode:

	      Select how the player interacts with NPCs.
	      
	        Options:
	            
	            Voice Only - 	    The player must speak in real life to talk to NPCs.
	                      	The focus cursor will be displayed.
	                      	
	            Voice, No Cursor-	  The player must speak in real life to talk to NPCs.
	                      	The focus cursor will not be displayed.
	                      	
	            Buttons Only-	    The player must use buttons ( attack ) to talk to NPCs.
	            
	            Buttons or Voice-	  The player may speak in real life or use buttons to 
	                      	speak with NPCs.
	                      	
	  Voice Commands:
	  
	      Select how Speech Recognition will be used to implement commands.
	      
	        Options:
	        
	            Menus and Weapons-	  Speaking will allow the player to enter and exit menus,
	                      	as well as select menus.
	                      	
	            Disabled-	      Voice Commands will not be utilized.
	            
	            Menus Only-	      Voice Commands will only activate menus, weapons will 
	                      	need to be selected manually.
	                      	
	Menu: UI Options:

	  Select user interface options.
	  
	  HUD Options Menu:
	  
	    Modify how the HUD is displayed.
	    
	    Location:	        
	        
	      Where the HUD will be positioned.
	                      	
	        Options:

	            Lock to Body-	    The hud position is relative to the player body.
	            
	            Lock to View-	    The hud position is relative to the player view.
	            
	    Hud Mode:
	    
	      When the hud is displayed.
	      
	        Options:
	        
	            Pitch Activate-	    The HUD will remain invisible until the player tilts their
	                      	head ( looks down ) beyond the angle defined by Reveal Angle.
	                
	            Disabled-	      The HUD will not be displayed.	
	            
	            Full On-	      The HUD will always be displayed.
	            
	    Reveal Angle:
	    
	        If the HUD is in Pitch Activate mode, tilting your head beyond this angle 
	        will cause the hud to be displayed.
	        
	    Low Health Reveal:
	    
	        If the HUD is in Pitch Activate mode, it will be switched on regardless of tilt if
	        player health drops below the value defined here. ( 0 = Disabled ) 
	        
	    Menu: Adjust Hud Position:

	        Options for changing the location and size of the HUD.
	        Values are in inches
	        
	        HUD Position Dist:  How far in front of the view or body the HUD will be displayed.
	        
	        HUD Position Vert: 	How far up or down from default in relation to the view or body
	                  the HUD will be displayed.
	                  
	        HUD Position Horiz:	How far to the left or right from default in relation to the view
	                  or body the HUD will be displayed.
	                  
	        HUD Pitch:	    The pitch of the HUD screen.
	        
	        HUD Scale:	    Size of the Hud
	        
	        Talk Cursor Adj:    Adjust the position of the Talk Cursor ( Cursor that identified the
	                  names of NPC as a % of HUD vertical size.)
	                  
	            
	  PDA Options Menu:

	    Modify the PDA position.
	    
	    PDA Location:
	    
	      Where the PDA will be displayed
	        
	        Options:
	        
	          Hold in Hand-	  When active, the PDA will be held in the non weapon hand,
	                    and can be moved via motion controls.
	          
	          Fix in Space-	  When Active, the PDA will be fixed in space in front of
	                    the player and will not move.
	                    
	          PDA Position Dist:  How far in front of the view or body the PDA will be displayed
	                    if Fix In Space is selected.
	        
	          PDA Position Vert: 	How far up or down from default in relation to the view or body
	                    the PDA will be displayed if Fix In Space is selected.
	                  
	          PDA Position Horiz:	How far to the left or right from default in relation to the view
	                    or body the PDA will be displayed if Fix In Space is selected.
	                  
	          PDA Pitch:	    The pitch of the PDA screen if Fix In Space is selected.
	                      	
	            
	            

	  Heading Beam:
	  
	    The heading beam is an optional directional indicator that may aid the player when controlling the
	    character.
	    
	      Options:
	      
	          Disabled-	    The heading beam will not be displayed.
	          
	          Solid-	      A solid heading beam will be displayed.
	          
	          Arrows-	      A heading beam with arrows will be displayed.
	          
	          Scrolling Arrows-	A heading beam will scrolling arrows will be displayed.
	          
	  Weapon Sight:
	  
	    Various weapon sights are available.
	    
	      Options:
	          Disabled-	  No laser sight.
	      
	          Laser Beam- 	  A laser beam will emit from the weapon to aim with.
	          
	          Red Dot-	    A red laser dot will be projected into the world to aim with.
	          
	          Circle Dot-	    A reticle with a red circle with a dot in the center will
	                    be projected into the world to aim with.
	          
	          Crosshair-	    A red crosshair will be projected into the world to aim with.
	          
	          Laser + Dot-	    Both the dot and the laser beam.
	          
	          Laser + Circle-	    Both the circle dot and the laser beam.
	          
	          Crosshair + Circle-	    Both the crosshair and the laser beam.
	          
	  Sight to Surface:
	  
	    If using one of the reticles instead of the Laser Sight, enabling this will align the 
	    reticle to the surface it hits.
	
	  Laser Sight Source:
	    
	    Choose whether the laser sight comes out of the barrel, or an emitter elsewhere on the weapon.
	    If the sight comes from somewhere else, you have to manually compensate an inch or two when you aim.
	    
	      Options:
	        Laser Emitter-	  The laser sight comes from a different part of the gun, but is parallel to where the bullets will go.
	                          You must take note of where the barrel is actually aiming relative to the laser for perfect shots.
	 
	        Barrel-	The laser comes out of the gun's barrel and shows exactly
	                where the bullets will go. This makes aiming easier, but
	                is less realistic.
	    
	  Haptic Feedback:
	  
	    Enable or disable haptic feedback on the game controller.
	    
	  GUI mode:
	  
	    Select how in game GUIS will be interacted with.
	    
	      Options:
	      
	          Touch Activated-	When the player gets close to a GUI and looks at it,
	                    the weapon will lower and the weapon hand will change
	                    to a pointer finger.  The GUI can be used as a virtual 
	                    touchscreen.
	                    
	          Aim Activated-	  When the player approaches a GUI and aims the weapon at it,
	                    the weapon will lower, but aiming the weapon will control the
	                    cursor on the GUI screen.  Fire will click the screen.  If the 
	                    current weapon doesn't have a muzzle ( grenade, fists, chainsaw,
	                    soul cube, artifact), the player gaze will control the cursor.
	                    
	          Look Activated-	  When the player approaches a GUI and looks at it, the weapon will
	                    lower and the player look direction will be used to control the cursor.
	                    
	                    
	Menu: Rendering Options:
	
	  Options that affect VR graphics.

	  Pixel Density - A multiplier used to increase the resolution of the rendered image.
	          Increasing this setting will dramatically impact performance.
	          Note that this setting will be used ON TOP OF any setting in your HMD
	          drivers.  If you are using some form of supersampling by default in 
	          your HMD software, you may want to set this to 1.
	          
	  MSAA Level-	  Disable MSAA, or set the desired level of sampling.
	  
	  Chaperone:
	      
		  This setting has moved here from the VR Options > Comfort + Safety Protocols.
		  
	      This setting is more about safety than motion sickness. It lets you turn the Chaperone or Guardian
	      grid on all the time, or only when you're throwing grenades, or when you're swinging around virtual
	      Melee weapons. The grid can't be disabled completely, but it can be set to "Near" for normal chaperone
	      behaviour. I broke my TV throwing things in Budget Cuts, so I recommend forcing the chaperone on when
	      throwing, at least. The options from most reckless to most safe are: "Near", "Throw", "Melee", or "Always".
	      The default is "Melee", which means the grid will be on whenever you're using grenades, the grabber,
	      the chainsaw, or fists.
	      
	        Options:	Melee-	      The Chaperone or Guardian grid will be on when using melee 
	                      	weapons. ( Grenades, grabber, chainsaw, or fists.)
	                      	
	              Always-	      The Chaperone or Guardian grid will always be on.
	              
	              Near-	      Normal Chaperone or Guardian behavior, the grid will
	                      	appear when you or your controllers are too close to
	                      	the virtual boundary.
	                      	
	              Throwing- 	    On when throwing grenades.

	  Asynchronous SpaceWarp:
	
	    Oculus only! Adjust how the game handles low framerates.
	      
	        Options:
	              Default - Don't mess with the Oculus default.
	              Disabled - use Asynchronous TimeWarp (works at any FPS) instead of Asynchronous SpaceWarp (recommended).
	              Enabled - use Asynchronous SpaceWarp at 45 FPS whenever FPS drops below 90 FPS.
	              45 FPS ATW - Always use Asynchronous TimeWarp, locked to 45 FPS
	              45 FPS ASW - Always use Asynchronous SpaceWarp, locked to 45 FPS
	              
	  3D Guis-	  The in game GUIS can look flat - enabling this setting adds some
	          depth.  May be disabled if effect is disagreeable.
	          
	          
	              
	Menu: VR Profile Options:

	  Active Profile:
	  
	    Select profile.
	    
	      Options:
	      
	          Official Profile-	Use the IPD and height provided by the HMD runtime.
	                    
	          Manual Profile-	Use the IPD and Height defined in the Manual Profile entries.
	          
	  Oculus Profile IPD:	The IPD reported by the HMD runtime. 
	  
	  Oculus Profile Height:	The Height reported by the HMD runtime. 
	  
	  Manual Profile IPD:	This IPD will be used if Active Profile is set to Manual.
	  
	  Manual Profile Height:	This height will be used if Active Profile is set to Manual.
	  
	  World Scale Adjust:	This works now. Multiply the size of the world by this. 
	  

___________________________________________

9) CONSOLE VARIABLES AND COMMANDS
__________________________________________

Access the console with the ~` key, or you can add these to your config file in:
%UserProfile%\Saved Games\id Software\DOOM 3 BFG\Fully Possessed\vr_oculus.cfg
(or vr_openvr.cfg)

Commands:

	restartMap - Restart the map, preserving your inventory. Use this if you get stuck, or you load a game from another mod
	             and you were near the start, to get rid of the glitches. Or use this if the glitches make it impossible to
	             continue.

	endLevel - Win this map and continue to the next level, with all your inventory and the level's important items.
	           Use this if you get stuck near the end of a map, due to glitches. I consider this to be sort of cheating.

	noclip - Toggle no-clipping mode, in case you accidentally teleport somewhere you can't get out of

	listDevices - lists all detected audio playback devices. ( use cvar s_device to select and audio device from this list )

	showDeviceInfo - display detailed information about current audio playback device.

	r_listVoiceCommands - list all phrases defined for speech recognition


Below are some console variables used by the game.  Many of these can be adjusted via the in game menus.

vr_chibi 1 FLOAT - changes the size of human heads (sometimes good if you change the world scale but it makes heads look weird).
vr_scale 0.93 FLOAT - scale of the virtual world. 1 = scale of previous version and original Doom 3.
vr_useFloorHeight 0 BOOL - keep the virtual floor level with the real floor.
pm_normalviewheight 68 FLOAT - 5 game-inches less than the player view height.
vr_laserSightUseOffset 1 BOOL - Emit laser sight from the gun's laser sight emitter instead of the gun's barrel.

com_fixedTic 0 BOOL - run a single game frame per render frame. Fixes frame dropping in VR.
vr_asw 0 INTEGER - Oculus Asynchronous SpaceWarp. 0 = force off (hack), 1 = enabled, 2 = 45 FPS ATW, 3 = 45 FPS ASW

com_showFPS 0,1,2 : Show frames rendered per second. 0: off 1: default bfg values, 2: only show FPS (classic view)
vr_comfortRepeat 100 INTEGER - Delay in MS between repeating comfort snap turns.
vr_joyCurves 0 INTEGER - Joy powercurves. 0 = Doom default  1 = Mixed curve with vr_joyCurveSensitivity and vr_joyCurveLin 
vr_joyCurveSensitivity 9 FLOAT - Sensitivity val 0 - 9 
vr_joyCurveLin 4 FLOAT - Linear point for joyCurves  sens < lin = power curve , sens = lin = linear , sens > lin = frac power curve. 
vr_grabberBeamWidth 4 INTEGER - Beam width for grabber in VR. Default 4.
vr_guiFocusPitchAdj 7 FLOAT - View pitch adjust to help activate in game touch screens
vr_slotDebug 0 BOOL - slot debug visualation
vr_slotMag 0.1 FLOAT - slot vibration magnitude (0 is off)
vr_slotDur 18 INTEGER - slot vibration duration in milliseconds
vr_slotDisable 0 BOOL - disable using slots.
vr_blink 1 FLOAT - Darkens the screen when head bumps walls and objects.
vr_throwPower 4.0 FLOAT - Throw power when motion throwing objects. Higher power will throw farther with same motion.
vr_guiH unused
vr_guiA unused
vr_rumbleDiv 1 FLOAT - rumble divisor - no need to adjust
vr_rumbleSkip 1 FLOAT - frames to skip - no need to adjust
vr_rumbleEnable 1 BOOL - Enable VR controller rumble 
vr_openVrStuckPadAxisFix 1 BOOL - Check for openVR controller stuck pad axis.
vr_openVrStuckPadAxisFixThresh 12 INTEGER - # of identical non zero input polls before axis flagged as stuck.
vr_mouseCapture 0 INTEGER - Constrain the mouse to the game window while running if true.
vr_debugTouchCursor 0 BOOL - Show the cursor position when using touch screen mode.
vr_pixelDensity 1.25 FLOAT - VR supersampling level.  If the Oculus Runtime or OpenVR have been configured to provide supersampling, set this to 1!
vr_vignette 1 INTEGER - unused
vr_enable 1 INTEGER - Enable VR mode. 0 = Disabled 1 = Enabled.
vr_FBOscale 1.0 FLOAT - unused
vr_useOculusProfile 1 INTEGER - Use official Profile values. 0 = use user defined profile, 1 = use official profile.
vr_manualIPDEnable 0 INTEGER - Override the HMD provided IPD value with value in vr_manualIPD 0 = disable 1= use manual iPD 
vr_manualIPD 64 FLOAT - User defined IPD value in MM
vr_manualHeight 70 FLOAT - User defined player height in inches
vr_minLoadScreenTime 6000 FLOAT - Min time to display load screens in ms. 0.0f, 10000.0f 
vr_clipPositional 1 BOOL - Clip positional tracking movement . 1 = Clip 0 = No clipping. 
vr_armIKenable 1 BOOL - Enable IK on arms when using motion controls and player body is visible.  1 = Enabled 0 = disabled 
vr_weaponHand 0 INTEGER - Which hand holds weapon.  0 = Right hand  1 = Left Hand  0, 1 
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
vr_forward_keyhole 11.25 unuser
vr_PDAfixLocation 0 BOOL - Fix PDA position in space in front of player  instead of holding in hand.
vr_weaponPivotOffsetForward 3 FLOAT - weaponPivotOffset CVars define the position of the weapon hand when using a gamepad
vr_weaponPivotOffsetHorizontal 0 FLOAT - 
vr_weaponPivotOffsetVertical 0 FLOAT - 
vr_weaponPivotForearmLength 16 FLOAT - ;
vr_guiScale 1 FLOAT - ( dont change ) scale reduction factor for full screen menu/pda scale in VR 0.0001f, 1.0f  //koz allow scaling of full screen guis/pda
vr_guiSeparation .01 FLOAT -( dont change ) Screen separation value for fullscreen guis.
vr_guiMode 2 INTEGER - Gui interaction mode.  0 = Weapon aim as cursor  1 = Look direction as cursor  2 = Touch screen 
vr_hudScale 1.0 FLOAT - Hud scale 0.1f, 2.0f 
vr_hudPosHor 0 FLOAT - Hud Horizontal offset in inches
vr_hudPosVer 7 FLOAT - Hud Vertical offset in inches
vr_hudPosDis 32 FLOAT - Hud Distance from view in inches
vr_hudPosAngle 30 FLOAT - Hud View Angle
vr_hudPosLock 1 INTEGER - Lock Hud to:  0 = Face, 1 = Body
// The following cvars allow individual hud elements to be enabled or disabled.
vr_hudType 2 INTEGER - VR Hud Type. 0 = Disable. 1 = Full 2 = Look Activate 0, 2 
vr_hudRevealAngle 48 FLOAT - HMD pitch to reveal HUD in look activate mode.
vr_hudTransparency 1 FLOATHud transparency. 0.0 = Invisible thru 1.0 = full 0.0, 100.0 
vr_hudOcclusion 1 BOOLHud occlusion. 0 = Objects occlude HUD, 1 = No occlusion 
vr_hudHealth 1 BOOL - Show Armor/Health in Hud.
vr_hudAmmo 1 BOOL - Show Ammo in Hud.
vr_hudPickUps 1 BOOL - Show item pick ups in Hud.
vr_hudTips 1 BOOL - Show tips Hud.
vr_hudLocation 1 BOOL - Show player location in Hud.
vr_hudObjective 1 BOOL - Show objectives in Hud.
vr_hudStamina 1 BOOL - Show stamina in Hud.
vr_hudPills 1 BOOL - Show weapon pills in Hud.
vr_hudComs 1 BOOL - Show communications in Hud.
vr_hudWeap 1 BOOL - Show weapon pickup/change icons in Hud.
vr_hudNewItems 1 BOOL - Show new items acquired in Hud.
vr_hudFlashlight 1 BOOL - Show flashlight in Hud.
vr_hudLowHealth 0 INTEGER0 = Disable, otherwise force hud if heath below this value.

vr_voiceRepeat 0 BOOL - 1 = computer speaks back whatever commands or lines you say
vr_voiceCommands 2 INTEGER - Enable voice commands. 0 = none, 1 = menus, 2 = menus and weapons
vr_talkWakeMonsters 1 INTEGER - Talking wakes monsters. 0 = no, 1 = both methods, 2 = like flashlight, 3 = like weapon. This was broken before but is now fixed.
vr_talkWakeMonstersRadius - radius in inches within which talking quietly will wake enemies. Talking louder wakes monsters further than this.
vr_talkMode 2 INTEGER - Talk to NPC 0 = buttons, 1 = buttons or voice, 2 = voice only, 3 = voice no cursor
vr_tweakTalkCursor 25 FLOAT - Tweak talk cursor y pos in VR. % val 0 to 99 ( talk cursor is NPC name display ) 
vr_flicksyncCharacter 0 INTEGER - Flicksync character. 0 = none, 1 = Betruger, 2 = Swan,
					3 = Campbell, 4 = DarkStar, 5 = Tower, 6 = Reception, 7 = Kelly, 8 = Brooks, 
					9 = Mark Ryan, 10 = Ishii, 11 = Roland, 12 = McNeil, 13 = Marine w PDA, 
					14 = Marine w Torch, 15 = Point, 16 = Bravo Lead, 17 = Player

vr_flicksyncCueCards 0 INTEGER - How many Cue Card Power-Ups to start with. Default = 0, max = 5
vr_cutscenesOnly 0 INTEGER - Skip action and only show cutscenes. 0 = normal game, 1 = cutscenes only, 2 = action only
vr_flicksyncScenes 1, INTEGER - 0 = all scenes, 1 = my chapter, 2 = from my start, 3 = my storyline, 4 = my scenes only
vr_flicksyncSpoiler 0 INTEGER - Don't show any cutscene past this point. 0 = allow spoilers, cutscene number = limit
g_stopTime 0 BOOL - Freeze time. This is used by the Flicksync to pause cutscenes. If there's a bug and everything is frozen try setting it to 0.

vr_wristStatMon 1 INTEGER - Use wrist status monitor. 0 = Disable 1 = Right Wrist 2 = Left Wrist 
vr_listMonitorName 0 BOOL - List monitor name with resolution. ( no longer as useful with direct mode)
vr_disableWeaponAnimation 1 BOOL - Disable weapon idle animations in VR. ( 1 = disabled )
vr_headKick 0 BOOL - Damage can 'kick' the players view. 0 = Disabled in VR.
vr_joystickMenuMapping 1 BOOL - Use alternate joy mapping  in menus/PDA.  0 = D3 Standard  1 = VR Mode. (Both joys can nav menus,  joy r/l to change select area in PDA.
vr_deadzonePitch 90 FLOAT - Vertical Aim Deadzone 0, 180 ( deadzone used with gamepad mode to define how far you can aim before view moves.)
vr_deadzoneYaw 30 FLOAT - Horizontal Aim Deadzone 0, 180 
vr_comfortDelta 10 FLOAT - Comfort Mode turning angle  0, 180 
vr_headingBeamMode 3 INTEGER - 0 = disabled, 1 = solid, 2 = arrows, 3 = scrolling arrows
vr_weaponSight 0 INTEGER - Weapon Sight.  0 = Lasersight  1 = Red dot  2 = Circle dot  3 = Crosshair 
vr_weaponSightToSurface 1 INTEGER - Map sight to surface. 0 = Disabled 1 = Enabled 
vr_motionWeaponPitchAdj 40 FLOAT - Weapon controller pitch adjust  ( use to change the default pitch of the weapon to your liking )
vr_motionFlashPitchAdj 40 FLOAT - Flashlight controller pitch adjust ( use to change the default pitch of the flashlight to your liking )

vr_nodalX -3 FLOAT - Forward offset from eyes to neck  				( In the real world, if you stand still and tilt your head, you don't want the character
vr_nodalZ -6 FLOAT - Vertical offset from neck to eye height		body to move.  Nodal X and X are used to define the base of the neck based on head
																	orientation.  If you stand still in the game, and tilting your head moves the player
																	you can adjust these so this won't happen.)

//	vr_vcx,y,and z define the distance from the tracked point on the controller to the center of the controller handle.  
	will be different for Oculus or Vive.
vr_vcx -3.5 FLOAT - Controller X offset to handle center // these values work for steam
vr_vcy 0 FLOAT - Controller Y offset to handle center
vr_vcz -.5 FLOAT - Controller Z offset to handle center

vr_mountx 0 FLOAT - If motion controller mounted on object, X offset from controller to object handle.  (Eg controller mounted on Topshot)
vr_mounty 0 FLOAT - If motion controller mounted on object, Y offset from controller to object handle.  (Eg controller mounted on Topshot)
vr_mountz 0 FLOAT - If motion controller mounted on object, Z offset from controller to object handle.  (Eg controller mounted on Topshot)
vr_mountedWeaponController 0 BOOL - If physical controller mounted on object (eg topshot), enable this to apply mounting offsets 0=disabled 1 = enabled
vr_3dgui 1 BOOL - 3d effects for in game guis. 0 = disabled 1 = enabled 
vr_shakeAmplitude 1.0 FLOAT - Screen shake amplitude 0.0 = disabled to 1.0 = full  0.0f, 1.0f 
vr_controllerStandard 0 INTEGER - If 1, use gamepad, not motion controllers Restart after changing 
vr_padDeadzone .25 FLOAT - Deadzone for steam pads.  0.0 = no deadzone 1.0 = dead 
vr_padToButtonThreshold .7 FLOAT - Threshold value for pad contact to register as button press  .1 high sensitiveity thru  .99 low sensitivity
vr_knockBack 0 BOOL - Enable damage knockback in VR. 0 = Disabled, 1 = Enabled
vr_jumpBounce 0 FLOAT - Enable view bounce after jumping. 0 = Disabled, 1 = Full 0.0f, 1.0f  // Carl
vr_stepSmooth 1 FLOAT - Enable smoothing when climbing stairs. 0 = Disabled, 1 = Full 0.0f, 1.0f  // Carl
vr_walkSpeedAdjust -20 FLOAT - Player walk speed adjustment in VR. (slow down default movement)
vr_wipPeriodMin 10.0 FLOAT - unused
vr_wipPeriodMax 2000.0 FLOAT - unused
vr_wipVelocityMin .05 FLOAT - unused
vr_wipVelocityMax 2.0 FLOAT - unused
vr_headbbox 10.0 FLOAT - unused
vr_pdaPosX 20 FLOAT - When using the PDA in fixed mode, instead of freehand mode, these values position the PDA in relation to the player.
vr_pdaPosY 0 FLOAT - 
vr_pdaPosZ -11 FLOAT - 
vr_pdaPitch 30 FLOAT - 
vr_movePoint 0 INTEGER - If enabled, move in the direction the off hand is pointing. ( enabled = Onward style movement )
vr_moveClick 0 INTEGER0 = Normal movement.  1 = Click and hold to walk, run button to run.
											2 = Click to start walking, then touch only. Run btn to run. 
											3 = Click to start walking, hold click to run.  
											4 = Click to start walking, then click toggles run 
vr_playerBodyMode 0 INTEGER - Player body mode: 0 = Display full body 1 = Just Hands  2 = Weapons only 
vr_bodyToMove 1 BOOL - Lock body orientaion to movement direction.
vr_crouchMode 0 INTEGER - Crouch Mode:  0 = Full motion crouch (In game matches real life)  1 = Crouch anim triggered by smaller movement.
vr_crouchTriggerDist 10 FLOAT - Distance ( in inches ) player must crouch in real life to toggle crouch if crouchmode set to 1
vr_frameCheck 0 INTEGER - 0 = bypass frame check
vr_forceOculusAudio 1 BOOL - Request openAL to search for Rift headphones instead of default device. Fails to default device if rift not found.
vr_stereoMirror 1 BOOL - Render mirror window with stereo views. 0 = Mono , 1 = Stereo Warped
vr_APISelect 0 INTEGER - VR API Select:  0 = Autodetect ( Oculus Native then OpenVR ) ,  1 = Oculus Native Only  2 = OpenVR only 
vr_teleport 2 INTEGER - Player can teleport at will. 0 = disabled, 1 = gun sight, 2 = right hand, 3 = left hand, 4 = head 0, 4 
vr_teleportMode 0 INTEGER - Teleport Mode. 0 = Blink (default), 1 = QuakeCon style (slow time and warp speed)
vr_teleportMaxTravel 950 INTEGER - Maximum teleport path length/complexity/time. About 250 or 500 are good choices, 
									but must be >= about 950 to use tightrope in MC Underground. 150, 5000 
vr_teleportThroughDoors 0 BOOL - Player can teleport somewhere visible even if the path to get there takes them through closed (but not locked) doors.
ai_debugMove 0 BOOL - Debug teleporter and monster navigation by showing teleport path
vr_teleportSkipHandrails 0 INTEGER - Teleport aim ingnores handrails. 1 = true
vr_teleportShowAimAssist 0 INTEGER - Move telepad target to reflect aim assist. 1 = true
vr_teleportButtonMode 0 BOOL - 0 = Press aim, release teleport. 1 = 1st press aim, 2nd press teleport
vr_teleportHint 0 BOOL - internal use.
vr_teleportVel 650 FLOAT ( these teleport cvars used to define the shape of the teleport aiming beam )
vr_teleportDist 60 FLOAT 
vr_teleportMaxPoints 24 FLOAT - 
vr_teleportMaxDrop 360 FLOAT - 
vr_motionSickness 10 INTEGER - Motion sickness prevention aids. 0 = None, 1 = Chaperone, 2 = Reduce FOV, 3 = Black Screen, 
																4 = Black & Chaperone, 5 = Reduce FOV & Chaperone, 6 = Slow Mo, 
																7 = Slow Mo & Chaperone, 8 = Slow Mo & Reduce FOV, 
																9 = Slow Mo, Chaperone, Reduce FOV, 10 = Third Person, 11 = Particles,
																12 = Particles & Chaperone 0, 12 
vr_strobeTime 500 INTEGER - Time in ms between flashes when blacking screen. 0 = no strobe
timescale 1.0 FLOAT - game speed multiplier. Used by slow mo motion sickness fix. If there's an issue with it, you may need to set timescale 1
vr_chaperone 2 INTEGER - Chaperone/Guardian mode. 0 = when near, 1 = when throwing, 2 = when melee, 3 = when dodging, 4 = always 0, 4 
vr_handSwapsAnalogs 0 BOOL - Should swapping the weapon hand affect analog controls (stick or touchpad) or just buttons/triggers? 0 = only swap buttons, 1 = swap all controls
vr_autoSwitchControllers 1 BOOL - Automatically switch to/from gamepad mode when using gamepad/motion controller. 
								Should be true unless you're trying to use both together, or you get false detections. 0 = no, 1 = yes.


r_useShadowMapping 0 - Use soft shadow mapping instead of hard stencil shadows

___________________________________________________

10) KNOWN ISSUES
__________________________________________

See https://github.com/KozGit/DOOM-3-BFG-VR/issues

Doom 3 wasn't designed to work with shadow maps so:

- Some lights cause shadow acne with shadow mapping
- Some shadows might almost disappear due to the shadow filtering

____________________________________________

11) GETTING THE SOURCE CODE
___________________________________

This project's GitHub.net Git repository can be checked out through Git with the following instruction set: 

	> git clone https://github.com/KozGit/DOOM-3-BFG-VR.git

If you don't want to use git, you can download the source as a zip file at
	https://github.com/KozGit/DOOM-3-BFG-VR/archive/master.zip



___________________________________________________________________

12) COMPILING ON WIN32 WITH VISUAL C++ 2013 EXPRESS EDITION
__________________________________________________________

1. Download and install the Visual C++ 2013 Express Edition.

2. Download and install the DirectX SDK (June 2010) here:
	http://www.microsoft.com/en-us/download/details.aspx?id=6812

3. Download and install the latest CMake, saying YES to adding CMake to your path.

4. Generate the VC13 projects using CMake by doubleclicking a matching configuration .bat file in the neo/ folder.

5. Go to https://developer.oculus.com/downloads/pc/1.9.0/Oculus_SDK_for_Windows/ then download and extract it somewhere. 
	Copy the LibOVR folder to DOOM-3-BFG/neo/libs

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

13) COMPILING ON GNU/LINUX
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

___________________________________________________

14) BUG REPORTS
__________________________________________

DOOM-3-BFG VR: Fully Possessed is not perfect, it is not bug free just like all other software.
To fixing as many problems as possible, we need as many bug reports as possible.
We cannot fix anything if we don't know it's broken :).

The best way to tell us about a bug is by submitting a bug report at our GitHub bug tracker page:

	https://github.com/KozGit/DOOM-3-BFG-VR/issues

The most important fact about this tracker is that we cannot simply forget to fix the bugs which are posted there. 
It is also a great way to keep track of fixed issues.

If you want to report an issue with the game, you should make sure that your report includes all information useful to characterize and reproduce the bug.

    * What version you are using, and what mods you have installed.
	* What hardware are you using ( Vive, Rift, Touch, Gamepad etc. )
    * Search on Google
    * Include the computer's hardware and software description ( CPU, RAM, 3D Card, distribution, kernel etc. )
    * If appropriate, send a console log, a screenshot, an strace ..
    * If you are sending a console log, make sure to enable developer output:

              Doom3BFGVR.exe +set developer 1 +set logfile 2

NOTE: We cannot help you with OS-specific issues like configuring OpenGL correctly, configuring ALSA or configuring the network.
	

	
	
_______________________________

15) LICENSE
______________________


See COPYING.txt for the GNU GENERAL PUBLIC LICENSE

ADDITIONAL TERMS:  The Doom 3 BFG Edition GPL Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU GPL which accompanied the Doom 3 BFG Edition GPL Source Code.  If not, please request a copy in writing from id Software at id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.


____________________________________________________________________________________

16) CODE LICENSE EXCEPTIONS - The parts that are not covered by the GPL:
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


