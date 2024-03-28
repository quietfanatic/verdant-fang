Verdant Fang - https://leafuw.itch.io/verdant-fang - v1.0.4
============

Magical warrior Verdant and her trusty sidekick Fang venture into a dark cave to
confront the evil within!

This game was made for Strawberry Jam 8.  The theme of the jam is sexuality,
so I packed a bunch of things I find erotic into this game, such as:
  - Snakes
  - Caves
  - Transformation scenes
  - Slightly unfair level design
  - Moddable game content (use a text editor)

WARNING: This game contains content that may disturb some people, including but
not limited to:
  - Blood (can be turned off)
  - Nudity (can be turned off)
  - Snakes (cannot be turned off)

### Controls

The default controls are as follows:
  - Arrow keys or WASD: Move around
  - Z or slash: Jump (Confirm)
  - X or period: Attack (Back)
  - R (on game over screen): Restart from checkpoint
  - P or Escape: Pause
  - F11, Alt-Enter, or F4: Toggle fullscreen mode
  - Ctrl-R: Restart from checkpoint at any time
  - Ctrl-Q or Alt-F4: Boss Key! (immediately save and exit; next time you open
    the game it will start paused at the same moment)

You can change the controls by editing `save/settings.ayu` with a text editor
(you must run the game once to generate the file).

### Options

Here's a detailed explanation of the in-game options.

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
will see an "S" symbol in the lower-left corner.  It stands for Super.

Advanced settings are in `save/settings.ayu` after running the game once.  They
are explained in comments in the file.

### FAQ

The combat is too difficult for me!

- There's an option to add more checkpoints and an option to make enemies a
  little easier. Do please use them! I'm not going to go all "You cheated not
  only the game but yourself" on you.
- Still struggling? There are cheat codes that let you use savestates, if you
  can figure out how to enable them in the configuration files.

How do I clear the first room?

- You have to jump up and hit the blue switch to open the door. Don't hold the
  attack button for too long. This room is practice for the combat later on.

How do I change the controls?

- For now you'll have to use a text editor to edit the file `save/settings.ayu`
  in the program directory. In-game control customization is on the feature
  roadmap.

There may or may not be more up-to-date FAQ on the game webpage.

### System Requirements

This game requires a video driver that supports OpenGL ES 3.0.  For the most
part, anything made since 2012 should work.

The precompiled binaries are only for x64 machines.  You should be able to
compile tbe source for other machines, but I have only tested it on x64.

### Running on Windows

Run `verdant-fang.exe`.  Everything required should be included.  If it doesn't
work, please let me know.

### Running on Linux

This requires a recent Linux distro (libc version 6 or higher), as well as these
system libraries: `libsdl2 libsdl2-image libsdl2-mixer`
Please install the equivalent packages with your system package manager.

After that, run `verdant-fang`.  If it doesn't work, try running it in a
terminal and tell me what the error message is.

If the Linux package doesn't work, you may have better luck either building from
source or running the Windows version with WINE.

### Modding

Almost all of the game's data is stored in editable text files.  The game world
(rooms and things) is stored at `res/vf/world/world.ayu`.  If you want you can
edit it with your favorite text editor to recreate the world to your liking.
And if you're clever enough, you can insert your own images and music too!  If
modifying the text data is not enough, you can download the source code version
and modify it too.  If you widely distribute a modified version of the game,
please alter the title screen (such as by adding or changing some text).

### Source Code

*NOTICE for GitHub users: If you cloned this from GitHub, your local copy will
be missing the images and sounds required to run the game.  You need to download
these things from https://leafuw.itch.io/verdant-fang to be able to build the
entire game.*

If you downloaded the source code version, you can modify and compile the source
yourself.  You'll need Perl to run the build system, as well as development
versions of the required SDL libraries for your platform.  Currently the code
only compiles with gcc-12; clang and MSVC are not supported.  To build the debug
version, use the following command
```
perl make.pl --jobs=7 out/deb/build
```
after which the program will be at `out/deb/verdant-fang`.  You can build other
flavors by replacing `deb` with one of the other configs in the build script.
If you're just here to play the game and not hack on it, you probably want to
build the `rel` configuration.

By default, the build script is configured for building on Linux.  To build on
Windows, you will need the mingw toolchain corresponding to GCC version 12, as
well as the mingw versions of the SDL libraries.  You'll have to tweak the build
script in `make.pl` in the following ways:

- Change `$mingw` (at or around line 34) to wherever your mingw folder is.
- Change `$sdl`, `$sdl_image`, and `$sdl_mixer` (right below) to match the
  folders of the SDL libraries you downloaded.

Since itch.io's command line tools ignore the `.git` directory, it has been
renamed to `_git`.  Please rename it back to `.git` if you want to use git
features.  Note that image and sound files are excluded from the repository
history to keep its size down.

This game was made in only a month, so some of the code may be a little messy,
especially the actor state management and animation code.

### Credits

Programming, art, music & sound: Leafuw
Playtesting: Anonymous
Help with compatibility debugging: Disposable Dev
Special thanks: Everyone in the Strawberry Jam community

#### Tools Used

Pixel art: The GIMP
Music composing: Sekaiju
MIDI rendering: TiMidity with eawpats
Recording and audio editing: Audacity

### Version History

#### v1.0.1

Initial public version.

#### v1.0.2

Bugfix release.

- Fixed a crash that only happens after another crash.
- Added some more things to README.md

#### v1.0.3

Bugfix release.

- Fixed bug (enemy) projectiles not being saved to savestates.
- Halved size of savestate file by removing whitespace.
- Adjusted position of an entity during final cutscene.
- Finished log.md and added FAQ and version history to README.md.

*Saved states from older versions are NOT compatible with this one.  If you get
an error message when starting the game after updating, you will need to
manually delete save/state.ayu.  We apologize for the inconvenience.  Better
state version checking is planned for the future.*

#### v1.0.4

Compatibility release.

- Changed graphics target from OpenGL Core 3.1 to OpenGL ES 3.0, as there are
  some graphics drivers that only support OpenGL ES.  This change might make
  the game unplayable on some older devices made between 2011 and 2012.  If
  this version doesn't work on your device, please let me know.
- Provided better instructions for building from source on Windows.

