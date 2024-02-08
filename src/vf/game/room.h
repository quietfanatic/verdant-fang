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

     // Get first resident whose types includes these one (bitwise and).
     // Returns null if there is none.
    Resident* find_with_types (uint32 types);
};

 // Hitboxes determine when Resident_on_collide is called.  The algorithm looks
 // like this.
 //     Resident a, b;
 //     Hitbox a_hb, b_hx;
 //     if (a_hb.layers_1 & b_hb.layers_2 &&
 //         overlaps(a_hb.box + a.pos, b_hb.box + b.pos)
 //     ) {
 //         a.Resident_on_collide(a_hb, b, b_hb);
 //     }
 //     else if (b_hb.layers_1 & a_hb.layers_2 &&
 //         overlaps(b_hb.box + b.pos, a_hb.box + a.pos)
 //     ) {
 //         b.Resident_on_collide(b_hb, a, a_hb);
 //     }
struct Hitbox {
    uint32 layers_1 = 0;
    uint32 layers_2 = 0;
    Rect box;
     // Mirror this hitbox horizontally around pos.x, keeping the Rect proper
    void fliph () {
        float t = -box.r; box.r = -box.l; box.l = t;
    }
};

struct Resident : Linked<Resident> {
    Room* room = null;
    Vec pos = GNAN;
    Slice<Hitbox> hitboxes;
    uint32 types = 0;
    Room* get_room () const { return room; }
    void set_room (Room* r) {
        if (room) unlink();
        room = r;
        if (room) link(&room->residents);
    }

    void set_pos (Vec p) { Resident_set_pos(p); }

     // The use case for Resident_set_pos is for Walker to reset its
     // walk_start_x.
    virtual void Resident_set_pos (Vec p) { pos = p; }
    virtual void Resident_on_enter () { }
    virtual void Resident_on_exit () { }
    virtual void Resident_before_step () { }
     // See Hitbox
    virtual void Resident_on_collide (
        const Hitbox&, // a_hb
        Resident&,  // b
        const Hitbox&  // b_hb
    ) { }
    virtual void Resident_after_step () { }
    virtual void Resident_draw () { }

    Resident () = default;
    Resident (const Resident&) = delete;
};

} // vf
