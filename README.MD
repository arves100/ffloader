# Fur Fighters Loader by Arves100
NetLib.dll replacement for updating Fur Fighters with Quality of Life improvements.

[Requires latest Fur Fighters v1.2](https://drive.google.com/file/d/1TxlPDFh1MC5UQElJR7qfq4QCHTwM_jyT)
Special thanks to ZaRR for netplay testing and support.

[Discord](https://discord.gg/zgSVGSs)

## Building
1. Download and install [Microsoft DirectX 8.1 SDK](https://archive.org/download/dx81sdk_full/dx81sdk_full.exe)
2. Go to your DirectX 8.1 installation directory, open the folder "include" and remove the following file "Basetsd.h"
3. Set an environment variable in your system called DX8SDK_DIR which points to the root of your DirectX 8.1 installation directory
4. Open the .sln and compile the DLL

## Network play
If your connection is under NAT, we suggest using solutions like ZeroTier.
The UDP port is 24900.

You can launch the game with command line if you want to easily access online functionalities:
 -host
 -connect (IP to connect)

### Extra CLI Arguments
Change player character:
-chang
-roofus
-bungalow
-juliette
-rico
-tweek

Enable first person mode:
-firstperson

Change name of the lobby:
-gamename (lobby name)

Change your player name:
-name (your name)

Change maximum number of players in a lobby:
-maxplayers (number)

## Installing
- Copy the "settings.txt", "levels.txt", "Levels" folder from a Fur Fighters CD to your Fur Fighters game
- Copy NetLib.dll inside Fur Fighters folder and replace the file

## Fixes
- Fixed a bug where "Change device" would block the parent window to use
- Fixed the display of the window border when playing the game in fullscreen
- Fixed missing saving of device selection

## New features
- Window mode functionality, you can set it on the "Change device" dialog
- Skip extra intro, you can set it on the "Change device" dialog
- NoCD mode, the game will load all the assets from it's game folder (and not from the CD anymore)
- Unlocked all game resolutions, you can now play in 1080p
- Completely rewritten Network stack with ENet, you can now enjoy network play with your friends.

## Troubleshooting
- **Graphical glitches or bad FPS.**
Please try using all the device options (some PCs works best with Direct3D HAL without T&L), if this
still doesn't work try using http://dege.freeweb.hu/dgVoodoo2/.
