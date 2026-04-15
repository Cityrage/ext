#include "teamcheck.h"
#include "classes/classes.h"
#include "globals.h"
#include "offsets.h"

using namespace roblox;

bool teamcheck::same_team(const roblox::player& lp, const roblox::player& target) {
    if (!lp.main.is_valid() || !target.main.is_valid()) return false;

    // Check Team instance
    if (lp.team.is_valid() && target.team.is_valid()) {
        if (lp.team.address == target.team.address) return true;
    }

    // Fallback to TeamColor (BrickColor) property on Player object
    int lp_color = read<int>(lp.main.address + offsets::TeamColor);
    int target_color = read<int>(target.main.address + offsets::TeamColor);

    return (lp_color != 0 && lp_color == target_color);
}
