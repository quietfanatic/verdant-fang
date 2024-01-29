#include "room.h"
#include "../dirt/ayu/reflection/describe.h"

namespace vf {

void Room::enter () {
    for (Resident& o : residents) {
        o.Resident_emerge();
    }
}
void Room::exit () {
    for (Resident& o : residents) {
        o.Resident_reclude();
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::Room,
    attrs()
)

AYU_DESCRIBE(vf::Resident,
    attrs(
        attr("room", value_methods<Room*, &Resident::get_room, &Resident::set_room>(), optional),
        attr("pos", &Resident::pos, optional),
        attr("box", &Resident::box, optional)
    )
)
