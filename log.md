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
#####2024-02-03 S
- Made a simple cave background,  It's passable and better than nothing.
- Did more animation work, including falling, landing, and crouching frames, and
  improved existing ones.
- Implemented attack hitbox.  No enemies to hit but you can bounce off walls.
- Added sound effects for footsteps and stabbing.
- Improved player control logic a bit.
- Revised settings and state resource loading.
