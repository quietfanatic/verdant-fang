#### 2024-02-01 H

- Some cancelled work making the ayu description system support llvm
- Started work on character art.  One standing frame, two attack frames, and
  almost two running frames.  Character ended up better looking than expected.
- Customized render pipeline a bit.  `GL_TEXTURE_RECTANGLE` and alpha blending.
- Started experimenting with sound effects.  Tentative landing SFX.

#### 2024-02-02 F

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

#### 2024-02-03 S

- Made a simple cave background,  It's passable and better than nothing.
- Did more animation work, including falling, landing, and crouching frames, and
  improved existing ones.
- Implemented attack hitbox.  No enemies to hit but you can bounce off walls.
- Added sound effects for footsteps and stabbing.
- Improved player control logic a bit.
- Revised settings and state resource loading.
- Planned work for tomorrow: Monster!

#### 2024-02-05 M

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

#### 2024-02-06 T

- Added damage and dead frames for player and monster.
- Made sound effects for player and monster damage.
- Implemented a bunch of logic around damage and dying.  Killing an enemy feels
  really stimulating already.
- Started drawing art for blood and started some logic around displaying it, but
  haven't finished it yet.
- Planned work for tomorrow: Actual blood!

#### 2024-02-07 W

- Animated blood for both player and monster.  A lot of work but worth it.
- Added a really snazzy snake-themed effect for room transitions, using some
  trig and a new shader.  Oh, and added room transitions.
- To make room transitions work smoothly, added a per-frame scheduling system.
- Fixed state loading and saving, including saving current room and scheduled
  actions.  Although room transitions are properly saved, the visual transition
  effect is not.  Saving it fully would require saving the texture of the old
  room, which is not worth the effort.
- Planned work for tomorrow: Buttons and doors and more enemies?

#### 2024-02-08 H

- Much of the work today was not very visible.  A lot of maintainance and
  interface improvements.
- Added another monster personality and did a lot of work on monster AI.  It's
  still not great but it's better.
- Animated a lot more blood.  Put blood on weapons and protagonist's head.
- Spent a lot of time making room transitions more sensible and reliable.
- Added a switch and a door.  Got the door's opening animation good-looking on
  the first try.  If you don't count the try where I got my lerper backwards and
  the door moved instantly.
- Added sounds for the switch and the door and a better sound for landing.
- Planned work for tomorrow: Boss?

#### 2024-02-09 F

- Mostly art work today.  Drew some frames for the witch boss, who came out much
  cuter than I expected.  I'm sorry cute witch, but you're the villain.
- Drew frames for falling forward, landing on elbows and knees, and laying
  forward.  Well, haven't drawn all the heads yet.
- Adjusted verdant frames and decals some more.
- Fixed some bugs with collision and stuff
- Added sounds for door closing (and fixed door closing animation).
- Planned work for tomorrow: Crushing by door and witch movement, and some
  architecture for specializing Walker type and adding custom states.

#### 2024-02-10 S

- A lot of work under the hood today.  I added a system to automatically
  determine the bounds of art frames, which makes it a lot faster to add new
  frames and modify existing ones.  Also implemented the system for specializing
  Walker types with custom states.  Haven't implemented any actual custom
  states, but I have added some custom state handling to the main character.
- Wasted something like two hours on a serious bug in said system which was just
  caused by an argument name shadowing a class member. XoX
- Finished art frames for falling and laying forwards, and added them to the
  program (much more pleasant with the automatic bounds detection).
- Implemented getting crushed by door, which was so easy it was almost
  anticlimatic, thanks to all the refactoring work.
- Planned work for Monday: Dunno!  Probably gonna play around with Witch some
  more.

#### 2024-02-12 M

- Added the transformation scene!  I decided this was even more important than
  the boss, to establish that the character is a magical-girl-like person and
  the spear is a snake.  Also, showing nudity in the transformation scene early
  on brings the player closer to the character and sets a precedent that nudity
  will exist, so it isn't as confusing when the witch has no pants.
- Also I haven't finalized plans for the witch yet.
- Did some other small art and drawing adjusments.
- Fixed the decals having bits cut off from the frame auto bounds system.
- Planned work for tomorrow: Semisolid platforms, crouch stab, hiding enemies,
  think about witch some more.

#### 2024-02-13 T

- Not as many big additions as I'd hoped, but knocked off a bunch of smaller
  todos.
- Made semisolid platforms
- Made an ambushing monster
- Spent a lot of time working on monster AI.  It shows that I haven't done any
  real NPC AI before.  I should reach for phase counters more quickly.  Maybe if
  I had more time I'd replace the phase counters with some sort of clever
  coroutine system, but I haven't really learned C++ coroutines yet.  Update:
  This would not be appropriate, as C++ coroutines use dynamic allocation under
  the hood.
- Implemented checkpoints.  No transition yet.
- Added crouch attacking.  This isn't really useful for combat but it feels nice
  to have.
- Planned work for tomorrow: Not entirely sure.  Could do restart transitions,
  troll doors, flying bugs, or the boss.

#### 2024-02-14 W

- Starting implementing bugs.  Like, insects.  They only have a few animation
  frames and no AI, but they can attack and be attacked.
- Spent far too long worrying about how much code to share between walkers and
  flyers before ending up with the choice of nearly all of it.
- Made semisolids grouped into a single resident like blocks.  Made hitboxes in
  linked lists instead of array slices so they can be non-contiguous.  Added z
  offset to texture layers so model part offsets can be given in data instead of
  code.
- Planned work for tomorrow: Make bug spit venom.  Venom will be an important
  mechanic for the story later on.

#### 2024-02-15 H

- Halfway point!
- Implemented more art, animation, AI, and decals for bugs.  Implemented a bunch
  of untested code for spitting venom, but haven't tried it out yet.
- Spent a long time on the architecture under checkpoints and transitions.
  They're better now, but I can't help but feel like the time wasn't very well spent.
- Implemented aperture-style transitions and a nice looking death screen, with a
  "press R to restart" message that looks better than I thought it would.
- Did some emergency performance work on AYU, not to a whole lot of avail.
  Keeping the location cache in to\_tree helps a lot though.
- Added decorative light streaming in from the cave entrance.  I though it
  wouldn't look good, but it surprisingly works if it has a subtle enough color.
  It balances against the blue switch on the other side of the room.
- Planned work for tomorrow: Spitting!  Maybe start work on Witch some, as
  another flying entity?  I should also maybe start in on music too...oh but
  there are a bunch of sound effects I need to add too...uhhhhh

#### 2024-02-16 F

- Still didn't implement much of the final boss, but I did get some very
  important work done.
- Made bugs spit poison and implemented poison/paralysis, including related art
  and sounds.  The bug AI still needs better aim.
- Revised transition and checkpoint code yet again, this time making checkpoint
  management somewhat saner (though it has to go through the transition system,
  even to save checkpoints while not in a transition).
- Implemented aperture open transition when loading a checkpoint.
- Implemented snake animation for game over screen, and revised and vastly
  improved the dead/force-restart-\>limbo-\>restart sequence.
- Haven't implemented anything yet, but drew some art for the captured-by-final-
  boss scene.
- Planned work for tomorrow: More boss cutscene work.  And maybe music.

#### 2024-02-17 S

- Today was cutscene day.  I implemented probably 50% of the endgame cutscenes.
  And that's about it.
- I guess I started programming stuff for Indigo (Witch) too.
- Also added a few sounds, mostly for the captured cutscene, but also a hissing
  sound when Fang revives you.  I tried adding a hiss to the transformation
  cutscene too, but there was nowhere it fit.
- Planned work for Monday: I really should start in on the music, and maybe the
  menus.  There's still a lot of cutscene work to do.  I might not have time to
  implement an actual fight for the boss.

#### 2024-02-19 M

- Music!!!  I made a main music track and it's really good.  Day well spent.
- Also made a title screen track, which is a little plain, but it might be
  better that way.
- Also did more cutscene work.  Implemented the cutscene of turning into a
  snake, and did some snake movement.
- Planned work for tomorrow: Menus, more work on Indigo.

#### 2024-02-20 T

- Didn't get a whole lot done today due to personal circumstances, but I did
  implement a pause menu, and drew text for all the menus.
- Also tweaked music a little.
- Planned work for tomorrow: Finish up menus, get back to Indigo.

#### 2024-02-21 W

- Progress was slower than I would have liked, partly for health reasons and
  partly because I spent all morning moving cutscene logic from Verdant to
  Indigo.  This way makes more sense, but it took a long time and wasn't really
  necessary.
- Did a bunch more cutscene work, including some of the next room.
- Implemented more snake behavior.
- Discovered and fixed a few bugs that really needed to be fixed.
- Planned work for tomorrow: Keep cracking at these cutscenes.  If all goes well
  I might actually finish them tomorrow, and have some time left over to work on
  menus.

#### 2024-02-22 H

- Aaaalmost done with the final cutscenes!  I've implemented biting, being
  shaken off, poisoning, and eating.
- Also tweaked some other parts of cutscenes.
- Oh and fully implemented all menus, including the options and main menus.
  Good day today.
- Added another room, "corridor".
- Planned work for tomorrow: Finish off final cutscene gauntlet with the
  transformation back to human.  Then I want to step back from cutscenes a bit
  and add more rooms, and come back to the cutscenes with a fresh mind to
  freshen them up.

#### 2024-02-23 F

- Basically done with every major thing in the game.  All that's left is
  polishing, adding a couple more rooms (which are already planned), and maybe a
  little more music and sound effects.
- Did more work for the final cutscenes.
- Added several more rooms.
- Added a bit more decoration.
- Finally fixed bug accuracy.  It's still not the best but it'll do, and it's
  weird enough that the player can't assume the projectile will always fly right
  toward them.
- Some other AI improvements for monsters and bugs.
- Implemented hide\_blood, hide\_nudity, and enemy\_difficulty options.  None of
  these were as hard as I thought they would be.
- Some improvements to aperture transitions and other miscellaneous things.
- Planned work for tomorrow: Add the final two rooms.  Place loose limbs in
  bedroom.  Work on music a little more, add a couple more animation frames...

#### 2024-02-24 S

- I feel like I'm running out of steam a little bit, but that's fine because I'm
  almost done.  I started a todo list in the morning, and there turned out to be
  more things than I expected and it was a bit daunting, but now at EOD it's
  almost completely filled out.
- Finished final cutscenes, including music behavior, limb placement, and end
  screen.
- Finally drew blood for the limb detachment scene.
- Added the Hallway room.
- Prevented various door-related softlocks.
- Improved title screen/snake music.  It's still not quite how I wanted it to
  be, but it's a lot closer.
- Planned work for Monday: All I have left on my todo list is add the Tunnel
  room and the kick-off-spear animation.  I started work on the latter but it's
  turning out surprisingly difficult, but I'll probably be able to do it in
  time.  After that, it's just packaging and playtesting.  If I have time left
  over, final boss fight?

#### 2024-02-26 M

- Starting today, I forgot to write my work in this log, so this and everything
  below will be written in the future by looking at my commit history
  (2024-03-09 S).
- Filled out README.md with a bunch of stuff.
- Added Tunnel room.
- Adjusted menus and added some more miscellaneous text.
- Figured out how to do deployment with butler, and did initial deploy for
  linux, windows, and source code.
- In response to playtesting, made lizard monsters much easier.
- Planned work for tomorrow: Knock off remaining todos and maybe implement
  actual boss fight?

#### 2024-02-27 T

- A bunch of random bufixes and small todos, some of which were found by
  playtesting.
- Some art improvements regarding clothes and accessories during cutscenes.
- Make indigo shoot projectiles, they don't do anything yet though.
- Planned work for tomorrow: Finish boss fight and final adjustments, leaving
  one extra day to put out last-minute fires.

#### 2024-02-28 W

- Implement actual boss fight!  You have to stab all the bubbles to advance to
  the next phase.  Indigo will dodge if you get too close.
- Made it so it actually looks like Indigo is using magic on you during capture
  cutscenes.
- Added some final sound effects.  It was too rainy to record in my room so I
  borrowed a relative's basement.
- Made game aware of when you're doing "hardcore" aka "deathless" aka "super"
  mode.
- A whole bunch more fixes and adjustments.

#### 2024-02-29 H

- The final day!
- Made Indigo's projectiles curve when moving, leading to a much more
  interesting and difficult fight.  Also fixed them getting stuck in the corner.
- In response to playtesting, made the bugs much more accurate with their
  spitting and made it more obvious when you (and Indigo) are poisoned by
  tinting your body purple (green for Indigo because she's already purple).
- More last-minute fixes
- Final deployment and submission!  About 2.5 hours before the deadline.
  Hope you enjoy!
