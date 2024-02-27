Verdant Fang - https://leafuw.itch.io/verdant-fang - v0.9.1
============

Magical warrior Verdant and her trusty sidekick Fang venture into a dark cave to
confront the evil within!

This game was made for Strawberry Jam 8.  The theme of the jam is sexuality,
so I packed a bunch of things I find sexy into this game, such as:
  - Snakes
  - Caves
  - Strong women
  - Transformation scenes
  - Slightly unfair level design
  - Moddable game content (with a text editor)

This game contains content that may disturb some people, including:
  - Blood (can be turned off)
  - Nudity (can be turned off)
  - Snakes (cannot be turned off)

###Controls

The default controls are as follows:
  - Arrow keys or WASD: Move around
  - Z or slash: Jump (Confirm)
  - X or period: Attack (Back)
  - R (on game over screen): Restart from checkpoint
  - P: Pause
  - F11: Toggle fullscreen mode
  - Escape: Boss Key (immediately save state and close)
  - Ctrl-R: Restart from checkpoint at any time

You can change the controls by editing `save/settings.ayu` (you must run the
game once to generate the file).

###Windows

Run `verdant-fang.exe`.  Everything required should be included.  If it doesn't
work, please let me know.

###Linux

This requires a recent Linux distro (libc version 6 or higher), as well as these
system libraries:
   libsdl2 libsdl2-image libsdl2-mixer
Please install the equivalent packages with your system package manager.

After that, run `verdant-fang`.  If it doesn't work, try running it in a
terminal and tell me what the error message is.

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
versions of the required SDL libraries for your platform.  Currently the code
only compiles with GCC; clang and MSVC are not supported.

Since itch.io's command line tools ignore the `.git` directory, it has been
renamed to `_git`.  Please rename it back to `.git` if you want to use git
features.

If you don't have the source code and want it, it can be downloaded at
https://leafuw.itch.io/verdant-fang

#####Wrap up checklist
[x] Actually trigger capture sequence
[x] Decals during capture sequence
[x] Place limbs in bedroom
[x] End screen
[x] Put snake on title screen
[x] Improve title screen music
[x] Set music during final cutscenes
[x] Hallway
[x] Tunnel
[ ] Package for Windows (stop opening terminal)
[x] Package for Linux
[x] Package source
[ ] Fix losing checkpoint when saving state
[ ] death animation while poisoned crash
[ ] revive animation multiple occurence
[ ] clothes on floor
[ ] don't make Indigo's hat disappear
[ ] Tweak monster's AI
[ ] Fix bed hitbox
[ ] Game over screen controls
[ ] Window title
Optional:
[ ] Kick off spear animation
[ ] Boss fight?
