#include "decals.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "walker.h"

namespace vf {

void draw_decal (Walker& w, const Pose& pose) {
    auto& data = *w.data->decals;
    if (w.paralyze_symbol_timer) {
        draw_frame(
            data.paralyze.symbol[(w.paralyze_symbol_timer / 4) % 2],
            0,
            w.pos + Vec(0, 30),
            Z::Symbol
        );
        w.paralyze_symbol_timer -= 1;
    }
    if (w.decal_type == DecalType::None || w.decal_index >= max_decals) return;
    auto& decal = w.decal_type == DecalType::Stab ? data.stab
                : w.decal_type == DecalType::SlashLow ? data.slash_low
                : w.decal_type == DecalType::SlashHigh ? data.slash_high
                : (never(), data.stab);
    Vec off = w.left_flip(pose.body->decals[w.decal_index]);
    uint8 dir = pose.body->decal_dirs[w.decal_index];
    if (!defined(off) || dir > 2) {
#ifndef NDEBUG
//        never();
#endif
        return;
    }
    expect(w.state == WS::Dead);
    const Frame* frame;
    switch (dir) {
        case 0: {
            expect(w.anim_phase <= 3);
            frame = &decal.dir_0[w.anim_phase];
            break;
        }
        case 1: {
            expect(w.anim_phase == 3);
            frame = &decal.dir_1[w.anim_phase - 3];
            break;
        }
        case 2: {
            expect(w.anim_phase >= 4);
            frame = &decal.dir_2[w.anim_phase - 4];
            break;
        }
        default: never();
    }
    float z = w.anim_phase < 3 ? Z::Overlap + Z::DecalOffset
            : pose.z + Z::DecalOffset;
    uint32 layer = w.data->flavor == WF::Lemon ? 1 : 0;
    draw_frame(*frame, layer, w.pos + off, z, {w.left ? -1 : 1, 1});
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

AYU_DESCRIBE(vf::Paralyze,
    attrs(
        attr("tex", &Paralyze::tex),
        attr("symbol", &Paralyze::symbol)
    )
)

AYU_DESCRIBE(vf::DecalData,
    attrs(
        attr("stab", &DecalData::stab),
        attr("slash_low", &DecalData::slash_low),
        attr("slash_high", &DecalData::slash_high),
        attr("paralyze", &DecalData::paralyze)
    )
)
