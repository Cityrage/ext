#include "wallcheck.h"
#include "../../util/globals.h"
#include <mutex>
#include <algorithm>

namespace wallcheck {
    static std::vector<uintptr_t> part_cache;
    static std::mutex cache_mutex;

    bool is_valid_vector(math::Vector3 v) {
        return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
    }

    void get_all_parts(roblox::instance parent, std::vector<uintptr_t>& parts) {
        if (!parent.is_valid()) return;

        for (auto& child : parent.get_children()) {
            if (!child.is_valid()) continue;

            std::string class_name = child.get_class_name();
            
            // Check if this instance is a character model by looking for a Humanoid
            // If it's a character, we skip its entire subtree to avoid accessories/limbs blocking view
            if (class_name == "Model") {
                if (child.findfirstchild("Humanoid").is_valid()) {
                    continue; 
                }
            }

            if (class_name == "Part" || class_name == "MeshPart" || class_name == "WedgePart" || 
                class_name == "CornerWedgePart" || class_name == "TrussPart" || class_name == "UnionOperation") {
                
                // Read transparency to filter out ghost parts often used for barriers or triggers
                if (child.get_transparency() > 0.95f) {
                    continue;
                }
                parts.push_back(child.address);
            }
            
            // Recursively search children (Folders, Models, etc.)
            get_all_parts(child, parts);
        }
    }

    void update_cache() {
        if (!globals::instances::workspace.is_valid()) return;

        std::vector<uintptr_t> new_cache;
        get_all_parts(globals::instances::workspace, new_cache);

        std::lock_guard<std::mutex> lock(cache_mutex);
        part_cache = std::move(new_cache);
    }

    // Slab Method for Ray-OBB Intersection
    bool intersects_obb(math::Vector3 ray_origin, math::Vector3 ray_dir, math::Vector3 part_pos, math::Matrix3 part_rot, math::Vector3 part_size, float& distance) {
        // Transform ray to local space of the part
        math::Vector3 relative_origin = ray_origin - part_pos;
        
        // Inverse rotation (transpose for orthonormal matrices)
        math::Vector3 local_origin = {
            relative_origin.x * part_rot.data[0] + relative_origin.y * part_rot.data[3] + relative_origin.z * part_rot.data[6],
            relative_origin.x * part_rot.data[1] + relative_origin.y * part_rot.data[4] + relative_origin.z * part_rot.data[7],
            relative_origin.x * part_rot.data[2] + relative_origin.y * part_rot.data[5] + relative_origin.z * part_rot.data[8]
        };

        math::Vector3 local_dir = {
            ray_dir.x * part_rot.data[0] + ray_dir.y * part_rot.data[3] + ray_dir.z * part_rot.data[6],
            ray_dir.x * part_rot.data[1] + ray_dir.y * part_rot.data[4] + ray_dir.z * part_rot.data[7],
            ray_dir.x * part_rot.data[2] + ray_dir.y * part_rot.data[5] + ray_dir.z * part_rot.data[8]
        };

        math::Vector3 half_size = part_size * 0.5f;
        float t_min = -1e30f;
        float t_max = 1e30f;

        // X planes
        if ((std::abs)(local_dir.x) > 1e-6f) {
            float t1 = (-half_size.x - local_origin.x) / local_dir.x;
            float t2 = (half_size.x - local_origin.x) / local_dir.x;
            t_min = (std::max)(t_min, (std::min)(t1, t2));
            t_max = (std::min)(t_max, (std::max)(t1, t2));
        } else if ((std::abs)(local_origin.x) > half_size.x) return false;

        // Y planes
        if ((std::abs)(local_dir.y) > 1e-6f) {
            float t1 = (-half_size.y - local_origin.y) / local_dir.y;
            float t2 = (half_size.y - local_origin.y) / local_dir.y;
            t_min = (std::max)(t_min, (std::min)(t1, t2));
            t_max = (std::min)(t_max, (std::max)(t1, t2));
        } else if ((std::abs)(local_origin.y) > half_size.y) return false;

        // Z planes
        if ((std::abs)(local_dir.z) > 1e-6f) {
            float t1 = (-half_size.z - local_origin.z) / local_dir.z;
            float t2 = (half_size.z - local_origin.z) / local_dir.z;
            t_min = (std::max)(t_min, (std::min)(t1, t2));
            t_max = (std::min)(t_max, (std::max)(t1, t2));
        } else if ((std::abs)(local_origin.z) > half_size.z) return false;

        if (t_max < 0 || t_min > t_max) return false;
        
        distance = t_min;
        return true;
    }

    bool can_see(math::Vector3 origin, math::Vector3 target) {
        if (!is_valid_vector(origin) || !is_valid_vector(target)) return false;

        math::Vector3 direction = target - origin;
        float target_dist = direction.magnitude();
        if (target_dist < 0.1f) return true;

        math::Vector3 unit_dir = direction / target_dist;

        std::uintptr_t local_char_addr = globals::instances::lp.instance.address;

        std::lock_guard<std::mutex> lock(cache_mutex);
        for (uintptr_t part_addr : part_cache) {
            roblox::instance part{ part_addr };
            if (!part.is_valid()) continue;

            // Basic filtering
            // Ignore if part belongs to local player character
            roblox::instance parent = part.read_parent();
            if (parent.address == local_char_addr) continue;

            // Ignore if part belongs to target player character (avoid self-blocking)
            // This is handled by aimbot by passing head pos slightly offset or checking distance

            math::Vector3 part_pos = part.get_pos();
            math::Vector3 part_size = part.get_part_size();
            math::Matrix3 part_rot = part.read_part_cframe();

            float hit_dist;
            if (intersects_obb(origin, unit_dir, part_pos, part_rot, part_size, hit_dist)) {
                if (hit_dist > 0.01f && hit_dist < target_dist - 0.5f) {
                    return false; // Obstruction found
                }
            }
        }

        return true;
    }
}
