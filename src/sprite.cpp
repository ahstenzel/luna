#include <luna/detail/sprite.hpp>

namespace luna {

Sprite::Sprite(const ResourceID textureID, float x, float y, int32_t image, int32_t depth, float scaleX, float scaleY, float rotation) :
	m_textureID(textureID),
	m_positionX(x),
	m_positionY(y),
	m_depth(depth),
	m_scaleX(scaleX),
	m_scaleY(scaleY),
	m_rotation(rotation) {
	// Verify texture
	auto texture = ResourceManager::GetTexture(m_textureID);
	if (!texture) {
		m_textureID = RESOURCE_ID_NULL;
		return;
	}

	// Calculate sprite properties
	m_animationFrame = (float)Wrap(image, 0, (std::int32_t)texture->GetNumFrames());
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

int32_t Sprite::GetImage() const {
	return (std::int32_t)std::floorf(m_animationFrame);
}

int32_t Sprite::GetNumImages() const {
	auto texture = ResourceManager::GetTexture(m_textureID);
	if (!texture) { return -1; }
	else { return (std::int32_t)texture->GetNumFrames(); }
}

int32_t Sprite::GetDepth() const {
	return m_depth;
}

ResourceID Sprite::GetTextureID() const {
	return m_textureID;
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
	auto texture = ResourceManager::GetTexture(m_textureID);
	auto resourceFile = ResourceManager::GetResourceFile(texture->GetFileID());
	auto texturePage = resourceFile->GetTexturePage(texture->GetTexturePageIndex());
	float pageWidth = (float)texturePage->GetWidth();
	float pageHeight = (float)texturePage->GetHeight();
	m_textureW = (float)texture->GetWidth() / pageWidth;
	m_textureH = (float)texture->GetHeight() / pageHeight;
	m_textureU = (float)texture->GetXOffset(currFrame) / pageWidth;
	m_textureV = (float)texture->GetYOffset(currFrame) / pageHeight;
}

SpriteID SpriteList::m_spriteIDCounter = SPRITE_ID_NULL;

SpriteList::SpriteList() {
	m_comparator.m_list = &m_sprites;
}

bool SpriteList::SpriteComp::operator()(SpriteID lhs, SpriteID rhs) {
	auto& lhsTexture = m_list->at(lhs);
	auto& rhsTexture = m_list->at(rhs);
	auto lhsTexturePageIndex = ResourceManager::GetTexture(lhsTexture.GetTextureID())->GetTexturePageIndex();
	auto rhsTexturePageIndex = ResourceManager::GetTexture(rhsTexture.GetTextureID())->GetTexturePageIndex();
	return (lhsTexturePageIndex < rhsTexturePageIndex) || (lhsTexturePageIndex == rhsTexturePageIndex && lhsTexture.GetDepth() < rhsTexture.GetDepth());
}

SpriteID SpriteList::AddSprite(const Sprite& sprite) { 
	SpriteID id = SpriteList::GenerateID();
	auto result = m_sprites.insert(std::make_pair(id, sprite));
	if (!result.second) { return SPRITE_ID_NULL; }
	else { result.first->second.SetSpriteList(this); }
	m_spriteOrder.push_back(id);
	m_dirty = true;
	return id;
}

Sprite* SpriteList::GetSprite(SpriteID spriteID) {
	return nullptr;
}

void SpriteList::RemoveSprite(SpriteID spriteID) {
	m_sprites.erase(spriteID);
	m_spriteOrder.erase(std::find(m_spriteOrder.begin(), m_spriteOrder.end(), spriteID));
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