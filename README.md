Verdant Fang - https://leafuw.itch.io/verdant-fang - v0.9.6
============

Magical warrior Verdant and her trusty sidekick Fang venture into a dark cave to
confront the evil within!

This game was made for Strawberry Jam 8.  The theme of the jam is sexuality,
so I packed a bunch of things I find erotic into this game, such as:
  - Snakes
  - Caves
  - Strong women
  - Transformation scenes
  - Slightly unfair level design
  - Moddable game content (use a text editor)

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
  - Ctrl-R: Restart from checkpoint at any time
  - Ctrl-Q or Escape while paused: Boss Key (immediately save state and close)

You can change the controls by editing `save/settings.ayu` (you must run the
game once to generate the file).

###Options

Detailed explanation of the in-game options:
  - Checkpoints: Controls how many checkpoints there are, thereby controlling
    the amount of frustration the player experiences.  "Lots" means one at just
    about every room, "some" means one about every 2 to 3 rooms, and "none"
    means exactly that: one mistake and you go back to the beginning.
  - Enemies: If you're struggling fighting the enemies, try changing this to
    "easy", which will make the enemies' attack animations slower.
  - Blood: If pixely blood makes you queasy, use this option to hide it.
  - Nudity: If you're looking for a slightly safer experience, use this option
    to reduce human nudity.  Some animations may also be removed or altered.
    Note that even with blood and nudity hidden, the game might still be
    considered NSFW, depending on your standards.

If you play from the beginning with checkpoints=none and enemies=normal, you
will see an "S" symbol in the lower-right corner.  It stands for Super.

Advanced settings are in `save/settings.ayu` after running the game once.  They
are explained in comments in the file.

###Running on Windows

Run `verdant-fang.exe`.  Everything required should be included.  If it doesn't
work, please let me know.

###Running on Linux

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
and modify it too.  If you widely distribute a modified version of the game,
please alter the title screen (such as by adding or changing some text).

###Source Code

If you downloaded the source code version, you can modify and compile the source
yourself.  You'll need Perl to run the build system, as well as development
versions of the required SDL libraries for your platform.  Currently the code
only compiles with GCC; clang and MSVC are not supported.  To build the debug
version, use
```
perl make.pl out/deb/build
```
after which the program will be at `out/deb/verdant-fang`.  You can build other
flavors by replacing `deb` with one of the other configs in the build script.

Since itch.io's command line tools ignore the `.git` directory, it has been
renamed to `_git`.  Please rename it back to `.git` if you want to use git
features.  Note that image and sound files are excluded from the repository
history to keep its size down.

This game was made in only a month, so some of the code may be a little messy,
especially the actor state management and animation code.

If you don't have the source code and want it, it can be downloaded at
https://leafuw.itch.io/verdant-fang

