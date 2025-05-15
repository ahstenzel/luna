#pragma once

#include "common.hpp"
#include "resources.hpp"
#include "render.hpp"

namespace luna {

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
	static Renderer* m_renderer;
	
};

} // luna