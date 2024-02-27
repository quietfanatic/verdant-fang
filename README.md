Verdant Fang - https://leafuw.itch.io/verdant-fang - v0.9.0
============

Magical warrior Verdant and her trusty sidekick Fang venture into a dark cave to
confront the evil within!

This game was made for Strawberry Jam 2024.  The theme of the jam is sexuality,
so I packed a bunch of things I find sexy into this game, including:
  - Snakes
  - Caves
  - Violence
  - Transformation scenes
  - Slightly unfair level design
  - Moddable game content (with a text editor)

This game contains content that may disturb some people, including:
  - Blood (can be turned off)
  - Nudity (can be turned off)
  - Snakes (cannot be turned off)

###Windows

Run `verdant-fang.exe`.  Everything required should be included in the folder.
If it doesn't work, please let me know.

###Linux

This requires libc version 6 or higher, as well as these libraries:
   libsdl2 libsdl2-image libsdl2-mixer
Please install the equivalent packages from your package manager.

After that, run `verdant-fang`.  If it doesn't work, try running it in a
terminal and tell me what the error message is.

###Controls

The default controls are as follows:
  - Arrow keys or WASD: Move around
  - Z or slash: Jump
  - X or period: Attack
  - R (on game over screen): Restart from checkpoint
  - Ctrl-R: Restart from checkpoint at any time
  - P: Pause
  - F11: Toggle fullscreen mode
  - Escape: Suspend game (save state and quit)

You can change the controls by editing `settings.ayu`

###Modding

Almost all of the game's data is stored in editable text files.  The game world
(rooms and things) is stored at `res/vf/world/world.ayu`.  If you want you can
edit it with your favorite text editor to recreate the world to your liking.
And if you're clever enough, you can insert your own images and music too!  If
modifying the text data is not enough, you can download the source code version
and modify it too.  I request that if you redistribute a modified version of the
game, you alter the title screen (such as by adding some text).

###Source Code

If you downloaded the source code version, you can modify and compile the source
yourself.  You'll need Perl to run the build system, as well as development
versions of the required SDL libraries for you platform.  Currently the code
only compiles with GCC; clang and MSVC are not supported.

#####TODO
[x] Actually trigger capture sequence
[x] Decals during capture sequence
[x] Place limbs in bedroom
[x] End screen
[x] Put snake on title screen
[x] Improve title screen music
[x] Set music during final cutscenes
[x] Hallway
[ ] Tunnel
[ ] Package for Windows
[ ] Package for Linux
Optional:
[ ] Kick off spear animation
[ ] Boss fight?
