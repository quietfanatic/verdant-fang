#include "decals.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "walker.h"

namespace vf {

void draw_decal (const Walker& w, const Pose& pose) {
    if (w.decal_type == DecalType::None || w.decal_index >= max_decals) return;
    auto& data = *w.data->decals;
    auto& decal = w.decal_type == DecalType::Stab ? data.stab
                : w.decal_type == DecalType::SlashLow ? data.slash_low
                : w.decal_type == DecalType::SlashHigh ? data.slash_high
                : (never(), data.stab);
    Vec off = w.left_flip(pose.body->decals[w.decal_index]);
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
            if (w.anim_phase < 4) phase = w.anim_phase;
            else phase = 3;
            z = Z::Overlap + Z::DecalOffset;
            break;
        }
        case WS::Dead: {
            expect(w.anim_phase < 7);
            phase = 3 + w.anim_phase;
            z = Z::Dead + Z::DecalOffset;
            break;
        }
        default: never();
    }
    const Frame* frame;
    switch (dir) {
        case 0: {
            expect(phase <= 3);
            frame = &decal.dir_0[phase];
            break;
        }
        case 1: {
            expect(phase == 3);
            frame = &decal.dir_1[phase - 3];
            break;
        }
        case 2: {
            expect(phase >= 3);
            frame = &decal.dir_2[phase - 3];
            break;
        }
        default: never();
    }
    draw_frame(*frame, 0, w.pos + off, z, {w.left ? -1 : 1, 1});
}

} using namespace vf;

AYU_DESCRIBE(vf::DecalType,
    values(
        value("none", DecalType::None),
        value("stab", DecalType::Stab),
        value("slash_low", DecalType::SlashLow),
        value("slash_high", DecalType::SlashHigh)
    )
)

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
        attr("slash_low", &DecalData::slash_low),
        attr("slash_high", &DecalData::slash_high)
    )
)
