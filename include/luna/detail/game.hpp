#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/resources.hpp>
#include <luna/detail/render.hpp>

namespace luna {

struct GameInit {
	unsigned int ticksPerSecond = 120;
	unsigned int windowW = 1366;
	unsigned int windowH = 768;
	bool enableGraphicsDebugging = false;
	std::string windowTitle = "luna";
	std::string appName = "luna";
	std::string appVersion = "1.0.0";
	std::string appIdentifier = "com.luna";
	std::function<void()> startFunc = []() {};
	std::function<void()> endFunc =   []() {};
	std::function<void(float)> preTickFunc =  [](float) {};
	std::function<void(float)> postTickFunc = [](float) {};
	std::function<void(Renderer*)> preDrawFunc =  [](Renderer*) {};
	std::function<void(Renderer*)> postDrawFunc = [](Renderer*) {};
	detail::AbstractRendererFactory* rendererFactory = {};
};

class Game {
public:
	LUNA_API Game() = delete;
	LUNA_API ~Game() = delete;

	LUNA_API static bool Init(GameInit* init = nullptr);
	LUNA_API static int Run();
	LUNA_API static void Quit();
	LUNA_API static SDL_Window* GetWindow();
	LUNA_API static unsigned int GetWindowWidth();
	LUNA_API static unsigned int GetWindowHeight();
	LUNA_API static SDL_GPUDevice* GetGPUDevice();
	LUNA_API static Renderer* GetRenderer();
	LUNA_API static bool GetGraphicsDebuggingEnabled();

private:
	static void Cleanup();

	static std::function<void()> m_startFunc;
	static std::function<void()> m_endFunc;
	static std::function<void(float)> m_preTickFunc;
	static std::function<void(float)> m_postTickFunc;
	static std::function<void(Renderer*)> m_preDrawFunc;
	static std::function<void(Renderer*)> m_postDrawFunc;

	static bool m_enableGraphicsDebugging;
	static bool m_quitFlag;
	static unsigned int m_windowW;
	static unsigned int m_windowH;
	static unsigned int m_ticksPerSecond;
	static SDL_Window* m_sdlWindow;
	static SDL_GPUDevice* m_sdlGPUDevice;
	static Renderer* m_renderer;
};

} // luna