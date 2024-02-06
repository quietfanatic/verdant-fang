#include "room.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/glow/gl.h"

namespace vf {

void Room::enter () {
    for (Resident& a : residents) {
        a.Resident_on_enter();
    }
}

void Room::exit () {
    for (Resident& a : residents) {
        a.Resident_on_exit();
    }
}

void Room::step () {
    for (Resident& a : residents) {
        a.Resident_before_step();
    }
     // Just some straightforward O(n^2) collision detection.
    for (auto a = residents.begin(); a != residents.end(); ++a) {
        auto b = a;
        for (++b; b != residents.end(); ++b) {
            for (auto& a_hb : a->hitboxes)
            for (auto& b_hb : b->hitboxes) {
                if (a_hb.layers_1 & b_hb.layers_2) {
                    if (overlaps(a_hb.box + a->pos, b_hb.box + b->pos)) {
                        a->Resident_on_collide(a_hb, *b, b_hb);
                    }
                }
                else if (b_hb.layers_1 & a_hb.layers_2) {
                    if (overlaps(b_hb.box + b->pos, a_hb.box + a->pos)) {
                        b->Resident_on_collide(b_hb, *a, a_hb);
                    }
                }
            }
        }
    }
    for (Resident& a : residents) {
        a.Resident_after_step();
    }
}

void Room::draw () {
    glClearColor(
        background_color.r / 255.0,
        background_color.g / 255.0,
        background_color.b / 255.0,
        background_color.a / 255.0
    );
    glClear(GL_COLOR_BUFFER_BIT);
    for (Resident& a : residents) {
        a.Resident_draw();
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::Room,
    attrs(
        attr("background_color", &Room::background_color, optional)
    )
)

AYU_DESCRIBE(vf::Resident,
    attrs(
        attr("room", value_methods<Room*, &Resident::get_room, &Resident::set_room>(), optional),
        attr("pos", &Resident::pos, optional)
    )
)
