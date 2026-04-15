#include "../combat.h"
#include <thread>
#include <chrono>
#include <Windows.h>
#include "../../../util/console/console.h"
#include <iostream>
#include <random>
#include "../../hook.h"
#include "../../wallcheck/wallcheck.h"
#include "../../../util/classes/math/structs.h" // Include for Vector2 and Vector3
#include "../../../util/classes/math/math.h"
#include "../../../util/teamcheck.h"

using namespace roblox;
using namespace math;

static math::Vector2 partpos = {};
static uint64_t cachedPositionX = 0;
static uint64_t cachedPositionY = 0;
static bool dataReady = false;

using namespace roblox;

bool shouldSilentAimBeActive() {
    return (globals::focused && keybind::is_roblox_focused() && globals::combat::silentaim && globals::combat::silentaimkeybind.enabled);
}

roblox::player gettargetclosesttomousesilent() {
    static HWND robloxWindow = nullptr;
    static auto lastWindowCheck = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();
    if (!robloxWindow || std::chrono::duration_cast<std::chrono::seconds>(now - lastWindowCheck).count() > 5) {
        robloxWindow = FindWindowA(nullptr, "Roblox");
        lastWindowCheck = now;
    }

    if (!robloxWindow) return {};
    
    // Don't find new targets if target was knocked
    if (globals::target_was_knocked) return {};

    POINT point;
    if (!GetCursorPos(&point) || !ScreenToClient(robloxWindow, &point)) {
        return {};
    }

    const math::Vector2 curpos = { static_cast<float>(point.x), static_cast<float>(point.y) };
    
    std::vector<roblox::player> current_players;
    {
        std::lock_guard<std::mutex> lock(globals::instances::cachedplayers_mutex); // Lock mutex
        current_players = globals::instances::cachedplayers;
    }

    if (current_players.empty()) return {};

    const bool useKnockCheck = globals::combat::knockcheck;
    const bool useWallCheck = globals::combat::wallcheck;
    const bool useFov = globals::combat::silentaimfov;
    const bool useHealthCheck = globals::combat::healthcheck; // Keep healthcheck as a separate boolean
    const float fovSize = globals::combat::silentaimfovsize;
    const float fovSizeSquared = fovSize * fovSize;
    const float healthThreshold = globals::combat::healththreshhold;
    const bool useTeamCheck = globals::combat::teamcheck || ((*globals::combat::flags)[0] != 0);
    const bool isArsenal = (globals::instances::gamename == "Arsenal");
    const std::string& localPlayerName = globals::instances::localplayer.get_name();
    const math::Vector3 cameraPos = globals::instances::camera.getPos();

    roblox::player closest = {};
    float closestDistanceSquared = 9e18f;

    for (auto player : current_players) { // Iterate over the local copy
        if (!player.head.is_valid() ||
            player.name == localPlayerName ||
            !player.main.is_valid()) { // Combined validity check
            continue;
        }

        // Skip if the player is marked as friendly
        if ((globals::bools::player_status.count(player.name) && globals::bools::player_status[player.name]) ||
            (globals::bools::player_status.count(player.displayname) && globals::bools::player_status[player.displayname])) {
            continue;
        }

        // Team check: skip players on the same team as local player
        if (useTeamCheck && teamcheck::same_team(globals::instances::lp, player)) continue;

        // If "Target Only" is enabled, only target players in the target_only_list
        if (globals::visuals::target_only_esp) {
            bool is_in_target_list = false;
            for (const auto& target_name : globals::visuals::target_only_list) {
                if (player.name == target_name) {
                    is_in_target_list = true;
                    break;
                }
            }
            if (!is_in_target_list) {
                continue;
            }
        }

        const math::Vector2 screenCoords = roblox::worldtoscreen(player.head.get_pos());
        if (screenCoords.x == -1.0f || screenCoords.y == -1.0f) continue;

        // Robust alive check (handles Arsenal)
        bool isAlive = true;
        if (globals::instances::gamename == "Arsenal") {
            try {
                auto nrpbs = player.main.findfirstchild("NRPBS");
                if (nrpbs.is_valid()) {
                    auto h = nrpbs.findfirstchild("Health");
                    if (h.is_valid()) {
                        double hv = h.read_double_value();
                        if (hv <= 0.0) isAlive = false;
                    }
                }
            } catch (...) {}
            if (isAlive && player.health <= 0) isAlive = false;
            // Release lock if player falls into the void (standard Arsenal behavior)
            if (isAlive && player.head.is_valid() && player.head.get_pos().y < -50.0f) isAlive = false;
        } else {
            auto humanoid = player.instance.findfirstchild("Humanoid");
            if (!humanoid.is_valid() || humanoid.get_health() <= 0) {
                isAlive = false;
            }
        }
        if (!isAlive) continue;

        const float dx = curpos.x - screenCoords.x;
        const float dy = curpos.y - screenCoords.y;
        const float distanceSquared = dx * dx + dy * dy;

        if (useFov && distanceSquared > fovSizeSquared) continue;

        // if (isArsenal) {
        //     auto nrpbs = player.main.findfirstchild("NRPBS");
        //     if (nrpbs.address) {
        //         auto health = nrpbs.findfirstchild("Health");
        //         if (health.address && (health.read_double_value() == 0.0 || player.hrp.get_pos().y < 0.0f)) {
        //             continue;
        //         }
        //     }
        // }

        auto bodyEffects = player.instance.findfirstchild("BodyEffects");
        if (bodyEffects.is_valid()) { // Use is_valid()
            if (bodyEffects.findfirstchild("Dead").read_bool_value()) continue;
            if (useKnockCheck && bodyEffects.findfirstchild("K.O").read_bool_value()) continue;
        }


        // const bool useforcefieldcheck = (*globals::combat::flags)[6];
        // bool hasaforcefield = false;
        // if (useforcefieldcheck) {
        //     auto children = player.instance.get_children();
        //     for (roblox::instance& instance : children) {
        //         if (instance.get_name() == "ForceField") {
        //             hasaforcefield = true;
        //             break;
        //         }
        //     }
        //     if (hasaforcefield) {
        //         continue;
        //     }
        // }
        // const bool usegrabbedcheck = (*globals::combat::flags)[5];
        // bool isgrabbed = false;
        // if (usegrabbedcheck) {
        //     auto children = player.instance.get_children();
        //     for (roblox::instance& instance : children) {
        //         if (instance.get_name() == "GRABBING_CONSTRAINT") {
        //             isgrabbed = true;
        //             break;
        //         }
        //     }
        //     if (isgrabbed) {
        //         continue;
        //     }
        // }


        if (useHealthCheck && player.health <= healthThreshold) continue;

        if (useWallCheck) {
            Vector3 player_head_pos = player.head.get_pos();
            Vector3 current_camera_pos = globals::instances::camera.getPos();
            if (!wallcheck::is_valid_vector(player_head_pos) || !wallcheck::is_valid_vector(current_camera_pos)) {
                continue; // Skip if positions are invalid
            }
            if (!wallcheck::can_see(player_head_pos, current_camera_pos)) continue;
        }

        // Check distance to target
        if (globals::combat::rangecheck) {
            Vector3 player_head_pos = player.head.get_pos();
            Vector3 current_camera_pos = globals::instances::camera.getPos();
            if (!wallcheck::is_valid_vector(player_head_pos) || !wallcheck::is_valid_vector(current_camera_pos)) {
                continue; // Skip if positions are invalid
            }
            float distanceToTarget = CalculateDistance1(player_head_pos, current_camera_pos);
            if (distanceToTarget > globals::combat::aim_distance) {
                continue;
            }
        }

        if (distanceSquared < closestDistanceSquared) {
            closestDistanceSquared = distanceSquared;
            closest = player;
        }
    }

    return closest;
}

void hooks::silentrun() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    HWND robloxWindow = FindWindowA(0, "Roblox");
    const std::chrono::milliseconds sleepTime(10);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        // Keybind updates are centralized in the aimbot loop to avoid
        // double-toggles when sharing the same key across modules.

        // Skip silent aim during teleport recovery to prevent auto-targeting
        if (globals::handlingtp || globals::prevent_auto_targeting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            dataReady = false;
            continue;
        }

        if (!shouldSilentAimBeActive()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            dataReady = false;
            // globals::instances::cachedtarget is now managed by aimbot.cpp
            continue;
        }
        
        const bool stickySilentActive = globals::combat::stickyaimsilent && shouldSilentAimBeActive();

        // Prefer cached aimbot target if it fits silent criteria; otherwise, pick our own
        roblox::player sTarget = {};
        if (!globals::target_was_knocked) {
            if (globals::instances::cachedtarget.head.is_valid()) {
                sTarget = globals::instances::cachedtarget;
            }
        }
        // If we don't have a candidate yet (or sticky silent not active), select a silent-only target so silent can lock independently
        const bool stickyLocked = stickySilentActive && sTarget.head.is_valid();
        if (!sTarget.head.is_valid() || !stickyLocked) {
            if (!stickyLocked) {
                roblox::player alt = gettargetclosesttomousesilent();
                if (alt.head.is_valid()) {
                    sTarget = alt;
                } else if (!sTarget.head.is_valid()) { // nothing to use
                    dataReady = false;
                    continue;
                }
            }
        }
        {
            // Scoped processing for current candidate target
            // Ensure cached target is not a teammate (handles on-the-fly team swaps)
            const bool useTeamCheckLive = globals::combat::teamcheck || ((*globals::combat::flags)[0] != 0);
            if (useTeamCheckLive && teamcheck::same_team(globals::instances::lp, sTarget)) { dataReady = false; continue; }

            // Re-evaluate FOV for the candidate target; if sticky silent aim is active, keep the locked target even outside FOV
            const bool useFov = globals::combat::silentaimfov;
            const float fovSize = globals::combat::silentaimfovsize;
            const float allowedRadius = fovSize;
            const float allowedRadiusSquared = allowedRadius * allowedRadius;
            if (useFov) {
                POINT point;
                if (!GetCursorPos(&point) || !ScreenToClient(robloxWindow, &point)) {
                    dataReady = false;
                    continue;
                }
                const math::Vector2 curpos = { static_cast<float>(point.x), static_cast<float>(point.y) };
                const math::Vector2 screenCoords = roblox::worldtoscreen(sTarget.head.get_pos());
                bool inFov = false;
                if (!(screenCoords.x == -1.0f || screenCoords.y == -1.0f)) {
                    const float dx = curpos.x - screenCoords.x;
                    const float dy = curpos.y - screenCoords.y;
                    const float distanceSquared = dx * dx + dy * dy;
                    inFov = (distanceSquared <= allowedRadiusSquared);
                }
                if (!inFov) {
                    if (stickyLocked) {
                        // Stick to current target but pause silent write when far outside allowed radius
                        dataReady = false;
                        continue;
                    }
                    // Try to find a better target within silent FOV
                    roblox::player alt = gettargetclosesttomousesilent();
                    if (alt.head.is_valid()) {
                        sTarget = alt;
                    } else {
                        dataReady = false;
                        continue;
                    }
                }
            }

            // If wallcheck is enabled and target is obstructed, reset dataReady
            if (globals::combat::wallcheck && !wallcheck::can_see(sTarget.head.get_pos(), globals::instances::camera.getPos())) {
                dataReady = false;
                continue; // Skip silent aim for this frame
            }

            roblox::instance targetPart;
            float closest_dist = FLT_MAX;

            enum HumanoidStateType {
                Jumping = 3,
                Freefall = 10
            };

            // Determine airborne state based on TARGET (not local player)
            int target_state = sTarget.humanoid.is_valid() ? sTarget.humanoid.get_state() : -1;
            bool target_air = (target_state == Jumping ||
                /* handle possible Freefall enum differences */ target_state == Freefall ||
                target_state == 6 /* Freefall in some builds */);
            // Velocity-based fallback to detect airborne reliably (target)
            if (!target_air) {
                roblox::instance body = sTarget.hrp.is_valid() ? sTarget.hrp : (sTarget.uppertorso.is_valid() ? sTarget.uppertorso : sTarget.head);
                if (body.is_valid()) {
                    auto v = body.get_velocity();
                    if (v.y > 2.0f || v.y < -2.0f) target_air = true;
                }
            }

            // Also consider local player's airborne state to satisfy both interpretations of "airpart"
            int local_state = globals::instances::lp.humanoid.is_valid() ? globals::instances::lp.humanoid.get_state() : -1;
            bool local_air = (local_state == Jumping || local_state == Freefall || local_state == 6);
            if (!local_air) {
                roblox::instance lbody = globals::instances::lp.hrp.is_valid() ? globals::instances::lp.hrp : (globals::instances::lp.uppertorso.is_valid() ? globals::instances::lp.uppertorso : globals::instances::lp.head);
                if (lbody.is_valid()) {
                    auto v = lbody.get_velocity();
                    if (v.y > 2.0f || v.y < -2.0f) local_air = true;
                }
            }

            bool is_airborne = (target_air || local_air);

            std::vector<int>* silentaimpart_to_use = is_airborne ? globals::combat::airsilentaimpart : globals::combat::silentaimpart;
            
            int selected_idx = -1;
            std::string selected_name;
            bool is_specific_part_selected = false;
            for (int i = 0; i < 14; ++i) {
                if ((*silentaimpart_to_use)[i]) {
                    is_specific_part_selected = true;
                    break;
                }
            }

            if (globals::combat::silent_closest_part) {
                // Check ALL parts (0-13) for the closest one
                for (int i = 0; i < 14; ++i) {
                    roblox::instance part = sTarget.instance.findfirstchild(globals::combat::hit_parts[i]);
                    if (!part.is_valid() && sTarget.rigtype == 1) {
                        if (i == 3) part = sTarget.instance.findfirstchild("UpperTorso");
                        else if (i == 4) part = sTarget.instance.findfirstchild("LeftUpperLeg");
                        else if (i == 5) part = sTarget.instance.findfirstchild("RightUpperLeg");
                    }
                    if (part.is_valid()) {
                        math::Vector2 screen_pos = roblox::worldtoscreen(part.get_pos());
                        if (screen_pos.x != -1) {
                            POINT mouse_point;
                            GetCursorPos(&mouse_point);
                            ScreenToClient(robloxWindow, &mouse_point);
                            float dist = sqrt(pow(screen_pos.x - mouse_point.x, 2) + pow(screen_pos.y - mouse_point.y, 2));
                            if (dist < closest_dist) {
                                closest_dist = dist;
                                targetPart = part;
                                selected_idx = i;
                                selected_name = part.get_name();
                            }
                        }
                    }
                }
            } else {
                // Randomize among SELECTED enabled parts with timer
                static std::mt19937 rng(std::random_device{}());
                static std::chrono::steady_clock::time_point last_switch_time = std::chrono::steady_clock::now();
                static int current_random_idx = -1;
                auto now = std::chrono::steady_clock::now();
                
                std::vector<std::pair<roblox::instance, int>> valid_parts;
                for (int i = 0; i < 14; ++i) {
                    if ((*silentaimpart_to_use)[i]) { // Only enabled parts
                        roblox::instance part = sTarget.instance.findfirstchild(globals::combat::hit_parts[i]);
                        if (!part.is_valid() && sTarget.rigtype == 1) {
                            if (i == 3) part = sTarget.instance.findfirstchild("UpperTorso");
                            else if (i == 4) part = sTarget.instance.findfirstchild("LeftUpperLeg");
                            else if (i == 5) part = sTarget.instance.findfirstchild("RightUpperLeg");
                        }
                        if (part.is_valid()) {
                            valid_parts.push_back({ part, i });
                        }
                    }
                }
                
                if (!valid_parts.empty()) {
                    // Initial pick or timer elapsed
                    if (current_random_idx == -1 || 
                        std::chrono::duration_cast<std::chrono::milliseconds>(now - last_switch_time).count() > 300) { // 300ms delay
                        std::uniform_int_distribution<size_t> dist(0, valid_parts.size() - 1);
                        current_random_idx = static_cast<int>(dist(rng));
                        last_switch_time = now;
                    }
                    
                    // Ensure index is valid for current set
                    if (current_random_idx >= valid_parts.size()) {
                        current_random_idx = 0;
                    }

                    auto& chosen = valid_parts[current_random_idx];
                    targetPart = chosen.first;
                    selected_idx = chosen.second;
                    selected_name = targetPart.get_name();
                } else {
                    current_random_idx = -1;
                    // Fallback if nothing selected
                    if (!is_specific_part_selected) {
                        // Check Head
                        roblox::instance part = sTarget.instance.findfirstchild("Head");
                        if (part.is_valid()) {
                            targetPart = part;
                            selected_idx = 0;
                            selected_name = "Head";
                        }
                    }
                }
            }

            // Record which part is currently used for silent aim (for debug/indicator)
            if (selected_idx >= 0) {
                globals::combat::last_used_aimpart = selected_name.empty() ? globals::combat::hit_parts[selected_idx] : selected_name;
                globals::combat::last_used_aimpart_air = is_airborne;
            }

            if (targetPart.is_valid()) {
                math::Vector3 part3d = targetPart.get_pos();
                
                if (globals::combat::silentpredictions) {
					Vector3 velocity = targetPart.get_velocity();
					Vector3 veloVector = DivVector3(velocity, { globals::combat::silentpredictionsx, globals::combat::silentpredictionsy, globals::combat::silentpredictionsx });
					part3d = AddVector3(part3d, veloVector);
				}

                partpos = roblox::worldtoscreen(part3d);

                math::Vector2 dimensions = globals::instances::visualengine.get_dimensions();
                POINT cursorPoint;
                GetCursorPos(&cursorPoint);
                ScreenToClient(robloxWindow, &cursorPoint);

                cachedPositionX = static_cast<uint64_t>(cursorPoint.x);
                cachedPositionY = static_cast<uint64_t>(dimensions.y - std::abs(dimensions.y - (cursorPoint.y)) - 58);
                // Publish the target so the write thread doesn't depend on aimbot's cache
                globals::instances::cachedtarget = sTarget;
                globals::instances::cachedlasttarget = sTarget;
                dataReady = true;
            }
            else {
                dataReady = false;
            }
        }
    }
}

void hooks::silentrun2() {
    roblox::instance mouseServiceInstance;
    mouseServiceInstance.address = globals::instances::mouseservice;
    std::random_device rd;
    std::mt19937 gen(rd());

    while (true) {
        // Skip silent aim during teleport recovery to prevent auto-targeting
        if (globals::handlingtp || globals::prevent_auto_targeting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        if (!shouldSilentAimBeActive()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (globals::instances::cachedtarget.head.is_valid() && dataReady) {
            if (globals::combat::silentaimtype == 0) { // Freeaim
                globals::instances::aim.setFramePosX(cachedPositionX);
                globals::instances::aim.setFramePosY(cachedPositionY);
                mouseServiceInstance.write_mouse_position(globals::instances::mouseservice, partpos.x, partpos.y);
            }
            else { // Mouse
                mouseServiceInstance.write_mouse_position(globals::instances::mouseservice, partpos.x, partpos.y);
            }
            std::cout << "Mouse position written to: (" << partpos.x << ", " << partpos.y << ")" << std::endl;
        }
    }
}

void hooks::silent() {
    std::thread primaryThread(&hooks::silentrun);
    std::thread secondaryThread(&hooks::silentrun2);
    primaryThread.detach();
    secondaryThread.detach();
}
