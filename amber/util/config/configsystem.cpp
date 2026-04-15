#include "configsystem.h"
#include <iostream>
#include <algorithm>
#include "../notification/notification.h"
#include "../globals.h"

// Function to remove invalid characters from a string
std::string sanitize_filename(const std::string& name) {
    std::string sanitized_name = name;
    // Remove invalid characters
    sanitized_name.erase(std::remove_if(sanitized_name.begin(), sanitized_name.end(), [](char c) {
        return c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|';
    }), sanitized_name.end());
    return sanitized_name;
}

ConfigSystem::ConfigSystem() {
        char* appdata_path;
    size_t len;
    _dupenv_s(&appdata_path, &len, "APPDATA");

    if (appdata_path) {
        config_directory = std::string(appdata_path) + "\\amber\\config";
        free(appdata_path);
    }

        if (!fs::exists(config_directory)) {
        fs::create_directories(config_directory);
    }

    refresh_config_list();
    autoload_config_name = load_autoload_setting(); // Load autoload setting on startup

    if (!autoload_config_name.empty()) {
        std::cout << "[CONFIG] Attempting to autoload config: " << autoload_config_name << "\n";
        load_config(autoload_config_name);
    }       
}

void ConfigSystem::refresh_config_list() {
    config_files.clear();
    // Also clear autoload_config_name if the file it points to no longer exists
    if (!autoload_config_name.empty()) {
        std::string autoload_filepath = config_directory + "\\" + autoload_config_name + ".json";
        if (!fs::exists(autoload_filepath)) {
            autoload_config_name.clear();
            save_autoload_setting(""); // Clear the autoload setting if the config file is gone
        }
    }

    if (fs::exists(config_directory)) {
        for (const auto& entry : fs::directory_iterator(config_directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                config_files.push_back(entry.path().stem().string());
            }
        }
    }
}

bool ConfigSystem::save_config(const std::string& name) {
    if (name.empty()) return false;

    std::string sanitized_name = sanitize_filename(name);
    if (sanitized_name.empty()) return false;

    json config_json;

    // Combat
    auto& combat = config_json["combat"];
    combat["rapidfire"] = globals::combat::rapidfire;
    combat["autostomp"] = globals::combat::autostomp;
    combat["aimbot"] = globals::combat::aimbot;
    combat["stickyaim"] = globals::combat::stickyaim;
    combat["aimbottype"] = globals::combat::aimbottype;
    combat["usefov"] = globals::combat::usefov;
    combat["drawfov"] = globals::combat::drawfov;
    combat["fovsize"] = globals::combat::fovsize;
    combat["glowfov"] = globals::combat::glowfov;
    combat["fovcolor"] = { globals::combat::fovcolor[0], globals::combat::fovcolor[1], globals::combat::fovcolor[2], globals::combat::fovcolor[3] };
    combat["fovtransparency"] = globals::combat::fovtransparency;
    combat["fovfill"] = globals::combat::fovfill;
    combat["fovfilltransparency"] = globals::combat::fovfilltransparency;
    combat["fovfillcolor"] = { globals::combat::fovfillcolor[0], globals::combat::fovfillcolor[1], globals::combat::fovfillcolor[2], globals::combat::fovfillcolor[3] };
    combat["fovglowcolor"] = { globals::combat::fovglowcolor[0], globals::combat::fovglowcolor[1], globals::combat::fovglowcolor[2], globals::combat::fovglowcolor[3] };
    combat["fovshape"] = globals::combat::fovshape;
    combat["fovthickness"] = globals::combat::fovthickness;
    combat["smoothing"] = globals::combat::smoothing;
    combat["smoothingx"] = globals::combat::smoothingx;
    combat["smoothingy"] = globals::combat::smoothingy;
    combat["smoothing_style"] = globals::combat::smoothing_style;
    combat["aimbot_shake"] = globals::combat::aimbot_shake;
    combat["aimbot_shake_x"] = globals::combat::aimbot_shake_x;
    combat["aimbot_shake_y"] = globals::combat::aimbot_shake_y;
    combat["predictions"] = globals::combat::predictions;
    combat["predictionsx"] = globals::combat::predictionsx;
    combat["predictionsy"] = globals::combat::predictionsy;
    combat["silentpredictions"] = globals::combat::silentpredictions;
    combat["silentpredictionsx"] = globals::combat::silentpredictionsx;
    combat["silentpredictionsy"] = globals::combat::silentpredictionsy;
    combat["silent_shake"] = globals::combat::silent_shake;
    combat["silent_shake_x"] = globals::combat::silent_shake_x;
    combat["silent_shake_y"] = globals::combat::silent_shake_y;
    combat["teamcheck"] = globals::combat::teamcheck;
    combat["knockcheck"] = globals::combat::knockcheck;
    combat["rangecheck"] = globals::combat::rangecheck;
    combat["healthcheck"] = globals::combat::healthcheck;
    combat["wallcheck"] = globals::combat::wallcheck;
    combat["flags"] = *globals::combat::flags;
    combat["range"] = globals::combat::range;
    combat["aim_distance"] = globals::combat::aim_distance;
    combat["healththreshhold"] = globals::combat::healththreshhold;
    combat["aimpart"] = *globals::combat::aimpart;
    combat["airaimpart"] = *globals::combat::airaimpart;
    combat["silentaimpart"] = *globals::combat::silentaimpart;
    combat["airsilentaimpart"] = *globals::combat::airsilentaimpart;
    combat["silentaim"] = globals::combat::silentaim;
    combat["stickyaimsilent"] = globals::combat::stickyaimsilent;
    combat["silentaimtype"] = globals::combat::silentaimtype;
    combat["silentaimfov"] = globals::combat::silentaimfov;
    combat["drawsilentaimfov"] = globals::combat::drawsilentaimfov;
    combat["silentaimfovsize"] = globals::combat::silentaimfovsize;
    combat["glowsilentaimfov"] = globals::combat::glowsilentaimfov;
    combat["silentaimfovcolor"] = { globals::combat::silentaimfovcolor[0], globals::combat::silentaimfovcolor[1], globals::combat::silentaimfovcolor[2], globals::combat::silentaimfovcolor[3] };
    combat["silentaimfovtransparency"] = globals::combat::silentaimfovtransparency;
    combat["silentaimfovfill"] = globals::combat::silentaimfovfill;
    combat["silentaimfovfilltransparency"] = globals::combat::silentaimfovfilltransparency;
    combat["silentaimfovfillcolor"] = { globals::combat::silentaimfovfillcolor[0], globals::combat::silentaimfovfillcolor[1], globals::combat::silentaimfovfillcolor[2], globals::combat::silentaimfovfillcolor[3] };
    combat["silentaimfovglowcolor"] = { globals::combat::silentaimfovglowcolor[0], globals::combat::silentaimfovglowcolor[1], globals::combat::silentaimfovglowcolor[2], globals::combat::silentaimfovglowcolor[3] };
    combat["silentaimfovshape"] = globals::combat::silentaimfovshape;
    combat["silentaimfovthickness"] = globals::combat::silentaimfovthickness;
    combat["silent_closest_part"] = globals::combat::silent_closest_part;
    combat["silent_smoothness"] = globals::combat::silent_smoothness;
    combat["silent_smoothness_amount"] = globals::combat::silent_smoothness_amount;
    combat["aimbot_closest_part"] = globals::combat::aimbot_closest_part;
    combat["spin_fov_aimbot"] = globals::combat::spin_fov_aimbot;
    combat["spin_fov_aimbot_speed"] = globals::combat::spin_fov_aimbot_speed;
    combat["spin_fov_silentaim"] = globals::combat::spin_fov_silentaim;
    combat["spin_fov_silentaim_speed"] = globals::combat::spin_fov_silentaim_speed;
    combat["orbit"] = globals::combat::orbit;
    combat["orbitspeed"] = globals::combat::orbitspeed;
    combat["orbitrange"] = globals::combat::orbitrange;
    combat["orbitheight"] = globals::combat::orbitheight;
    combat["drawradiusring"] = globals::combat::drawradiusring;
    combat["orbittype"] = globals::combat::orbittype;
    combat["antiaim"] = globals::combat::antiaim;
    combat["underground_antiaim"] = globals::combat::underground_antiaim;
    combat["triggerbot"] = globals::combat::triggerbot;
    combat["triggerbot_delay"] = globals::combat::triggerbot_delay;
    combat["triggerbot_item_checks"] = globals::combat::triggerbot_item_checks;

    // Combat Binds
    auto& c_kb = combat["keybinds"];
    c_kb["aimbotkeybind"] = {{"key", globals::combat::aimbotkeybind.key}, {"type", (int)globals::combat::aimbotkeybind.type}};
    c_kb["silentaimkeybind"] = {{"key", globals::combat::silentaimkeybind.key}, {"type", (int)globals::combat::silentaimkeybind.type}};
    c_kb["orbitkeybind"] = {{"key", globals::combat::orbitkeybind.key}, {"type", (int)globals::combat::orbitkeybind.type}};
    c_kb["locktargetkeybind"] = {{"key", globals::combat::locktargetkeybind.key}, {"type", (int)globals::combat::locktargetkeybind.type}};
    c_kb["triggerbotkeybind"] = {{"key", globals::combat::triggerbotkeybind.key}, {"type", (int)globals::combat::triggerbotkeybind.type}};

        std::cout << "[VISUALS] Saving visual settings..." << "\n";
    // Visuals
    auto& visuals = config_json["visuals"];
    visuals["visuals"] = globals::visuals::visuals;
    visuals["boxes"] = globals::visuals::boxes;
    visuals["boxfill"] = globals::visuals::boxfill;
    visuals["static_size"] = globals::visuals::static_size;
    visuals["static_size_value"] = globals::visuals::static_size_value;
    visuals["lockedindicator"] = globals::visuals::lockedindicator;
    visuals["oofarrows"] = globals::visuals::oofarrows;
    visuals["lockedesp"] = globals::visuals::lockedesp;
    visuals["lockedespcolor"] = { globals::visuals::lockedespcolor[0], globals::visuals::lockedespcolor[1], globals::visuals::lockedespcolor[2], globals::visuals::lockedespcolor[3] };
    visuals["snapline"] = globals::visuals::snapline;
    visuals["snaplinetype"] = globals::visuals::snaplinetype;
    visuals["snaplineoverlaytype"] = globals::visuals::snaplineoverlaytype;
    visuals["glowesp"] = globals::visuals::glowesp;
    visuals["boxtype"] = globals::visuals::boxtype;
    visuals["health"] = globals::visuals::health;
    visuals["healthbar"] = globals::visuals::healthbar;
    visuals["name"] = globals::visuals::name;
    visuals["nametype"] = globals::visuals::nametype;
    visuals["toolesp"] = globals::visuals::toolesp;
    visuals["distance"] = globals::visuals::distance;
    visuals["visuals_flags"] = *globals::visuals::visuals_flags;
    visuals["chams"] = globals::visuals::chams;
    visuals["skeletons"] = globals::visuals::skeletons;
    visuals["localplayer"] = globals::visuals::localplayer;
    visuals["aimviewer"] = globals::visuals::aimviewer;
    visuals["esppreview"] = globals::visuals::esppreview;
    visuals["esp_preview_mode"] = globals::visuals::esp_preview_mode;
    visuals["predictionsdot"] = globals::visuals::predictionsdot;
    visuals["boxcolors"] = { globals::visuals::boxcolors[0], globals::visuals::boxcolors[1], globals::visuals::boxcolors[2], globals::visuals::boxcolors[3] };
    visuals["boxfillcolor"] = { globals::visuals::boxfillcolor[0], globals::visuals::boxfillcolor[1], globals::visuals::boxfillcolor[2], globals::visuals::boxfillcolor[3] };
    visuals["glowcolor"] = { globals::visuals::glowcolor[0], globals::visuals::glowcolor[1], globals::visuals::glowcolor[2], globals::visuals::glowcolor[3] };
    visuals["glow_size"] = globals::visuals::glow_size;
    visuals["glow_opacity"] = globals::visuals::glow_opacity;
    visuals["lockedcolor"] = { globals::visuals::lockedcolor[0], globals::visuals::lockedcolor[1], globals::visuals::lockedcolor[2], globals::visuals::lockedcolor[3] };
    visuals["oofcolor"] = { globals::visuals::oofcolor[0], globals::visuals::oofcolor[1], globals::visuals::oofcolor[2], globals::visuals::oofcolor[3] };
    visuals["snaplinecolor"] = { globals::visuals::snaplinecolor[0], globals::visuals::snaplinecolor[1], globals::visuals::snaplinecolor[2], globals::visuals::snaplinecolor[3] };
    visuals["healthbarcolor"] = { globals::visuals::healthbarcolor[0], globals::visuals::healthbarcolor[1], globals::visuals::healthbarcolor[2], globals::visuals::healthbarcolor[3] };
    visuals["healthbarcolor1"] = { globals::visuals::healthbarcolor1[0], globals::visuals::healthbarcolor1[1], globals::visuals::healthbarcolor1[2], globals::visuals::healthbarcolor1[3] };
    visuals["enable_health_glow"] = globals::visuals::enable_health_glow;
    visuals["health_glow_size"] = globals::visuals::health_glow_size;
    visuals["health_glow_opacity"] = globals::visuals::health_glow_opacity;
    visuals["healthglowcolor"] = { globals::visuals::healthglowcolor[0], globals::visuals::healthglowcolor[1], globals::visuals::healthglowcolor[2], globals::visuals::healthglowcolor[3] };
    visuals["namecolor"] = { globals::visuals::namecolor[0], globals::visuals::namecolor[1], globals::visuals::namecolor[2], globals::visuals::namecolor[3] };
    visuals["toolespcolor"] = { globals::visuals::toolespcolor[0], globals::visuals::toolespcolor[1], globals::visuals::toolespcolor[2], globals::visuals::toolespcolor[3] };
    visuals["distancecolor"] = { globals::visuals::distancecolor[0], globals::visuals::distancecolor[1], globals::visuals::distancecolor[2], globals::visuals::distancecolor[3] };
    visuals["chamscolor"] = { globals::visuals::chamscolor[0], globals::visuals::chamscolor[1], globals::visuals::chamscolor[2], globals::visuals::chamscolor[3] };
    visuals["chamscolor1"] = { globals::visuals::chamscolor1[0], globals::visuals::chamscolor1[1], globals::visuals::chamscolor1[2], globals::visuals::chamscolor1[3] };
    visuals["health_bar_outline"] = globals::visuals::health_bar_outline;
    visuals["health_bar_gradient"] = globals::visuals::health_bar_gradient;
    visuals["health_bar_position"] = globals::visuals::health_bar_position;
    visuals["skeletonscolor"] = { globals::visuals::skeletonscolor[0], globals::visuals::skeletonscolor[1], globals::visuals::skeletonscolor[2], globals::visuals::skeletonscolor[3] };
    visuals["fortniteindicator"] = globals::visuals::fortniteindicator;
    visuals["hittracer"] = globals::visuals::hittracer;
    visuals["trail"] = globals::visuals::trail;
    visuals["hitbubble"] = globals::visuals::hitbubble;
    visuals["targetchams"] = globals::visuals::targetchams;
    visuals["targetskeleton"] = globals::visuals::targetskeleton;
    visuals["localplayerchams"] = globals::visuals::localplayerchams;
    visuals["localgunchams"] = globals::visuals::localgunchams;
    visuals["enemycheck"] = globals::visuals::enemycheck;
    visuals["friendlycheck"] = globals::visuals::friendlycheck;
    visuals["teamcheck"] = globals::visuals::teamcheck;
    visuals["rangecheck"] = globals::visuals::rangecheck;
    visuals["range"] = globals::visuals::range;
    visuals["visual_range"] = globals::visuals::visual_range;
    visuals["sonar"] = globals::visuals::sonar;
    visuals["sonar_range"] = globals::visuals::sonar_range;
    visuals["sonar_thickness"] = globals::visuals::sonar_thickness;
    visuals["sonar_speed"] = globals::visuals::sonar_speed;
    visuals["sonarcolor"] = { globals::visuals::sonarcolor[0], globals::visuals::sonarcolor[1], globals::visuals::sonarcolor[2], globals::visuals::sonarcolor[3] };
    visuals["sonar_detect_players"] = globals::visuals::sonar_detect_players;
    visuals["sonar_detect_color_out"] = { globals::visuals::sonar_detect_color_out[0], globals::visuals::sonar_detect_color_out[1], globals::visuals::sonar_detect_color_out[2], globals::visuals::sonar_detect_color_out[3] };
    visuals["sonar_detect_color_in"] = { globals::visuals::sonar_detect_color_in[0], globals::visuals::sonar_detect_color_in[1], globals::visuals::sonar_detect_color_in[2], globals::visuals::sonar_detect_color_in[3] };
    visuals["box_overlay_flags"] = *globals::visuals::box_overlay_flags;
    visuals["name_overlay_flags"] = *globals::visuals::name_overlay_flags;
    visuals["healthbar_overlay_flags"] = *globals::visuals::healthbar_overlay_flags;
    visuals["tool_overlay_flags"] = *globals::visuals::tool_overlay_flags;
    visuals["distance_overlay_flags"] = *globals::visuals::distance_overlay_flags;
    visuals["skeleton_overlay_flags"] = *globals::visuals::skeleton_overlay_flags;
    visuals["chams_overlay_flags"] = *globals::visuals::chams_overlay_flags;
    visuals["snapline_overlay_flags"] = *globals::visuals::snapline_overlay_flags;
    visuals["oof_overlay_flags"] = *globals::visuals::oof_overlay_flags;
    visuals["target_only_esp"] = globals::visuals::target_only_esp;
    visuals["target_only_list"] = globals::visuals::target_only_list;
    visuals["fog_enabled"] = globals::visuals::fog_enabled;
    visuals["fog_start"] = globals::visuals::fog_start;
    visuals["fog_end"] = globals::visuals::fog_end;
    visuals["fog_color"] = { globals::visuals::fog_color[0], globals::visuals::fog_color[1], globals::visuals::fog_color[2], globals::visuals::fog_color[3] };

    // Misc
    auto& misc = config_json["misc"];
    misc["speed"] = globals::misc::speed;
    misc["speedtype"] = globals::misc::speedtype;
    misc["speedvalue"] = globals::misc::speedvalue;
    misc["nojumpcooldown"] = globals::misc::nojumpcooldown;
    misc["jumppower"] = globals::misc::jumppower;
    misc["jumppowervalue"] = globals::misc::jumppowervalue;
    misc["flight"] = globals::misc::flight;
    misc["flighttype"] = globals::misc::flighttype;
    misc["flightvalue"] = globals::misc::flightvalue;
    misc["hipheight"] = globals::misc::hipheight;
    misc["hipheightvalue"] = globals::misc::hipheightvalue;
    misc["rapidfire"] = globals::misc::rapidfire;
    misc["autoarmor"] = globals::misc::autoarmor;
    misc["autoreload"] = globals::misc::autoreload;
    misc["autostomp"] = globals::misc::autostomp;
    misc["antistomp"] = globals::misc::antistomp;
    misc["bikefly"] = globals::misc::bikefly;
    misc["spectate"] = globals::misc::spectate;
    misc["vsync"] = globals::misc::vsync;
    misc["targethud"] = globals::misc::targethud;
    misc["playerlist"] = globals::misc::playerlist;
    misc["performance_mode"] = globals::misc::performance_mode;
    misc["keybinds"] = globals::misc::keybinds;
    misc["font_index"] = globals::misc::font_index;
    misc["unlock_fps"] = globals::misc::unlock_fps;
    misc["fps_cap"] = globals::misc::fps_cap;
    misc["show_fps"] = globals::misc::show_fps;
    misc["spotify"] = globals::misc::spotify;
    misc["colors"] = globals::misc::colors;
    misc["streamproof"] = globals::misc::streamproof;
    misc["custom_cursor"] = globals::misc::custom_cursor;
    misc["rotate360"] = globals::misc::rotate360;
    misc["camera_rotation_yaw"] = globals::misc::camera_rotation_yaw;
    misc["camera_rotation_pitch"] = globals::misc::camera_rotation_pitch;
    misc["rotate360_speed"] = globals::misc::rotate360_speed;
    misc["rotate360_vspeed"] = globals::misc::rotate360_vspeed;
    misc["menuglow"] = globals::misc::menuglow;
    misc["menuglowcolor"] = { globals::misc::menuglowcolor[0], globals::misc::menuglowcolor[1], globals::misc::menuglowcolor[2], globals::misc::menuglowcolor[3] };
    misc["overlay_color"] = { globals::misc::overlay_color[0], globals::misc::overlay_color[1], globals::misc::overlay_color[2], globals::misc::overlay_color[3] };
    misc["accent_color"] = { globals::misc::accent_color[0], globals::misc::accent_color[1], globals::misc::accent_color[2], globals::misc::accent_color[3] };
    misc["overlay_stars"] = globals::misc::overlay_stars;
    misc["keybinds"] = globals::misc::keybinds;
    misc["overlay_star_color"] = { globals::misc::overlay_star_color[0], globals::misc::overlay_star_color[1], globals::misc::overlay_star_color[2], globals::misc::overlay_star_color[3] };
    misc["health_full_color"] = { globals::misc::health_full_color[0], globals::misc::health_full_color[1], globals::misc::health_full_color[2], globals::misc::health_full_color[3] };
    misc["health_low_color"] = { globals::misc::health_low_color[0], globals::misc::health_low_color[1], globals::misc::health_low_color[2], globals::misc::health_low_color[3] };
    misc["enable_notifications"] = globals::misc::enable_notifications;
    misc["music_max_volume"] = globals::misc::music_max_volume;
    misc["autofriend_enabled"] = globals::misc::autofriend_enabled;
    misc["autofriend_group_members"] = globals::misc::autofriend_group_members;
    misc["autofriend_group_id"] = globals::misc::autofriend_group_id;
    
    // Theme Colors
    misc["theme_window_bg"] = { globals::misc::WindowBG[0], globals::misc::WindowBG[1], globals::misc::WindowBG[2] };
    misc["theme_border"] = { globals::misc::Border[0], globals::misc::Border[1], globals::misc::Border[2] };
    misc["theme_child"] = { globals::misc::Child[0], globals::misc::Child[1], globals::misc::Child[2] };
    misc["theme_color"] = { globals::misc::ThemeColor[0], globals::misc::ThemeColor[1], globals::misc::ThemeColor[2], globals::misc::ThemeColor[3] };
    misc["overlay_border"] = { globals::misc::OverlayBorder[0], globals::misc::OverlayBorder[1], globals::misc::OverlayBorder[2], globals::misc::OverlayBorder[3] };
    misc["AccentActive"] = { globals::misc::AccentActive[0], globals::misc::AccentActive[1], globals::misc::AccentActive[2], globals::misc::AccentActive[3] };
    misc["Header"] = { globals::misc::Header[0], globals::misc::Header[1], globals::misc::Header[2], globals::misc::Header[3] };
    misc["PopupBG"] = { globals::misc::PopupBG[0], globals::misc::PopupBG[1], globals::misc::PopupBG[2], globals::misc::PopupBG[3] };
    misc["Text"] = { globals::misc::Text[0], globals::misc::Text[1], globals::misc::Text[2], globals::misc::Text[3] };
    misc["TextDisabled"] = { globals::misc::TextDisabled[0], globals::misc::TextDisabled[1], globals::misc::TextDisabled[2], globals::misc::TextDisabled[3] };
    misc["Button"] = { globals::misc::Button[0], globals::misc::Button[1], globals::misc::Button[2], globals::misc::Button[3] };
    misc["ButtonHovered"] = { globals::misc::ButtonHovered[0], globals::misc::ButtonHovered[1], globals::misc::ButtonHovered[2], globals::misc::ButtonHovered[3] };
    misc["ButtonActive"] = { globals::misc::ButtonActive[0], globals::misc::ButtonActive[1], globals::misc::ButtonActive[2], globals::misc::ButtonActive[3] };
    misc["FrameBG"] = { globals::misc::FrameBG[0], globals::misc::FrameBG[1], globals::misc::FrameBG[2], globals::misc::FrameBG[3] };
    misc["FrameBGHovered"] = { globals::misc::FrameBGHovered[0], globals::misc::FrameBGHovered[1], globals::misc::FrameBGHovered[2], globals::misc::FrameBGHovered[3] };
    misc["FrameBGActive"] = { globals::misc::FrameBGActive[0], globals::misc::FrameBGActive[1], globals::misc::FrameBGActive[2], globals::misc::FrameBGActive[3] };
    misc["ScrollbarBG"] = { globals::misc::ScrollbarBG[0], globals::misc::ScrollbarBG[1], globals::misc::ScrollbarBG[2], globals::misc::ScrollbarBG[3] };
    misc["ScrollbarGrab"] = { globals::misc::ScrollbarGrab[0], globals::misc::ScrollbarGrab[1], globals::misc::ScrollbarGrab[2], globals::misc::ScrollbarGrab[3] };
    misc["ScrollbarGrabHovered"] = { globals::misc::ScrollbarGrabHovered[0], globals::misc::ScrollbarGrabHovered[1], globals::misc::ScrollbarGrabHovered[2], globals::misc::ScrollbarGrabHovered[3] };
    misc["ScrollbarGrabActive"] = { globals::misc::ScrollbarGrabActive[0], globals::misc::ScrollbarGrabActive[1], globals::misc::ScrollbarGrabActive[2], globals::misc::ScrollbarGrabActive[3] };
    misc["SliderGrab"] = { globals::misc::SliderGrab[0], globals::misc::SliderGrab[1], globals::misc::SliderGrab[2], globals::misc::SliderGrab[3] };
    misc["SliderGrabActive"] = { globals::misc::SliderGrabActive[0], globals::misc::SliderGrabActive[1], globals::misc::SliderGrabActive[2], globals::misc::SliderGrabActive[3] };

    // Misc Binds
    auto& m_kb = misc["keybinds_data"];
    m_kb["speedkeybind"] = {{"key", globals::misc::speedkeybind.key}, {"type", (int)globals::misc::speedkeybind.type}};
    m_kb["jumppowerkeybind"] = {{"key", globals::misc::jumppowerkeybind.key}, {"type", (int)globals::misc::jumppowerkeybind.type}};
    m_kb["flightkeybind"] = {{"key", globals::misc::flightkeybind.key}, {"type", (int)globals::misc::flightkeybind.type}};
    m_kb["stompkeybind"] = {{"key", globals::misc::stompkeybind.key}, {"type", (int)globals::misc::stompkeybind.type}};
    m_kb["spectatebind"] = {{"key", globals::misc::spectatebind.key}, {"type", (int)globals::misc::spectatebind.type}};
    m_kb["rotate360keybind"] = {{"key", globals::misc::rotate360keybind.key}, {"type", (int)globals::misc::rotate360keybind.type}};
    m_kb["menu_hotkey"] = {{"key", globals::misc::menu_hotkey.key}, {"type", (int)globals::misc::menu_hotkey.type}};
        

    std::string filepath = config_directory + "\\" + sanitized_name + ".json";
    std::ofstream file(filepath);

    if (file.is_open()) {
        file << config_json.dump(2);         file.close();

        refresh_config_list();
        current_config_name = sanitized_name;
        std::cout << "[CONFIG] Successfully saved config: " << sanitized_name << "\n";
        return true;
    }

    std::cout << "[CONFIG] Failed to save config: " << sanitized_name << "\n";
    return false;
}

bool ConfigSystem::save_autoload_setting(const std::string& name) {
    std::string autoload_filepath = config_directory + "\\autoload.txt";
    std::ofstream file(autoload_filepath);
    if (file.is_open()) {
        file << name;
        file.close();
        autoload_config_name = name;
        std::cout << "[CONFIG] Autoload setting saved: " << name << "\n";
        return true;
    }
    std::cout << "[CONFIG] Failed to save autoload setting.\n";
    return false;
}

std::string ConfigSystem::load_autoload_setting() {
    std::string autoload_filepath = config_directory + "\\autoload.txt";
    std::ifstream file(autoload_filepath);
    std::string name;
    if (file.is_open()) {
        std::getline(file, name);
        file.close();
        std::cout << "[CONFIG] Autoload setting loaded: " << name << "\n";
        return name;
    }
    std::cout << "[CONFIG] No autoload setting found or failed to open autoload.txt.\n";
    return "";
}

bool ConfigSystem::load_config(const std::string& name) {
    if (name.empty()) return false;

    std::string sanitized_name = sanitize_filename(name);
    if (sanitized_name.empty()) return false;

    std::string filepath = config_directory + "\\" + sanitized_name + ".json";
    std::ifstream file(filepath);

    if (file.is_open()) {
        try {
            json config_json;
            file >> config_json;

            std::cout << "[CONFIG] Loading config: " << name << "\n";

                // Reset combat toggles that should disable when not present in config
                globals::combat::fovfill = false;
                globals::combat::silentaimfovfill = false;

                if (config_json.contains("combat")) {
                    auto& combat = config_json["combat"];
                    if (combat.contains("rapidfire")) globals::combat::rapidfire = combat["rapidfire"];
                    if (combat.contains("autostomp")) globals::combat::autostomp = combat["autostomp"];
                    if (combat.contains("aimbot")) globals::combat::aimbot = combat["aimbot"];
                    if (combat.contains("stickyaim")) globals::combat::stickyaim = combat["stickyaim"];
                    if (combat.contains("aimbottype")) globals::combat::aimbottype = combat["aimbottype"];
                    if (combat.contains("usefov")) globals::combat::usefov = combat["usefov"];
                    if (combat.contains("drawfov")) globals::combat::drawfov = combat["drawfov"];
                    if (combat.contains("fovsize")) globals::combat::fovsize = combat["fovsize"];
                    if (combat.contains("glowfov")) globals::combat::glowfov = combat["glowfov"];
                    if (combat.contains("fovcolor")) {
                        auto colors = combat["fovcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::combat::fovcolor[i] = colors[i];
                    }
                    if (combat.contains("fovtransparency")) globals::combat::fovtransparency = combat["fovtransparency"];
                    if (combat.contains("fovfill")) globals::combat::fovfill = combat["fovfill"];
                    if (combat.contains("fovfilltransparency")) globals::combat::fovfilltransparency = combat["fovfilltransparency"];
                    if (combat.contains("fovfillcolor")) {
                        auto colors = combat["fovfillcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::combat::fovfillcolor[i] = colors[i];
                    }
                    if (combat.contains("fovglowcolor")) {
                        auto colors = combat["fovglowcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::combat::fovglowcolor[i] = colors[i];
                    }
                    if (combat.contains("fovshape")) globals::combat::fovshape = combat["fovshape"];
                    if (combat.contains("fovthickness")) globals::combat::fovthickness = combat["fovthickness"];
                    if (combat.contains("smoothing")) globals::combat::smoothing = combat["smoothing"];
                    if (combat.contains("smoothingx")) globals::combat::smoothingx = combat["smoothingx"];
                    if (combat.contains("smoothingy")) globals::combat::smoothingy = combat["smoothingy"];
                    if (combat.contains("smoothing_style")) globals::combat::smoothing_style = combat["smoothing_style"];
                    if (combat.contains("aimbot_shake")) globals::combat::aimbot_shake = combat["aimbot_shake"];
                    if (combat.contains("aimbot_shake_x")) globals::combat::aimbot_shake_x = combat["aimbot_shake_x"];
                    if (combat.contains("aimbot_shake_y")) globals::combat::aimbot_shake_y = combat["aimbot_shake_y"];
                    if (combat.contains("predictions")) globals::combat::predictions = combat["predictions"];
                    if (combat.contains("predictionsx")) globals::combat::predictionsx = combat["predictionsx"];
                    if (combat.contains("predictionsy")) globals::combat::predictionsy = combat["predictionsy"];
                    if (combat.contains("silentpredictions")) globals::combat::silentpredictions = combat["silentpredictions"];
                    if (combat.contains("silentpredictionsx")) globals::combat::silentpredictionsx = combat["silentpredictionsx"];
                    if (combat.contains("silentpredictionsy")) globals::combat::silentpredictionsy = combat["silentpredictionsy"];
                    if (combat.contains("silent_shake")) globals::combat::silent_shake = combat["silent_shake"];
                    if (combat.contains("silent_shake_x")) globals::combat::silent_shake_x = combat["silent_shake_x"];
                    if (combat.contains("silent_shake_y")) globals::combat::silent_shake_y = combat["silent_shake_y"];
                    if (combat.contains("teamcheck")) globals::combat::teamcheck = combat["teamcheck"];
                    if (combat.contains("knockcheck")) globals::combat::knockcheck = combat["knockcheck"];
                    if (combat.contains("rangecheck")) globals::combat::rangecheck = combat["rangecheck"];
                    if (combat.contains("healthcheck")) globals::combat::healthcheck = combat["healthcheck"];
                    if (combat.contains("wallcheck")) globals::combat::wallcheck = combat["wallcheck"];
                    if (combat.contains("flags")) *globals::combat::flags = combat["flags"].get<std::vector<int>>();
                    if (combat.contains("range")) globals::combat::range = combat["range"];
                    if (combat.contains("aim_distance")) globals::combat::aim_distance = combat["aim_distance"];
                    if (combat.contains("healththreshhold")) globals::combat::healththreshhold = combat["healththreshhold"];
                    if (combat.contains("aimpart")) *globals::combat::aimpart = combat["aimpart"].get<std::vector<int>>();
                    if (combat.contains("airaimpart")) *globals::combat::airaimpart = combat["airaimpart"].get<std::vector<int>>();
                    if (combat.contains("silentaimpart")) *globals::combat::silentaimpart = combat["silentaimpart"].get<std::vector<int>>();
                    if (combat.contains("airsilentaimpart")) *globals::combat::airsilentaimpart = combat["airsilentaimpart"].get<std::vector<int>>();
                    if (combat.contains("silentaim")) globals::combat::silentaim = combat["silentaim"];
                    if (combat.contains("stickyaimsilent")) globals::combat::stickyaimsilent = combat["stickyaimsilent"];
                    if (combat.contains("silentaimtype")) globals::combat::silentaimtype = combat["silentaimtype"];
                    if (combat.contains("silentaimfov")) globals::combat::silentaimfov = combat["silentaimfov"];
                    if (combat.contains("drawsilentaimfov")) globals::combat::drawsilentaimfov = combat["drawsilentaimfov"];
                    if (combat.contains("silentaimfovsize")) globals::combat::silentaimfovsize = combat["silentaimfovsize"];
                    if (combat.contains("glowsilentaimfov")) globals::combat::glowsilentaimfov = combat["glowsilentaimfov"];
                    if (combat.contains("silentaimfovcolor")) {
                        auto colors = combat["silentaimfovcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::combat::silentaimfovcolor[i] = colors[i];
                    }
                    if (combat.contains("silentaimfovtransparency")) globals::combat::silentaimfovtransparency = combat["silentaimfovtransparency"];
                    if (combat.contains("silentaimfovfill")) globals::combat::silentaimfovfill = combat["silentaimfovfill"];
                    if (combat.contains("silentaimfovfilltransparency")) globals::combat::silentaimfovfilltransparency = combat["silentaimfovfilltransparency"];
                    if (combat.contains("silentaimfovfillcolor")) {
                        auto colors = combat["silentaimfovfillcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::combat::silentaimfovfillcolor[i] = colors[i];
                    }
                    if (combat.contains("silentaimfovglowcolor")) {
                        auto colors = combat["silentaimfovglowcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::combat::silentaimfovglowcolor[i] = colors[i];
                    }
                    if (combat.contains("silentaimfovshape")) globals::combat::silentaimfovshape = combat["silentaimfovshape"];
                    if (combat.contains("silentaimfovthickness")) globals::combat::silentaimfovthickness = combat["silentaimfovthickness"];
                    if (combat.contains("silent_closest_part")) globals::combat::silent_closest_part = combat["silent_closest_part"];
                    if (combat.contains("silent_smoothness")) globals::combat::silent_smoothness = combat["silent_smoothness"];
                    if (combat.contains("silent_smoothness_amount")) globals::combat::silent_smoothness_amount = combat["silent_smoothness_amount"];
                    if (combat.contains("aimbot_closest_part")) globals::combat::aimbot_closest_part = combat["aimbot_closest_part"];
                    if (combat.contains("spin_fov_aimbot")) globals::combat::spin_fov_aimbot = combat["spin_fov_aimbot"];
                    if (combat.contains("spin_fov_aimbot_speed")) globals::combat::spin_fov_aimbot_speed = combat["spin_fov_aimbot_speed"];
                    if (combat.contains("spin_fov_silentaim")) globals::combat::spin_fov_silentaim = combat["spin_fov_silentaim"];
                    if (combat.contains("spin_fov_silentaim_speed")) globals::combat::spin_fov_silentaim_speed = combat["spin_fov_silentaim_speed"];
                    if (combat.contains("orbit")) globals::combat::orbit = combat["orbit"];
                    if (combat.contains("orbitspeed")) globals::combat::orbitspeed = combat["orbitspeed"];
                    if (combat.contains("orbitrange")) globals::combat::orbitrange = combat["orbitrange"];
                    if (combat.contains("orbitheight")) globals::combat::orbitheight = combat["orbitheight"];
                    if (combat.contains("drawradiusring")) globals::combat::drawradiusring = combat["drawradiusring"];
                    if (combat.contains("orbittype")) globals::combat::orbittype = combat["orbittype"].get<int>();
                    if (combat.contains("antiaim")) globals::combat::antiaim = combat["antiaim"];
                    if (combat.contains("underground_antiaim")) globals::combat::underground_antiaim = combat["underground_antiaim"];
                    if (combat.contains("triggerbot")) globals::combat::triggerbot = combat["triggerbot"];
                    if (combat.contains("triggerbot_delay")) globals::combat::triggerbot_delay = combat["triggerbot_delay"];
                    if (combat.contains("triggerbot_item_checks")) globals::combat::triggerbot_item_checks = combat["triggerbot_item_checks"];

                    if (combat.contains("keybinds")) {
                        auto& binds = combat["keybinds"];
                        if (binds.contains("aimbotkeybind")) {
                            globals::combat::aimbotkeybind.key = binds["aimbotkeybind"]["key"];
                            globals::combat::aimbotkeybind.type = (keybind::c_keybind_type)binds["aimbotkeybind"]["type"];
                        }
                        if (binds.contains("silentaimkeybind")) {
                            globals::combat::silentaimkeybind.key = binds["silentaimkeybind"]["key"];
                            globals::combat::silentaimkeybind.type = (keybind::c_keybind_type)binds["silentaimkeybind"]["type"];
                        }
                        if (binds.contains("orbitkeybind")) {
                            globals::combat::orbitkeybind.key = binds["orbitkeybind"]["key"];
                            globals::combat::orbitkeybind.type = (keybind::c_keybind_type)binds["orbitkeybind"]["type"];
                        }
                        if (binds.contains("locktargetkeybind")) {
                            globals::combat::locktargetkeybind.key = binds["locktargetkeybind"]["key"];
                            globals::combat::locktargetkeybind.type = (keybind::c_keybind_type)binds["locktargetkeybind"]["type"];
                        }
                        if (binds.contains("triggerbotkeybind")) {
                            globals::combat::triggerbotkeybind.key = binds["triggerbotkeybind"]["key"];
                            globals::combat::triggerbotkeybind.type = (keybind::c_keybind_type)binds["triggerbotkeybind"]["type"];
                        }
                    }
                }

                        // Reset visuals toggles that should default off if missing
                        globals::visuals::sonar = false;
                        globals::visuals::sonar_detect_players = false;

                if (config_json.contains("visuals")) {
                    auto& visuals = config_json["visuals"];
                    if (visuals.contains("visuals")) globals::visuals::visuals = visuals["visuals"];
                    if (visuals.contains("boxes")) globals::visuals::boxes = visuals["boxes"];
                    if (visuals.contains("boxfill")) globals::visuals::boxfill = visuals["boxfill"];
                    if (visuals.contains("static_size")) globals::visuals::static_size = visuals["static_size"];
                    if (visuals.contains("static_size_value")) globals::visuals::static_size_value = visuals["static_size_value"];
                    if (visuals.contains("lockedindicator")) globals::visuals::lockedindicator = visuals["lockedindicator"];
                    if (visuals.contains("oofarrows")) globals::visuals::oofarrows = visuals["oofarrows"];
                    if (visuals.contains("lockedesp")) globals::visuals::lockedesp = visuals["lockedesp"];
                    if (visuals.contains("lockedespcolor")) {
                        auto colors = visuals["lockedespcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::lockedespcolor[i] = colors[i];
                    }
                    if (visuals.contains("snapline")) globals::visuals::snapline = visuals["snapline"];
                    if (visuals.contains("snaplinetype")) globals::visuals::snaplinetype = visuals["snaplinetype"];
                    if (visuals.contains("snaplineoverlaytype")) globals::visuals::snaplineoverlaytype = visuals["snaplineoverlaytype"];
                    if (visuals.contains("glowesp")) globals::visuals::glowesp = visuals["glowesp"];
                    if (visuals.contains("boxtype")) globals::visuals::boxtype = visuals["boxtype"];
                    if (visuals.contains("health")) globals::visuals::health = visuals["health"];
                    if (visuals.contains("healthbar")) globals::visuals::healthbar = visuals["healthbar"];
                    if (visuals.contains("name")) globals::visuals::name = visuals["name"];
                    if (visuals.contains("nametype")) globals::visuals::nametype = visuals["nametype"];
                    if (visuals.contains("toolesp")) globals::visuals::toolesp = visuals["toolesp"];
                    if (visuals.contains("distance")) globals::visuals::distance = visuals["distance"];
                    if (visuals.contains("visuals_flags")) *globals::visuals::visuals_flags = visuals["visuals_flags"].get<std::vector<int>>();
                    if (visuals.contains("chams")) globals::visuals::chams = visuals["chams"];
                    if (visuals.contains("skeletons")) globals::visuals::skeletons = visuals["skeletons"];
                    if (visuals.contains("localplayer")) globals::visuals::localplayer = visuals["localplayer"];
                    if (visuals.contains("aimviewer")) globals::visuals::aimviewer = visuals["aimviewer"];
                    if (visuals.contains("esppreview")) globals::visuals::esppreview = visuals["esppreview"];
                    if (visuals.contains("esp_preview_mode")) globals::visuals::esp_preview_mode = visuals["esp_preview_mode"];
                    if (visuals.contains("predictionsdot")) globals::visuals::predictionsdot = visuals["predictionsdot"];
                    if (visuals.contains("boxcolors")) {
                        auto colors = visuals["boxcolors"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::boxcolors[i] = colors[i];
                    }
                    if (visuals.contains("boxfillcolor")) {
                        auto colors = visuals["boxfillcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::boxfillcolor[i] = colors[i];
                    }
                    if (visuals.contains("glowcolor")) {
                        auto colors = visuals["glowcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::glowcolor[i] = colors[i];
                    }
                    if (visuals.contains("glow_size")) globals::visuals::glow_size = visuals["glow_size"];
                    if (visuals.contains("glow_opacity")) globals::visuals::glow_opacity = visuals["glow_opacity"];
                    if (visuals.contains("lockedcolor")) {
                        auto colors = visuals["lockedcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::lockedcolor[i] = colors[i];
                    }
                    if (visuals.contains("oofcolor")) {
                        auto colors = visuals["oofcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::oofcolor[i] = colors[i];
                    }
                    if (visuals.contains("snaplinecolor")) {
                        auto colors = visuals["snaplinecolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::snaplinecolor[i] = colors[i];
                    }
                    if (visuals.contains("healthbarcolor")) {
                        auto colors = visuals["healthbarcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::healthbarcolor[i] = colors[i];
                    }
                    if (visuals.contains("healthbarcolor1")) {
                        auto colors = visuals["healthbarcolor1"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::healthbarcolor1[i] = colors[i];
                    }
                    if (visuals.contains("enable_health_glow")) globals::visuals::enable_health_glow = visuals["enable_health_glow"];
                    if (visuals.contains("health_glow_size")) globals::visuals::health_glow_size = visuals["health_glow_size"];
                    if (visuals.contains("health_glow_opacity")) globals::visuals::health_glow_opacity = visuals["health_glow_opacity"];
                    if (visuals.contains("healthglowcolor")) {
                        auto colors = visuals["healthglowcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::healthglowcolor[i] = colors[i];
                    }
                    if (visuals.contains("namecolor")) {
                        auto colors = visuals["namecolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::namecolor[i] = colors[i];
                    }
                    if (visuals.contains("toolespcolor")) {
                        auto colors = visuals["toolespcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::toolespcolor[i] = colors[i];
                    }
                    if (visuals.contains("distancecolor")) {
                        auto colors = visuals["distancecolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::distancecolor[i] = colors[i];
                    }
                    if (visuals.contains("chamscolor")) {
                        auto colors = visuals["chamscolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::chamscolor[i] = colors[i];
                    }
                    if (visuals.contains("chamscolor1")) {
                        auto colors = visuals["chamscolor1"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::chamscolor1[i] = colors[i];
                    }
                    if (visuals.contains("health_bar_outline")) globals::visuals::health_bar_outline = visuals["health_bar_outline"];
                    if (visuals.contains("health_bar_gradient")) globals::visuals::health_bar_gradient = visuals["health_bar_gradient"];
                    if (visuals.contains("health_bar_position")) globals::visuals::health_bar_position = visuals["health_bar_position"];
            if (visuals.contains("skeletonscolor")) {
                        auto colors = visuals["skeletonscolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::skeletonscolor[i] = colors[i];
                    }
                    if (visuals.contains("fortniteindicator")) globals::visuals::fortniteindicator = visuals["fortniteindicator"];
                    if (visuals.contains("hittracer")) globals::visuals::hittracer = visuals["hittracer"];
                    if (visuals.contains("trail")) globals::visuals::trail = visuals["trail"];
                    if (visuals.contains("hitbubble")) globals::visuals::hitbubble = visuals["hitbubble"];
                    if (visuals.contains("targetchams")) globals::visuals::targetchams = visuals["targetchams"];
                    if (visuals.contains("targetskeleton")) globals::visuals::targetskeleton = visuals["targetskeleton"];
                    if (visuals.contains("localplayerchams")) globals::visuals::localplayerchams = visuals["localplayerchams"];
                    if (visuals.contains("localgunchams")) globals::visuals::localgunchams = visuals["localgunchams"];
                    if (visuals.contains("enemycheck")) globals::visuals::enemycheck = visuals["enemycheck"];
                    if (visuals.contains("friendlycheck")) globals::visuals::friendlycheck = visuals["friendlycheck"];
                    if (visuals.contains("teamcheck")) globals::visuals::teamcheck = visuals["teamcheck"];
                    if (visuals.contains("rangecheck")) globals::visuals::rangecheck = visuals["rangecheck"];
                    if (visuals.contains("range")) globals::visuals::range = visuals["range"];
                    if (visuals.contains("visual_range")) globals::visuals::visual_range = visuals["visual_range"];
                    if (visuals.contains("sonar")) globals::visuals::sonar = visuals["sonar"];
                    if (visuals.contains("sonar_range")) globals::visuals::sonar_range = visuals["sonar_range"];
                    if (visuals.contains("sonar_thickness")) globals::visuals::sonar_thickness = visuals["sonar_thickness"];
                    if (visuals.contains("sonar_speed")) globals::visuals::sonar_speed = visuals["sonar_speed"];
                    if (visuals.contains("sonarcolor")) {
                        auto colors = visuals["sonarcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::sonarcolor[i] = colors[i];
                    }
                    if (visuals.contains("sonar_detect_players")) globals::visuals::sonar_detect_players = visuals["sonar_detect_players"];
                    if (visuals.contains("sonar_detect_color_out")) {
                        auto colors = visuals["sonar_detect_color_out"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::sonar_detect_color_out[i] = colors[i];
                    }
                    if (visuals.contains("sonar_detect_color_in")) {
                        auto colors = visuals["sonar_detect_color_in"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::sonar_detect_color_in[i] = colors[i];
                    }
                    if (visuals.contains("box_overlay_flags")) *globals::visuals::box_overlay_flags = visuals["box_overlay_flags"].get<std::vector<int>>();
                    if (visuals.contains("name_overlay_flags")) *globals::visuals::name_overlay_flags = visuals["name_overlay_flags"].get<std::vector<int>>();
                    if (visuals.contains("healthbar_overlay_flags")) *globals::visuals::healthbar_overlay_flags = visuals["healthbar_overlay_flags"].get<std::vector<int>>();
                    if (visuals.contains("tool_overlay_flags")) *globals::visuals::tool_overlay_flags = visuals["tool_overlay_flags"].get<std::vector<int>>();
                    if (visuals.contains("distance_overlay_flags")) *globals::visuals::distance_overlay_flags = visuals["distance_overlay_flags"].get<std::vector<int>>();
                    if (visuals.contains("skeleton_overlay_flags")) *globals::visuals::skeleton_overlay_flags = visuals["skeleton_overlay_flags"].get<std::vector<int>>();
                    if (visuals.contains("chams_overlay_flags")) *globals::visuals::chams_overlay_flags = visuals["chams_overlay_flags"].get<std::vector<int>>();
                    if (visuals.contains("snapline_overlay_flags")) *globals::visuals::snapline_overlay_flags = visuals["snapline_overlay_flags"].get<std::vector<int>>();
                    if (visuals.contains("oof_overlay_flags")) *globals::visuals::oof_overlay_flags = visuals["oof_overlay_flags"].get<std::vector<int>>();
                    if (visuals.contains("target_only_esp")) globals::visuals::target_only_esp = visuals["target_only_esp"];
                    if (visuals.contains("target_only_list")) globals::visuals::target_only_list = visuals["target_only_list"].get<std::vector<std::string>>();
                    if (visuals.contains("fog_enabled")) globals::visuals::fog_enabled = visuals["fog_enabled"];
                    if (visuals.contains("fog_start")) globals::visuals::fog_start = visuals["fog_start"];
                    if (visuals.contains("fog_end")) globals::visuals::fog_end = visuals["fog_end"];
                    if (visuals.contains("fog_color")) {
                        auto colors = visuals["fog_color"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::visuals::fog_color[i] = colors[i];
                    }
                }

                if (config_json.contains("misc")) {
                    auto& misc = config_json["misc"];
                    if (misc.contains("menuglowcolor")) {
                        auto colors = misc["menuglowcolor"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::misc::menuglowcolor[i] = colors[i];
                    }
                    if (misc.contains("overlay_color")) {
                        auto colors = misc["overlay_color"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::misc::overlay_color[i] = colors[i];
                    }
                    if (misc.contains("accent_color")) {
                        auto colors = misc["accent_color"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::misc::accent_color[i] = colors[i];
                    }
                    if (misc.contains("overlay_star_color")) {
                        auto colors = misc["overlay_star_color"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::misc::overlay_star_color[i] = colors[i];
                    }
                    if (misc.contains("health_full_color")) {
                        auto colors = misc["health_full_color"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::misc::health_full_color[i] = colors[i];
                    }
                    if (misc.contains("health_low_color")) {
                        auto colors = misc["health_low_color"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::misc::health_low_color[i] = colors[i];
                    }
                    if (misc.contains("custom_cursor")) globals::misc::custom_cursor = misc["custom_cursor"];
                    if (misc.contains("rotate360")) globals::misc::rotate360 = misc["rotate360"];
                    if (misc.contains("camera_rotation_yaw")) globals::misc::camera_rotation_yaw = misc["camera_rotation_yaw"];
                    if (misc.contains("camera_rotation_pitch")) globals::misc::camera_rotation_pitch = misc["camera_rotation_pitch"];
                    if (misc.contains("rotate360_speed")) globals::misc::rotate360_speed = misc["rotate360_speed"];
                    if (misc.contains("rotate360_vspeed")) globals::misc::rotate360_vspeed = misc["rotate360_vspeed"];
                    if (misc.contains("menuglow")) globals::misc::menuglow = misc["menuglow"];
                    if (misc.contains("overlay_stars")) globals::misc::overlay_stars = misc["overlay_stars"];
                    if (misc.contains("enable_notifications")) globals::misc::enable_notifications = misc["enable_notifications"];
                    if (misc.contains("music_max_volume")) globals::misc::music_max_volume = misc["music_max_volume"];
                    if (misc.contains("keybinds")) globals::misc::keybinds = misc["keybinds"];
                    if (misc.contains("autofriend_enabled")) globals::misc::autofriend_enabled = misc["autofriend_enabled"];
                    if (misc.contains("autofriend_group_members")) globals::misc::autofriend_group_members = misc["autofriend_group_members"];
                    if (misc.contains("autofriend_status")) globals::misc::autofriend_status = misc["autofriend_status"];

                    if (misc.contains("theme_window_bg")) {
                        auto colors = misc["theme_window_bg"].get<std::vector<float>>();
                        for (int i = 0; i < 3 && i < colors.size(); i++) globals::misc::WindowBG[i] = colors[i];
                    }
                    if (misc.contains("theme_border")) {
                        auto colors = misc["theme_border"].get<std::vector<float>>();
                        for (int i = 0; i < 3 && i < colors.size(); i++) globals::misc::Border[i] = colors[i];
                    }
                    if (misc.contains("theme_child")) {
                        auto colors = misc["theme_child"].get<std::vector<float>>();
                        for (int i = 0; i < 3 && i < colors.size(); i++) globals::misc::Child[i] = colors[i];
                    }
                    if (misc.contains("theme_color")) {
                        auto colors = misc["theme_color"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::misc::ThemeColor[i] = colors[i];
                    }
                    if (misc.contains("overlay_border")) {
                        auto colors = misc["overlay_border"].get<std::vector<float>>();
                        for (int i = 0; i < 4 && i < colors.size(); i++) globals::misc::OverlayBorder[i] = colors[i];
                    }

                    auto loadThemeColor = [&](const char* key, float* target, int count) {
                        if (misc.contains(key)) {
                            auto col = misc[key].get<std::vector<float>>();
                            for (int i = 0; i < count && i < col.size(); i++) target[i] = col[i];
                        }
                    };

                    loadThemeColor("AccentActive", globals::misc::AccentActive, 4);
                    loadThemeColor("Header", globals::misc::Header, 4);
                    loadThemeColor("PopupBG", globals::misc::PopupBG, 4);
                    loadThemeColor("Text", globals::misc::Text, 4);
                    loadThemeColor("TextDisabled", globals::misc::TextDisabled, 4);
                    loadThemeColor("Button", globals::misc::Button, 4);
                    loadThemeColor("ButtonHovered", globals::misc::ButtonHovered, 4);
                    loadThemeColor("ButtonActive", globals::misc::ButtonActive, 4);
                    loadThemeColor("FrameBG", globals::misc::FrameBG, 4);
                    loadThemeColor("FrameBGHovered", globals::misc::FrameBGHovered, 4);
                    loadThemeColor("FrameBGActive", globals::misc::FrameBGActive, 4);
                    loadThemeColor("ScrollbarBG", globals::misc::ScrollbarBG, 4);
                    loadThemeColor("ScrollbarGrab", globals::misc::ScrollbarGrab, 4);
                    loadThemeColor("ScrollbarGrabHovered", globals::misc::ScrollbarGrabHovered, 4);
                    loadThemeColor("ScrollbarGrabActive", globals::misc::ScrollbarGrabActive, 4);
                    loadThemeColor("SliderGrab", globals::misc::SliderGrab, 4);
                    loadThemeColor("SliderGrabActive", globals::misc::SliderGrabActive, 4);

                    if (misc.contains("speed")) globals::misc::speed = misc["speed"];
                    if (misc.contains("speedtype")) globals::misc::speedtype = misc["speedtype"];
                    if (misc.contains("speedvalue")) globals::misc::speedvalue = misc["speedvalue"];
                    if (misc.contains("nojumpcooldown")) globals::misc::nojumpcooldown = misc["nojumpcooldown"];
                    if (misc.contains("jumppower")) globals::misc::jumppower = misc["jumppower"];
                    if (misc.contains("jumppowervalue")) globals::misc::jumppowervalue = misc["jumppowervalue"];
                    if (misc.contains("flight")) globals::misc::flight = misc["flight"];
                    if (misc.contains("flighttype")) globals::misc::flighttype = misc["flighttype"];
                    if (misc.contains("flightvalue")) globals::misc::flightvalue = misc["flightvalue"];
                    if (misc.contains("hipheight")) globals::misc::hipheight = misc["hipheight"];
                    if (misc.contains("hipheightvalue")) globals::misc::hipheightvalue = misc["hipheightvalue"];
                    if (misc.contains("rapidfire")) globals::misc::rapidfire = misc["rapidfire"];
                    if (misc.contains("autoarmor")) globals::misc::autoarmor = misc["autoarmor"];
                    if (misc.contains("autoreload")) globals::misc::autoreload = misc["autoreload"];
                    if (misc.contains("autostomp")) globals::misc::autostomp = misc["autostomp"];
                    if (misc.contains("antistomp")) globals::misc::antistomp = misc["antistomp"];
                    if (misc.contains("bikefly")) globals::misc::bikefly = misc["bikefly"];
                    if (misc.contains("spectate")) globals::misc::spectate = misc["spectate"];
                    if (misc.contains("vsync")) globals::misc::vsync = misc["vsync"];
                    if (misc.contains("targethud")) globals::misc::targethud = misc["targethud"];
                    if (misc.contains("playerlist")) globals::misc::playerlist = misc["playerlist"];
                    if (misc.contains("performance_mode")) globals::misc::performance_mode = misc["performance_mode"];
                    if (misc.contains("keybinds")) globals::misc::keybinds = misc["keybinds"];
                    if (misc.contains("font_index")) globals::misc::font_index = misc["font_index"];
                    if (misc.contains("unlock_fps")) globals::misc::unlock_fps = misc["unlock_fps"];
                    if (misc.contains("fps_cap")) globals::misc::fps_cap = misc["fps_cap"];
                    if (misc.contains("show_fps")) globals::misc::show_fps = misc["show_fps"];
                    if (misc.contains("spotify")) globals::misc::spotify = misc["spotify"];
                    if (misc.contains("colors")) globals::misc::colors = misc["colors"];
                    if (misc.contains("streamproof")) globals::misc::streamproof = misc["streamproof"];

                    if (misc.contains("keybinds_data")) {
                        auto& binds = misc["keybinds_data"];
                        if (binds.contains("speedkeybind")) {
                            globals::misc::speedkeybind.key = binds["speedkeybind"]["key"];
                            globals::misc::speedkeybind.type = (keybind::c_keybind_type)binds["speedkeybind"]["type"];
                        }
                        if (binds.contains("jumppowerkeybind")) {
                            globals::misc::jumppowerkeybind.key = binds["jumppowerkeybind"]["key"];
                            globals::misc::jumppowerkeybind.type = (keybind::c_keybind_type)binds["jumppowerkeybind"]["type"];
                        }
                        if (binds.contains("flightkeybind")) {
                            globals::misc::flightkeybind.key = binds["flightkeybind"]["key"];
                            globals::misc::flightkeybind.type = (keybind::c_keybind_type)binds["flightkeybind"]["type"];
                        }
                        if (binds.contains("stompkeybind")) {
                            globals::misc::stompkeybind.key = binds["stompkeybind"]["key"];
                            globals::misc::stompkeybind.type = (keybind::c_keybind_type)binds["stompkeybind"]["type"];
                        }
                        if (binds.contains("spectatebind")) {
                            globals::misc::spectatebind.key = binds["spectatebind"]["key"];
                            globals::misc::spectatebind.type = (keybind::c_keybind_type)binds["spectatebind"]["type"];
                        }
                        if (binds.contains("rotate360keybind")) {
                            globals::misc::rotate360keybind.key = binds["rotate360keybind"]["key"];
                            globals::misc::rotate360keybind.type = (keybind::c_keybind_type)binds["rotate360keybind"]["type"];
                        }
                        if (binds.contains("menu_hotkey")) {
                            globals::misc::menu_hotkey.key = binds["menu_hotkey"]["key"];
                            globals::misc::menu_hotkey.type = (keybind::c_keybind_type)binds["menu_hotkey"]["type"];
                        }
                    }
                }

            file.close();
            current_config_name = sanitized_name;
            globals::misc::config_loaded_flag = true; // Set flag after successful config load
            ImAdd::ClearWidgetStates(); // Clear static states of ImGui widgets
            std::cout << "[CONFIG] Successfully loaded config: " << name << "\n";
            return true;
        }
        catch (const json::parse_error& e) {
            std::cout << "[CONFIG] Failed to parse config file: " << e.what() << "\n";
            file.close();
            return false;
        }
        catch (...) {
            std::cout << "[CONFIG] Failed to load config (unknown error): " << name << "\n";
            file.close();
            return false;
        }
    }

    std::cout << "[CONFIG] Failed to open config file: " << name << "\n";
    return false;
}

bool ConfigSystem::delete_config(const std::string& name) {
    if (name.empty()) return false;

    std::string sanitized_name = sanitize_filename(name);
    if (sanitized_name.empty()) return false;

    std::string filepath = config_directory + "\\" + sanitized_name + ".json";

    if (fs::exists(filepath)) {
        fs::remove(filepath);
        refresh_config_list();

        if (current_config_name == name) {
            current_config_name.clear();
        }

        std::cout << "[CONFIG] Successfully deleted config: " << name << "\n";
        return true;
    }

    std::cout << "[CONFIG] Config file not found: " << name << "\n";
    return false;
}

bool ConfigSystem::rename_config(const std::string& old_name, const std::string& new_name) {
    if (old_name.empty() || new_name.empty()) return false;

    std::string sanitized_old = sanitize_filename(old_name);
    std::string sanitized_new = sanitize_filename(new_name);
    if (sanitized_old.empty() || sanitized_new.empty()) return false;
    if (sanitized_old == sanitized_new) return false;

    std::string old_path = config_directory + "\\" + sanitized_old + ".json";
    std::string new_path = config_directory + "\\" + sanitized_new + ".json";

    if (!fs::exists(old_path) || fs::exists(new_path)) {
        return false;
    }

    try {
        fs::rename(old_path, new_path);
    }
    catch (...) {
        return false;
    }

    refresh_config_list();

    if (current_config_name == old_name || current_config_name == sanitized_old) {
        current_config_name = sanitized_new;
    }

    if (autoload_config_name == old_name || autoload_config_name == sanitized_old) {
        autoload_config_name = sanitized_new;
        save_autoload_setting(autoload_config_name);
    }

    std::cout << "[CONFIG] Renamed config from " << old_name << " to " << new_name << "\n";
    return true;
}

    void ConfigSystem::render_config_ui(float width, float height) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8, 4 });

        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##cfg_name", "Config name", config_name_buffer, sizeof(config_name_buffer));

        // --- Buttons Row ---
        float b_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3) / 4;
        
        if (ImGui::Button("Create", { b_width, 24 })) {
            if (strlen(config_name_buffer) > 0) {
                if (save_config(std::string(config_name_buffer))) {
                    Notifications::Success("Created config: " + std::string(config_name_buffer));
                    memset(config_name_buffer, 0, sizeof(config_name_buffer));
                    refresh_config_list();
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save", { b_width, 24 })) {
            if (strlen(config_name_buffer) > 0) {
                if (save_config(std::string(config_name_buffer))) {
                    Notifications::Success("Saved config: " + std::string(config_name_buffer));
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete", { b_width, 24 })) {
            if (strlen(config_name_buffer) > 0) {
                if (delete_config(std::string(config_name_buffer))) {
                    Notifications::Success("Deleted config: " + std::string(config_name_buffer));
                    memset(config_name_buffer, 0, sizeof(config_name_buffer));
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Rename", { b_width, 24 })) {
            if (strlen(config_name_buffer) > 0 && !current_config_name.empty()) {
                std::string new_name = std::string(config_name_buffer);
                if (rename_config(current_config_name, new_name)) {
                    Notifications::Success("Renamed '" + current_config_name + "' to '" + new_name + "'");
                    current_config_name = new_name;
                } else {
                    Notifications::Error("Failed to rename config!");
                }
            } else {
                Notifications::Warning("Select a config and type a new name!");
            }
        }

        // Calculate space needed for bottom elements: autoload label + combo + 2 buttons + spacing
        float bottom_height = ImGui::GetTextLineHeight() + ImGui::GetFrameHeight() + 16 + 16 + (ImGui::GetStyle().ItemSpacing.y * 3) + 6;
        float list_height = ImGui::GetContentRegionAvail().y - bottom_height;
        
        if (ImGui::BeginChild("##cfg_list_viewer", { 0, list_height }, false)) {
            if (config_files.empty()) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No configs found");
            } else {
                for (const auto& config : config_files) {
                    bool is_selected = (current_config_name == config);
                    if (ImGui::Selectable(config.c_str(), is_selected)) {
                        current_config_name = config;
                        strcpy_s(config_name_buffer, sizeof(config_name_buffer), config.c_str());
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                        load_config(config);
                    }
                }
            }
        }
        ImGui::EndChild();

        // Reduce gap slightly
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);

        // Autoload selection
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::Text("Autoload");
        ImGui::PopStyleColor();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##autoload_combo", autoload_config_name.empty() ? "None" : autoload_config_name.c_str())) {
            bool is_none_selected = autoload_config_name.empty();
            if (ImGui::Selectable("None", is_none_selected)) {
                save_autoload_setting("");
            }
            if (is_none_selected) {
                ImGui::SetItemDefaultFocus();
            }

            for (const auto& config : config_files) {
                bool is_selected = (autoload_config_name == config);
                if (ImGui::Selectable(config.c_str(), is_selected)) {
                    save_autoload_setting(config);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        
        if (ImGui::Button("Exit", { -1, 16 })) {
            exit(0);
        }
        if (ImGui::Button("Refresh List", { -1, 16 })) {
            refresh_config_list();
            Notifications::Info("Refreshed config list");
        }

        ImGui::PopStyleVar();
    }

const std::string& ConfigSystem::get_current_config() const {
    return current_config_name;
}
