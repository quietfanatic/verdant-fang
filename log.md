#####2024-02-01 H
- Some cancelled work making the ayu description system support llvm
- Started work on character art.  One standing frame, two attack frames, and
  almost two running frames.  Character ended up better looking than expected.
- Customized render pipeline a bit.  `GL_TEXTURE_RECTANGLE` and alpha blending.
- Started experimenting with sound effects.  Tentative landing SFX.
#####2024-02-02 F
- Did a bunch more work on character art.  Have one standing frame, two better
  attack frames, and six running frames, all fully clothed.  I tried to work
  with four running frames but it's just not enough.
- Implemented basic character state management, movement, and animation.  It's
  surprisingly hard to do this well.  The tricks are similar to other
  programming areas: minimize state and try to make it orthogonal, consider what
  kind of concepts you are actually trying to represent and make the program
  flow match them.
- Tried to figure out a better way to manage frames and layers together in GIMP.
  Didn't find one.
- Implemented part animation system.  Currently just head and body.
- Spent a little too long getting true fullscreen to work when I really should
  just have reduced the resolution of my laptop screen.  My laptop is a lot
  quieter now.
- Looked up cave references.  Caves are very chaotic and illegible, so it'll be
  a big challenge making that work with game rooms which need to be geometric
  and legible.  Also my art skill level.
- Planned work for tomorrow: Cave!
#####2024-02-03 S
- Made a simple cave background,  It's passable and better than nothing.
- Did more animation work, including falling, landing, and crouching frames, and
  improved existing ones.
- Implemented attack hitbox.  No enemies to hit but you can bounce off walls.
- Added sound effects for footsteps and stabbing.
- Improved player control logic a bit.
- Revised settings and state resource loading.
- Planned work for tomorrow: Monster!
#####2024-02-05 M
- Some adjustments to player physics.
- Add a simple monster.  I say simple, but it shares almost all code with the
  player.  Behavior is separated by resource data and a Mind interface, which
  can either read keyboard input or do some basic AI.
- Wasted some time trying to switch to libresprite and went back to GIMP.
- Implement some basic physics around killing.
- Sort draw commands by z, so you can dynamically decide which objects cover up
  which other objects.
- Wasted a lot of time with collision bugs.  Reworked collision system to
  support multiple hitboxes per resident to reduce the chances of this happening
  again.
- Planned work for tomorrow: Blood!  And maybe room transitions or music or more
  sound effects.
