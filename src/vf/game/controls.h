#pragma once

#include <array>
#include "common.h"

namespace vf {

 // Not enum class so it can index an array more easily
namespace _ {
enum Control : uint8 {
    Confirm,
    Back,
    Left,
    Right,
    Up,
    Down,
    Jump,
    Attack,
    Special,
    N_Controls,
};
} using _::Control;

 // Number of frames this control has been held, maxes out at 255.  This number
 // might only be meaningful for Player; other Minds might always put 1 for
 // everything.
using Controls = std::array<uint8, Control::N_Controls>;

struct Mind {
    virtual Controls Mind_think (Resident&) = 0;
};

struct Player : Mind {
    Controls controls = {};
    Controls Mind_think (Resident&) override;
};

} // vf

