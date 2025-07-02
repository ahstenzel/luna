#include <luna/detail/shader.hpp>
#include <luna/detail/shader/shader_encoded.hpp>
#include <luna/detail/game.hpp>

namespace luna {

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
	shader_info.enable_debug = Game::GetGraphicsDebuggingEnabled();
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

SpriteBatchShaderPipeline::SpriteBatchShaderPipeline() {
	SDL_GPUDevice* device = Game::GetGPUDevice();
	SDL_Window* window = Game::GetWindow();

	// Decode & compile shaders
	m_fragShader = CompileDefaultShaderHLSL(device, SpriteBatch_frag_hlsl, SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);
	m_vertShader = CompileDefaultShaderHLSL(device, SpriteBatch_vert_hlsl, SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
	if (!m_fragShader || !m_vertShader) {
		Clear();
		return; 
	}

	// Build pipeline
	SDL_GPUColorTargetDescription colorTargetDescription{};
	colorTargetDescription.format = SDL_GetGPUSwapchainTextureFormat(device, window);
	colorTargetDescription.blend_state = {};
	colorTargetDescription.blend_state.enable_blend = true;
	colorTargetDescription.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
	colorTargetDescription.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
	colorTargetDescription.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colorTargetDescription.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	colorTargetDescription.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colorTargetDescription.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	
	SDL_GPUGraphicsPipelineCreateInfo createInfo{};
	createInfo.vertex_shader = m_vertShader;
	createInfo.fragment_shader = m_fragShader;
	createInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

	createInfo.target_info = {};
	createInfo.target_info.num_color_targets = 1;
	createInfo.target_info.color_target_descriptions = &colorTargetDescription;
	createInfo.target_info.has_depth_stencil_target = true;
	createInfo.target_info.depth_stencil_format = detail::GetDepthStencilFormat(device);
	
	createInfo.depth_stencil_state = {};
	createInfo.depth_stencil_state.front_stencil_state = {};
	createInfo.depth_stencil_state.front_stencil_state.compare_op = SDL_GPU_COMPAREOP_NEVER;
	createInfo.depth_stencil_state.front_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;
	createInfo.depth_stencil_state.front_stencil_state.pass_op = SDL_GPU_STENCILOP_KEEP;
	createInfo.depth_stencil_state.front_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
	createInfo.depth_stencil_state.back_stencil_state = {};
	createInfo.depth_stencil_state.back_stencil_state.compare_op = SDL_GPU_COMPAREOP_NEVER;
	createInfo.depth_stencil_state.back_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;
	createInfo.depth_stencil_state.back_stencil_state.pass_op = SDL_GPU_STENCILOP_KEEP;
	createInfo.depth_stencil_state.back_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
	createInfo.depth_stencil_state.write_mask = 0xFF;
	createInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_GREATER;
	createInfo.depth_stencil_state.enable_depth_test = true;
	createInfo.depth_stencil_state.enable_depth_write = true;
	createInfo.depth_stencil_state.enable_stencil_test = false;

	createInfo.rasterizer_state = {};
	createInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
	createInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	createInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

	m_pipeline = SDL_CreateGPUGraphicsPipeline(device, &createInfo);
	SDL_ReleaseGPUShader(device, m_fragShader);
	SDL_ReleaseGPUShader(device, m_vertShader);
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
	SDL_GPUDevice* device = Game::GetGPUDevice();
	if (m_pipeline) { SDL_ReleaseGPUGraphicsPipeline(device, m_pipeline); }
	if (m_fragShader) { SDL_ReleaseGPUShader(device, m_fragShader); }
	if (m_vertShader) { SDL_ReleaseGPUShader(device, m_vertShader); }
	m_pipeline = nullptr;
	m_fragShader = nullptr;
	m_vertShader = nullptr;
}

bool SpriteBatchShaderPipeline::IsValid() const {
	return (m_pipeline);
}

PrimitiveBatchShaderPipeline::PrimitiveBatchShaderPipeline(bool wireframe) {
	SDL_GPUDevice* device = Game::GetGPUDevice();
	SDL_Window* window = Game::GetWindow();

	// Decode & compile shaders
	m_fragShader = CompileDefaultShaderHLSL(device, PrimitiveBatch_frag_hlsl, SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);
	m_vertShader = CompileDefaultShaderHLSL(device, PrimitiveBatch_vert_hlsl, SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
	if (!m_fragShader || !m_vertShader) {
		Clear();
		return;
	}

	// Build pipeline
	SDL_GPUColorTargetDescription colorTargetDescription{};
	colorTargetDescription.format = SDL_GetGPUSwapchainTextureFormat(device, window);
	colorTargetDescription.blend_state = {};
	colorTargetDescription.blend_state.enable_blend = true;
	colorTargetDescription.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
	colorTargetDescription.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
	colorTargetDescription.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colorTargetDescription.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	colorTargetDescription.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colorTargetDescription.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

	SDL_GPUGraphicsPipelineCreateInfo createInfo{};
	createInfo.vertex_shader = m_vertShader;
	createInfo.fragment_shader = m_fragShader;
	createInfo.primitive_type = (wireframe) ? SDL_GPU_PRIMITIVETYPE_LINELIST : SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

	createInfo.target_info = {};
	createInfo.target_info.num_color_targets = 1;
	createInfo.target_info.color_target_descriptions = &colorTargetDescription;
	createInfo.target_info.has_depth_stencil_target = true;
	createInfo.target_info.depth_stencil_format = detail::GetDepthStencilFormat(device);

	createInfo.depth_stencil_state = {};
	createInfo.depth_stencil_state.front_stencil_state = {};
	createInfo.depth_stencil_state.front_stencil_state.compare_op = SDL_GPU_COMPAREOP_NEVER;
	createInfo.depth_stencil_state.front_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;
	createInfo.depth_stencil_state.front_stencil_state.pass_op = SDL_GPU_STENCILOP_KEEP;
	createInfo.depth_stencil_state.front_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
	createInfo.depth_stencil_state.back_stencil_state = {};
	createInfo.depth_stencil_state.back_stencil_state.compare_op = SDL_GPU_COMPAREOP_NEVER;
	createInfo.depth_stencil_state.back_stencil_state.fail_op = SDL_GPU_STENCILOP_REPLACE;
	createInfo.depth_stencil_state.back_stencil_state.pass_op = SDL_GPU_STENCILOP_KEEP;
	createInfo.depth_stencil_state.back_stencil_state.depth_fail_op = SDL_GPU_STENCILOP_KEEP;
	createInfo.depth_stencil_state.write_mask = 0xFF;
	createInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_GREATER;
	createInfo.depth_stencil_state.enable_depth_test = true;
	createInfo.depth_stencil_state.enable_depth_write = true;
	createInfo.depth_stencil_state.enable_stencil_test = false;

	createInfo.rasterizer_state = {};
	createInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
	createInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	createInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

	SDL_GPUVertexBufferDescription vertexBufferDescription = {};
	vertexBufferDescription.slot = 0;
	vertexBufferDescription.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vertexBufferDescription.instance_step_rate = 0;
	vertexBufferDescription.pitch = sizeof(VertexPosColor);

	SDL_GPUVertexAttribute vertexAttributes[2] = {};
	vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertexAttributes[0].buffer_slot = 0;
	vertexAttributes[0].location = 0;
	vertexAttributes[0].offset = 0;
	vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	vertexAttributes[1].buffer_slot = 0;
	vertexAttributes[1].location = 1;
	vertexAttributes[1].offset = sizeof(float) * 3;

	createInfo.vertex_input_state = {};
	createInfo.vertex_input_state.num_vertex_buffers = 1;
	createInfo.vertex_input_state.vertex_buffer_descriptions = &vertexBufferDescription;
	createInfo.vertex_input_state.num_vertex_attributes = 2;
	createInfo.vertex_input_state.vertex_attributes = &vertexAttributes[0];

	m_pipeline = SDL_CreateGPUGraphicsPipeline(device, &createInfo);
	SDL_ReleaseGPUShader(device, m_fragShader);
	SDL_ReleaseGPUShader(device, m_vertShader);
	m_fragShader = nullptr;
	m_vertShader = nullptr;
}

PrimitiveBatchShaderPipeline::~PrimitiveBatchShaderPipeline() {
	Clear();
}

SDL_GPUGraphicsPipeline* PrimitiveBatchShaderPipeline::GetPipeline() const {
	return m_pipeline;
}

void PrimitiveBatchShaderPipeline::Clear() {
	SDL_GPUDevice* device = Game::GetGPUDevice();
	if (m_pipeline) { SDL_ReleaseGPUGraphicsPipeline(device, m_pipeline); }
	if (m_fragShader) { SDL_ReleaseGPUShader(device, m_fragShader); }
	if (m_vertShader) { SDL_ReleaseGPUShader(device, m_vertShader); }
	m_pipeline = nullptr;
	m_fragShader = nullptr;
	m_vertShader = nullptr;
}

bool PrimitiveBatchShaderPipeline::IsValid() const {
	return (m_pipeline);
}

} // luna