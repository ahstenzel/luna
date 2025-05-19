#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/shader.hpp>

namespace luna {

class Renderer {
public:
	virtual bool IsValid() const = 0;
};

class SpriteRenderer : public Renderer {
public:
	SpriteRenderer(SDL_GPUDevice* device, SDL_Window* window);
	~SpriteRenderer();

	bool IsValid() const override;

private:
	SDL_GPUSampler* m_sdlGPUSampler = nullptr;
	SDL_GPUTexture* m_sdlGPUTexture = nullptr;
	SDL_GPUGraphicsPipeline* m_sdlSpriteRenderPipeline = nullptr;
	SDL_GPUTransferBuffer* m_sdlSpriteDataTransferBuffer = nullptr;
	SDL_GPUBuffer* m_sdlSpriteDataBuffer = nullptr;
	SpriteBatchShaderPipeline* m_pipeline = nullptr;
};

} // luna