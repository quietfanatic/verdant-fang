#pragma once
#include "common.h"
#include "../../dirt/iri/iri.h"

struct Mix_Chunk;

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

} // vf
