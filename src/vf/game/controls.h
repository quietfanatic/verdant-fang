#pragma once

#include <array>
#include "common.h"

namespace vf {

 // Not enum class so it can index an array more easily
namespace _ {
enum Control : uint8 {
    Confirm,
    Back,
    Pause,
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

using Controls = std::array<bool, Control::N_Controls>;

struct Mind {
    virtual Controls Mind_think (Resident&) = 0;
};

struct PlayerMind : Mind {
    Controls Mind_think (Resident&) override;
};

} // vf

