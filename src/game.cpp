#include "game.hpp"

bool Game::quitFlag = false;
SDL_Window* Game::sdlWindow = nullptr;
SDL_GPUDevice* Game::sdlGPUDevice = nullptr;
SDL_GPUSampler* Game::sdlGPUSampler = nullptr;
SDL_GPUTexture* Game::sdlGPUTexture = nullptr;
SDL_GPUGraphicsPipeline* Game::sdlSpriteRenderPipeline = nullptr;
SDL_GPUTransferBuffer* Game::sdlSpriteDataTransferBuffer = nullptr;
SDL_GPUBuffer* Game::sdlSpriteDataBuffer = nullptr;

bool Game::Init() {
	// Initialize SDL
	if (!SDL_SetAppMetadata("luna", "1.0.0", "com.luna")) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_SetAppMetadata failed! %s", SDL_GetError());
		Game::Cleanup();
		return false;
	}
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed! %s", SDL_GetError());
		Game::Cleanup();
		return false;
	}

	// Create window
	Game::sdlWindow = SDL_CreateWindow("luna", 1366, 768, 0);
	if (!Game::sdlWindow) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow failed! %s", SDL_GetError());
		Game::Cleanup();
		return false;
	}
	
	// Create GPU device
	Game::sdlGPUDevice = SDL_CreateGPUDevice(
		SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
		false,
		nullptr
	);
	if (!Game::sdlGPUDevice) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateGPUDevice failed!");
		Game::Cleanup();
		return false;
	}
	if (!SDL_ClaimWindowForGPUDevice(Game::sdlGPUDevice, Game::sdlWindow)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_ClaimWindowForGPUDevice failed!");
		Game::Cleanup();
		return false;
	}

	// Set render parameters
	SDL_GPUPresentMode presentMode = SDL_GPU_PRESENTMODE_VSYNC;
	if (SDL_WindowSupportsGPUPresentMode(
		Game::sdlGPUDevice,
		Game::sdlWindow,
		SDL_GPU_PRESENTMODE_IMMEDIATE
	)) {
		presentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
	}
	else if (SDL_WindowSupportsGPUPresentMode(
		Game::sdlGPUDevice,
		Game::sdlWindow,
		SDL_GPU_PRESENTMODE_MAILBOX
	)) {
		presentMode = SDL_GPU_PRESENTMODE_MAILBOX;
	}
	SDL_SetGPUSwapchainParameters(
		Game::sdlGPUDevice,
		Game::sdlWindow,
		SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
		presentMode
	);

	// Create shader pipelines

	return true;
}

int Game::Run() {
	while (!quitFlag) {
		// Check events
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_EVENT_QUIT) {
				quitFlag = true;
			}
		}
		if (quitFlag) { break; }

		// Update game state

		// Draw
	}
	Game::Cleanup();
	return 0;
}

void Game::Quit() {
	quitFlag = true;
}

void Game::Cleanup() {
	if (Game::sdlGPUDevice && Game::sdlWindow) { SDL_ReleaseWindowFromGPUDevice(Game::sdlGPUDevice, Game::sdlWindow); }
	if (Game::sdlWindow) { SDL_DestroyWindow(Game::sdlWindow); }
	if (Game::sdlGPUDevice) { SDL_DestroyGPUDevice(Game::sdlGPUDevice); }
}