#include "autofriend.h"
#include "../../../util/globals.h"
#include <winhttp.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>

#pragma comment(lib, "winhttp.lib")

namespace autofriend {
    AutoFriendManager g_auto_friend_manager;
    
    bool AutoFriendManager::fetchGroupMembers(const std::string& groupId) {
        if (is_loading.load()) {
            return false; // Already loading
        }
        
        is_loading.store(true);
        last_error.clear();
        
        // Run in separate thread to avoid blocking
        std::thread([this, groupId]() {
            try {
                // Clear previous members
                cached_members.clear();
                std::string cursor = "";
                bool has_more = true;
                int total_fetched = 0;

                HINTERNET hSession = WinHttpOpen(L"RobloxOverlay/1.0",
                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                    WINHTTP_NO_PROXY_NAME,
                    WINHTTP_NO_PROXY_BYPASS, 0);

                if (!hSession) {
                    last_error = "Failed to create HTTP session";
                    is_loading.store(false);
                    return;
                }
                WinHttpSetTimeouts(hSession, 30000, 30000, 30000, 30000);

                std::wstring whost(L"groups.roblox.com");
                HINTERNET hConnect = WinHttpConnect(hSession, whost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);

                if (!hConnect) {
                    last_error = "Failed to connect to Roblox API";
                    WinHttpCloseHandle(hSession);
                    is_loading.store(false);
                    return;
                }

                while (has_more) {
                     // Build API endpoint URL with pagination support
                    std::string path = "/v1/groups/" + groupId + "/users?limit=100&sortOrder=Asc";
                    if (!cursor.empty()) {
                        path += "&cursor=" + cursor;
                    }
                    std::wstring wpath(path.begin(), path.end());

                    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wpath.c_str(),
                        NULL, WINHTTP_NO_REFERER,
                        WINHTTP_DEFAULT_ACCEPT_TYPES,
                        WINHTTP_FLAG_SECURE);

                    if (!hRequest) {
                        last_error = "Failed to create HTTP request";
                        break;
                    }

                    std::wstring headers = L"Accept: application/json\r\n";
                    WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

                    BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

                    if (bResults) {
                        bResults = WinHttpReceiveResponse(hRequest, NULL);
                    }

                     std::string response;
                    if (bResults) {
                        DWORD dwStatusCode = 0;
                        DWORD dwSize = sizeof(dwStatusCode);
                        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);

                        if (dwStatusCode == 200) {
                            DWORD dwBytesRead = 0;
                            char buffer[8192];
                            do {
                                if (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &dwBytesRead)) {
                                    buffer[dwBytesRead] = '\0';
                                    response += buffer;
                                }
                            } while (dwBytesRead > 0);
                        } else {
                            last_error = "API request failed with status: " + std::to_string(dwStatusCode);
                            WinHttpCloseHandle(hRequest);
                            break;
                        }
                    } else {
                         last_error = "Failed to receive response";
                         WinHttpCloseHandle(hRequest);
                         break;
                    }
                    WinHttpCloseHandle(hRequest);

                    // Parse JSON response for data and cursor
                    // Find cursor
                    size_t cursorPos = response.find("\"nextPageCursor\":");
                    std::string next_cursor = "";
                    if (cursorPos != std::string::npos) {
                        cursorPos += 17; // Skip "nextPageCursor":
                         size_t start = response.find("\"", cursorPos);
                         if (start != std::string::npos) {
                             start++;
                             size_t end = response.find("\"", start);
                             if (end != std::string::npos) {
                                 next_cursor = response.substr(start, end - start);
                             }
                         }
                    }
                    
                    // Update main cursor
                    cursor = next_cursor;
                    if (cursor.empty() || cursor == "null") {
                        has_more = false;
                    }

                     // Parse data array
                    size_t dataStart = response.find("\"data\":[");
                    if (dataStart != std::string::npos) {
                        dataStart += 8; 
                        size_t dataEnd = response.rfind("]"); // Find LAST bracket to be safe? No, find matching bracket.
                        // Simplified: Assume data array ends before next fields like "nextPageCursor" usually, but JSON order varies.
                        // Robust way: scan for matching bracket.
                        int brackets = 1;
                        size_t pos = dataStart;
                        while (pos < response.length() && brackets > 0) {
                            if (response[pos] == '[') brackets++;
                            if (response[pos] == ']') brackets--;
                            pos++;
                        }
                        
                        if (brackets == 0) {
                             std::string dataArray = response.substr(dataStart, pos - dataStart - 1); 
                             
                             // Parse objects inside
                            size_t objPos = 0;
                            while ((objPos = dataArray.find("{", objPos)) != std::string::npos) {
                                size_t objEnd = dataArray.find("}", objPos);
                                if (objEnd != std::string::npos) {
                                    std::string userObj = dataArray.substr(objPos, objEnd - objPos + 1);
                                    
                                     // Extract username
                                    size_t uPos = userObj.find("\"username\":");
                                    std::string username = "";
                                    if (uPos != std::string::npos) {
                                        size_t start = userObj.find("\"", uPos + 11);
                                        if (start != std::string::npos) {
                                            size_t end = userObj.find("\"", start + 1);
                                            if (end != std::string::npos) username = userObj.substr(start + 1, end - start - 1);
                                        }
                                    }

                                     // Extract displayName
                                    size_t dPos = userObj.find("\"displayName\":");
                                    std::string displayName = username;
                                    if (dPos != std::string::npos) {
                                        size_t start = userObj.find("\"", dPos + 14);
                                        if (start != std::string::npos) {
                                            size_t end = userObj.find("\"", start + 1);
                                            if (end != std::string::npos) displayName = userObj.substr(start + 1, end - start - 1);
                                        }
                                    }

                                    // Extract userId
                                    size_t iPos = userObj.find("\"userId\":");
                                    uint64_t userId = 0;
                                    if (iPos != std::string::npos) {
                                        size_t start = iPos + 9;
                                        while (start < userObj.length() && !isdigit(userObj[start])) start++;
                                        size_t end = start;
                                        while (end < userObj.length() && isdigit(userObj[end])) end++;
                                        if (end > start) {
                                            try { userId = std::stoull(userObj.substr(start, end - start)); } catch(...) {}
                                        }
                                    }

                                    if (!username.empty()) {
                                        cached_members.push_back({ username, displayName, userId, "Member" });
                                        total_fetched++;
                                    }
                                    objPos = objEnd + 1;
                                } else {
                                    break;
                                }
                            }
                        }
                    }
                } // End while loop

                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);

                 // Update globals
                globals::misc::group_members_loaded = true;
                globals::misc::loading_group_members = false;
                
                if (cached_members.empty()) {
                    globals::misc::autofriend_status = "No members found";
                } else {
                    for (const auto& member : cached_members) {
                        globals::bools::player_status[member.username] = true;
                    }
                    globals::misc::autofriend_status = "Added " + std::to_string(total_fetched) + " friends";
                }

            } catch (const std::exception& e) {
                last_error = "Exception: " + std::string(e.what());
                globals::misc::autofriend_status = "Error: " + last_error;
            }

            is_loading.store(false);
        }).detach();
        
        return true;
    }
}
