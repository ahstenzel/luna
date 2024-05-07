#include "luna/game.h"

int main() {
	// Load settings
	SettingsList* settings = GenerateDefaultSettings();
	LoadSettingsFile("settings.ini", settings);

	// Define resource files
	ResourceListDesc resourceLists = {
		.resourceFile = "",
		.resourcePassword = "",
		.resourceName = ""
	};

	// Game state
	CreateGame(settings, &resourceLists, 1);
	UpdateGame();
	DestroyGame();

	// Cleanup
	free(settings);
	return 0;
}