#include <luna/detail/render.hpp>
#include <luna/detail/game.hpp>
#include <luna/detail/room.hpp>

namespace luna {

SpriteRenderer::SpriteRenderer() {
	// Build shader pipeline
	SDL_GPUDevice* device = Game::GetGPUDevice();
	SDL_Window* window = Game::GetWindow();
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
}

SpriteRenderer::~SpriteRenderer() {
	SDL_GPUDevice* device = Game::GetGPUDevice();
	delete m_pipeline;
	SDL_ReleaseGPUSampler(device, m_sdlGPUSampler);
	SDL_ReleaseGPUTexture(device, m_sdlGPUTexture);
	SDL_ReleaseGPUBuffer(device, m_sdlSpriteDataBuffer);
	SDL_ReleaseGPUTransferBuffer(device, m_sdlSpriteDataTransferBuffer);
	SDL_ReleaseGPUBuffer(device, m_sdlSpriteDataBuffer);
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
	glm::mat4x4 cameraMatrix = CameraProjectionOrtho(0.0f, 1366.0f, 768.0f, 0.0f, 0.0f, -1.0f);

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
		ResourceID lastTexturePageID = RESOURCE_ID_NULL;
		spriteList->Sort();
		for(std::size_t i = 0; i < spriteList->Size(); ++i) {
			SpriteID spriteID = spriteList->GetSpriteID(i);
			Sprite* sprite = spriteList->GetSprite(spriteID);
			ResourceID texturePageID = sprite->GetTextureID();
			if (texturePageID != lastTexturePageID) {
				// Render current batch
				if (spriteCount > 0) {
					RenderSpriteListBatch(commandBuffer, swapchainTexture, &cameraMatrix, spriteList, spriteBegin, spriteCount);
				}

				// Reset count
				spriteBegin = i;
				spriteCount = 1;

				// Swap to new texture page
				const TexturePage* texturePage = sprite->GetTexturePage();
				SetTexturePage(texturePage);
			}
			else { spriteCount++; }
		}
		// Render final batch
		if (spriteCount > 0) {
			RenderSpriteListBatch(commandBuffer, swapchainTexture, &cameraMatrix, spriteList, spriteBegin, spriteCount);
		}
	}
	SDL_SubmitGPUCommandBuffer(commandBuffer);
}

void SpriteRenderer::RenderSpriteListBatch(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* swapchainTexture, glm::mat4x4* cameraMatrix, SpriteList* spriteList, std::size_t spriteBegin, std::size_t spriteCount) {
	// Resize transfer buffer if needed
	SDL_GPUDevice* device = Game::GetGPUDevice();
	if (spriteCount != m_lastSpriteBatchSize) {
		m_lastSpriteBatchSize = spriteCount;
		SDL_ReleaseGPUTransferBuffer(device, m_sdlSpriteDataTransferBuffer);
		SDL_ReleaseGPUBuffer(device, m_sdlSpriteDataBuffer);

		SDL_GPUTransferBufferCreateInfo sdlGPUTransferBufferCreateInfo = {};
		sdlGPUTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		sdlGPUTransferBufferCreateInfo.size = spriteCount * sizeof(SpriteBatchInfo);
		m_sdlSpriteDataTransferBuffer = SDL_CreateGPUTransferBuffer(device, &sdlGPUTransferBufferCreateInfo);

		SDL_GPUBufferCreateInfo sdlGPUBufferCreateInfo = {};
		sdlGPUBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
		sdlGPUBufferCreateInfo.size = spriteCount * sizeof(SpriteBatchInfo);
		m_sdlSpriteDataBuffer = SDL_CreateGPUBuffer(device, &sdlGPUBufferCreateInfo);
	}

	// Build sprite instance buffer
	SpriteBatchInfo* dataPtr = (SpriteBatchInfo*)SDL_MapGPUTransferBuffer(device, m_sdlSpriteDataTransferBuffer, true);
	std::size_t i = 0;
	for(std::size_t i = 0; i < spriteCount; ++i) {
		SpriteID spriteID = spriteList->GetSpriteID(spriteBegin + i);
		Sprite* sprite = spriteList->GetSprite(spriteID);
		SpriteTextureCoords spriteTextureCoords = sprite->GetTextureCoords();
		SDL_Color spriteColor = sprite->GetBlend();
		dataPtr[i].x = sprite->GetPositionX();
		dataPtr[i].y = sprite->GetPositionY();
		dataPtr[i].z = 0.0f;
		dataPtr[i].rotation = sprite->GetRotation();
		dataPtr[i].w = sprite->GetWidth();
		dataPtr[i].h = sprite->GetHeight();
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
	SDL_GPUTransferBufferLocation sdlGPUTransferBufferLocation = {};
	sdlGPUTransferBufferLocation.transfer_buffer = m_sdlSpriteDataTransferBuffer;
	sdlGPUTransferBufferLocation.offset = 0;
	SDL_GPUBufferRegion sdlGPUBufferRegion = {};
	sdlGPUBufferRegion.buffer = m_sdlSpriteDataBuffer;
	sdlGPUBufferRegion.offset = 0;
	sdlGPUBufferRegion.size = spriteCount * sizeof(SpriteBatchInfo);
	SDL_UploadToGPUBuffer(copyPass, &sdlGPUTransferBufferLocation, &sdlGPUBufferRegion, true);
	SDL_EndGPUCopyPass(copyPass);

	// Render sprites
	SDL_GPUColorTargetInfo sdlGPUColorTargetInfo = {};
	sdlGPUColorTargetInfo.texture = swapchainTexture;
	sdlGPUColorTargetInfo.cycle = false;
	sdlGPUColorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	sdlGPUColorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	sdlGPUColorTargetInfo.clear_color = { 0.f, 0.f, 0.2f, 1.f };
	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &sdlGPUColorTargetInfo, 1, nullptr);
	SDL_BindGPUGraphicsPipeline(renderPass, m_pipeline->GetPipeline());
	SDL_BindGPUVertexStorageBuffers(renderPass, 0, &m_sdlSpriteDataBuffer, 1);
	SDL_GPUTextureSamplerBinding sdlGPUTextureSamplerBinding = {};
	sdlGPUTextureSamplerBinding.texture = m_sdlGPUTexture;
	sdlGPUTextureSamplerBinding.sampler = m_sdlGPUSampler;
	SDL_BindGPUFragmentSamplers(renderPass, 0, &sdlGPUTextureSamplerBinding, 1);
	SDL_PushGPUVertexUniformData(commandBuffer, 0, &cameraMatrix[0][0], sizeof(glm::mat4x4));
	SDL_DrawGPUPrimitives(renderPass, spriteCount * 6, 1, 0, 0);
	SDL_EndGPURenderPass(renderPass);
}

void SpriteRenderer::SetTexturePage(const TexturePage* texturePage) {
	// Upload image data to transfer buffer
	SDL_GPUDevice* device = Game::GetGPUDevice();
	std::uint32_t bufferSize = texturePage->GetWidth() * texturePage->GetHeight() * 4u;
	SDL_GPUTransferBufferCreateInfo textureTransferBufferCreateInfo = {};
	textureTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	textureTransferBufferCreateInfo.size = bufferSize;
	SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(device, &textureTransferBufferCreateInfo);
	Uint8* textureTransferPtr = (Uint8*)SDL_MapGPUTransferBuffer(device, textureTransferBuffer, false);
	SDL_memcpy(textureTransferPtr, texturePage->GetData(), bufferSize);
	SDL_UnmapGPUTransferBuffer(device, textureTransferBuffer);

	// Get GPU texture
	if (m_sdlGPUTexture) { SDL_ReleaseGPUTexture(device, m_sdlGPUTexture); }
	SDL_GPUTextureCreateInfo textureCreateInfo = {};
	textureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
	textureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	textureCreateInfo.width = texturePage->GetWidth();
	textureCreateInfo.height = texturePage->GetHeight();
	textureCreateInfo.layer_count_or_depth = 1;
	textureCreateInfo.num_levels = 1;
	textureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	m_sdlGPUTexture = SDL_CreateGPUTexture(device, &textureCreateInfo);

	// Use transfer buffer to copy data to texture
	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
	SDL_GPUTextureTransferInfo textureTransferInfo = {};
	textureTransferInfo.transfer_buffer = textureTransferBuffer;
	textureTransferInfo.offset = 0;
	SDL_GPUTextureRegion textureRegion = {};
	textureRegion.texture = m_sdlGPUTexture;
	textureRegion.w = texturePage->GetWidth();
	textureRegion.h = texturePage->GetHeight();
	textureRegion.d = 1;
	SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);
	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(commandBuffer);
	SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);
}

} // luna