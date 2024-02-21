#include "room.h"

#include "../../dirt/ayu/reflection/describe.h"
#include "../../dirt/glow/gl.h"

namespace vf {

void Resident::set_room (Room* r) {
    if (room) {
        for (auto& p : room->residents) {
            if (p == this) {
                room->residents.erase(&p);
                goto found;
            }
        }
        never();
        found:;
    }
    room = r;
    if (room) room->residents.push_back(this);
}

void Room::enter () {
    auto tmp = residents;
    for (Resident* a : tmp) {
        a->Resident_on_enter();
    }
}

void Room::exit () {
    auto tmp = residents;
    for (Resident* a : tmp) {
        a->Resident_on_exit();
    }
}

static void check_collision (Resident& a, Resident& b) {
    for (auto a_hb = a.first_hitbox; a_hb; a_hb = a_hb->next)
    for (auto b_hb = b.first_hitbox; b_hb; b_hb = b_hb->next) {
        if (a_hb->layers_1 & b_hb->layers_2) {
            if (overlaps(a_hb->box + a.pos, b_hb->box + b.pos)) {
                a.Resident_on_collide(*a_hb, b, *b_hb);
            }
        }
        else if (b_hb->layers_1 & a_hb->layers_2) {
            if (overlaps(b_hb->box + b.pos, a_hb->box + a.pos)) {
                b.Resident_on_collide(*b_hb, a, *a_hb);
            }
        }
    }
}

void Room::step () {
    auto tmp = residents;
    for (Resident* a : tmp) {
        a->Resident_before_step();
    }
         // Just some straightforward O(n^2) collision detection.
    tmp = residents;
    for (auto& a : tmp) {
        for (Resident* b : Slice<Resident*>(&a+1, tmp.end())) {
            check_collision(*a, *b);
        }
    }
    tmp = residents;
    for (Resident* a : tmp) {
        a->Resident_after_step();
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
#ifndef NDEBUG
     // Draw commands should not modify the residents array.
    auto end = residents.end();
#endif
    for (Resident* a : residents) {
        a->Resident_draw();
    }
#ifndef NDEBUG
    expect(residents.end() == end);
#endif
}

Resident* Room::find_with_types (uint32 types) {
    for (Resident* a : residents) {
        if (a->types & types) return a;
    }
    return null;
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
