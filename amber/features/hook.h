#include "../util/classes/classes.h"
#include "../util/globals.h"

using namespace engine;


class player_cache {
public:
	static void Initialize();
	static void UpdateCache();
	static void Cleanup();
	static void Refresh();
};



namespace hooks {
	void listener();
	void anti_aim();
	void cache();
	void silentrun();
	void silentrun2();
	void silent_combined();
	void silent();
	void speed();
	void jumppower();
	void flight();
	void hipheight();
	void antisit();
	void combat();
	void misc();
	void overlay();
	void autoreload();
	void spinbot();
	void rapidfire();
	void autostomp();
	void antistomp();
	void voidhide();
	void orbit();
	void aimbot();
	void triggerbot();
	void nojumpcooldown(); // Declare the nojumpcooldown function
	void macro(); // Declare macro function
}
namespace hooking {
	void launchThreads();
}
