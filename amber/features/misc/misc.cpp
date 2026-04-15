#include "misc.h"
#include "../../util/globals.h"
#include "../../util/classes/classes.h"
#include "../hook.h"

void hooks::jumppower() {
    while (true) {
        if (!globals::unattach && globals::focused && keybind::is_roblox_focused()) {
            globals::misc::jumppowerkeybind.update();
            if (globals::instances::lp.humanoid.is_valid()) {
                if (globals::misc::jumppower && globals::misc::jumppowerkeybind.enabled) {
                    globals::instances::lp.humanoid.write_jumppower(globals::misc::jumppowervalue);
                } else {
                    // Reset to default Roblox jump power when disabled
                    globals::instances::lp.humanoid.write_jumppower(50.0f);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void hooks::macro() {
    while (true) {
        if (!globals::unattach && globals::focused && keybind::is_roblox_focused()) {
            globals::misc::macro_keybind.update();
            if (globals::misc::macro_enabled && globals::misc::macro_keybind.enabled) {
                // Send Middle Click
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds((int)globals::misc::macro_delay));

                // Send Middle Click again
                mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds((int)globals::misc::macro_delay));

                // Send Wheel Up (Positive delta)
                mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 120, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds((int)globals::misc::macro_delay));

                // Send Wheel Down (Negative delta)
                mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -120, 0);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

namespace AMBER {
	namespace misc {
		void run() {
			// This function can be used for general misc features that run continuously
		}

		void autofriend() {
			// Auto-friend functionality is handled in the UI
			// This function can be used for any continuous autofriend operations
		}

	}
}
