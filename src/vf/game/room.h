#pragma once
#include "../../dirt/glow/colors.h"
#include "../../dirt/uni/linked.h"
#include "common.h"

namespace vf {

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
    Hitbox* next = null;
    uint32 layers_1 = 0;
    uint32 layers_2 = 0;
    Rect box;
     // Mirror this hitbox horizontally around pos.x, keeping the Rect proper
    void fliph () {
        float t = -box.r; box.r = -box.l; box.l = t;
    }
};

struct Resident {
    Room* room = null;
    Vec pos = GNAN;
    Hitbox* first_hitbox = null;
    uint32 types = 0;
    Room* get_room () const { return room; }
    void set_room (Room* r);

    void set_pos (Vec p) { Resident_set_pos(p); }

    void clear_hitboxes () { first_hitbox = null; }
    void add_hitbox (Hitbox& hb) { hb.next = first_hitbox; first_hitbox = &hb; }
    void set_hitbox (Hitbox& hb) { hb.next = null; first_hitbox = &hb; }

     // The use case for Resident_set_pos is for Walker to reset its
     // walk_start_x.
    virtual void Resident_set_pos (Vec p) { pos = p; }
    virtual void Resident_before_step () { }
     // See Hitbox
    virtual void Resident_on_collide (
        const Hitbox&, // a_hb
        Resident&,  // b
        const Hitbox&  // b_hb
    ) { }
    virtual void Resident_after_step () { }
    virtual void Resident_draw () { }
    virtual void Resident_on_enter () { }
    virtual void Resident_on_exit () { }

    Resident () = default;
    Resident (const Resident&) = delete;
};

struct Room {
     // Copy this AnyArray whenever you iterate it, in case a resident changed
     // its room while you're iterating.  The AnyArray is copy-on-write, so it
     // won't actually copy the buffer unless the array actually gets modified.
    AnyArray<Resident*> residents;
    glow::RGBA8 background_color;
    void enter ();
    void exit ();
    void step ();
    void draw ();

     // Get first resident whose types includes these ones (bitwise and).
     // Returns null if there is none.
    Resident* find_with_types (uint32 types);
};

} // vf
