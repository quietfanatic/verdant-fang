#pragma once
#include "../../dirt/glow/colors.h"
#include "../../dirt/uni/linked.h"
#include "common.h"

namespace vf {

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
    Vec pos = GNAN;
     // For collision.  If a.layers_1 & b.layers_2, then a.Resident_collide(b)
     // will be called, and vice versa.
    uint32 layers_1 = {};
    uint32 layers_2 = {};
    Room* get_room () const { return room; }
    void set_room (Room* r) {
        if (room) unlink();
        room = r;
        if (room) link(&room->residents);
    }
    virtual void Resident_emerge () { }
    virtual void Resident_reclude () { }
    virtual void Resident_before_step () { }
    virtual void Resident_collide (Resident&) { }
    virtual void Resident_after_step () { }
    virtual void Resident_draw () { }
};

} // vf
