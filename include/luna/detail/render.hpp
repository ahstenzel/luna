#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/sprite.hpp>
#include <luna/detail/shapes.hpp>

namespace luna {

// Forward declarations
class Game;
class Camera;
class SpriteBatchShaderPipeline;
class PrimitiveBatchShaderPipeline;

/// <summary>
/// Abstract rendering base class.
/// </summary>
class Renderer {
public:
	LUNA_API virtual bool IsValid() const = 0;
	LUNA_API virtual void DrawSprite(Sprite sprite) = 0;
	LUNA_API virtual void DrawPrimitive(Primitive primitive) = 0;

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
	LUNA_API void DrawSprite(Sprite sprite) override;
	LUNA_API void DrawPrimitive(Primitive primitive) override;

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
	enum RenderableType {
		Unknown,
		SpriteType,
		PrimitiveType
	};

	/// <summary>
	/// Generic wrapper for any object that can be drawn onto the screen.
	/// Includes sprites, text, primitives, etc.
	/// </summary>
	struct Renderable {
		SpriteRenderer* m_renderer;
		std::size_t m_renderableIndex;
		RenderableType m_renderableType;
		bool m_opaque;

		Renderable();
		Renderable(SpriteRenderer* renderer, std::size_t renderableIndex, RenderableType renderableType, bool opaque);
		Renderable(const Renderable& renderable);
		Renderable(Renderable&& renderable) noexcept;

		bool IsValid() const;
		Sprite* GetSprite() const;
		Primitive* GetPrimitive() const;
		
		Renderable& operator=(const Renderable& other);
		Renderable& operator=(Renderable&& other) noexcept;
		bool operator==(const Renderable& other) const;
	};

	/// <summary>
	/// List of drawable objects.
	/// </summary>
	using RenderableList = std::vector<Renderable>;

	/// <summary>
	/// Container for a list of similar drawable objects, including metadata about them.
	/// All objects in a batch can be drawn in the same render pass, without changing GPU state.
	/// </summary>
	struct RenderableBatch {
		RenderableType m_renderableType;
		RenderableList m_renderableList;
		bool m_opaque;

		RenderableBatch();
		RenderableBatch(RenderableType type, const RenderableList& renderableList, bool opaque);
		RenderableBatch(const RenderableBatch& other);
		RenderableBatch(RenderableBatch&& other) noexcept;
		bool MatchRenderable(const Renderable& renderable) const;
		void AddRenderable(const Renderable& renderable);
		RenderableBatch& operator=(const RenderableBatch& other);
		RenderableBatch& operator=(RenderableBatch&& other) noexcept;
		bool operator==(const RenderableBatch& other) const;
	};

	struct CompRenderableDepth {
		bool operator()(const Renderable& lhs, const Renderable& rhs);
	};

	struct CompRenderableType {
		bool operator()(const Renderable& lhs, const Renderable& rhs);
	};

	struct CompRenderableSpriteTexturePage {
		bool operator()(const Renderable& lhs, const Renderable& rhs);
	};

	struct CompRenderablePrimitiveShapeType {
		bool operator()(const Renderable& lhs, const Renderable& rhs);
	};
	
	void RenderSpriteListBatch(SDL_GPUCommandBuffer* commandBuffer, glm::mat4* cameraMatrix, const RenderableList& sprites);
	void SetTexturePage(SDL_GPUCommandBuffer* commandBuffer, const TexturePage* texturePage);
	void RenderPrimitiveListBatch(SDL_GPUCommandBuffer* commandBuffer, glm::mat4* cameraMatrix, const RenderableList& primitives);

	SpriteList m_sprites;
	PrimitiveList m_primitives;
	RenderableList m_opaqueRenderables;
	RenderableList m_translucentRenderables;

	std::size_t m_lastSpriteBatchCount = 0;
	std::size_t m_lastPrimitiveVertexCount = 0;
	std::size_t m_lastPrimitiveIndexCount = 0;
	TexturePageID m_currentTexturePageID = TEXTURE_PAGE_ID_NULL;

	SpriteBatchShaderPipeline* m_spriteBatchPipeline = nullptr;
	SDL_GPUTransferBuffer* m_sdlSpriteDataTransferBuffer = nullptr;
	SDL_GPUBuffer* m_sdlSpriteDataBuffer = nullptr;

	PrimitiveBatchShaderPipeline* m_primitiveBatchPipeline = nullptr;
	SDL_GPUTransferBuffer* m_sdlPrimitiveDataTransferBuffer = nullptr;
	SDL_GPUBuffer* m_sdlPrimitiveDataVertexBuffer = nullptr;
	SDL_GPUBuffer* m_sdlPrimitiveDataIndexBuffer = nullptr;

	bool m_clearTarget = false;
	SDL_GPUColorTargetInfo m_sdlRenderColorTargetInfo = {};
	SDL_GPUDepthStencilTargetInfo m_sdlRenderDepthStencilTargetInfo = {};
	SDL_GPUSampler* m_sdlGPUSampler = nullptr;
	SDL_GPUTexture* m_sdlGPUAtlasTexture = nullptr;
	SDL_GPUTexture* m_sdlGPUDepthTexture = nullptr;
	SDL_GPUTransferBuffer* m_sdlTextureTransferBuffer = nullptr;
};

namespace detail {

/// <summary>
/// Abstract base class for renderer factories.
/// </summary>
class AbstractRendererFactory {
public:
	LUNA_API virtual Renderer* generate() = 0;
};

// TODO find a class to tuck this into?
LUNA_API SDL_GPUTextureFormat GetDepthStencilFormat(SDL_GPUDevice* device);

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