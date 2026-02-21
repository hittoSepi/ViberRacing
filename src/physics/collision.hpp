#pragma once

#include "core/types.hpp"

namespace viber {

namespace CollisionGroups {
    enum : short {
        None = 0,
        Default = 1 << 0,
        Static = 1 << 1,
        Vehicle = 1 << 2,
        Wheel = 1 << 3,
        Trigger = 1 << 4,
        Checkpoint = 1 << 5,
        Prop = 1 << 6,
    };
}

namespace CollisionMasks {
    enum : short {
        None = 0,
        All = -1,
        Static = CollisionGroups::Static | CollisionGroups::Default,
        Vehicle = CollisionGroups::Static | CollisionGroups::Vehicle | 
                  CollisionGroups::Default | CollisionGroups::Prop,
        Wheel = CollisionGroups::Static | CollisionGroups::Default,
        Trigger = CollisionGroups::Vehicle,
        Checkpoint = CollisionGroups::Vehicle,
    };
}

inline short makeCollisionMask(short groups) {
    return groups;
}

inline bool shouldCollide(short groupA, short maskA, short groupB, short maskB) {
    return (groupA & maskB) != 0 && (groupB & maskA) != 0;
}

}
