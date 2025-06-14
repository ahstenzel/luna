#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/sprite.hpp>

namespace luna {

// Forward declarations
class Game;
class Camera;
class SpriteBatchShaderPipeline;

/// <summary>
/// Abstract rendering base class.
/// </summary>
class Renderer {
public:
	LUNA_API virtual bool IsValid() const = 0;
	LUNA_API virtual void DrawSprite(Sprite* sprite) = 0;

protected:
	friend class Game;
	virtual void PreDraw() = 0;
	virtual void Draw() = 0;
	virtual void PostDraw() = 0;
};

/// <summary>
/// 2D Sprite batching renderer.
/// </summary>
class SpriteRenderer : public Renderer {
public:
	LUNA_API SpriteRenderer();
	LUNA_API ~SpriteRenderer();

	LUNA_API bool IsValid() const override;
	LUNA_API void DrawSprite(Sprite* sprite);

protected:
	friend class Game;
	void PreDraw() override;
	void Draw() override;
	void PostDraw() override;

	struct SpriteBatchInfo {
		float x, y, z, rotation;
		float w, h, _paddingA, _paddingB;
		float scaleX, scaleY, originX, originY;
		float texU, texV, texW, texH;
		float r, g, b, a;
	};

private:
	void RenderSpriteList(SDL_Window* window, SpriteList* spriteList, SDL_FColor clearColor);
	void RenderSpriteListBatch(SDL_GPUCommandBuffer* commandBuffer, SDL_FColor clearColor, SDL_GPUTexture* swapchainTexture, glm::mat4* cameraMatrix, float zNear, float zFar, SpriteList* spriteList, std::size_t spriteBegin, std::size_t spriteCount);
	void SetTexturePage(SDL_GPUCommandBuffer* commandBuffer, const TexturePage* texturePage);

	SpriteList m_opaqueSpriteList;
	SpriteList m_translucentSpriteList;

	std::uint32_t m_lastSpriteBatchSize = 0;
	TexturePageID m_lastTexturePageID = TEXTURE_PAGE_ID_NULL;
	SDL_GPUSampler* m_sdlGPUSampler = nullptr;
	SDL_GPUTexture* m_sdlGPUAtlasTexture = nullptr;
	SDL_GPUTexture* m_sdlGPUDepthTexture = nullptr;
	SDL_GPUTransferBuffer* m_sdlSpriteDataTransferBuffer = nullptr;
	SDL_GPUBuffer* m_sdlSpriteDataBuffer = nullptr;
	SDL_GPUTransferBuffer* m_sdlTextureTransferBuffer = nullptr;
	SpriteBatchShaderPipeline* m_pipeline = nullptr;
};

namespace detail {

/// <summary>
/// Abstract base class for renderer factories.
/// </summary>
class AbstractRendererFactory {
public:
	LUNA_API virtual Renderer* generate() = 0;
};

} // detail

/// <summary>
/// Factory class template for creating a renderer.
/// </summary>
/// <typeparam name="T">Renderer subclass</typeparam>
template<typename T>
class RendererFactory : public detail::AbstractRendererFactory {
public:
	LUNA_API T* generate() override {
		static_assert(std::is_base_of_v<Renderer, T>);
		return new T();
	}
};

using SpriteRendererFactory = RendererFactory<SpriteRenderer>;

} // luna