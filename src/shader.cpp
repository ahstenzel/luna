#include <luna/detail/shader.hpp>
#include <luna/detail/shader/shader_encoded.hpp>

namespace luna {

static bool g_enableShaderDebug = false;

static SDL_GPUShader* CompileDefaultShaderHLSL(SDL_GPUDevice* device, const ShaderInfo& shader, SDL_ShaderCross_ShaderStage shaderStage, const std::string& entrypoint = "main", SDL_ShaderCross_HLSL_Define* defines = nullptr) {
	// Validate info
	if (shader.m_samplers < 0 || shader.m_storageBuffers < 0 || shader.m_storageTextures < 0 || shader.m_uniformBuffers < 0) {
		return nullptr;
	}

	// Decode source
	std::size_t encoded_len = shader.m_source.length();
	std::string decoded_src;
	decoded_src.resize(encoded_len + 1);
	std::size_t decoded_len = 0;
	base64_decode(shader.m_source.c_str(), encoded_len, decoded_src.data(), &decoded_len, 0);

	// Create shader
	SDL_ShaderCross_HLSL_Info shader_info{};
	shader_info.source = decoded_src.c_str();
	shader_info.entrypoint = entrypoint.c_str();
	shader_info.defines = defines;
	shader_info.shader_stage = shaderStage;
	shader_info.enable_debug = g_enableShaderDebug;
	shader_info.name = shader.m_name.c_str();
	SDL_ShaderCross_GraphicsShaderMetadata shader_metadata{};
	shader_metadata.num_samplers = (Uint32)shader.m_samplers;
	shader_metadata.num_storage_buffers = (Uint32)shader.m_storageBuffers;
	shader_metadata.num_storage_textures = (Uint32)shader.m_storageTextures;
	shader_metadata.num_uniform_buffers = (Uint32)shader.m_uniformBuffers;
	SDL_GPUShader* shaderCompiled = SDL_ShaderCross_CompileGraphicsShaderFromHLSL(
		device,
		&shader_info,
		&shader_metadata
	);
	if (!shaderCompiled) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Shader compilation (%s) failed!", shader.m_name.c_str());
	}
	return shaderCompiled;
}

SpriteBatchShaderPipeline::SpriteBatchShaderPipeline(SDL_GPUDevice* device, SDL_Window* window) {
	m_device = device;

	// Decode & compile shaders
	m_fragShader = CompileDefaultShaderHLSL(device, SpriteBatch_frag_hlsl, SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);
	m_vertShader = CompileDefaultShaderHLSL(device, SpriteBatch_vert_hlsl, SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
	if (!m_fragShader || !m_vertShader) {
		Clear();
		return; 
	}

	// Build pipeline
	SDL_GPUColorTargetBlendState colorTargetBlendState{};
	colorTargetBlendState.enable_blend = true;
	colorTargetBlendState.color_blend_op = SDL_GPU_BLENDOP_ADD;
	colorTargetBlendState.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
	colorTargetBlendState.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colorTargetBlendState.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	colorTargetBlendState.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colorTargetBlendState.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	SDL_GPUColorTargetDescription colorTargetDescription{};
	colorTargetDescription.format = SDL_GetGPUSwapchainTextureFormat(m_device, window);
	colorTargetDescription.blend_state = colorTargetBlendState;
	SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
	targetInfo.num_color_targets = 1;
	targetInfo.color_target_descriptions = &colorTargetDescription;
	SDL_GPUGraphicsPipelineCreateInfo createInfo{};
	createInfo.vertex_shader = m_vertShader;
	createInfo.fragment_shader = m_fragShader;
	createInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	createInfo.target_info = targetInfo;
	m_pipeline = SDL_CreateGPUGraphicsPipeline(m_device, &createInfo);
	SDL_ReleaseGPUShader(m_device, m_fragShader);
	SDL_ReleaseGPUShader(m_device, m_vertShader);
	m_fragShader = nullptr;
	m_vertShader = nullptr;
}

SpriteBatchShaderPipeline::~SpriteBatchShaderPipeline() {
	Clear();
}

SDL_GPUGraphicsPipeline* SpriteBatchShaderPipeline::GetPipeline() const {
	return m_pipeline;
}

void SpriteBatchShaderPipeline::Clear() {
	if (m_pipeline) { SDL_ReleaseGPUGraphicsPipeline(m_device, m_pipeline); }
	if (m_fragShader) { SDL_ReleaseGPUShader(m_device, m_fragShader); }
	if (m_vertShader) { SDL_ReleaseGPUShader(m_device, m_vertShader); }
	m_pipeline = nullptr;
	m_fragShader = nullptr;
	m_vertShader = nullptr;
}

bool SpriteBatchShaderPipeline::IsValid() const {
	return (m_pipeline);
}

} // luna