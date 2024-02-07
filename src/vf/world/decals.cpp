#include "decals.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "walker.h"

namespace vf {

static void draw_stab_decal (const Walker& w, const Pose& pose) {
    auto& data = *w.data->decals;
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
            frame = &data.stab_0[phase];
            break;
        }
        case 1: {
            expect(phase == 2);
            frame = &data.stab_1[phase - 2];
            break;
        }
        case 2: {
            expect(phase >= 2);
            frame = &data.stab_2[phase - 2];
            break;
        }
        default: never();
    }
    draw_frame(w.pos + off, *frame, data.stab_tex, {w.left ? -1 : 1, 1}, z);
}

void draw_decal (const Walker& w, const Pose& pose) {
    if (w.decal_index >= max_decals) return;
    switch (w.decal_type) {
        case DecalType::None: return;
        case DecalType::Stab: draw_stab_decal(w, pose); break;
        default: never();
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::DecalData,
    attrs(
        attr("stab_tex", &DecalData::stab_tex),
        attr("stab_0", &DecalData::stab_0),
        attr("stab_1", &DecalData::stab_1),
        attr("stab_2", &DecalData::stab_2)
    )
)
