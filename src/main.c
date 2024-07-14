#include "luna/game.h"
#include "scenes.h"

int main() {
	// Load settings
	SettingsList* settings = GenerateDefaultSettings();
	LoadSettingsFile("settings.ini", settings);

	// Define resource files
	ResourceListDesc resourceDesc = {
		.resourceFile = "luna.rres",
		.resourcePassword = "password12345"
	};

	// Initialize game state
	CreateGame(settings, resourceDesc);
	GameRegisterScenes();

	// Run game
	UpdateGame();
	DestroyGame();

	// Cleanup
	free(settings);
	return 0;
}