#include <luna/detail/game.hpp>
#include <luna/detail/room.hpp>
#include <SDL3_shadercross/SDL_shadercross.h>

namespace luna {

std::function<void()> Game::m_startFunc = {};
std::function<void()> Game::m_endFunc = {};
std::function<void(float)> Game::m_preTickFunc = {};
std::function<void(float)> Game::m_postTickFunc = {};
std::function<void(Renderer*)> Game::m_preDrawFunc = {};
std::function<void(Renderer*)> Game::m_postDrawFunc = {};

bool Game::m_quitFlag = false;
unsigned int Game::m_ticksPerSecond = 0;
SDL_Window* Game::m_sdlWindow = nullptr;
SDL_GPUDevice* Game::m_sdlGPUDevice = nullptr;
Renderer* Game::m_renderer = nullptr;

bool Game::Init(GameInit* init) {
	// Save init values
	if (!init || !init->rendererFactory) { return false; }
	m_startFunc = init->startFunc;
	m_endFunc = init->endFunc;
	m_preTickFunc = init->preTickFunc;
	m_postTickFunc = init->postTickFunc;
	m_preDrawFunc = init->preDrawFunc;
	m_postDrawFunc = init->postDrawFunc;
	m_ticksPerSecond = init->ticksPerSecond;

	// Initialize SDL
	if (!SDL_SetAppMetadata(init->appName.c_str(), init->appVersion.c_str(), init->appIdentifier.c_str())) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_SetAppMetadata failed! %s", SDL_GetError());
		Cleanup();
		return false;
	}
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed! %s", SDL_GetError());
		Cleanup();
		return false;
	}
	SDL_srand(0);

	// Create window
	m_sdlWindow = SDL_CreateWindow(init->windowTitle.c_str(), int(init->windowW), int(init->windowH), 0);
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
	m_renderer = init->rendererFactory->generate();
	if (!m_renderer || !m_renderer->IsValid()) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Renderer creation failed!");
		Cleanup();
		return false;
	}

	return true;
}

int Game::Run() {
	double tickRate = (m_ticksPerSecond > 0) ? (1.0 / (double)m_ticksPerSecond) : 0.0;
	double timeAccum = 0.0;
	auto timePointLast = std::chrono::high_resolution_clock::now();
	m_startFunc();
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
		auto timePointCurrent = std::chrono::high_resolution_clock::now();
		auto timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(timePointCurrent - timePointLast);
		timePointLast = timePointCurrent;
		double frameTime = timeSpan.count();
		if (m_ticksPerSecond > 0) {
			timeAccum += frameTime;
			while (timeAccum >= tickRate) {
				m_preTickFunc(float(tickRate));
				RoomManager::Tick(float(tickRate));
				m_postTickFunc(float(tickRate));
				timeAccum -= tickRate;
			}
		}
		else {
			m_preTickFunc(float(frameTime));
			RoomManager::Tick(float(frameTime));
			m_postTickFunc(float(frameTime));
		}

		// Draw
		m_preDrawFunc(m_renderer);
		m_renderer->Draw();
		m_postDrawFunc(m_renderer);
	}
	m_endFunc();
	Cleanup();
	return 0;
}

void Game::Quit() {
	m_quitFlag = true;
}

SDL_Window* Game::GetWindow() {
	return m_sdlWindow;
}

SDL_GPUDevice* Game::GetGPUDevice() {
	return m_sdlGPUDevice;
}

Renderer* Game::GetRenderer() {
	return m_renderer;
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