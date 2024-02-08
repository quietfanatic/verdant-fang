#include "decals.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "walker.h"

namespace vf {

void draw_decal (const Walker& w, const Pose& pose) {
    if (w.decal_type == DecalType::None || w.decal_index >= max_decals) return;
    auto& data = *w.data->decals;
    auto& decal = w.decal_type == DecalType::Stab ? data.stab
                : w.decal_type == DecalType::Slash ? data.slash
                : (never(), data.stab);
    Vec off = pose.body->decals[w.decal_index];
    if (w.left) off.x = -off.x;
    uint8 dir = pose.body->decal_dirs[w.decal_index];
    if (!defined(off) || dir > 2) {
#ifndef NDEBUG
        never();
#endif
        return;
    }
    uint8 phase;
    float z;
    switch (w.state) {
        case WS::Damage: {
            if (w.anim_phase < 3) phase = w.anim_phase;
            else phase = 2;
            z = Z::Overlap + Z::DecalOffset;
            break;
        }
        case WS::Dead: {
            expect(w.anim_phase < 7);
            phase = 2 + w.anim_phase;
            z = Z::Dead + Z::DecalOffset;
            break;
        }
        default: never();
    }
    const Frame* frame;
    switch (dir) {
        case 0: {
            expect(phase <= 2);
            frame = &decal.dir_0[phase];
            break;
        }
        case 1: {
            expect(phase == 2);
            frame = &decal.dir_1[phase - 2];
            break;
        }
        case 2: {
            expect(phase >= 2);
            frame = &decal.dir_2[phase - 2];
            break;
        }
        default: never();
    }
    draw_frame(w.pos + off, *frame, decal.tex, {w.left ? -1 : 1, 1}, z);
}

} using namespace vf;

AYU_DESCRIBE(vf::Decal,
    attrs(
        attr("tex", &Decal::tex),
        attr("dir_0", &Decal::dir_0),
        attr("dir_1", &Decal::dir_1),
        attr("dir_2", &Decal::dir_2)
    )
)

AYU_DESCRIBE(vf::DecalData,
    attrs(
        attr("stab", &DecalData::stab),
        attr("slash", &DecalData::slash)
    )
)
