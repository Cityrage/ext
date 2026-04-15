#pragma once

class AvatarManager;

namespace overlay
{
	void thread();
	void addnotification(const char* log);
    AvatarManager* get_avatar_manager();
    void init_avatar_manager(void* device, void* context);
}