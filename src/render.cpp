#include <luna/detail/render.hpp>
#include <luna/detail/game.hpp>
#include <luna/detail/room.hpp>
#include <luna/detail/shader.hpp>
#include <luna/detail/sprite.hpp>
#include <luna/detail/camera.hpp>

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

void SpriteRenderer::PreDraw() {
	m_spriteList.clear();
}

void SpriteRenderer::Draw() {
	Room* currentRoom = RoomManager::GetCurrentRoom();
	if (!currentRoom) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SpriteRenderer::Draw failed: Room stack is empty!");
		return;
	}

	// Stitch together multiple sprite lists

	// Sort into batches
	SpriteListComp cmp;
	intro_sort(m_spriteList.begin(), m_spriteList.end(), cmp);

	// Render batches
	RenderSpriteList(Game::GetWindow(), &m_spriteList, ConvertToFColor(currentRoom->GetClearColor()));
}

void SpriteRenderer::PostDraw() {}

void SpriteRenderer::DrawSprite(Sprite* sprite) {
	// Only draw valid sprites
	if (!sprite->IsValid()) { return; }

	// Cull sprites outside the camera view
	Camera* camera = RoomManager::GetCurrentRoom()->GetActiveCamera();
	if (camera) {
		float left = sprite->GetPositionX();
		float top = sprite->GetPositionY();
		float right = sprite->GetPositionX() + (sprite->GetWidth() * sprite->GetScaleX());
		float bottom = sprite->GetPositionY() + (sprite->GetHeight() * sprite->GetScaleY());
		if (!camera->RegionOnCamera(left, right, top, bottom)) { return; }
	}

	// Add to draw queue
	m_spriteList.push_back(sprite);
}

void SpriteRenderer::RenderSpriteList(SDL_Window* window, SpriteList* spriteList, SDL_FColor clearColor) {
	// Get camera
	const Camera* roomCamera = RoomManager::GetCurrentRoom()->GetActiveCamera();
	if (!roomCamera) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SpriteRenderer::RenderSpriteList failed: Active room camera is invalid!");
		return;
	}
	glm::mat4 cameraMatrix = roomCamera->ProjectionOrtho();

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

		for (std::size_t i = 0; i < spriteList->size(); ++i) {
			// Get current sprite
			const Sprite* sprite = spriteList->at(i);
			TexturePageID texturePageID = sprite->GetTexturePageID();

			// Check for GPU state change
			if (texturePageID != m_lastTexturePageID) {
				m_lastTexturePageID = texturePageID;

				// Render current batch
				if (spriteCount > 0) {
					RenderSpriteListBatch(commandBuffer, clearColor, swapchainTexture, &cameraMatrix, roomCamera->GetNearPlane(), roomCamera->GetFarPlane(), spriteList, spriteBegin, spriteCount);
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
			RenderSpriteListBatch(commandBuffer, clearColor, swapchainTexture, &cameraMatrix, roomCamera->GetNearPlane(), roomCamera->GetFarPlane(), spriteList, spriteBegin, spriteCount);
		}
	}
	SDL_SubmitGPUCommandBuffer(commandBuffer);
}

void SpriteRenderer::RenderSpriteListBatch(SDL_GPUCommandBuffer* commandBuffer, SDL_FColor clearColor, SDL_GPUTexture* swapchainTexture, glm::mat4* cameraMatrix, float zNear, float zFar, SpriteList* spriteList, std::size_t spriteBegin, std::size_t spriteCount) {
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
		depthTextureCreateInfo.width = Game::GetWindowWidth();
		depthTextureCreateInfo.height = Game::GetWindowHeight();
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
		const Sprite* sprite = spriteList->at(spriteBegin + i);
		SpriteTextureCoords spriteTextureCoords = sprite->GetTextureCoords();
		SDL_FColor spriteColor = ConvertToFColor(sprite->GetBlend());
		dataPtr[i].x = sprite->GetPositionX();
		dataPtr[i].y = sprite->GetPositionY();
		dataPtr[i].z = float(sprite->GetDepth());
		dataPtr[i].rotation = sprite->GetRotation();
		dataPtr[i].w = sprite->GetWidth();
		dataPtr[i].h = sprite->GetHeight();
		dataPtr[i].scaleX = sprite->GetScaleX();
		dataPtr[i].scaleY = sprite->GetScaleY();
		dataPtr[i].originX = sprite->GetOriginX();
		dataPtr[i].originY = sprite->GetOriginY();
		dataPtr[i].texU = spriteTextureCoords.textureU;
		dataPtr[i].texV = spriteTextureCoords.textureV;
		dataPtr[i].texW = spriteTextureCoords.textureW;
		dataPtr[i].texH = spriteTextureCoords.textureH;
		dataPtr[i].r = spriteColor.r;
		dataPtr[i].g = spriteColor.g;
		dataPtr[i].b = spriteColor.b;
		dataPtr[i].a = spriteColor.a;
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
	renderColorTargetInfo.clear_color = clearColor;
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