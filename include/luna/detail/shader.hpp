#pragma once

#include <luna/detail/common.hpp>
#include <SDL3_shadercross/SDL_shadercross.h>

namespace luna {

class ShaderPipeline {
public:
	LUNA_API virtual bool IsValid() const = 0;
	LUNA_API virtual void Clear() = 0;

	LUNA_API virtual SDL_GPUGraphicsPipeline* GetPipeline() const = 0;

protected:
	SDL_GPUGraphicsPipeline* m_pipeline = nullptr;
};

class SpriteBatchShaderPipeline : public ShaderPipeline {
public:
	LUNA_API SpriteBatchShaderPipeline();
	LUNA_API ~SpriteBatchShaderPipeline();

	LUNA_API SDL_GPUGraphicsPipeline* GetPipeline() const override;

	LUNA_API void Clear();
	LUNA_API bool IsValid() const override;

private:
	SDL_GPUShader* m_fragShader = nullptr;
	SDL_GPUShader* m_vertShader = nullptr;
};

class PrimitiveBatchShaderPipeline : public ShaderPipeline {
public:
	LUNA_API PrimitiveBatchShaderPipeline(bool wireframe = false);
	LUNA_API ~PrimitiveBatchShaderPipeline();

	LUNA_API SDL_GPUGraphicsPipeline* GetPipeline() const override;

	LUNA_API void Clear();
	LUNA_API bool IsValid() const override;

private:
	SDL_GPUShader* m_fragShader = nullptr;
	SDL_GPUShader* m_vertShader = nullptr;
};

} // luna