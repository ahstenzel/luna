#include <luna/detail/game.hpp>
#include <SDL3_shadercross/SDL_shadercross.h>

namespace luna {

bool Game::m_quitFlag = false;
SDL_Window* Game::m_sdlWindow = nullptr;
SDL_GPUDevice* Game::m_sdlGPUDevice = nullptr;
Renderer* Game::m_renderer = nullptr;

bool Game::Init() {
	// Initialize SDL
	if (!SDL_SetAppMetadata("luna", "1.0.0", "com.luna")) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_SetAppMetadata failed! %s", SDL_GetError());
		Cleanup();
		return false;
	}
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed! %s", SDL_GetError());
		Cleanup();
		return false;
	}

	// Create window
	m_sdlWindow = SDL_CreateWindow("luna", 1366, 768, 0);
	if (!m_sdlWindow) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow failed! %s", SDL_GetError());
		Cleanup();
		return false;
	}

	// Create GPU device
	m_sdlGPUDevice = SDL_CreateGPUDevice(
		SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
		false,
		nullptr
	);
	if (!m_sdlGPUDevice) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateGPUDevice failed!");
		Cleanup();
		return false;
	}
	if (!SDL_ClaimWindowForGPUDevice(m_sdlGPUDevice, m_sdlWindow)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_ClaimWindowForGPUDevice failed!");
		Cleanup();
		return false;
	}

	// Set render parameters
	SDL_GPUPresentMode presentMode = SDL_GPU_PRESENTMODE_VSYNC;
	if (SDL_WindowSupportsGPUPresentMode(
		m_sdlGPUDevice,
		m_sdlWindow,
		SDL_GPU_PRESENTMODE_IMMEDIATE
	)) {
		presentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
	}
	else if (SDL_WindowSupportsGPUPresentMode(
		m_sdlGPUDevice,
		m_sdlWindow,
		SDL_GPU_PRESENTMODE_MAILBOX
	)) {
		presentMode = SDL_GPU_PRESENTMODE_MAILBOX;
	}
	SDL_SetGPUSwapchainParameters(
		m_sdlGPUDevice,
		m_sdlWindow,
		SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
		presentMode
	);

	// Create shader pipelines
	SDL_ShaderCross_Init();
	m_renderer = new SpriteRenderer(m_sdlGPUDevice, m_sdlWindow);
	if (!m_renderer || !m_renderer->IsValid()) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SpriteRenderer creation failed!");
		Cleanup();
		return false;
	}

	return true;
}

int Game::Run() {
	while (!m_quitFlag) {
		// Check events
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_EVENT_QUIT) {
				m_quitFlag = true;
			}
		}
		if (m_quitFlag) { break; }

		// Update game state

		// Draw
	}
	Cleanup();
	return 0;
}

void Game::Quit() {
	m_quitFlag = true;
}

void Game::Cleanup() {
	SDL_ShaderCross_Quit();
	if (m_renderer) { delete m_renderer; }
	if (m_sdlGPUDevice && m_sdlWindow) { SDL_ReleaseWindowFromGPUDevice(m_sdlGPUDevice, m_sdlWindow); }
	if (m_sdlWindow) { SDL_DestroyWindow(m_sdlWindow); }
	if (m_sdlGPUDevice) { SDL_DestroyGPUDevice(m_sdlGPUDevice); }
	m_renderer = nullptr;
	m_sdlGPUDevice = nullptr;
	m_sdlWindow = nullptr;
}

} // luna