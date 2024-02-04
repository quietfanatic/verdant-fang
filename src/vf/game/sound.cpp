#include "sound.h"

#include <SDL2/SDL_mixer.h>
#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/glow/common.h"

namespace vf {

Sound::Sound (const iri::IRI& src) : source(src) {
    load();
}

bool sound_initted = false;
static void init_sound () {
    if (sound_initted) return;
     // OGG doesn't seem to work on mingw
    auto flags = MIX_INIT_MP3;
    glow::require_sdl(Mix_Init(flags) & flags);
    glow::require_sdl(!Mix_OpenAudio(
        44100, AUDIO_S16SYS, 2, 512
    ));
    sound_initted = true;
}

void Sound::load () {
    require(volume >= 0 && volume <= 1);
    init_sound();
    auto filename = ayu::resource_filename(source);
    chunk = glow::require_sdl(Mix_LoadWAV(filename.c_str()));
    Mix_VolumeChunk(chunk, volume * 128);
}

Sound::~Sound () {
    Mix_FreeChunk(chunk);
}

void Sound::play () {
    glow::require_sdl(Mix_PlayChannel(channel, chunk, 0) >= 0);
}

} using namespace vf;

AYU_DESCRIBE(vf::Sound,
    attrs(
        attr("source", &Sound::source),
        attr("channel", &Sound::channel, optional),
        attr("volume", &Sound::volume, optional)
    ),
    init<&Sound::load>()
)
