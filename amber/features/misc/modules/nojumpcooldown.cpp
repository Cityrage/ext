#include "nojumpcooldown.h"
#include "../../../util/globals.h"
#include "../../../util/classes/classes.h"
#include "../../hook.h"

void hooks::nojumpcooldown() {
    while (true) {
        if (!globals::unattach && globals::focused && keybind::is_roblox_focused()) {
            if (globals::misc::nojumpcooldown && globals::instances::lp.humanoid.is_valid()) {
                globals::instances::lp.humanoid.write_jumppower(55.0f);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

