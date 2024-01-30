#include "room.h"

#include "../dirt/ayu/reflection/describe.h"
#include "../dirt/glow/gl.h"

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
void Room::step () {
    for (Resident& o : residents) {
        o.Resident_step();
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
    for (Resident& o : residents) {
        o.Resident_draw();
    }
}

} using namespace vf;

AYU_DESCRIBE(vf::Room,
    attrs(
        attr("background_color", &Room::background_color)
    )
)

AYU_DESCRIBE(vf::Resident,
    attrs(
        attr("room", value_methods<Room*, &Resident::get_room, &Resident::set_room>(), optional),
        attr("pos", &Resident::pos, optional),
        attr("box", &Resident::box, optional)
    )
)
