#pragma once

#include <luna/detail/common.hpp>

namespace luna {

typedef std::uint32_t ResourceID;
constexpr ResourceID RESOURCE_ID_NULL = 0;
typedef std::int32_t TexturePageID;
constexpr TexturePageID TEXTURE_PAGE_ID_NULL = -1;

// Forward declarations
class ResourceFile;
class TexturePage;

/// <summary>
/// Base class for all types of resources.
/// </summary>
class ResourceBase {
public:
	LUNA_API ResourceID GetID() const;
	LUNA_API ResourceID GetFileID() const;

protected:
	void SetID(ResourceID id);
	void SetFileID(ResourceID id);
	virtual void Load(ResourceFile* file, const Buffer& block) = 0;
	virtual bool IsValid() const = 0;
	std::string ErrorMessage() const;
	std::string m_errorMessage = "";
	ResourceID m_resourceID = RESOURCE_ID_NULL;
	ResourceID m_resourceFileID = RESOURCE_ID_NULL;
};

/// <summary>
/// Resource class representing a texture in a page.
/// </summary>
class ResourceTexture : public ResourceBase {
public:
	LUNA_API ResourceTexture() = default;

	LUNA_API bool IsValid() const override;

	LUNA_API std::uint32_t GetWidth() const;
	LUNA_API std::uint32_t GetHeight() const;
	LUNA_API std::uint32_t GetNumFrames() const;
	LUNA_API TexturePageID GetTexturePageID() const;
	LUNA_API std::uint32_t GetOffsetX(std::int32_t animationFrame = -1) const;
	LUNA_API std::uint32_t GetOffsetY(std::int32_t animationFrame = -1) const;
	LUNA_API std::int32_t GetOriginX() const;
	LUNA_API std::int32_t GetOriginY() const;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;

private:
	const TexturePage* m_texturePage = nullptr;
	TexturePageID m_texturePageID = 0;
	std::uint32_t m_texturePageXOffset = 0;
	std::uint32_t m_texturePageYOffset = 0;
	std::uint32_t m_texturePageWidth = 0;
	std::uint32_t m_texturePageHeight = 0;
	std::uint32_t m_animationFrameCount = 1;
	std::uint32_t m_animationFramesPerRow = 1;
	std::uint32_t m_animationFrameRows = 1;
	std::uint32_t m_animationXOffset = 0;
	std::uint32_t m_animationYOffset = 0;
	std::uint32_t m_animationXSpacing = 0;
	std::uint32_t m_animationYSpacing = 0;
	std::uint32_t m_frameWidth = 0;
	std::uint32_t m_frameHeight = 0;
	std::int32_t m_originX = 0;
	std::int32_t m_originY = 0;
};

/// <summary>
/// Resource class representing a sound file.
/// </summary>
class ResourceSound : public ResourceBase {
public:
	LUNA_API ResourceSound() = default;

	LUNA_API bool IsValid() const override;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;
};

/// <summary>
/// Resource class representing a 3D model file.
/// </summary>
class ResourceMesh : public ResourceBase {
public:
	LUNA_API ResourceMesh() = default;

	LUNA_API bool IsValid() const override;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;
};

/// <summary>
/// Resource class representing a text file.
/// </summary>
class ResourceText : public ResourceBase {
public:
	LUNA_API ResourceText() = default;

	LUNA_API bool IsValid() const override;
	LUNA_API std::string GetContents() const;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;

private:
	std::string m_contents = "";
};

/// <summary>
/// Resource class representing a binary file.
/// </summary>
class ResourceBinary : public ResourceBase {
public:
	LUNA_API ResourceBinary() = default;

	LUNA_API bool IsValid() const override;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;
};

/// <summary>
/// Resource class representing a complete texture page.
/// </summary>
class TexturePage {
public:
	LUNA_API TexturePage() = default;

	LUNA_API bool IsValid() const;
	LUNA_API std::string ErrorMessage() const;

	LUNA_API std::string GetName() const;
	LUNA_API std::uint8_t* GetData() const;
	LUNA_API SDL_PixelFormat GetFormat() const;
	LUNA_API SDL_Color GetPixel(unsigned int x, unsigned int y) const;
	LUNA_API std::uint32_t GetWidth() const;
	LUNA_API std::uint32_t GetHeight() const;
	LUNA_API bool WriteToFile(std::filesystem::path outputFile = "") const;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block);

private:
	ResourceID m_resourceFileID = RESOURCE_ID_NULL;
	std::string m_errorMessage = "";
	std::string m_name = "";
	std::uint32_t m_width = 0;
	std::uint32_t m_height = 0;
	SDL_PixelFormat m_format = SDL_PixelFormat::SDL_PIXELFORMAT_UNKNOWN;
	SDL_Palette m_palette = { 0 };
	Buffer m_buffer;
};

/// <summary>
/// Container for all resources loaded from an ARC file.
/// </summary>
class ResourceFile {
public:
	LUNA_API ResourceFile(ResourceID resourceFileID, const std::string& filename, const std::string& password);

	LUNA_API bool IsValid() const;
	LUNA_API std::string ErrorMessage() const;
	LUNA_API std::string GetFilename() const;
	LUNA_API ResourceID GetID() const;

	LUNA_API TexturePageID GetTexturePageID(const std::string& name) const;
	LUNA_API const TexturePage* GetTexturePage(TexturePageID texturePageID) const;
	LUNA_API std::size_t GetTexturePageCount() const;

	LUNA_API ResourceID GetTextureID(const std::string& name) const;
	LUNA_API const ResourceTexture* GetTexture(ResourceID resourceTextureID) const;
	LUNA_API std::size_t GetTextureCount() const;

private:
	ResourceID m_resourceFileID;
	std::string m_errorMessage;
	std::string m_filename;
	std::unordered_map<std::string, std::size_t> m_resourceNameMap;
	std::unordered_map<ResourceID, std::size_t> m_resourceIDMap;
	std::vector<TexturePage> m_texturePages;
	std::vector<ResourceTexture> m_textures;
};

/// <summary>
/// Static interface for managing resource files.
/// </summary>
class ResourceManager {
public:
	LUNA_API ResourceManager() = delete;
	LUNA_API ~ResourceManager() = delete;

	LUNA_API static std::string ErrorMessage();

	LUNA_API static ResourceID LoadResourceFile(const std::string& filename, const std::string& password = "");
	LUNA_API static ResourceFile* GetResourceFile(ResourceID resourceFileID);
	LUNA_API static void UnloadResourceFile(ResourceID resourceFileID);
	LUNA_API static bool ResourceFileExists(ResourceID resourceFileID);

	LUNA_API static TexturePageID GetTexturePageID(const std::string& name, ResourceID resourceFileID = RESOURCE_ID_NULL);
	LUNA_API static const TexturePage* GetTexturePage(TexturePageID texturePageID, ResourceID resourceFileID = RESOURCE_ID_NULL);

	LUNA_API static ResourceID GetTextureID(const std::string& name, ResourceID resourceFileID = RESOURCE_ID_NULL);
	LUNA_API static const ResourceTexture* GetTexture(ResourceID resourceTextureID, ResourceID resourceFileID = RESOURCE_ID_NULL);

protected:
	friend class ResourceFile;
	static ResourceID GenerateID();

private:
	static std::string m_errorMessage;
	static ResourceID m_resourceIDCounter;
	static std::unordered_map<ResourceID, ResourceFile> m_resourceFiles;
};

} // luna