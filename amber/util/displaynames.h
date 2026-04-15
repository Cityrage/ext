#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <algorithm>
#include <cctype>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#include "../util/classes/classes.h"

namespace displaynames {
inline char to_lower_ascii(unsigned char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : static_cast<char>(c);
}

// Simple cache to avoid repeated network calls for the same user id
inline std::unordered_map<uint64_t, std::string> cache;
inline std::unordered_set<uint64_t> inflight;
inline std::mutex cache_mutex;

inline std::string fetch_display_name_api(uint64_t userid) {
    if (userid == 0) return {};

    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache.find(userid);
        if (it != cache.end()) return it->second;
    }

    std::wstring host = L"users.roblox.com";
    std::wstring path = L"/v1/users/" + std::to_wstring(userid);

    std::string response;
    HINTERNET hSession = WinHttpOpen(L"LainDisplay/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return {};

    WinHttpSetTimeouts(hSession, 3000, 3000, 3000, 3000);

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return {};
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return {};
    }

    bool ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (ok) ok = WinHttpReceiveResponse(hRequest, NULL);

    if (ok) {
        DWORD dwSize = 0;
        do {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize) || dwSize == 0) break;
            std::string buffer(dwSize, 0);
            DWORD dwDownloaded = 0;
            if (WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded) && dwDownloaded > 0) {
                buffer.resize(dwDownloaded);
                response.append(buffer);
            }
        } while (dwSize > 0);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (response.empty()) return {};

    std::string display;
    const std::string key = "\"displayName\":\"";
    auto pos = response.find(key);
    if (pos != std::string::npos) {
        pos += key.size();
        auto end = response.find('"', pos);
        if (end != std::string::npos && end > pos) {
            display = response.substr(pos, end - pos);
        }
    }

    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        cache[userid] = display;
    }

    return display;
}

inline std::string ensure_display_name_async(uint64_t userid) {
    if (userid == 0) return {};

    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache.find(userid);
        if (it != cache.end()) return it->second;
        if (inflight.count(userid)) return {};
        inflight.insert(userid);
    }

    std::thread([userid]() {
        std::string display = fetch_display_name_api(userid);
        std::lock_guard<std::mutex> lock(cache_mutex);
        cache[userid] = display;
        inflight.erase(userid);
    }).detach();

    return {};
}

inline std::string sanitize(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), value.end());
    return value;
}

inline bool is_valid_display(const std::string& candidate, const std::string& username) {
    if (candidate.empty()) return false;
    std::string lower = candidate;
    for (auto& ch : lower) ch = to_lower_ascii(static_cast<unsigned char>(ch));
    if (lower == "humanoid") return false;
    std::string user_lower = username;
    for (auto& ch : user_lower) ch = to_lower_ascii(static_cast<unsigned char>(ch));
    return lower != user_lower;
}

inline std::string get_best_display(roblox::player& player) {
    std::string username = player.name;
    if (username == "NPC") return "NPC";

    std::string display = sanitize(player.displayname);
    if (!is_valid_display(display, username)) {
        std::string pd = sanitize(player.main.get_displayname());
        if (is_valid_display(pd, username)) display = pd;
    }
    if (!is_valid_display(display, username)) {
        std::string hd = sanitize(player.humanoid.get_humdisplayname());
        if (is_valid_display(hd, username)) display = hd;
    }
    if (!is_valid_display(display, username)) {
        std::string api_display = ensure_display_name_async(player.userid.address);
        if (is_valid_display(api_display, username)) display = api_display;
    }
    if (!is_valid_display(display, username)) display = username;
    return display;
}
}
