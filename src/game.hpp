#pragma once

#include "common.hpp"
#include "resources.hpp"

class Game {
public:
	Game() = delete;
	~Game() = delete;

	static bool Init();
	static int Run();
	static void Quit();

private:
	static void Cleanup();

	static bool m_quitFlag;
	static SDL_Window* m_sdlWindow;
	static SDL_GPUDevice* m_sdlGPUDevice;
	static SDL_GPUSampler* m_sdlGPUSampler;
	static SDL_GPUTexture* m_sdlGPUTexture;
	static SDL_GPUGraphicsPipeline* m_sdlSpriteRenderPipeline;
	static SDL_GPUTransferBuffer* m_sdlSpriteDataTransferBuffer;
	static SDL_GPUBuffer* m_sdlSpriteDataBuffer;
};