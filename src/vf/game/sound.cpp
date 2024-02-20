#include "sound.h"

#include "../../dirt/ayu/resources/resource.h"
#include "../../dirt/glow/common.h"
#include "game.h"
#include "settings.h"

namespace vf {

bool sound_initted = false;
static void init_sound () {
    if (sound_initted) return;
     // OGG doesn't seem to work on mingw
    auto flags = MIX_INIT_MP3;
    glow::require_sdl(Mix_Init(flags) & flags);
    uint32 buffer;
    if (current_game) {
        buffer = current_game->settings().audio_buffer;
    }
    else buffer = 512;  // Sound initted too early!?
    if (current_game) 
    glow::require_sdl(!Mix_OpenAudio(
        44100, AUDIO_S16SYS, 2, buffer
    ));
    sound_initted = true;
}

Sound::Sound (const iri::IRI& src) : source(src) {
    load();
}

void Sound::load () {
    require(volume >= 0 && volume <= 1);
    init_sound();
    auto filename = ayu::resource_filename(source);
    chunk = glow::require_sdl(Mix_LoadWAV(filename.c_str()));
    glow::require_sdl(Mix_VolumeChunk(chunk, round(volume * 128)) >= 0);
}

Sound::~Sound () {
    Mix_FreeChunk(chunk);
}

void Sound::play () {
    glow::require_sdl(Mix_PlayChannel(channel, chunk, 0) >= 0);
}

void Sound::stop () {
    expect(channel != -1);
    glow::require_sdl(Mix_HaltChannel(channel) >= 0);
}

Music::Music (const iri::IRI& src) : source(src) {
    load();
}

void Music::load () {
    require(volume >= 0 && volume <= 1);
    init_sound();
    auto filename = ayu::resource_filename(source);
    music = glow::require_sdl(Mix_LoadMUS(filename.c_str()));
     // Can't set the volume of the music until it's playing!
}

Music::~Music () {
    Mix_FreeMusic(music);
}

void Music::play () {
     // I suspect this will only loop 65535 times because SDL2_mixer is that
     // way.  I really need to switch to mixing my own audio when I have the
     // time.
    glow::require_sdl(Mix_PlayMusic(music, -1) == 0);
    glow::require_sdl(Mix_VolumeMusic(round(volume * 128)) >= 0);
}
void Music::stop () {
    Mix_HaltMusic();
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
AYU_DESCRIBE(vf::Music,
    attrs(
        attr("source", &Music::source),
        attr("volume", &Music::volume, optional)
    ),
    init<&Music::load>()
)
