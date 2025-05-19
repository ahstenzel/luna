#pragma once

#include <luna/detail/common.hpp>

namespace luna {

typedef std::uint32_t ResourceID;
constexpr ResourceID RESOURCE_ID_NULL = 0;

// Forward declarations
class ResourceFile;
class TexturePage;

/// <summary>
/// Base class for all types of resources.
/// </summary>
class ResourceBase {
public:
	ResourceID GetID() const;
	ResourceID GetFileID() const;

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
	ResourceTexture() = default;

	bool IsValid() const override;

	std::uint32_t GetWidth() const;
	std::uint32_t GetHeight() const;
	std::uint32_t GetNumFrames() const;
	std::size_t GetTexturePageIndex() const;
	std::uint32_t GetXOffset(std::int32_t animationFrame = -1) const;
	std::uint32_t GetYOffset(std::int32_t animationFrame = -1) const;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;

private:
	const TexturePage* m_texturePage = nullptr;
	std::size_t m_texturePageIndex = 0;
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
};

/// <summary>
/// Resource class representing a sound file.
/// </summary>
class ResourceSound : public ResourceBase {
public:
	ResourceSound() = default;

	bool IsValid() const override;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;
};

/// <summary>
/// Resource class representing a 3D model file.
/// </summary>
class ResourceMesh : public ResourceBase {
public:
	ResourceMesh() = default;

	bool IsValid() const override;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;
};

/// <summary>
/// Resource class representing a text file.
/// </summary>
class ResourceText : public ResourceBase {
public:
	ResourceText() = default;

	bool IsValid() const override;
	std::string GetContents() const;

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
	ResourceBinary() = default;

	bool IsValid() const override;

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;
};

/// <summary>
/// Resource class representing a complete texture page.
/// </summary>
class TexturePage {
public:
	TexturePage() = default;

	bool IsValid() const;
	std::string ErrorMessage() const;

	std::string GetName() const;
	Buffer GetBuffer() const;
	SDL_PixelFormat GetFormat() const;
	SDL_Color GetPixel(unsigned int x, unsigned int y) const;
	std::uint32_t GetWidth() const;
	std::uint32_t GetHeight() const;

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
	ResourceFile(ResourceID resourceFileID, const std::string& filename, const std::string& password);

	bool IsValid() const;
	std::string ErrorMessage() const;
	std::string GetFilename() const;
	ResourceID GetID() const;

	const TexturePage* GetTexturePage(std::size_t index) const;
	std::size_t GetTexturePageCount() const;
	ResourceID GetTextureID(const std::string& name) const;
	const ResourceTexture* GetTexture(ResourceID resourceTextureID) const;
	std::size_t GetTextureCount() const;

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
	ResourceManager() = delete;
	~ResourceManager() = delete;

	static std::string ErrorMessage();

	static ResourceID LoadResourceFile(const std::string& filename, const std::string& password = "");
	static ResourceFile* GetResourceFile(ResourceID resourceFileID);
	static void UnloadResourceFile(ResourceID resourceFileID);
	static bool ResourceFileExists(ResourceID resourceFileID);

	static ResourceID GetTextureID(const std::string& name, ResourceID resourceFileID = RESOURCE_ID_NULL);
	static const ResourceTexture* GetTexture(ResourceID resourceTextureID, ResourceID resourceFileID = RESOURCE_ID_NULL);

protected:
	friend class ResourceFile;
	static ResourceID GenerateID();

private:

	static std::string m_errorMessage;
	static ResourceID m_resourceIDCounter;
	static std::unordered_map<ResourceID, ResourceFile> m_resourceFiles;
};

} // luna