#include <luna/detail/render.hpp>
#include <luna/detail/game.hpp>
#include <luna/detail/room.hpp>

namespace luna {

SpriteRenderer::SpriteRenderer() {
	// Build shader pipeline
	m_pipeline = new SpriteBatchShaderPipeline();
}

SpriteRenderer::~SpriteRenderer() {
	SDL_GPUDevice* device = Game::GetGPUDevice();
	delete m_pipeline;
	SDL_ReleaseGPUSampler(device, m_sdlGPUSampler);
	SDL_ReleaseGPUTexture(device, m_sdlGPUAtlasTexture);
	SDL_ReleaseGPUTexture(device, m_sdlGPUDepthTexture);
	SDL_ReleaseGPUBuffer(device, m_sdlSpriteDataBuffer);
	SDL_ReleaseGPUTransferBuffer(device, m_sdlSpriteDataTransferBuffer);
	SDL_ReleaseGPUBuffer(device, m_sdlSpriteDataBuffer);
	SDL_ReleaseGPUTransferBuffer(device, m_sdlTextureTransferBuffer);
}

bool SpriteRenderer::IsValid() const {
	return (m_pipeline && m_pipeline->IsValid());
}

void SpriteRenderer::Draw() {
	Room* currentRoom = RoomManager::CurrentRoom();
	if (currentRoom) { RenderSpriteList(Game::GetWindow(), currentRoom->GetSpriteList()); }
}

void SpriteRenderer::RenderSpriteList(SDL_Window* window, SpriteList* spriteList) {
	// Get camera
	glm::mat4 cameraMatrix = CameraProjectionOrtho(0.0f, 1366.0f, 768.0f, 0.0f, -100.0f, 100.0f);

	// Get GPU resources
	SDL_GPUDevice* device = Game::GetGPUDevice();
	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
	if (!commandBuffer) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_AcquireGPUCommandBuffer failed! %s", SDL_GetError());
		return;
	}
	SDL_GPUTexture* swapchainTexture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, nullptr, nullptr)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_WaitAndAcquireGPUSwapchainTexture failed! %s", SDL_GetError());
		return;
	}
	if (swapchainTexture) {
		// Partition list into texture page batches
		std::size_t spriteBegin = 0;
		std::size_t spriteCount = 0;
		//spriteList->Sort();

		for (std::size_t i = 0; i < spriteList->Size(); ++i) {
			SpriteID spriteID = spriteList->GetSpriteID(i);
			Sprite* sprite = spriteList->GetSprite(spriteID);
			TexturePageID texturePageID = sprite->GetTexturePageID();
			if (texturePageID != m_lastTexturePageID) {
				m_lastTexturePageID = texturePageID;

				// Render current batch
				if (spriteCount > 0) {
					RenderSpriteListBatch(commandBuffer, swapchainTexture, &cameraMatrix, -100.0f, 100.0f, spriteList, spriteBegin, spriteCount);
				}

				// Reset count
				spriteBegin = i;
				spriteCount = 1;

				// Swap to new texture page
				const TexturePage* texturePage = sprite->GetTexturePage();
				SetTexturePage(commandBuffer, texturePage);
			}
			else { spriteCount++; }
		}
		// Render final batch
		if (spriteCount > 0) {
			RenderSpriteListBatch(commandBuffer, swapchainTexture, &cameraMatrix, -100.0f, 100.0f, spriteList, spriteBegin, spriteCount);
		}
	}
	SDL_SubmitGPUCommandBuffer(commandBuffer);
}

void SpriteRenderer::RenderSpriteListBatch(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* swapchainTexture, glm::mat4* cameraMatrix, float zNear, float zFar, SpriteList* spriteList, std::size_t spriteBegin, std::size_t spriteCount) {
	if (spriteCount == 0) { return; }

	// Resize transfer buffer if needed
	SDL_GPUDevice* device = Game::GetGPUDevice();
	if (spriteCount != m_lastSpriteBatchSize) {
		m_lastSpriteBatchSize = spriteCount;
		SDL_ReleaseGPUTransferBuffer(device, m_sdlSpriteDataTransferBuffer);
		SDL_ReleaseGPUBuffer(device, m_sdlSpriteDataBuffer);

		SDL_GPUTransferBufferCreateInfo spriteDataTransferBufferCreateInfo = {};
		spriteDataTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		spriteDataTransferBufferCreateInfo.size = spriteCount * sizeof(SpriteBatchInfo);
		m_sdlSpriteDataTransferBuffer = SDL_CreateGPUTransferBuffer(device, &spriteDataTransferBufferCreateInfo);

		SDL_GPUBufferCreateInfo spriteDataBufferCreateInfo = {};
		spriteDataBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
		spriteDataBufferCreateInfo.size = spriteCount * sizeof(SpriteBatchInfo);
		m_sdlSpriteDataBuffer = SDL_CreateGPUBuffer(device, &spriteDataBufferCreateInfo);
	}

	// Initialize depth texture
	if (!m_sdlGPUDepthTexture) {
		SDL_GPUTextureCreateInfo depthTextureCreateInfo = {};
		depthTextureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
		depthTextureCreateInfo.format = m_pipeline->GetDepthStencilFormat();
		depthTextureCreateInfo.width = 1366;
		depthTextureCreateInfo.height = 768;
		depthTextureCreateInfo.layer_count_or_depth = 1;
		depthTextureCreateInfo.num_levels = 1;
		depthTextureCreateInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
		depthTextureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
		m_sdlGPUDepthTexture = SDL_CreateGPUTexture(device, &depthTextureCreateInfo);
	}

	// Build sprite instance buffer
	SpriteBatchInfo* dataPtr = (SpriteBatchInfo*)SDL_MapGPUTransferBuffer(device, m_sdlSpriteDataTransferBuffer, true);
	if (!dataPtr) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to map GPU transfer buffer: %s", SDL_GetError());
		return;
	}
	for(std::size_t i = 0; i < spriteCount; ++i) {
		SpriteID spriteID = spriteList->GetSpriteID(spriteBegin + i);
		Sprite* sprite = spriteList->GetSprite(spriteID);
		SpriteTextureCoords spriteTextureCoords = sprite->GetTextureCoords();
		SDL_Color spriteColor = sprite->GetBlend();
		dataPtr[i].x = sprite->GetPositionX();
		dataPtr[i].y = sprite->GetPositionY();
		dataPtr[i].z = -float(sprite->GetDepth());
		dataPtr[i].rotation = sprite->GetRotation();
		dataPtr[i].w = sprite->GetWidth() * sprite->GetScaleX();
		dataPtr[i].h = sprite->GetHeight() * sprite->GetScaleY();
		dataPtr[i].texU = spriteTextureCoords.textureU;
		dataPtr[i].texV = spriteTextureCoords.textureV;
		dataPtr[i].texW = spriteTextureCoords.textureW;
		dataPtr[i].texH = spriteTextureCoords.textureH;
		dataPtr[i].r = spriteColor.r / 255.f;
		dataPtr[i].g = spriteColor.g / 255.f;
		dataPtr[i].b = spriteColor.b / 255.f;
		dataPtr[i].a = spriteColor.a / 255.f;
	}
	SDL_UnmapGPUTransferBuffer(device, m_sdlSpriteDataTransferBuffer);

	// Upload instance data
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
	SDL_GPUTransferBufferLocation spriteDataTransferBufferLocation = {};
	spriteDataTransferBufferLocation.transfer_buffer = m_sdlSpriteDataTransferBuffer;
	spriteDataTransferBufferLocation.offset = 0;
	SDL_GPUBufferRegion spriteDataBufferRegion = {};
	spriteDataBufferRegion.buffer = m_sdlSpriteDataBuffer;
	spriteDataBufferRegion.offset = 0;
	spriteDataBufferRegion.size = spriteCount * sizeof(SpriteBatchInfo);
	SDL_UploadToGPUBuffer(copyPass, &spriteDataTransferBufferLocation, &spriteDataBufferRegion, true);
	SDL_EndGPUCopyPass(copyPass);

	// Render sprites
	SDL_GPUColorTargetInfo renderColorTargetInfo = {};
	renderColorTargetInfo.texture = swapchainTexture;
	renderColorTargetInfo.cycle = false;
	renderColorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	renderColorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	renderColorTargetInfo.clear_color = { 0.f, 0.f, 0.2f, 1.f };
	SDL_GPUDepthStencilTargetInfo renderDepthStencilTargetInfo = {};
	renderDepthStencilTargetInfo.texture = m_sdlGPUDepthTexture;
	renderDepthStencilTargetInfo.cycle = true;
	renderDepthStencilTargetInfo.clear_depth = zFar;
	renderDepthStencilTargetInfo.clear_stencil = 0;
	renderDepthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	renderDepthStencilTargetInfo.store_op = SDL_GPU_STOREOP_DONT_CARE;
	renderDepthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
	renderDepthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &renderColorTargetInfo, 1, &renderDepthStencilTargetInfo);
	SDL_BindGPUGraphicsPipeline(renderPass, m_pipeline->GetPipeline());
	SDL_BindGPUVertexStorageBuffers(renderPass, 0, &m_sdlSpriteDataBuffer, 1);
	SDL_GPUTextureSamplerBinding renderTextureSamplerBinding = {};
	renderTextureSamplerBinding.texture = m_sdlGPUAtlasTexture;
	renderTextureSamplerBinding.sampler = m_sdlGPUSampler;
	SDL_BindGPUFragmentSamplers(renderPass, 0, &renderTextureSamplerBinding, 1);
	SDL_PushGPUVertexUniformData(commandBuffer, 0, &(cameraMatrix[0][0]), sizeof(glm::mat4));
	//float depthPlanes[2] = { 100.0f, -100.0f };
	//SDL_PushGPUFragmentUniformData(commandBuffer, 0, &depthPlanes[0], sizeof(float) * 2);
	SDL_DrawGPUPrimitives(renderPass, spriteCount * 6, 1, 0, 0);
	SDL_EndGPURenderPass(renderPass);
}

void SpriteRenderer::SetTexturePage(SDL_GPUCommandBuffer* commandBuffer, const TexturePage* texturePage) {
	SDL_GPUDevice* device = Game::GetGPUDevice();

	// Create GPU texture
	if (m_sdlGPUAtlasTexture) { SDL_ReleaseGPUTexture(device, m_sdlGPUAtlasTexture); }
	SDL_GPUTextureCreateInfo atlasTextureCreateInfo = {};
	atlasTextureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
	atlasTextureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
	atlasTextureCreateInfo.width = texturePage->GetWidth();
	atlasTextureCreateInfo.height = texturePage->GetHeight();
	atlasTextureCreateInfo.layer_count_or_depth = 1;
	atlasTextureCreateInfo.num_levels = 1;
	atlasTextureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	m_sdlGPUAtlasTexture = SDL_CreateGPUTexture(device, &atlasTextureCreateInfo);

	// Create GPU sampler
	if (m_sdlGPUSampler) { SDL_ReleaseGPUSampler(device, m_sdlGPUSampler); }
	SDL_GPUSamplerCreateInfo atlasSamplerCreateInfo = {};
	atlasSamplerCreateInfo.min_filter = SDL_GPU_FILTER_NEAREST;
	atlasSamplerCreateInfo.mag_filter = SDL_GPU_FILTER_NEAREST;
	atlasSamplerCreateInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	atlasSamplerCreateInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	atlasSamplerCreateInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	atlasSamplerCreateInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	m_sdlGPUSampler = SDL_CreateGPUSampler(device, &atlasSamplerCreateInfo);

	// Upload image data to transfer buffer
	if (m_sdlTextureTransferBuffer) { SDL_ReleaseGPUTransferBuffer(device, m_sdlTextureTransferBuffer); }
	std::uint32_t bufferSize = texturePage->GetWidth() * texturePage->GetHeight() * 4u;
	SDL_GPUTransferBufferCreateInfo textureTransferBufferCreateInfo = {};
	textureTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	textureTransferBufferCreateInfo.size = bufferSize;
	m_sdlTextureTransferBuffer = SDL_CreateGPUTransferBuffer(device, &textureTransferBufferCreateInfo);
	Uint8* textureTransferPtr = (Uint8*)SDL_MapGPUTransferBuffer(device, m_sdlTextureTransferBuffer, false);
	std::uint8_t* data = texturePage->GetData();
	SDL_memcpy(textureTransferPtr, texturePage->GetData(), bufferSize);
	SDL_UnmapGPUTransferBuffer(device, m_sdlTextureTransferBuffer);

	// Use transfer buffer to copy data to texture
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
	SDL_GPUTextureTransferInfo textureTransferInfo = {};
	textureTransferInfo.transfer_buffer = m_sdlTextureTransferBuffer;
	textureTransferInfo.offset = 0;
	SDL_GPUTextureRegion textureRegion = {};
	textureRegion.texture = m_sdlGPUAtlasTexture;
	textureRegion.w = texturePage->GetWidth();
	textureRegion.h = texturePage->GetHeight();
	textureRegion.d = 1;
	SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);
	SDL_EndGPUCopyPass(copyPass);
}

} // luna