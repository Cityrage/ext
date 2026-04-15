#pragma once
#include "classes/classes.h"
#include "globals.h"
#include <unordered_map>
#include <chrono>

namespace teamcheck {
	bool same_team(const roblox::player& lp, const roblox::player& target);
}