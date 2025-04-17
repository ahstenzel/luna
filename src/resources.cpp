#include "resources.hpp"

std::string ResourceBase::ErrorMessage() const {
	return m_errorMessage;
}

void ResourceTexture::Load(ResourceFile* file, const Buffer& block) {
	m_errorMessage.clear();
	m_texturePage = nullptr;

	try {
		// Validate
		if (!file) {
			throw std::exception("Invalid resource file reference");
		}

		// Get header data
		std::uint32_t headerCRC = block.get_uint32(4);
		std::string headerName = block.get_string(16, 32);
		std::uint64_t headerUncompressedSize = block.get_uint64(64);
		std::uint64_t headerCompressedSize = block.get_uint64(72);

		// Decompress data
		Buffer assetData = block.get_chunk(80, headerCompressedSize);
		if (headerUncompressedSize != headerCompressedSize) {
			Buffer uncompressedAssetData(headerUncompressedSize, 0);
			if (LZ4_decompress_safe((char*)assetData.data(), (char*)uncompressedAssetData.data(), (int)headerCompressedSize, (int)headerUncompressedSize) != headerUncompressedSize) {
				std::stringstream msg;
				msg << "Failed to decompress asset (" << headerName << ")";
				throw std::exception(msg.str().c_str());
			}
			using detail::swap;
			swap(assetData, uncompressedAssetData);
		}

		// Verify CRC
		std::uint32_t dataCRC = Crc32Calculate(assetData.data(), headerUncompressedSize);
		if (headerCRC != dataCRC) {
			std::stringstream msg;
			msg << "Failed to decode asset (" << headerName << ")";
			throw std::exception(msg.str().c_str());
		}

		// Validate texture page
		m_texturePageIndex = assetData.get_uint32(0);
		m_texturePage = file->GetTexturePage(m_texturePageIndex);
		if (!m_texturePage) { 
			std::stringstream msg;
			msg << "Invalid texture page (" << m_texturePageIndex << ")";
			throw std::exception(msg.str().c_str());
		}

		// Get other data
		m_texturePageXOffset = assetData.get_uint32(4);
		m_texturePageYOffset = assetData.get_uint32(8);
		m_texturePageWidth = assetData.get_uint32(12);
		m_texturePageHeight = assetData.get_uint32(16);
		m_animationFrameCount = assetData.get_uint32(20);
		m_animationFramesPerRow = assetData.get_uint32(24);
		m_animationFrameRows = assetData.get_uint32(28);
		m_animationXOffset = assetData.get_uint32(32);
		m_animationYOffset = assetData.get_uint32(36);
		m_animationXSpacing = assetData.get_uint32(40);
		m_animationYSpacing = assetData.get_uint32(44);

		// Calculate size
		std::uint32_t cols = (std::uint32_t)std::ceilf((float)m_animationFrameCount / m_animationFramesPerRow);
		m_frameWidth = ((m_texturePageWidth - m_animationXOffset) / cols) - m_animationXSpacing;
		m_frameHeight = ((m_texturePageHeight - m_animationYOffset) / m_animationFramesPerRow) - m_animationYSpacing;
	}
	catch (std::exception& e) {
		m_errorMessage = e.what();
		m_texturePage = nullptr;
	}
}

bool ResourceTexture::IsValid() const {
	return m_texturePage != nullptr;
}

std::uint32_t ResourceTexture::GetWidth() const {
	return m_frameWidth;
}

std::uint32_t ResourceTexture::GetHeight() const {
	return m_frameHeight;
}

std::uint32_t ResourceTexture::GetNumFrames() const {
	return m_animationFrameCount;
}

std::size_t ResourceTexture::GetTexturePageIndex() const {
	return m_texturePageIndex;
}

void ResourceSound::Load(ResourceFile* file, const Buffer& block) {

}

void ResourceMesh::Load(ResourceFile* file, const Buffer& block) {

}

void ResourceText::Load(ResourceFile* file, const Buffer& block) {

}

void ResourceBinary::Load(ResourceFile* file, const Buffer& block) {

}

bool TexturePage::IsValid() const {
	return !m_name.empty();
}

std::string TexturePage::ErrorMessage() const {
	return m_errorMessage;
}

std::string TexturePage::GetName() const {
	return m_name;
}

Buffer TexturePage::GetBuffer() const {
	return m_buffer;
}

SDL_PixelFormat TexturePage::GetFormat() const {
	return m_format;
}

SDL_Color TexturePage::GetPixel(unsigned int x, unsigned int y) const {
	SDL_Color color = { 0 };
	if (x >= m_width || y >= m_height) { return color; }
	std::uint32_t bpp = SDL_BYTESPERPIXEL(m_format);
	std::uint32_t index = (y * m_width) + x;
	std::uint32_t pixelValue = m_buffer.get_uint32((std::size_t)(index * bpp));
	// TODO indexed format support
	//const SDL_Palette* palette = SDL_ISPIXELFORMAT_INDEXED(m_format) ? &m_palette : NULL;
	const SDL_Palette* palette = NULL;
	SDL_GetRGBA(pixelValue, SDL_GetPixelFormatDetails(m_format), palette, &color.r, &color.g, &color.b, &color.a);
	return color;
}

void TexturePage::Load(ResourceFile* file, const Buffer& block) {
	SDL_UNUSED(file);
	m_errorMessage.clear();
	m_name.clear();

	// Header
	std::string headerName = block.get_string(0, 32);
	std::uint64_t headerUncompressedSize = block.get_uint64(32);
	std::uint64_t headerCompressedSize = block.get_uint64(40);
	std::uint32_t headerCrc = block.get_uint32(48);
	std::uint32_t headerFormat = block.get_uint32(52);
	std::uint32_t headerWidth = block.get_uint32(56);
	std::uint32_t headerHeight = block.get_uint32(60);

	// Decompress data
	Buffer imageData = block.get_chunk(64, headerCompressedSize);
	if (headerCompressedSize != headerUncompressedSize) {
		Buffer uncompressedData(headerUncompressedSize, 0);
		if (LZ4_decompress_safe((char*)imageData.data(), (char*)uncompressedData.data(), (int)headerCompressedSize, (int)headerUncompressedSize) != (int)headerUncompressedSize) {
			std::stringstream msg;
			msg << "Failed to decompress texture page (" << headerName << ")";
			m_errorMessage = msg.str();
			return;
		}
		using detail::swap;
		swap(imageData, uncompressedData);
	}

	// Verify data
	std::uint32_t dataCRC = Crc32Calculate(imageData.data(), headerUncompressedSize);
	if (headerCrc != dataCRC) {
		std::stringstream msg;
		msg << "Failed to decode texture page" << headerName << ")";
		m_errorMessage = msg.str();
		return;
	}

	// Save data
	m_name = headerName;
	m_format = SDL_PixelFormat(headerFormat);
	m_width = headerWidth;
	m_height = headerHeight;
	m_buffer = imageData;
}

ResourceFile::ResourceFile(const std::string& filename, const std::string& password) :
	m_filename(filename) {
	m_errorMessage.clear();

	try {
		// Open file & read contents
		std::ifstream file(filename, std::ios::in | std::ios::ate | std::ios::binary);
		if (!file.is_open()) { throw std::exception("Failed to open file"); }
		std::size_t fileSize = file.tellg();
		Buffer fileBuffer(fileSize, 0);
		file.seekg(0, std::ios::beg);
		file.read((char*)fileBuffer.data(), fileSize);
		file.close();
		if (fileBuffer.empty()) { throw std::exception("Empty file"); }

		// Parse signature
		std::string headerSignature = fileBuffer.get_string(0, 4);
		if (headerSignature != "ARCF") { throw std::exception("Incompatible file format"); }

		// Parse version
		uint8_t headerVersionMajor = fileBuffer.get_uint8(4);
		uint8_t headerVersionMinor = fileBuffer.get_uint8(5);
		uint8_t headerVersionPatch = fileBuffer.get_uint8(6);
		std::stringstream msg;
		msg << std::to_string(headerVersionMajor) << "." << std::to_string(headerVersionMinor) << "." << std::to_string(headerVersionPatch);
		std::string headerVersion = msg.str();
		if (!VersionStringMatch(headerVersion, APOLLO_VERSION_STR)) { throw std::exception("Outdated file format"); }

		// Decode file
		bool encoded = false;
		std::string headerAES = fileBuffer.get_string(16, 32);
		for (char c : headerAES) {
			if (c != 0) {
				encoded = true;
				break;
			}
		}
		if (encoded) {
			// Pad out password to 32 characters
			uint8_t key[32] = { 0 };
			for (size_t i = 0; i < password.size(); ++i) { key[i] = (uint8_t)password[i]; }
			AES_ctx ctx;
			AES_init_ctx_iv(&ctx, &key[0], (uint8_t*)headerAES.data());
			AES_CBC_decrypt_buffer(&ctx, fileBuffer.data(48), fileBuffer.size() - 48);
		}

		// Parse CRC
		std::uint32_t headerCRC = fileBuffer.get_uint32(8);
		std::uint32_t fileCRC = Crc32Calculate(fileBuffer.data(48), fileBuffer.size() - 48);
		if (headerCRC != fileCRC) { throw std::exception("Invalid CRC"); }
		std::uint64_t offsetTexturePages = fileBuffer.get_uint64(48); 
		std::uint64_t offsetDataChunks = fileBuffer.get_uint64(56);
		std::uint64_t offsetAssetTable = fileBuffer.get_uint64(64);

		// Read texture pages
		std::string textureHeaderSignature = fileBuffer.get_string(offsetTexturePages, 4);
		if (textureHeaderSignature != "ATXG") { throw std::exception("Invalid texture page format"); }
		std::uint32_t textureHeaderPageCount = fileBuffer.get_uint32(offsetTexturePages + 4);
		std::uint64_t textureHeaderPageStride = fileBuffer.get_uint64(offsetTexturePages + 8);
		for (std::uint64_t pageNum = 0; pageNum < textureHeaderPageCount; ++pageNum) {
			std::uint64_t pageOffset = offsetTexturePages + 16 + (pageNum * textureHeaderPageStride);
			TexturePage page;
			page.Load(this, fileBuffer.get_chunk(pageOffset, textureHeaderPageStride));
			if (!page.IsValid()) {
				std::stringstream msg;
				msg << "Failed to initialize texture page; " << page.ErrorMessage();
				throw std::exception(msg.str().c_str());
			}
			m_texturePages.push_back(std::move(page));
		}

		// Read resources
		std::string assetTableSignature = fileBuffer.get_string(offsetAssetTable, 4);
		if (assetTableSignature != "ARFT") { throw std::exception("Invalid file table format"); }
		std::uint32_t assetTableCount = fileBuffer.get_uint32(offsetAssetTable + 4);
		std::uint32_t assetTableCapacity = fileBuffer.get_uint32(offsetAssetTable + 8);
		Buffer assetTableCtrlBlock = fileBuffer.get_chunk(offsetAssetTable + 16, assetTableCapacity);
		for (std::size_t i = 0; i < assetTableCapacity; ++i) {
			// Iterate through control bytes
			std::uint8_t ctrl = assetTableCtrlBlock.get_uint8(i);
			if (ctrl & 0x80) {
				// Get asset data
				std::uint64_t offsetAssetBucket = offsetAssetTable + 16 + assetTableCapacity + (i * 40);
				std::uint64_t offsetAsset = fileBuffer.get_uint64(offsetAssetBucket + 32);
				std::string assetType = fileBuffer.get_string(offsetAsset, 4);
				std::string assetName = fileBuffer.get_string(offsetAsset + 16, 32);
				std::uint64_t assetCompressedSize = fileBuffer.get_uint64(offsetAsset + 72);

				// Create resource
				Buffer assetData = fileBuffer.get_chunk(offsetAsset, 80 + assetCompressedSize);
				if (assetType == "AIMG") {
					ResourceTexture assetTexture;
					assetTexture.Load(this, assetData);
					if (!assetTexture.IsValid()) {
						std::stringstream msg;
						msg << "Failed to initialize texture (" << assetName << "); " << assetTexture.ErrorMessage();
						throw std::exception(msg.str().c_str());
					}
					m_resourceMap.insert(std::make_pair(assetName, m_textures.size()));
					m_textures.push_back(std::move(assetTexture));
				}
				else {
					std::stringstream msg;
					msg << "Unknown asset type (" << assetType << ")";
					throw std::exception(msg.str().c_str());
				}
			}
		}
	}
	catch (std::exception& e) {
		m_errorMessage = std::string(e.what());
		m_filename.clear();
	}
}

std::string ResourceFile::GetFilename() const {
	return m_filename;
}

const TexturePage* ResourceFile::GetTexturePage(std::size_t index) const {
	return (index < m_texturePages.size()) ? &m_texturePages[index] : nullptr;
}

std::size_t ResourceFile::GetTexturePageCount() const {
	return m_textures.size();
}

const ResourceTexture* ResourceFile::GetTexture(const std::string& name) const {
	auto it = m_resourceMap.find(name);
	return it == m_resourceMap.end() ? nullptr : &m_textures[it->second];
}

std::size_t ResourceFile::GetTextureCount() const {
	return m_texturePages.size();
}

bool ResourceFile::IsValid() const {
	return !m_filename.empty();
}

std::string ResourceFile::ErrorMessage() const {
	return m_errorMessage;
}

std::string ResourceManager::m_errorMessage = "";
ResourceFileID ResourceManager::m_resourceFileIDCounter = 0;
std::unordered_map<ResourceFileID, ResourceFile> ResourceManager::m_resourceFiles = {};

ResourceFileID ResourceManager::LoadResourceFile(const std::string& filename, const std::string& password) {
	m_errorMessage.clear();

	// Check if the file is already loaded
	for (auto& pair : m_resourceFiles) {
		if (pair.second.GetFilename() == filename) {
			return pair.first;
		}
	}

	// Create the resource file & check if it initialzed
	//ResourceFile resourceFile(filename);
	auto success = m_resourceFiles.emplace(std::make_pair(++m_resourceFileIDCounter, ResourceFile(filename)));
	if (success.second) {
		if (success.first->second.IsValid()) {
			return m_resourceFileIDCounter;
		}
		else {
			std::stringstream msg;
			msg << "Failed to load resource file (" << filename << "); " << success.first->second.ErrorMessage();
			m_errorMessage = msg.str();
			m_resourceFiles.erase(m_resourceFileIDCounter);
			return RESOURCE_FILE_NULL;
		}
	}
	else {
		std::stringstream msg;
		msg << "Failed to load resource file (" << filename << "); Could not create object";
		m_errorMessage = msg.str();
		return RESOURCE_FILE_NULL;
	}
}

void ResourceManager::UnloadResourceFile(ResourceFileID resourceFileID) {
	m_errorMessage.clear();
	m_resourceFiles.erase(resourceFileID);
}

bool ResourceManager::ResourceFileExists(ResourceFileID resourceFileID) {
	m_errorMessage.clear();
	auto file = GetResourceFile(resourceFileID);
	return (file != nullptr && file->IsValid());
}

const TexturePage* ResourceManager::GetTexturePage(ResourceFileID resourceFileID, std::size_t index) {
	m_errorMessage.clear();
	auto file = GetResourceFile(resourceFileID);
	if (!file) {
		m_errorMessage = "Failed to find resource file";
		return nullptr;
	}
	if (!file->IsValid()) {
		m_errorMessage = "Resource file is invalid";
		return nullptr;
	}
	auto texturePage = file->GetTexturePage(index);
	if (!texturePage) {
		m_errorMessage = "Failed to find texture page";
		return nullptr;
	}
	return texturePage;
}

const ResourceTexture* ResourceManager::GetTexture(ResourceFileID resourceFileID, const std::string& name) {
	m_errorMessage.clear();
	auto file = GetResourceFile(resourceFileID);
	if (!file) {
		m_errorMessage = "Failed to find resource file";
		return nullptr;
	}
	if (!file->IsValid()) {
		m_errorMessage = "Resource file is invalid";
		return nullptr;
	}
	auto texture = file->GetTexture(name);
	if (!texture) {
		m_errorMessage = "Failed to find texture";
		return nullptr;
	}
	return texture;
}

ResourceFile* ResourceManager::GetResourceFile(ResourceFileID resourceFileID) {
	auto it = m_resourceFiles.find(resourceFileID);
	return it == m_resourceFiles.end() ? nullptr : &it->second;
}

std::string ResourceManager::ErrorMessage() {
	return m_errorMessage;
}
