#pragma once
#include "../dirt/glow/colors.h"
#include "../dirt/uni/linked.h"
#include "common.h"

namespace vf {

enum class Layers : uint32 {
    Player_Block = 1
};
DECLARE_ENUM_BITWISE_OPERATORS(Layers)

struct Room {
    LinkedList<Resident> residents;
    glow::RGBA8 background_color;
    void enter ();
    void exit ();
    void step ();
    void draw ();
};

struct Resident : Linked<Resident> {
    Room* room = null;
    Rect bounds = GNAN;
    Vec pos;
     // For collision.  If a.layers_1 & b.layers_2, then a.Resident_collide(b)
     // will be called, and vice versa.
    Layers layers_1 = {};
    Layers layers_2 = {};
    Room* get_room () const { return room; }
    void set_room (Room* r) {
        if (room) unlink();
        if (r) link(&r->residents);
    }
    virtual void Resident_emerge () { }
    virtual void Resident_reclude () { }
    virtual void Resident_before_step () { }
    virtual void Resident_collide (Resident&) { }
    virtual void Resident_after_step () { }
    virtual void Resident_draw () { }
};

} // vf
