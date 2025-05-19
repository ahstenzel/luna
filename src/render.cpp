#include <luna/detail/render.hpp>

namespace luna {

SpriteRenderer::SpriteRenderer(SDL_GPUDevice* device, SDL_Window* window) {
	// Build shader pipeline
	m_pipeline = new SpriteBatchShaderPipeline(device, window);

	// Create GPU resources
	SDL_GPUSamplerCreateInfo sdlGPUSamplerCreateInfo = {};
	sdlGPUSamplerCreateInfo.min_filter = SDL_GPU_FILTER_NEAREST;
	sdlGPUSamplerCreateInfo.mag_filter = SDL_GPU_FILTER_NEAREST;
	sdlGPUSamplerCreateInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	sdlGPUSamplerCreateInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sdlGPUSamplerCreateInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sdlGPUSamplerCreateInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	m_sdlGPUSampler = SDL_CreateGPUSampler(device, &sdlGPUSamplerCreateInfo);
	//SDL_GPUTextureCreateInfo sdlGPUTextureCreateInfo = {};
	//sdlGPUTextureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
	//sdlGPUTextureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	//sdlGPUTextureCreateInfo.width = 

	SDL_GPUTransferBufferCreateInfo sdlGPUTransferBufferCreateInfo = {};
	sdlGPUTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	sdlGPUTransferBufferCreateInfo.size = m_maxSpriteCount * sizeof(SpriteBatchInfo);
	m_sdlSpriteDataTransferBuffer = SDL_CreateGPUTransferBuffer(device, &sdlGPUTransferBufferCreateInfo);

	SDL_GPUBufferCreateInfo sdlGPUBufferCreateInfo = {};
	sdlGPUBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	sdlGPUBufferCreateInfo.size = m_maxSpriteCount * sizeof(SpriteBatchInfo);
	m_sdlSpriteDataBuffer = SDL_CreateGPUBuffer(device, &sdlGPUBufferCreateInfo);
}

SpriteRenderer::~SpriteRenderer() {
	delete m_pipeline;
}

bool SpriteRenderer::IsValid() const {
	return (m_pipeline && m_pipeline->IsValid());
}

void SpriteRenderer::RenderSpriteList(const SpriteList& spriteList) {
	// Get camera
	
}

void SpriteRenderer::SetTextureAtlas(const ResourceTexture* texture) {

}

} // luna