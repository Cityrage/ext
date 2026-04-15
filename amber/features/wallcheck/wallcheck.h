#pragma once
#include "../../util/classes/classes.h"
#include <vector>

namespace wallcheck {
    bool can_see(math::Vector3 origin, math::Vector3 target);
    void update_cache();
    bool is_valid_vector(math::Vector3 v);
}
