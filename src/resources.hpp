#pragma once

#include "common.hpp"
#include "buffer.hpp"

// Forward declarations
class ResourceFile;
class TexturePage;

/// <summary>
/// Types of assets recognized by the engine.
/// </summary>
enum class ResourceType {
	Unknown = 0,
	Texture,
	Sound,
	Mesh,
	Text,
	Binary,
};

/// <summary>
/// Base class for all types of resources.
/// </summary>
class ResourceBase {
protected:
	virtual void Load(ResourceFile* file, const Buffer& block) = 0;
	std::string ErrorMessage() const;
	std::string m_errorMessage;
};

/// <summary>
/// Resource class representing a texture in a page.
/// </summary>
class ResourceTexture : public ResourceBase {
public:
	ResourceTexture() = default;

	bool IsValid() const;

	std::uint32_t GetWidth() const;
	std::uint32_t GetHeight() const;
	std::uint32_t GetNumFrames() const;
	std::size_t GetTexturePageIndex() const;

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

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block) override;
};

/// <summary>
/// Resource class representing a binary file.
/// </summary>
class ResourceBinary : public ResourceBase {
public:
	ResourceBinary() = default;

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

protected:
	friend class ResourceFile;
	void Load(ResourceFile* file, const Buffer& block);

private:
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
	ResourceFile(const std::string& filename, const std::string& password = "");

	bool IsValid() const;
	std::string ErrorMessage() const;
	std::string GetFilename() const;

	const TexturePage* GetTexturePage(std::size_t index) const;
	std::size_t GetTexturePageCount() const;
	const ResourceTexture* GetTexture(const std::string& name) const;
	std::size_t GetTextureCount() const;

private:
	std::string m_errorMessage;
	std::string m_filename;
	std::unordered_map<std::string, std::size_t> m_resourceMap;
	std::vector<TexturePage> m_texturePages;
	std::vector<ResourceTexture> m_textures;
};

typedef std::uint32_t ResourceFileID;
constexpr ResourceFileID RESOURCE_FILE_NULL = 0;

/// <summary>
/// Static interface for managing resource files.
/// </summary>
class ResourceManager {
public:
	ResourceManager() = delete;
	~ResourceManager() = delete;

	static std::string ErrorMessage();

	static ResourceFileID LoadResourceFile(const std::string& filename, const std::string& password = "");
	static void UnloadResourceFile(ResourceFileID resourceFileID);
	static bool ResourceFileExists(ResourceFileID resourceFileID);

	static const TexturePage* GetTexturePage(ResourceFileID resourceFileID, std::size_t index);
	static const ResourceTexture* GetTexture(ResourceFileID resourceFileID, const std::string& name);

private:
	static ResourceFile* GetResourceFile(ResourceFileID resourceFileID);

	static std::string m_errorMessage;
	static ResourceFileID m_resourceFileIDCounter;
	static std::unordered_map<ResourceFileID, ResourceFile> m_resourceFiles;
};