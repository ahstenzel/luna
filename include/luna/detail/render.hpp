#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/shader.hpp>
#include <luna/detail/sprite.hpp>

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

	void RenderSpriteList(const SpriteList& spriteList);

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
	float m_uCoords[4] = { 0.0f, 0.5f, 0.0f, 0.5f };
	float m_vCoords[4] = { 0.0f, 0.0f, 0.5f, 0.5f };

	void SetTextureAtlas(const ResourceTexture* texture);

	SDL_GPUSampler* m_sdlGPUSampler = nullptr;
	SDL_GPUTexture* m_sdlGPUTexture = nullptr;
	SDL_GPUTransferBuffer* m_sdlSpriteDataTransferBuffer = nullptr;
	SDL_GPUBuffer* m_sdlSpriteDataBuffer = nullptr;
	SpriteBatchShaderPipeline* m_pipeline = nullptr;
};

} // luna