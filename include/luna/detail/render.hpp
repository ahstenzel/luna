#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/shader.hpp>
#include <luna/detail/sprite.hpp>
#include <luna/detail/camera.hpp>

namespace luna {

class Renderer {
public:
	virtual bool IsValid() const = 0;

protected:
	SDL_GPUDevice* m_device = nullptr;
};

class SpriteRenderer : public Renderer {
public:
	SpriteRenderer(SDL_GPUDevice* device, SDL_Window* window);
	~SpriteRenderer();

	bool IsValid() const override;

	void RenderSpriteList(SDL_Window* window, SpriteList* spriteList);

protected:
	struct SpriteBatchInfo {
		float x, y, z;
		float rotation;
		float w, h, _paddingA, _paddingB;
		float texU, texV, texW, texH;
		float r, g, b, a;
	};

	const std::uint32_t m_maxSpriteCount = 8192;

private:
	void RenderSpriteListBatch(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* swapchainTexture, glm::mat4x4* cameraMatrix, SpriteList* spriteList, std::size_t spriteBegin, std::size_t spriteCount);
	void SetTexturePage(const TexturePage* texturePage);

	std::uint32_t m_lastSpriteBatchSize = 0;
	SDL_GPUSampler* m_sdlGPUSampler = nullptr;
	SDL_GPUTexture* m_sdlGPUTexture = nullptr;
	SDL_GPUTransferBuffer* m_sdlSpriteDataTransferBuffer = nullptr;
	SDL_GPUBuffer* m_sdlSpriteDataBuffer = nullptr;
	SpriteBatchShaderPipeline* m_pipeline = nullptr;
};

} // luna