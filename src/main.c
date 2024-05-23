#include "luna/game.h"
#include "scenes.h"

int main() {
	// Load settings
	SettingsList* settings = GenerateDefaultSettings();
	LoadSettingsFile("settings.ini", settings);

	// Define resource files
	ResourceListDesc resourceLists = {
		.resourceFile = "luna.rres",
		.resourcePassword = "password12345",
		.resourceName = "default"
	};

	// Initialize game state
	CreateGame(settings, &resourceLists, 1);
	GameRegisterScenes();

	// Run game
	UpdateGame();
	DestroyGame();

	// Cleanup
	free(settings);
	return 0;
}