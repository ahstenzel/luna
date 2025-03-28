#pragma once

#include "common.hpp"

class Game {
public:
	Game() = delete;
	~Game() = delete;

	static bool Init();
	static int Run();
	static void Quit();

private:
	static void Cleanup();

	static bool quitFlag;
	static SDL_Window* sdlWindow;
	static SDL_GPUDevice* sdlGPUDevice;
	static SDL_GPUSampler* sdlGPUSampler;
	static SDL_GPUTexture* sdlGPUTexture;
	static SDL_GPUGraphicsPipeline* sdlSpriteRenderPipeline;
	static SDL_GPUTransferBuffer* sdlSpriteDataTransferBuffer;
	static SDL_GPUBuffer* sdlSpriteDataBuffer;
};