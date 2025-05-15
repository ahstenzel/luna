#pragma once

#include "common.hpp"
#include "SDL3_shadercross/SDL_shadercross.h"

namespace luna {

class ShaderPipeline {
public:
	virtual bool IsValid() const = 0;
	virtual void Clear() = 0;

	virtual const SDL_GPUGraphicsPipeline* GetPipeline() const = 0;

protected:
	SDL_GPUGraphicsPipeline* m_pipeline = nullptr;
	SDL_GPUDevice* m_device = nullptr;
};

class SpriteBatchShaderPipeline : public ShaderPipeline {
public:
	SpriteBatchShaderPipeline(SDL_GPUDevice* device, SDL_Window* window);
	~SpriteBatchShaderPipeline();

	const SDL_GPUGraphicsPipeline* GetPipeline() const override;

	void Clear();
	bool IsValid() const override;

private:
	SDL_GPUShader* m_fragShader = nullptr;
	SDL_GPUShader* m_vertShader = nullptr;
};

} // luna