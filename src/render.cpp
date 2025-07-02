#include <luna/detail/render.hpp>
#include <luna/detail/game.hpp>
#include <luna/detail/room.hpp>
#include <luna/detail/shader.hpp>
#include <luna/detail/sprite.hpp>
#include <luna/detail/camera.hpp>

namespace luna {

SpriteRenderer::SpriteRenderer() {
	// Build shader pipelines
	m_spriteBatchPipeline = new SpriteBatchShaderPipeline();
	m_primitiveBatchPipeline = new PrimitiveBatchShaderPipeline(false);
	m_primitiveLineBatchPipeline = new PrimitiveBatchShaderPipeline(true);
}

SpriteRenderer::~SpriteRenderer() {
	SDL_GPUDevice* device = Game::GetGPUDevice();
	delete m_spriteBatchPipeline;
	delete m_primitiveBatchPipeline;
	delete m_primitiveLineBatchPipeline;
	SDL_ReleaseGPUSampler(device, m_sdlGPUSampler);
	SDL_ReleaseGPUTexture(device, m_sdlGPUAtlasTexture);
	SDL_ReleaseGPUTexture(device, m_sdlGPUDepthTexture);
	SDL_ReleaseGPUBuffer(device, m_sdlSpriteDataBuffer);
	SDL_ReleaseGPUTransferBuffer(device, m_sdlSpriteDataTransferBuffer);
	SDL_ReleaseGPUBuffer(device, m_sdlPrimitiveDataVertexBuffer);
	SDL_ReleaseGPUBuffer(device, m_sdlPrimitiveDataIndexBuffer);
	SDL_ReleaseGPUTransferBuffer(device, m_sdlPrimitiveDataTransferBuffer);
	SDL_ReleaseGPUTransferBuffer(device, m_sdlTextureTransferBuffer);
}

bool SpriteRenderer::IsValid() const {
	return (
		m_spriteBatchPipeline && m_primitiveBatchPipeline && m_primitiveLineBatchPipeline &&
		m_spriteBatchPipeline->IsValid() && m_primitiveBatchPipeline->IsValid() && m_primitiveLineBatchPipeline->IsValid()
		);
}

void SpriteRenderer::DrawSprite(Sprite sprite) {
	// Only draw valid sprites
	if (!sprite.IsValid()) { return; }

	// Cull sprites outside the camera view
	Camera* camera = RoomManager::GetCurrentRoom()->GetActiveCamera();
	if (camera) {
		if (!camera->RegionOnCamera(sprite.GetShapeCircle())) { return; }
	}

	// Add to draw queue
	if (sprite.GetTranslucent()) { m_translucentRenderables.emplace_back(this, m_sprites.size(), RenderableType::SpriteType, false); }
	else { m_opaqueRenderables.emplace_back(this, m_sprites.size(), RenderableType::SpriteType, true); }
	m_sprites.push_back(sprite);
}

void SpriteRenderer::DrawPrimitive(Primitive primitive) {
	// Cull primitives outside the camera view
	Camera* camera = RoomManager::GetCurrentRoom()->GetActiveCamera();
	if (camera) {
		if (!camera->RegionOnCamera(primitive.GetShape(), primitive.GetShapeType())) {
			return;
		}
	}

	// Add to draw queue
	if (primitive.GetAlpha() < 1.f) { m_translucentRenderables.emplace_back(this, m_primitives.size(), RenderableType::PrimitiveType, false); }
	else { m_opaqueRenderables.emplace_back(this, m_primitives.size(), RenderableType::PrimitiveType, true); }
	m_primitives.push_back(primitive);
}

void SpriteRenderer::PreDraw() {
	m_sprites.clear();
	m_primitives.clear();
	m_opaqueRenderables.clear();
	m_translucentRenderables.clear();
}

void SpriteRenderer::Draw() {
	Room* currentRoom = RoomManager::GetCurrentRoom();
	if (!currentRoom) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SpriteRenderer::Draw failed: Room stack is empty!");
		return;
	}
	if (m_opaqueRenderables.empty() && m_translucentRenderables.empty()) { return; }

	// Stitch together lists from multiple threads

	// Sort opaque & translucent renderables seperately
	merge_sort(m_opaqueRenderables.begin(), m_opaqueRenderables.end(), CompRenderableSpriteTexturePage());
	merge_sort(m_opaqueRenderables.begin(), m_opaqueRenderables.end(), CompRenderablePrimitiveWireframe());
	merge_sort(m_opaqueRenderables.begin(), m_opaqueRenderables.end(), CompRenderableType());
	merge_sort(m_translucentRenderables.begin(), m_translucentRenderables.end(), CompRenderableSpriteTexturePage());
	merge_sort(m_translucentRenderables.begin(), m_translucentRenderables.end(), CompRenderablePrimitiveWireframe());
	merge_sort(m_translucentRenderables.begin(), m_translucentRenderables.end(), CompRenderableType());
	merge_sort(m_translucentRenderables.begin(), m_translucentRenderables.end(), CompRenderableDepth());

	// Stitch together opaque & translucent renderables
	RenderableList completeList;
	completeList.insert(completeList.end(), std::make_move_iterator(m_opaqueRenderables.begin()), std::make_move_iterator(m_opaqueRenderables.end()));
	completeList.insert(completeList.end(), std::make_move_iterator(m_translucentRenderables.begin()), std::make_move_iterator(m_translucentRenderables.end()));

	// Split into batches
	std::vector<RenderableBatch> batches;
	RenderableBatch currentBatch;
	for (std::size_t i = 0; i < completeList.size(); ++i) {
		Renderable currRendereable = completeList[i];
		if (!currentBatch.MatchRenderable(currRendereable)) {
			batches.push_back(std::move(currentBatch));
			currentBatch = RenderableBatch();
		}
		currentBatch.AddRenderable(currRendereable);
	}
	batches.push_back(std::move(currentBatch));
	 
	// Get GPU resources
	SDL_Window* window = Game::GetWindow();
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
		// Initialize depth texture
		if (!m_sdlGPUDepthTexture) {
			SDL_GPUTextureCreateInfo depthTextureCreateInfo = {};
			depthTextureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
			depthTextureCreateInfo.format = detail::GetDepthStencilFormat(device);
			depthTextureCreateInfo.width = Game::GetWindowWidth();
			depthTextureCreateInfo.height = Game::GetWindowHeight();
			depthTextureCreateInfo.layer_count_or_depth = 1;
			depthTextureCreateInfo.num_levels = 1;
			depthTextureCreateInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
			depthTextureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
			m_sdlGPUDepthTexture = SDL_CreateGPUTexture(Game::GetGPUDevice(), &depthTextureCreateInfo);
		}

		// Initialize render targets
		Camera* currentCamera = currentRoom->GetActiveCamera();
		auto cameraMatrix = currentCamera->ProjectionOrtho();
		m_sdlRenderColorTargetInfo = {};
		m_sdlRenderColorTargetInfo.texture = swapchainTexture;
		m_sdlRenderColorTargetInfo.cycle = false;
		m_sdlRenderColorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		m_sdlRenderColorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
		m_sdlRenderColorTargetInfo.clear_color = ConvertToFColor(currentRoom->GetClearColor());
		m_sdlRenderDepthStencilTargetInfo = {};
		m_sdlRenderDepthStencilTargetInfo.texture = m_sdlGPUDepthTexture;
		m_sdlRenderDepthStencilTargetInfo.cycle = false;
		m_sdlRenderDepthStencilTargetInfo.clear_depth = currentCamera->GetNearPlane();
		m_sdlRenderDepthStencilTargetInfo.clear_stencil = 0;
		m_sdlRenderDepthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		m_sdlRenderDepthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
		m_sdlRenderDepthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
		m_sdlRenderDepthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
		m_clearTarget = true;

		// Render batches one at a time
		for (auto& batch : batches) {
			SDL_assert(!batch.m_renderableList.empty());
			switch (batch.m_renderableType) {
			case RenderableType::SpriteType: RenderSpriteListBatch(commandBuffer, &cameraMatrix, batch.m_renderableList); break;
			case RenderableType::PrimitiveType: 
				auto primitive = batch.m_renderableList[0].GetPrimitive();
				RenderPrimitiveListBatch(commandBuffer, &cameraMatrix, batch.m_renderableList, primitive->IsWireframe()); 
			break;
			}
		}
	}
	SDL_SubmitGPUCommandBuffer(commandBuffer);
}

void SpriteRenderer::PostDraw() {}

SpriteRenderer::Renderable::Renderable() :
	m_renderer(nullptr),
	m_renderableType(RenderableType::Unknown),
	m_renderableIndex(0),
	m_opaque(false) {}

SpriteRenderer::Renderable::Renderable(SpriteRenderer* renderer, std::size_t renderableIndex, RenderableType renderableType, bool opaque) :
	m_renderer(renderer),
	m_renderableIndex(renderableIndex),
	m_renderableType(renderableType),
	m_opaque(opaque) {}

SpriteRenderer::Renderable::Renderable(const Renderable& renderable) :
	m_renderer(renderable.m_renderer),
	m_renderableIndex(renderable.m_renderableIndex),
	m_renderableType(renderable.m_renderableType),
	m_opaque(renderable.m_opaque) {}

SpriteRenderer::Renderable::Renderable(Renderable&& renderable) noexcept :
	m_renderer(std::move(renderable.m_renderer)),
	m_renderableIndex(std::move(renderable.m_renderableIndex)),
	m_renderableType(std::move(renderable.m_renderableType)),
	m_opaque(std::move(renderable.m_opaque)) {}

bool SpriteRenderer::Renderable::IsValid() const {
	return (m_renderer && m_renderableType != RenderableType::Unknown);
}

Sprite* SpriteRenderer::Renderable::GetSprite() const {
	if (m_renderableType == RenderableType::SpriteType) {
		return &m_renderer->m_sprites[m_renderableIndex];
	}
	else return nullptr;
}

Primitive* SpriteRenderer::Renderable::GetPrimitive() const {
	if (m_renderableType == RenderableType::PrimitiveType) {
		return &m_renderer->m_primitives[m_renderableIndex];
	}
	else return nullptr;
}

SpriteRenderer::Renderable& SpriteRenderer::Renderable::operator=(const Renderable& other) {
	if (this == &other) { return *this; }
	m_renderer = other.m_renderer;
	m_renderableIndex = other.m_renderableIndex;
	m_renderableType = other.m_renderableType;
	m_opaque = other.m_opaque;
	return *this;
}

SpriteRenderer::Renderable& SpriteRenderer::Renderable::operator=(Renderable&& other) noexcept {
	if (this == &other) { return *this; }
	m_renderer = std::move(other.m_renderer);
	m_renderableIndex = std::move(other.m_renderableIndex);
	m_renderableType = std::move(other.m_renderableType);
	m_opaque = std::move(other.m_opaque);
	return *this;
}

bool SpriteRenderer::Renderable::operator==(const Renderable& other) const {
	return (
		m_renderer == other.m_renderer &&
		m_renderableIndex == other.m_renderableIndex &&
		m_renderableType == other.m_renderableType &&
		m_opaque == other.m_opaque
		);
}

SpriteRenderer::RenderableBatch::RenderableBatch() :
	m_renderableType(RenderableType::Unknown),
	m_renderableList(),
	m_opaque(false) {
}

SpriteRenderer::RenderableBatch::RenderableBatch(RenderableType type, const RenderableList& renderableList, bool opaque) :
	m_renderableType(type),
	m_renderableList(renderableList),
	m_opaque(opaque) {
}

SpriteRenderer::RenderableBatch::RenderableBatch(const RenderableBatch& other) :
	m_renderableType(other.m_renderableType),
	m_renderableList(other.m_renderableList),
	m_opaque(other.m_opaque) {
}

SpriteRenderer::RenderableBatch::RenderableBatch(RenderableBatch&& other) noexcept :
	m_renderableType(std::move(other.m_renderableType)),
	m_renderableList(std::move(other.m_renderableList)),
	m_opaque(std::move(other.m_opaque)) {
}

bool SpriteRenderer::RenderableBatch::MatchRenderable(const Renderable& renderable) const {
	if (!m_renderableList.empty()) {
		// Split if its a different transparency
		if (renderable.m_opaque != m_opaque) { return false; }
		// Split if its a different type of renderable
		if (renderable.m_renderableType != m_renderableType) { return false; }
		else if (renderable.m_renderableType == RenderableType::SpriteType) {
			// Split if a new texture page needs to be loaded
			Sprite* matchSprite = renderable.GetSprite();
			Sprite* thisSprite = m_renderableList[0].GetSprite();
			if (matchSprite->GetTexturePageID() != thisSprite->GetTexturePageID()) { return false; }
		}
		else if (renderable.m_renderableType == RenderableType::PrimitiveType) {
			// Split if a new shape type needs to be drawn
			Primitive* matchPrimitive = renderable.GetPrimitive();
			Primitive* thisPrimitive = m_renderableList[0].GetPrimitive();
			if (matchPrimitive->IsWireframe() != thisPrimitive->IsWireframe()) { return false; }
		}
		else { return false; }
	}
	return true;
}

void SpriteRenderer::RenderableBatch::AddRenderable(const Renderable& renderable) {
	if (m_renderableList.empty()) {
		m_renderableType = renderable.m_renderableType;
		m_opaque = renderable.m_opaque;
	}
	m_renderableList.push_back(renderable);
}

SpriteRenderer::RenderableBatch& SpriteRenderer::RenderableBatch::operator=(const RenderableBatch& other) {
	if (this == &other) { return *this; }
	m_renderableType = other.m_renderableType;
	m_renderableList = other.m_renderableList;
	m_opaque = other.m_opaque;
	return *this;
}

SpriteRenderer::RenderableBatch& SpriteRenderer::RenderableBatch::operator=(RenderableBatch&& other) noexcept {
	if (this == &other) { return *this; }
	m_renderableType = std::move(other.m_renderableType);
	m_renderableList = std::move(other.m_renderableList);
	m_opaque = std::move(other.m_opaque);
	return *this;
}

bool SpriteRenderer::RenderableBatch::operator==(const RenderableBatch& other) const {
	return (
		m_opaque == other.m_opaque &&
		m_renderableType == other.m_renderableType &&
		m_renderableList == other.m_renderableList
		);
}

void SpriteRenderer::RenderSpriteListBatch(SDL_GPUCommandBuffer* commandBuffer, glm::mat4* cameraMatrix, const RenderableList& sprites) {
	SDL_GPUDevice* device = Game::GetGPUDevice();

	// Change texture page if needed
	const Sprite* firstSprite = &m_sprites[sprites[0].m_renderableIndex];
	if (m_currentTexturePageID != firstSprite->GetTexturePageID()) {
		m_currentTexturePageID = firstSprite->GetTexturePageID();
		SetTexturePage(commandBuffer, firstSprite->GetTexturePage());
	}

	// Resize transfer buffer if needed
	std::size_t spriteCount = sprites.size();
	if (spriteCount != m_lastSpriteBatchCount) {
		m_lastSpriteBatchCount = spriteCount;
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

	// Build sprite instance buffer
	SpriteBatchInfo* dataPtr = (SpriteBatchInfo*)SDL_MapGPUTransferBuffer(device, m_sdlSpriteDataTransferBuffer, true);
	if (!dataPtr) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to map GPU transfer buffer: %s", SDL_GetError());
		return;
	}
	for(std::size_t i = 0; i < spriteCount; ++i) {
		const Sprite* sprite = &m_sprites[sprites[i].m_renderableIndex];
		SpriteTextureCoords spriteTextureCoords = sprite->GetTextureCoords();
		SDL_FColor spriteColor = ConvertToFColor(sprite->GetBlend());
		dataPtr[i].x = sprite->GetPositionX();
		dataPtr[i].y = sprite->GetPositionY();
		dataPtr[i].z = -float(sprite->GetDepth());
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
	if (m_clearTarget) m_clearTarget = false;
	else {
		m_sdlRenderColorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
		m_sdlRenderDepthStencilTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
	}
	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &m_sdlRenderColorTargetInfo, 1, &m_sdlRenderDepthStencilTargetInfo);
	SDL_BindGPUGraphicsPipeline(renderPass, m_spriteBatchPipeline->GetPipeline());
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
	std::uint8_t* textureTransferPtr = (std::uint8_t*)SDL_MapGPUTransferBuffer(device, m_sdlTextureTransferBuffer, false);
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

void SpriteRenderer::RenderPrimitiveListBatch(SDL_GPUCommandBuffer* commandBuffer, glm::mat4* cameraMatrix, const RenderableList& primitives, bool wireframe) {
	SDL_GPUDevice* device = Game::GetGPUDevice();

	// Build vertex & instance buffers
	std::size_t primitiveCount = primitives.size();
	std::vector<VertexPosColor> vertices;
	std::vector<std::uint16_t> indices;
	for (std::size_t i = 0; i < primitiveCount; ++i) {
		Primitive* primitive = &m_primitives[primitives[i].m_renderableIndex];
		auto primitiveVertices = primitive->GetVertices();
		auto primitiveIndices = primitive->GetIndices();

		// Offset indices
		std::uint16_t offset = vertices.size();
		for (auto& index : primitiveIndices) {
			index += offset;
		}

		// Add to master list
		vertices.insert(vertices.end(), primitiveVertices.begin(), primitiveVertices.end());
		indices.insert(indices.end(), primitiveIndices.begin(), primitiveIndices.end());
	}

	// Resize transfer buffer if needed
	std::size_t vertexCount = vertices.size();
	std::size_t indexCount = indices.size();
	std::size_t vertexSize = vertexCount * sizeof(VertexPosColor);
	std::size_t indexSize = indexCount * sizeof(std::uint16_t);
	if (vertexCount != m_lastPrimitiveVertexCount || indexCount != m_lastPrimitiveIndexCount) {
		m_lastPrimitiveVertexCount = vertexCount;
		m_lastPrimitiveIndexCount = indexCount;

		SDL_ReleaseGPUTransferBuffer(device, m_sdlPrimitiveDataTransferBuffer);
		SDL_ReleaseGPUBuffer(device, m_sdlPrimitiveDataVertexBuffer);
		SDL_ReleaseGPUBuffer(device, m_sdlPrimitiveDataIndexBuffer);

		SDL_GPUTransferBufferCreateInfo primitiveDataTransferBufferCreateInfo = {};
		primitiveDataTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		primitiveDataTransferBufferCreateInfo.size = vertexSize + indexSize;
		m_sdlPrimitiveDataTransferBuffer = SDL_CreateGPUTransferBuffer(device, &primitiveDataTransferBufferCreateInfo);

		SDL_GPUBufferCreateInfo primitiveDataVertexBufferCreateInfo = {};
		primitiveDataVertexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
		primitiveDataVertexBufferCreateInfo.size = vertexSize;
		m_sdlPrimitiveDataVertexBuffer = SDL_CreateGPUBuffer(device, &primitiveDataVertexBufferCreateInfo);

		SDL_GPUBufferCreateInfo primitiveDataIndexBufferCreateInfo = {};
		primitiveDataIndexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
		primitiveDataIndexBufferCreateInfo.size = indexSize;
		m_sdlPrimitiveDataIndexBuffer = SDL_CreateGPUBuffer(device, &primitiveDataIndexBufferCreateInfo);
	}

	// Build vertex buffer
	/*
	{
		std::uint8_t* testBuffer = new std::uint8_t[vertexSize + indexSize];
		SDL_memcpy(testBuffer, &vertices[0][0], vertexSize);
		std::uint16_t* indexDataPtr = (std::uint16_t*)&testBuffer[vertexSize];
		SDL_memcpy(indexDataPtr, &indices[0], indexSize);
		for (std::size_t v = 0; v < vertexCount; ++v) {
			VertexPosColor* p = reinterpret_cast<VertexPosColor*>(&testBuffer[v * sizeof(VertexPosColor)]);
			std::cout << "Vertex " << v << ": " << *p << std::endl;
		}
		for (std::size_t i = 0; i < indexCount; ++i) {
			std::uint16_t* p = reinterpret_cast<std::uint16_t*>(&testBuffer[vertexSize + (i * sizeof(std::uint16_t))]);
			std::cout << "Index " << i << ": " << *p << std::endl;
		}
		std::cout << "---" << std::endl;
	}
	*/

	std::uint8_t* vertexDataPtr = (std::uint8_t*)SDL_MapGPUTransferBuffer(device, m_sdlPrimitiveDataTransferBuffer, false);
	if (!vertexDataPtr) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to map GPU transfer buffer: %s", SDL_GetError());
		return;
	}
	SDL_memcpy(vertexDataPtr, &vertices[0][0], vertexSize);
	std::uint16_t* indexDataPtr = (std::uint16_t*)&vertexDataPtr[vertexSize];
	SDL_memcpy(indexDataPtr, &indices[0], indexSize);
	SDL_UnmapGPUTransferBuffer(device, m_sdlPrimitiveDataTransferBuffer);

	// Upload vertex data
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
	SDL_GPUTransferBufferLocation primitiveVertexDataTransferBufferLocation = {};
	primitiveVertexDataTransferBufferLocation.transfer_buffer = m_sdlPrimitiveDataTransferBuffer;
	primitiveVertexDataTransferBufferLocation.offset = 0;
	SDL_GPUBufferRegion primitiveVertexDataBufferRegion = {};
	primitiveVertexDataBufferRegion.buffer = m_sdlPrimitiveDataVertexBuffer;
	primitiveVertexDataBufferRegion.offset = 0;
	primitiveVertexDataBufferRegion.size = vertexSize;
	SDL_GPUTransferBufferLocation primitiveIndexDataTransferBufferLocation = {};
	primitiveIndexDataTransferBufferLocation.transfer_buffer = m_sdlPrimitiveDataTransferBuffer;
	primitiveIndexDataTransferBufferLocation.offset = vertexSize;
	SDL_GPUBufferRegion primitiveIndexDataBufferRegion = {};
	primitiveIndexDataBufferRegion.buffer = m_sdlPrimitiveDataIndexBuffer;
	primitiveIndexDataBufferRegion.offset = 0;
	primitiveIndexDataBufferRegion.size = indexSize;
	SDL_UploadToGPUBuffer(copyPass, &primitiveVertexDataTransferBufferLocation, &primitiveVertexDataBufferRegion, false);
	SDL_UploadToGPUBuffer(copyPass, &primitiveIndexDataTransferBufferLocation, &primitiveIndexDataBufferRegion, false);
	SDL_EndGPUCopyPass(copyPass);

	// Render primitives
	SDL_GPUBufferBinding vertexBufferBinding = {};
	vertexBufferBinding.buffer = m_sdlPrimitiveDataVertexBuffer;
	vertexBufferBinding.offset = 0;
	SDL_GPUBufferBinding indexBufferBinding = {};
	indexBufferBinding.buffer = m_sdlPrimitiveDataIndexBuffer;
	indexBufferBinding.offset = 0;
	if (m_clearTarget) m_clearTarget = false;
	else {
		m_sdlRenderColorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
		m_sdlRenderDepthStencilTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
	}
	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &m_sdlRenderColorTargetInfo, 1, &m_sdlRenderDepthStencilTargetInfo);
	auto pipeline = (wireframe) ? m_primitiveLineBatchPipeline->GetPipeline() : m_primitiveBatchPipeline->GetPipeline();
	SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
	SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);
	SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	SDL_PushGPUVertexUniformData(commandBuffer, 0, &(cameraMatrix[0][0]), sizeof(glm::mat4));
	SDL_DrawGPUIndexedPrimitives(renderPass, indexCount, 1, 0, 0, 0);
	SDL_EndGPURenderPass(renderPass);
}

bool SpriteRenderer::CompRenderableDepth::operator()(const Renderable& lhs, const Renderable& rhs) {
	float d1 = 0.f, d2 = 0.f;
	switch (lhs.m_renderableType) {
	case RenderableType::SpriteType: d1 = lhs.m_renderer->m_sprites[lhs.m_renderableIndex].GetDepth(); break;
	case RenderableType::PrimitiveType: d1 = lhs.m_renderer->m_primitives[lhs.m_renderableIndex].GetDepth(); break;
	}
	switch (rhs.m_renderableType) {
	case RenderableType::SpriteType: d2 = rhs.m_renderer->m_sprites[rhs.m_renderableIndex].GetDepth(); break;
	case RenderableType::PrimitiveType: d2 = rhs.m_renderer->m_sprites[rhs.m_renderableIndex].GetDepth(); break;
	}
	return d1 < d2;
}

bool SpriteRenderer::CompRenderableType::operator()(const Renderable& lhs, const Renderable& rhs) {
	return lhs.m_renderableType < rhs.m_renderableType;
}

bool SpriteRenderer::CompRenderableSpriteTexturePage::operator()(const Renderable& lhs, const Renderable& rhs) {
	TexturePageID t1 = TEXTURE_PAGE_ID_NULL, t2 = TEXTURE_PAGE_ID_NULL;
	if (lhs.m_renderableType == RenderableType::SpriteType) { t1 = lhs.m_renderer->m_sprites[lhs.m_renderableIndex].GetTexturePageID(); }
	else { return false; }
	if (rhs.m_renderableType == RenderableType::SpriteType) { t2 = lhs.m_renderer->m_sprites[lhs.m_renderableIndex].GetTexturePageID(); }
	else { return true; }
	return t1 < t2;
}

bool SpriteRenderer::CompRenderablePrimitiveWireframe::operator()(const Renderable& lhs, const Renderable& rhs) {
	bool t1, t2;
	if (lhs.m_renderableType == RenderableType::PrimitiveType) { t1 = lhs.m_renderer->m_primitives[lhs.m_renderableIndex].IsWireframe(); }
	else { return false; }
	if (rhs.m_renderableType == RenderableType::PrimitiveType) { t2 = lhs.m_renderer->m_primitives[lhs.m_renderableIndex].IsWireframe(); }
	else { return true; }
	return t1 < t2;
}

namespace detail {

SDL_GPUTextureFormat GetDepthStencilFormat(SDL_GPUDevice* device) {
	SDL_GPUTextureFormat format;
	if (SDL_GPUTextureSupportsFormat(
		device,
		SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
		SDL_GPU_TEXTURETYPE_2D,
		SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET
	)) {
		format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
	}
	else if (SDL_GPUTextureSupportsFormat(
		device,
		SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
		SDL_GPU_TEXTURETYPE_2D,
		SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET
	)) {
		format = SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
	}
	else { format = SDL_GPU_TEXTUREFORMAT_D16_UNORM; }
	return format;
}

} // detail

} // luna