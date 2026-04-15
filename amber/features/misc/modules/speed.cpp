#include "../../hook.h"
#include "../../../util/console/console.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include "../../../util/classes/math/math.h"

void hooks::speed() {
    static float original_walkspeed = 16.0f;
    static bool walkspeed_modified = false;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Small sleep to prevent busy-waiting

        globals::misc::speedkeybind.update();

        if (!globals::focused || !keybind::is_roblox_focused()) {
            if (walkspeed_modified) {
                globals::instances::lp.humanoid.write_walkspeed(original_walkspeed);
                walkspeed_modified = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (globals::misc::speed && globals::misc::speedkeybind.enabled) {
            if (!walkspeed_modified) {
                original_walkspeed = globals::instances::lp.humanoid.read_walkspeed();
                walkspeed_modified = true;
            }

            switch (globals::misc::speedtype) {
                case 0: { // WalkSpeed + Velocity (Hybrid for instant speed)
                    globals::instances::lp.humanoid.write_walkspeed(globals::misc::speedvalue);

                    auto hrp = globals::instances::lp.hrp;
                    math::Vector3 move_dir = globals::instances::lp.humanoid.get_move_dir(); // Corrected source
                    
                    // Only apply velocity if we are actually trying to move
                    if (move_dir.x != 0 || move_dir.z != 0) {
                        math::Vector3 current_vel = hrp.get_velocity();
                        // Preserve vertical velocity (gravity/jumping) but override horizontal
                        math::Vector3 new_vel = {
                            move_dir.x * globals::misc::speedvalue,
                            current_vel.y,
                            move_dir.z * globals::misc::speedvalue
                        };
                        hrp.write_velocity(new_vel);
                    }
                    else { // Stop instantly when key is released (for responsiveness)
                         math::Vector3 current_vel = hrp.get_velocity();
                         math::Vector3 new_vel = { 0, current_vel.y, 0 };
                         hrp.write_velocity(new_vel);
                    }
                    break;
                }
                case 1: { // Velocity (Pure)
                    auto hrp = globals::instances::lp.hrp;
                    math::Vector3 move_dir = globals::instances::lp.humanoid.get_move_dir(); // Corrected source

                    if (move_dir.x != 0 || move_dir.z != 0) {
                        math::Vector3 current_vel = hrp.get_velocity();
                        math::Vector3 new_vel = {
                            move_dir.x * globals::misc::speedvalue,
                            current_vel.y,
                            move_dir.z * globals::misc::speedvalue
                        };
                        hrp.write_velocity(new_vel);
                    }
                    else {
                        // If not moving keys, kill horizontal momentum to stop sliding
                        math::Vector3 current_vel = hrp.get_velocity();
                         math::Vector3 new_vel = { 0, current_vel.y, 0 };
                         hrp.write_velocity(new_vel);
                    }
                    break;
                }
            }
        } else { // Correctly close if and open else
            if (walkspeed_modified) {
                globals::instances::lp.humanoid.write_walkspeed(original_walkspeed);
                walkspeed_modified = false;
            }
        }
    }
}