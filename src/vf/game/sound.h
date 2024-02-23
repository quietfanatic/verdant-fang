#pragma once

#include <SDL2/SDL_mixer.h>
#include "common.h"
#include "../../dirt/iri/iri.h"

namespace vf {

struct Sound {
     // Serialized
    iri::IRI source;
     // If this is not -1, then this sound will interrupt any other sound
     // playing on the same channel.
    int32 channel = -1;
     // 0..1
    float volume = 1;
     // Not serialized
    Mix_Chunk* chunk = null;
    operator Mix_Chunk* () { return chunk; }

    Sound () { }
    Sound (const iri::IRI& src);
    void load ();
    ~Sound ();
    void play ();
    void stop ();
};

struct RandomSound {
    UniqueArray<Sound*> sounds;
    int32 last_played = -1;
    void play ();
};

 // SDL2_mixer has different types for music and sound effects.  You can do more
 // with music (like loop it
struct Music {
    iri::IRI source;
    double loop_start = 0;
    double loop_end = GNAN;
    float volume = 1;
    Mix_Music* music = null;
    operator Mix_Music* () { return music; }

    Music () { }
    Music (const iri::IRI& src);
    void load ();
    ~Music ();
    void play ();
    void stop ();
};

} // vf
