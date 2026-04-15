#pragma once
#include <array>
#include <map>
#include <vector>
#include <string>
#include <mutex>
#include "../util/classes/classes.h"
#include "../drawing/overlay/keybind/keybind.h"
namespace globals
{
	inline uint64_t place_id;
	inline std::string game = "Universal";
	inline bool focused;
	inline bool prevent_auto_targeting = false; // Prevents auto-targeting after rescanning
	inline bool target_was_knocked = false; // Prevents auto-targeting when current target is knocked
	inline int timezone_index = 0; // 0 = Local, 1 = EST, 2 = GMT, 3 = PST
	
	// Hood Customs Fight Detection System
	inline bool hood_customs_fighting = false; // Tracks if currently fighting in hood customs
	inline bool hood_customs_lock_during_fight = true; // Prevents unlocking during fights in hood customs
	inline std::chrono::steady_clock::time_point last_fight_time; // Last time a fight was detected
	inline std::vector<roblox::wall> walls;
	namespace bools {
		inline bool bring, kill;
		inline std::string name;
		inline roblox::player entity;
		inline std::map<std::string, bool> player_status; // true for friendly, false for enemy (default)
	}
	namespace instances {
		inline std::vector<std::string> whitelist;
		inline std::vector<std::string> blacklist;
		inline roblox::instance visualengine;
		inline roblox::instance datamodel;
		inline roblox::instance workspace;
		inline roblox::instance players;
		inline roblox::player lp;
		inline roblox::instance lighting;
		inline roblox::camera camera;
		inline roblox::instance localplayer;
		inline std::string gamename = "Universal";
		inline std::string username = "rico x kira";
		inline std::vector<roblox::player> cachedplayers;
		inline std::vector<roblox::instance> bots;
		inline roblox::player cachedtarget;
		inline roblox::player cachedlasttarget;
		inline roblox::instance aim;
		inline float path_copied_time = -10.0f;
		inline float addr_copied_time = -10.0f;
		inline float refresh_refreshed_time = -10.0f;
		inline uintptr_t mouseservice;
		inline ImDrawList* draw;
		inline std::mutex cachedplayers_mutex; // Mutex for cachedplayers
	}
    namespace combat {
		inline bool rapidfire = false;
		inline bool autostomp = false;
				inline bool aimbot = false;
		inline bool stickyaim = false;
		inline int aimbottype = 0;
		inline keybind aimbotkeybind("aimbotkeybind");
				inline bool usefov = false;
		inline bool drawfov = false;
		inline float fovsize = 50;
		inline bool glowfov = false;
inline float fovcolor[4] = {1,1,1,1};
		inline float fovtransparency = 1.0f;
		inline bool fovfill = false;
		inline float fovfilltransparency = 1.0f; // 0-1
		inline float fovfillcolor[4] = {1,1,1,1};
		inline float fovglowcolor[4] = {1,1,1,1};
		inline int fovshape = 0; // 0 = Circle, 1 = Square, 2 = Triangle, 3 = Pentagon, 4 = Hexagon, 5 = Octagon
		inline float fovthickness = 0.5f;
		inline bool smoothing = false;
		inline float smoothingx = 5;
		inline float smoothingy = 5;
		inline int smoothing_style = 0;
		inline const char* smoothing_styles[11] = { "None", "Linear", "EaseInQuad", "EaseOutQuad", "EaseInOutQuad", "EaseInCubic", "EaseOutCubic", "EaseInOutCubic", "EaseInSine", "EaseOutSine", "EaseInOutSine" };
		inline bool aimbot_shake = false;
		inline float aimbot_shake_x = 0.0f;
		inline float aimbot_shake_y = 0.0f;
		inline bool predictions = false;
		inline float predictionsx = 5;
		inline float predictionsy = 5;
		inline bool silentpredictions = false;
		inline float silentpredictionsx = 5;
		inline float silentpredictionsy = 5;
		inline bool silent_shake = false;
		inline float silent_shake_x = 0.0f;
		inline float silent_shake_y = 0.0f;
		inline bool teamcheck = false;
		inline bool knockcheck = false;
		inline bool rangecheck = false;
		inline bool healthcheck = false;
		inline bool wallcheck = false; // Disabled due to instability
		inline std::vector<int>* flags = new std::vector<int>{ 0, 0, 0, 0, 0,0};
		inline float range = 1000;
		inline float aim_distance = 5000.0f; // New slider for aimbot/silent aim distance (10-5000)
        inline float healththreshhold = 10;
		inline std::vector<int>* aimpart = new std::vector<int>{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		inline std::vector<int>* airaimpart = new std::vector<int>{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		inline std::vector<int>* silentaimpart = new std::vector<int>{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		inline std::vector<int>* airsilentaimpart = new std::vector<int>{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		inline const char* hit_parts[] = { "Head", "HumanoidRootPart", "UpperTorso", "Torso", "LeftLeg", "RightLeg", "LeftUpperLeg", "RightUpperLeg", "LeftLowerLeg", "RightLowerLeg", "LeftUpperArm", "RightUpperArm", "LeftLowerArm", "RightLowerArm", "All" };
		inline bool silentaim = false;
		inline bool stickyaimsilent = false;
		inline int silentaimtype = 0;
		inline keybind silentaimkeybind("silentaimkeybind");
		inline keybind locktargetkeybind("locktargetkeybind");
		inline bool silentaimfov = false;
		inline bool drawsilentaimfov = false;
		inline float silentaimfovsize = 50;
		inline bool glowsilentaimfov = false;
inline float silentaimfovcolor[4] = { 1,1,1,1 };
		inline float silentaimfovtransparency = 1.0f;
		inline bool silentaimfovfill = false;
		inline float silentaimfovfilltransparency = 1.0f; // 0-1
		inline float silentaimfovfillcolor[4] = { 1,1,1,1 };
		inline float silentaimfovglowcolor[4] = { 1,1,1,1 };
inline int silentaimfovshape = 0; // 0 = Circle, 1 = Square, 2 = Triangle, 3 = Pentagon, 4 = Hexagon, 5 = Octagon
inline float silentaimfovthickness = 0.5f;
		inline float silentaimfovtolerance = 5.0f; // New variable for silent aim FOV tolerance

		// Silent Aim Enhancements
		inline bool silent_closest_part = false;
		inline bool silent_smoothness = false;
		inline float silent_smoothness_amount = 1.0f;

		// Aimbot Enhancements
		inline bool aimbot_closest_part = false;

		inline bool spin_fov_aimbot = false;
		inline float spin_fov_aimbot_speed = 1.0f;
		inline bool spin_fov_silentaim = false;
		inline float spin_fov_silentaim_speed = 1.0f;
				inline bool orbit = false;
				inline int orbittype = 0; // 0 = Random, 1 = X Axis, 2 = Y Axis
		inline float orbitspeed = 8;
		inline float orbitrange = 3;
        inline float orbitheight = 1;
        inline bool drawradiusring = false;
        inline keybind orbitkeybind("orbitkeybind                 ");
	//	inline std::vector<int>* orbittype;
				inline bool antiaim = false;
				inline bool underground_antiaim = false;
		inline bool triggerbot = false;
		inline keybind triggerbotkeybind("triggerbotkeybind");
        inline float triggerbot_delay = 50.0f; // Delay in milliseconds
        inline std::array<bool, 4> triggerbot_item_checks = { false, false, false, false }; // spray, wallet, food, knife

        // Debug/indicator: track the last used aim part and whether it was selected due to air state
        inline std::string last_used_aimpart = "";
        inline bool last_used_aimpart_air = false;
    }
	namespace visuals {
				inline bool visuals = true;
		inline bool boxes = false;
		inline bool boxfill = false;
		inline bool static_size = false;
		inline float static_size_value = 1.0f;
		inline bool lockedindicator = false;
		inline bool oofarrows = false;
		inline bool lockedesp = false; // New variable for locked ESP toggle
		inline float lockedespcolor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Default black color for locked ESP
		inline bool snapline = false;
		inline int snaplinetype = 0; // 0 = Top, 1 = Bottom, 2 = Center, 3 = Crosshair
		inline int snaplineoverlaytype = 0; // 0 = Straight, 1 = Spiderweb
		inline bool glowesp = false;
		inline int boxtype = 1;
		inline bool health = false; // Main toggle for health options
		inline bool healthbar = false; // Health bar sub-option
		inline bool name = false;
		inline int nametype = 0; // 0 = username (default), 1 = display name
		inline bool toolesp = false;
		inline bool distance = false;
		inline std::vector<int>* visuals_flags = new std::vector<int>{ 0, 0, 0, 0 }; // Team, Knocked, Distance, WallCheck
		inline bool chams = false;
		inline bool skeletons = false;
		inline bool boxes_prev = false;
		inline bool health_prev = false;
		inline bool name_prev = false;
		inline bool toolesp_prev = false;
		inline bool distance_prev = false;
		inline bool chams_prev = false;
		inline bool skeletons_prev = false;
		inline bool snapline_prev = false;
		inline bool fog_enabled_prev = false;
		inline bool sonar_prev = false;
		inline bool target_only_esp_prev = false;
		inline bool lockedesp_prev = false;
		inline bool localplayer = false;
		inline bool aimviewer = false;
		inline bool esppreview = false;
		inline bool predictionsdot = false;
		inline bool esp_preview_mode = false;
		inline bool sonar = false;
		inline float sonar_range = 20.0f;
		inline float sonar_thickness = 1.0f;
		inline float sonar_speed = 1.0f;
		inline float sonarcolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline bool sonar_detect_players = false;
		inline float sonar_detect_color_out[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
		inline float sonar_detect_color_in[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

		inline float boxcolors[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // White (Single picker now)
		inline float boxcolors1[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Resetting secondary to white by default
		inline float boxfillcolor[4] = { 1.0f, 1.0f, 1.0f, 0.3f };
		inline float glowcolor[4] =  { 1.0f, 1.0f, 1.0f, 1.0f};
		inline float glow_size = 25.0f; // Box glow blur radius
		inline float glow_opacity = 1.0f; // Box glow opacity (0.0 - 1.0)
		inline float lockedcolor[4] =  { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float oofcolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float snaplinecolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float healthbarcolor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
		inline float healthbarcolor1[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		inline bool enable_health_glow = false;
		inline float health_glow_size = 25.0f;
		inline float health_glow_opacity = 1.0f;
		inline float healthglowcolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float namecolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float toolespcolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float distancecolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float chamscolor[4] = { 1.0f, 1.0f, 1.0f, 0.3f };
		inline float chamscolor1[4] = { 1.0f, 1.0f, 1.0f, 0.3f }; // Secondary chams color
		inline float skeletonscolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

				inline bool fortniteindicator = false;
		inline bool hittracer = false;
		inline bool trail = false;
		inline bool hitbubble = false;
		inline bool targetchams = false;
		inline bool targetskeleton = false;
		inline bool localplayerchams = false;
		inline bool localgunchams = false;
				inline bool enemycheck = false;
		inline bool friendlycheck = false;
		inline bool teamcheck = false;
		inline bool rangecheck = false;
		inline float range = 1000;
		inline float visual_range = 5000; // Increased default visual range
			inline std::vector<int>* box_overlay_flags = new std::vector<int>{ 0, 0, 0 }; // Outline, Glow, Fill
		inline std::vector<int>* name_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* healthbar_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* tool_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* distance_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* skeleton_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* chams_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* snapline_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* oof_overlay_flags = new std::vector<int>{ 0, 0, 0 };
		inline bool target_only_esp = false;
		inline std::vector<std::string> target_only_list;

		// Fog Changer variables
		inline bool fog_enabled = false;
		inline float fog_start = 100.0f;
		inline float fog_end = 1000.0f;

		// Sonar variables are already mostly there, just need to ensure correct naming/types if anything changed.
		// Sonar is lines 181-188.

		// New Visual Options specific variables if needed, though most seem to reuse existing or just need flags.
		// Box Overlay: Outline(0), Glow(1), Fill(2) - Vector<int> box_overlay_flags

		// Health Overlay: Outline(0), Gradient(1)
		inline bool health_bar_outline = true; // Outline toggle
		inline bool health_bar_gradient = false; // Gradient toggle
		inline int health_bar_position = 0; // 0 = Left, 1 = Right

		// Chams Overlay: Outline -> Fill
		// User said "chams overlay dropdown: outline -> fill"
		inline int chamstype = 0; // 0 = Outline, 1 = Fill

		// Snapline Type: Top -> Bottom -> Center -> Crosshair
		// Already have snaplinetype.
		// Snapline Overlay: Straight -> Spiderweb
		// Already have snaplineoverlaytype.

		// Target Only ESP
		// Already added target_only_esp on line 227.

		// Locked ESP (already added on line 152)

		// Box Type: Corners -> Bounding
		// Already have boxtype variable (line 158).


        inline float fog_color[4] = { 0.7f, 0.7f, 0.7f, 1.0f }; // Default light gray
	}
    namespace misc {
		inline keybind menu_hotkey("Menu Hotkey", VK_RSHIFT); // Initialize with RShift as default
				inline bool speed = false;
		inline int speedtype = 0;
		inline float speedvalue = 30;
		inline keybind speedkeybind("speedkeybind");
				inline bool nojumpcooldown = false; // Automatically on
		inline bool jumppower = false; // Added jumppower toggle
		inline float jumppowervalue = 50.0f; // Default jumppower value
		inline keybind jumppowerkeybind("jumppowerkeybind"); // Keybind for jumppower
				inline bool flight = false;
		inline int flighttype = 0;
		inline float flightvalue = 16;
		inline keybind flightkeybind("flightkeybind");
				inline bool hipheight = false;
		inline  float hipheightvalue = 16;
				inline bool rapidfire = false;
		inline bool autoarmor = false;
		inline bool autoreload = false;
		inline bool autostomp = false;
		inline bool antistomp = false;
		inline bool bikefly = false;
		inline keybind stompkeybind("stompkeybind");

		inline bool spectate = false;
		inline std::string spectate_target_name = "";
		inline keybind spectatebind("spectatebind");

		inline bool rotate360 = false;
		inline keybind rotate360keybind("rotate360keybind");
		inline float camera_rotation_yaw = 0.0f;
		inline float rotate360_speed = 10.0f; // New slider for rotation speed
		inline float rotate360_vspeed = 0.0f;
		inline float camera_rotation_pitch = 0.0f;
		inline math::Matrix3 original_camera_rotation; // Store original camera rotation

		// Macro Feature
		inline bool macro_enabled = false;
		inline float macro_delay = 6.0f; // Default delay as requested
		inline keybind macro_keybind("macrokeybind"); // No default key

				inline bool vsync = false;
		inline bool unlock_fps = false;
		inline float fps_cap = 240.0f;
		inline bool show_fps = false;
		inline bool targethud = false;
		inline bool playerlist = true;
		inline bool streamproof = false;
		inline bool custom_cursor = true; // Enable custom overlay cursor by default
		inline bool performance_mode = true;
		inline bool keybinds = false;
		inline int font_index = 2; // 0 = Tahoma, 1 = Default, 2 = Verdana
		inline bool spotify = false;
		inline bool colors = false;
		inline bool menuglow = true; // Menu glow always on
		inline float menuglowcolor[4] = { 252.0f / 255.0f, 253.0f / 255.0f, 148.0f / 255.0f, 1.0f }; // Default glow color (#FCFD94FF)
		inline float overlay_color[4] = { 12.0f / 255.0f, 13.0f / 255.0f, 12.0f / 255.0f, 1.0f }; // Default overlay/playerlist background (#0C0D0CFF)
		inline float accent_color[4] = { 252.0f / 255.0f, 253.0f / 255.0f, 148.0f / 255.0f, 1.0f }; // Primary accent color (#FCFD94FF)
		inline float overlay_star_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Color used for overlay star effect
		inline float health_full_color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
		inline float health_low_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		inline bool config_loaded_flag = false; // New flag to indicate config has been loaded
		inline bool main_window_first_time = true; // Added for overlay window initial positioning
        inline bool enable_notifications = false; // New flag to control all notifications
        inline float music_max_volume = 0.3f; // Max volume for music (0.0 to 1.0, scaled to 0-1000 for MCI)
        inline bool overlay_stars = true; // Enable star effect in overlay background
		
		// Auto-friend feature
		inline bool autofriend_enabled = false;
		inline bool autofriend_group_members = false;
		inline std::string autofriend_group_id = "872867055";
		inline std::vector<std::string> group_members;
		inline bool group_members_loaded = false;
		inline bool loading_group_members = false;
		inline std::string autofriend_status = "Ready";
		inline bool loading = false;

		// Theme configuration
		inline float WindowBG[4] = { 25.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 1.0f };
		inline float Border[4] = { 30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f };
		inline float Child[4] = { 15.0f / 255.0f, 15.0f / 255.0f, 15.0f / 255.0f, 1.0f };
		inline float ThemeColor[4] = { 218.0f / 255.0f, 71.0f / 255.0f, 122.0f / 255.0f, 1.0f };
		inline float OverlayBorder[4] = { 218.0f / 255.0f, 71.0f / 255.0f, 122.0f / 255.0f, 1.0f };
		inline float AccentActive[4] = { 172.0f / 255.0f, 56.0f / 255.0f, 96.0f / 255.0f, 1.0f };
		inline float Header[4] = { 35.0f / 255.0f, 35.0f / 255.0f, 35.0f / 255.0f, 1.0f };
		inline float PopupBG[4] = { 20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 1.0f };
		inline float Text[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float TextDisabled[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
		inline float Button[4] = { 30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f };
		inline float ButtonHovered[4] = { 40.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 1.0f };
		inline float ButtonActive[4] = { 50.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 1.0f };
		inline float FrameBG[4] = { 25.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 1.0f };
		inline float FrameBGHovered[4] = { 35.0f / 255.0f, 35.0f / 255.0f, 35.0f / 255.0f, 1.0f };
		inline float FrameBGActive[4] = { 45.0f / 255.0f, 45.0f / 255.0f, 45.0f / 255.0f, 1.0f };
		inline float ScrollbarBG[4] = { 15.0f / 255.0f, 15.0f / 255.0f, 15.0f / 255.0f, 1.0f };
		inline float ScrollbarGrab[4] = { 45.0f / 255.0f, 45.0f / 255.0f, 45.0f / 255.0f, 1.0f };
		inline float ScrollbarGrabHovered[4] = { 55.0f / 255.0f, 55.0f / 255.0f, 55.0f / 255.0f, 1.0f };
		inline float ScrollbarGrabActive[4] = { 65.0f / 255.0f, 65.0f / 255.0f, 65.0f / 255.0f, 1.0f };
		inline float SliderGrab[4] = { 218.0f / 255.0f, 71.0f / 255.0f, 122.0f / 255.0f, 1.0f };
		inline float SliderGrabActive[4] = { 172.0f / 255.0f, 56.0f / 255.0f, 96.0f / 255.0f, 1.0f };
    }

	inline bool unattach = false;
	inline bool stop;
	inline bool firstreceived = false;
	inline bool handlingtp = false;
	inline bool overlay_open_requested = false; // New global to track user's intent to open/close overlay
}
