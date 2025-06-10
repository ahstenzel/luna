#include <luna/detail/sprite.hpp>

namespace luna {

Sprite::Sprite() :
	m_textureID(RESOURCE_ID_NULL),
	m_positionX(0.f),
	m_positionY(0.f),
	m_animationSpeed(0.f),
	m_depth(0),
	m_scaleX(1.f),
	m_scaleY(1.f),
	m_rotation(0.f),
	m_blend(LunaColorClear) {
	m_texture = nullptr;
	m_width = 0.f;
	m_height = 0.f;
	m_animationFrame = 0.f;
	m_textureU = 0.f;
	m_textureV = 0.f;
	m_textureW = 0.f;
	m_textureH = 0.f;
	m_originX = 0;
	m_originY = 0;
}

Sprite::Sprite(const Sprite& sprite) :
	m_textureID(sprite.m_textureID),
	m_positionX(sprite.m_positionX),
	m_positionY(sprite.m_positionY),
	m_animationSpeed(sprite.m_animationSpeed),
	m_depth(sprite.m_depth),
	m_scaleX(sprite.m_scaleX),
	m_scaleY(sprite.m_scaleY),
	m_rotation(sprite.m_rotation),
	m_blend(sprite.m_blend) {
	m_texture = sprite.m_texture;
	m_width = sprite.m_width;
	m_height = sprite.m_height;
	m_animationFrame = sprite.m_animationFrame;
	m_textureU = sprite.m_textureU;
	m_textureV = sprite.m_textureV;
	m_textureW = sprite.m_textureW;
	m_textureH = sprite.m_textureH;
	m_originX = sprite.m_originX;
	m_originY = sprite.m_originY;
}

Sprite::Sprite(Sprite&& sprite) noexcept :
	m_textureID(std::move(sprite.m_textureID)),
	m_positionX(std::move(sprite.m_positionX)),
	m_positionY(std::move(sprite.m_positionY)),
	m_animationSpeed(std::move(sprite.m_animationSpeed)),
	m_depth(std::move(sprite.m_depth)),
	m_scaleX(std::move(sprite.m_scaleX)),
	m_scaleY(std::move(sprite.m_scaleY)),
	m_rotation(std::move(sprite.m_rotation)),
	m_blend(std::move(sprite.m_blend)) {
	std::swap(m_texture, sprite.m_texture);
	std::swap(m_width, sprite.m_width);
	std::swap(m_height, sprite.m_height);
	std::swap(m_animationFrame, sprite.m_animationFrame);
	std::swap(m_textureU, sprite.m_textureU);
	std::swap(m_textureV, sprite.m_textureV);
	std::swap(m_textureW, sprite.m_textureW);
	std::swap(m_textureH, sprite.m_textureH);
	std::swap(m_originX, sprite.m_originX);
	std::swap(m_originY, sprite.m_originY);
}

Sprite::Sprite(const ResourceID textureID, float x, float y, int32_t image, float imageSpeed, int32_t depth, float scaleX, float scaleY, float rotation, SDL_Color blend) :
	m_textureID(textureID),
	m_positionX(x),
	m_positionY(y),
	m_animationSpeed(imageSpeed),
	m_depth(depth),
	m_scaleX(scaleX),
	m_scaleY(scaleY),
	m_rotation(rotation),
	m_blend(blend) {
	// Verify texture
	m_texture = ResourceManager::GetTexture(m_textureID);
	if (!m_texture) {
		m_textureID = RESOURCE_ID_NULL;
		m_width = 0.f;
		m_height = 0.f;
		m_originX = 0;
		m_originY = 0;
		m_animationFrame = 0.f;
		return;
	}
	m_width = float(m_texture->GetWidth());
	m_height = float(m_texture->GetHeight());
	m_originX = float(m_texture->GetOriginX());
	m_originY = float(m_texture->GetOriginY());

	// Calculate sprite properties
	m_animationFrame = float(Wrap(image, 0, std::int32_t(m_texture->GetNumFrames())));
	CalculateUVs();
}

bool Sprite::IsValid() const {
	return m_textureID != RESOURCE_ID_NULL;
}

float Sprite::GetPositionX() const {
	return m_positionX;
}

float Sprite::GetPositionY() const {
	return m_positionY;
}

float Sprite::GetScaleX() const {
	return m_scaleX;
}

float Sprite::GetScaleY() const {
	return m_scaleY;
}

float Sprite::GetRotation() const {
	return m_rotation;
}

float Sprite::GetWidth() const {
	return m_width;
}

float Sprite::GetHeight() const {
	return m_height;
}

int32_t Sprite::GetImage() const {
	return (std::int32_t)std::floorf(m_animationFrame);
}

int32_t Sprite::GetNumImages() const {
	return (m_texture) ? (std::int32_t)m_texture->GetNumFrames() : -1;
}

int32_t Sprite::GetDepth() const {
	return m_depth;
}

ResourceID Sprite::GetTextureID() const {
	return m_textureID;
}

SpriteTextureCoords Sprite::GetTextureCoords() const {
	SpriteTextureCoords coords = {};
	coords.textureU = m_textureU;
	coords.textureV = m_textureV;
	coords.textureW = m_textureW;
	coords.textureH = m_textureH;
	return coords;
}

TexturePageID Sprite::GetTexturePageID() const {
	auto resourceFile = ResourceManager::GetResourceFile(m_texture->GetFileID());
	return m_texture->GetTexturePageID();
}

const TexturePage* Sprite::GetTexturePage() const {
	auto resourceFile = ResourceManager::GetResourceFile(m_texture->GetFileID());
	auto texturePage = resourceFile->GetTexturePage(m_texture->GetTexturePageID());
	return texturePage;
}

SDL_Color Sprite::GetBlend() const {
	return m_blend;
}

float Sprite::GetAlpha() const {
	return m_blend.a / 255.f;
}

float Sprite::GetOriginX() const {
	return m_originX;
}

float Sprite::GetOriginY() const {
	return m_originY;
}

bool Sprite::GetTranslucent() const {
	return (
		(m_blend.a > 0 && m_blend.a < 255) ||
		m_texture->GetProperties() & 0x01
	);
}

void Sprite::SetPositionX(float x) {
	m_positionX = x;
}

void Sprite::SetPositionY(float y) {
	m_positionY = y;
}

void Sprite::SetScaleX(float scaleX) {
	m_scaleX = scaleX;
}

void Sprite::SetScaleY(float scaleY) {
	m_scaleY = scaleY;
}

void Sprite::SetRotation(float rotation) {
	m_rotation = rotation;
}

void Sprite::SetImage(int32_t image) {
	m_animationFrame = (float)(image);
}

void Sprite::SetDepth(int32_t depth) {
	m_depth = depth;
}

void Sprite::SetBlend(const SDL_Color& blend) {
	m_blend = blend;
}

void Sprite::SetAlpha(float alpha) {
	alpha = std::clamp(alpha, 0.f, 1.f);
	m_blend.a = Uint8(alpha * 255.f);
}

void Sprite::SetOriginX(std::int32_t originX) {
	m_originX = originX;
}

void Sprite::SetOriginY(std::int32_t originY) {
	m_originY = originY;
}

Sprite& Sprite::operator=(const Sprite& other) {
	if (this == &other) { return *this; }
	m_textureID = other.m_textureID;
	m_positionX = other.m_positionX;
	m_positionY = other.m_positionY;
	m_animationSpeed = other.m_animationSpeed;
	m_depth = other.m_depth;
	m_scaleX = other.m_scaleX;
	m_scaleY = other.m_scaleY;
	m_rotation = other.m_rotation;
	m_blend = other.m_blend;
	m_texture = other.m_texture;
	m_width = other.m_width;
	m_height = other.m_height;
	m_animationFrame = other.m_animationFrame;
	m_textureU = other.m_textureU;
	m_textureV = other.m_textureV;
	m_textureW = other.m_textureW;
	m_textureH = other.m_textureH;
	m_originX = other.m_originX;
	m_originY = other.m_originY;
	return *this;
}

Sprite& Sprite::operator=(Sprite&& other) noexcept {
	if (this == &other) { return *this; }
	std::swap(m_textureID, other.m_textureID);
	std::swap(m_positionX, other.m_positionX);
	std::swap(m_positionY, other.m_positionY);
	std::swap(m_animationSpeed, other.m_animationSpeed);
	std::swap(m_depth, other.m_depth);
	std::swap(m_scaleX, other.m_scaleX);
	std::swap(m_scaleY, other.m_scaleY);
	std::swap(m_rotation, other.m_rotation);
	std::swap(m_blend, other.m_blend);
	std::swap(m_texture, other.m_texture);
	std::swap(m_width, other.m_width);
	std::swap(m_height, other.m_height);
	std::swap(m_animationFrame, other.m_animationFrame);
	std::swap(m_textureU, other.m_textureU);
	std::swap(m_textureV, other.m_textureV);
	std::swap(m_textureW, other.m_textureW);
	std::swap(m_textureH, other.m_textureH);
	std::swap(m_originX, other.m_originX);
	std::swap(m_originY, other.m_originY);
	return *this;
}

bool Sprite::operator==(const Sprite& other) const {
	return (
		m_textureID == other.m_textureID &&
		m_positionX == other.m_positionX &&
		m_positionY == other.m_positionY &&
		m_animationSpeed == other.m_animationSpeed &&
		m_depth == other.m_depth &&
		m_scaleX == other.m_scaleX &&
		m_scaleY == other.m_scaleY &&
		m_rotation == other.m_rotation &&
		m_blend.r == other.m_blend.r &&
		m_blend.g == other.m_blend.g &&
		m_blend.b == other.m_blend.b &&
		m_blend.a == other.m_blend.a &&
		m_texture == other.m_texture &&
		m_width == other.m_width &&
		m_height == other.m_height &&
		m_animationFrame == other.m_animationFrame &&
		m_textureU == other.m_textureU &&
		m_textureV == other.m_textureV &&
		m_textureW == other.m_textureW &&
		m_textureH == other.m_textureH &&
		m_originX == other.m_originX &&
		m_originY == other.m_originY
	);
}

bool Sprite::Tick(float dt) {
	// Check if sprite is no longer valid
	auto texture = ResourceManager::GetTexture(m_textureID);
	if (!texture) { m_textureID = RESOURCE_ID_NULL; }
	if (!IsValid()) { return false; }

	// Advance animation
	std::uint32_t currFrame = (std::uint32_t)std::floorf(m_animationFrame);
	m_animationFrame += dt * m_animationSpeed;
	std::uint32_t newFrame = (std::uint32_t)std::floorf(m_animationFrame);
	if (currFrame != newFrame) { CalculateUVs(); }

	return true;
}

void Sprite::CalculateUVs() {
	std::uint32_t currFrame = (std::uint32_t)std::floorf(m_animationFrame);
	auto texturePage = GetTexturePage();
	float pageWidth = (float)texturePage->GetWidth();
	float pageHeight = (float)texturePage->GetHeight();
	m_textureW = (float)m_texture->GetWidth() / pageWidth;
	m_textureH = (float)m_texture->GetHeight() / pageHeight;
	m_textureU = (float)m_texture->GetOffsetX(currFrame) / pageWidth;
	m_textureV = (float)m_texture->GetOffsetY(currFrame) / pageHeight;
}

bool SpriteListTexturePageComp::operator()(const Sprite* lhs, const Sprite* rhs) {
	return lhs->GetTexturePageID() < rhs->GetTexturePageID();
}

bool SpriteListDepthComp::operator()(const Sprite* lhs, const Sprite* rhs) {
	return lhs->GetDepth() < rhs->GetDepth();
}

} // luna