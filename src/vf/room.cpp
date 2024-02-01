#include "room.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/glow/gl.h"

namespace vf {

void Room::enter () {
    for (Resident& a : residents) {
        a.Resident_emerge();
    }
}

void Room::exit () {
    for (Resident& a : residents) {
        a.Resident_reclude();
    }
}

void Room::step () {
    for (Resident& a : residents) {
        a.Resident_before_step();
    }
     // Just some straightforward O(n^2) collision detection.
    for (auto a = residents.begin(); a != residents.end(); ++a) {
        Rect a_box = a->bounds + a->pos;
        auto b = a;
        for (++b; b != residents.end(); ++b) {
            if (!!(a->layers_1 & b->layers_2)) {
                auto b_box = b->bounds + b->pos;
                if (overlaps(a_box, b_box)) {
                    a->Resident_collide(*b);
                }
            }
            else if (!!(b->layers_1 & a->layers_2)) {
                if (overlaps(b->bounds + b->pos, a_box)) {
                    b->Resident_collide(*a);
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
