#include <luna/detail/sprite.hpp>

namespace luna {

Sprite::Sprite(const ResourceID textureID, float x, float y, int32_t image, int32_t depth, float scaleX, float scaleY, float rotation, SDL_Color blend) :
	m_textureID(textureID),
	m_positionX(x),
	m_positionY(y),
	m_depth(depth),
	m_scaleX(scaleX),
	m_scaleY(scaleY),
	m_rotation(rotation),
	m_blend(blend) {
	// Verify texture
	m_texture = ResourceManager::GetTexture(m_textureID);
	if (!m_texture) {
		m_textureID = RESOURCE_ID_NULL;
		return;
	}
	m_width = (float)m_texture->GetWidth();
	m_height = (float)m_texture->GetHeight();

	// Calculate sprite properties
	m_animationFrame = (float)Wrap(image, 0, (std::int32_t)m_texture->GetNumFrames());
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
	if (m_spriteList) { m_spriteList->MarkDirty(); }
}

void Sprite::SetBlend(const SDL_Color& blend) {
	m_blend = blend;
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

void Sprite::SetSpriteList(SpriteList* spriteList) {
	m_spriteList = spriteList;
}

void Sprite::CalculateUVs() {
	std::uint32_t currFrame = (std::uint32_t)std::floorf(m_animationFrame);
	auto texturePage = GetTexturePage();
	float pageWidth = (float)texturePage->GetWidth();
	float pageHeight = (float)texturePage->GetHeight();
	m_textureW = (float)m_texture->GetWidth() / pageWidth;
	m_textureH = (float)m_texture->GetHeight() / pageHeight;
	m_textureU = (float)m_texture->GetXOffset(currFrame) / pageWidth;
	m_textureV = (float)m_texture->GetYOffset(currFrame) / pageHeight;
}

SpriteID SpriteList::m_spriteIDCounter = SPRITE_ID_NULL;

SpriteList::SpriteList() {
	m_comparator.m_list = &m_sprites;
}

bool SpriteList::SpriteComp::operator()(SpriteID lhs, SpriteID rhs) {
	auto& lhsTexture = m_list->at(lhs);
	auto& rhsTexture = m_list->at(rhs);
	//auto lhsTexturePageIndex = ResourceManager::GetTexture(lhsTexture.GetTextureID())->GetTexturePageIndex();
	//auto rhsTexturePageIndex = ResourceManager::GetTexture(rhsTexture.GetTextureID())->GetTexturePageIndex();
	//return (lhsTexturePageIndex < rhsTexturePageIndex) || (lhsTexturePageIndex == rhsTexturePageIndex && lhsTexture.GetDepth() < rhsTexture.GetDepth());
	return lhsTexture.GetDepth() < rhsTexture.GetDepth();
}

SpriteID SpriteList::AddSprite(const Sprite& sprite) { 
	SpriteID id = SpriteList::GenerateID();
	auto result = m_sprites.insert(std::make_pair(id, sprite));
	if (!result.second) { return SPRITE_ID_NULL; }
	else { result.first->second.SetSpriteList(this); }
	m_spriteOrder.push_back(id);
	MarkDirty();
	return id;
}

Sprite* SpriteList::GetSprite(SpriteID spriteID) {
	auto pair = m_sprites.find(spriteID);
	return (pair == m_sprites.end()) ? nullptr : &pair->second;
}

SpriteID SpriteList::GetSpriteID(std::size_t idx) {
	if (idx >= m_spriteOrder.size()) { return SPRITE_ID_NULL; }
	return m_spriteOrder[idx];
}

void SpriteList::RemoveSprite(SpriteID spriteID) {
	m_sprites.erase(spriteID);
	m_spriteOrder.erase(std::find(m_spriteOrder.begin(), m_spriteOrder.end(), spriteID));
}

std::uint32_t SpriteList::Size() const {
	return m_spriteOrder.size();
}

void SpriteList::Sort() {
	if (m_dirty) {
		intro_sort(m_spriteOrder.begin(), m_spriteOrder.end(), m_comparator);
	}
}

std::vector<SpriteID>::const_iterator SpriteList::begin() const {
	return m_spriteOrder.begin();
}

std::vector<SpriteID>::const_iterator SpriteList::end() const {
	return m_spriteOrder.end();
}

void SpriteList::MarkDirty() {
	m_dirty = true;
}

void SpriteList::Tick(float dt) {
	for (auto& [key, value] : m_sprites) {
		value.Tick(dt);
	}
}

SpriteID SpriteList::GenerateID() {
	return ++m_spriteIDCounter;
}

} // luna