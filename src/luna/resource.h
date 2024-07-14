#ifndef LUNA_RESOURCE_H
#define LUNA_RESOURCE_H
/**
 * resource.h
 * Wrapper for loading and caching game resources.
*/
#include "luna/common.h"

/// @brief Internal sized data block type
typedef struct {
	size_t _size;	// Data buffer size
	void* _data;	// Data buffer
} _lunaDataResource;

/// @brief Internal sized text block type
typedef struct {
	size_t _size;	// Text buffer size (including null terminator)
	char* _text;	// Text buffer
} _lunaTextResource;

/// @brief Organized list of resources
typedef struct {
	unordered_map_str_t* _dataCache;		// Cache for data blocks 
	unordered_map_str_t* _textCache;		// Cache for text blocks
	unordered_map_str_t* _textureCache;		// Cache for textures
	unordered_map_str_t* _waveCache;		// Cache for waves
	unordered_map_str_t* _fontCache;		// Cache for fonts
	unordered_map_str_t* _meshCache;		// Cache for meshes
	char* _resourceFile;					// Resource file name
	char* _resourcePassword;				// Resource file password
	rresCentralDir _directory;				// Resource central directory
} ResourceList;

typedef struct {
	const char* resourceFile;				// Resource file name
	const char* resourcePassword;			// Resource file password
} ResourceListDesc;

/// @brief Create a new resource list.
/// @param _desc Resource list descriptor
/// @return Resource list pointer
ResourceList* _CreateResourceList(ResourceListDesc _desc);

/// @brief Deallocate a resource list.
/// @param _list Resource list pointer
void _DestroyResourceList(ResourceList* _list);

/// @brief Clear all data files from the lists cache.
/// @param _list Resource list pointer
void ClearDataCache(ResourceList* _list);

/// @brief Clear all text files from the lists cache.
/// @param _list Resource list pointer
void ClearTextCache(ResourceList* _list);

/// @brief Clear all texture files from the lists cache.
/// @param _list Resource list pointer
void ClearTextureCache(ResourceList* _list);

/// @brief Clear all wave files from the lists cache.
/// @param _list Resource list pointer
void ClearWaveCache(ResourceList* _list);

/// @brief Clear all font files from the lists cache.
/// @param _list Resource list pointer
void ClearFontCache(ResourceList* _list);

/// @brief Clear all mesh files from the lists cache.
/// @param _list Resource list pointer
void ClearMeshCache(ResourceList* _list);

/// @brief Retrieve a data file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Data file name
/// @param _data Pointer to destination buffer contents
/// @param _size Pointer to destination buffer size
void GetData(ResourceList* _list, char* _filename, void** _data, size_t* _size);

/// @brief Retrieve a text file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Text file name
/// @param _text Pointer to destination buffer contents
/// @param _size Pointer to destination buffer size
void GetText(ResourceList* _list, char* _filename, char** _text, size_t* _size);

/// @brief Retrieve a texture file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Texture file name
/// @param _texture Pointer to destination variable
void GetTexture(ResourceList* _list, char* _filename, Texture2D* _texture);

/// @brief Retrieve a wave file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Wave file name
/// @param _wave Pointer to destination variable
void GetWave(ResourceList* _list, char* _filename, Wave* _wave);

/// @brief Retrieve a font file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Font file name
/// @param _font Pointer to destination variable
void GetFont(ResourceList* _list, char* _filename, Font* _font);

/// @brief Retrieve a mesh file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Mesh file name
/// @param _mesh Pointer to destination variable
void GetMesh(ResourceList* _list, char* _filename, Mesh* _mesh);

/// @brief Remove the data file from cache, unloading it from memory.
/// @param _list Resource list pointer
/// @param _filename Data file name
void DeleteData(ResourceList* _list, char* _filename);

/// @brief Remove the text file from cache, unloading it from memory.
/// @param _list Resource list pointer
/// @param _filename Text file name
void DeleteText(ResourceList* _list, char* _filename);

/// @brief Remove the texture file from cache, unloading it from memory.
/// @param _list Resource list pointer
/// @param _filename Texture file name
void DeleteTexture(ResourceList* _list, char* _filename);

/// @brief Remove the wave file from cache, unloading it from memory.
/// @param _list Resource list pointer
/// @param _filename Wave file name
void DeleteWave(ResourceList* _list, char* _filename);

/// @brief Remove the font file from cache, unloading it from memory.
/// @param _list Resource list pointer
/// @param _filename Font file name
void DeleteFont(ResourceList* _list, char* _filename);

/// @brief Remove the mesh file from cache, unloading it from memory.
/// @param _list Resource list pointer
/// @param _filename Mesh file name
void DeleteMesh(ResourceList* _list, char* _filename);

#endif