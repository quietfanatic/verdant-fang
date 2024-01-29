#pragma once
#include "../dirt/uni/linked.h"
#include "common.h"

namespace vf {

struct Room {
    LinkedList<Resident> residents;
    void enter ();
    void exit ();
};

struct Resident : Linked<Resident> {
    Room* room = null;
    Vec pos = GNAN;
    Rect box = GNAN;
    Room* get_room () const { return room; }
    void set_room (Room* r) {
        if (room) unlink();
        if (r) link(&r->residents);
    }
    virtual void Resident_emerge () { }
    virtual void Resident_reclude () { }
};

} // vf
