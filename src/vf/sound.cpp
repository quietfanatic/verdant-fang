#include "sound.h"

#include <SDL2/SDL_mixer.h>
#include "../dirt/ayu/resources/resource.h"
#include "../dirt/glow/common.h"

namespace vf {

Sound::Sound (const iri::IRI& src) : source(src) {
    load();
}

bool sound_initted = false;
static void init_sound () {
    if (sound_initted) return;
    glow::require_sdl(!Mix_OpenAudio(
        44100, AUDIO_S16SYS, 2, 512
    ));
    sound_initted = true;
}

void Sound::load () {
    init_sound();
    auto filename = ayu::resource_filename(source);
    chunk = glow::require_sdl(Mix_LoadWAV(filename.c_str()));
}

Sound::~Sound () {
    Mix_FreeChunk(chunk);
}

void Sound::play () {
    expect(Mix_PlayChannel(channel, chunk, 0) >= 0);
}

} using namespace vf;

AYU_DESCRIBE(vf::Sound,
    attrs(
        attr("source", &Sound::source),
        attr("channel", &Sound::channel, optional)
    ),
    init<&Sound::load>()
)
