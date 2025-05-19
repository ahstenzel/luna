#include <luna/detail/render.hpp>

namespace luna {

SpriteRenderer::SpriteRenderer(SDL_GPUDevice* device, SDL_Window* window) {
	m_pipeline = new SpriteBatchShaderPipeline(device, window);
}

SpriteRenderer::~SpriteRenderer() {
	delete m_pipeline;
}

bool SpriteRenderer::IsValid() const {
	return (m_pipeline && m_pipeline->IsValid());
}

} // luna