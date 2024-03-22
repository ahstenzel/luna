#ifndef LUNA_RESOURCE_H
#define LUNA_RESOURCE_H
/**
 * resource.h
 * Wrapper for loading and caching game resources.
*/
#include "luna/common.h"

/// @brief Internal sized data block type
typedef struct {
	size_t size;
	void* data;
} _lunaDataResource;

/// @brief Internal sized text block type
typedef struct {
	size_t size;
	char* text;
} _lunaTextResource;

/// @brief Organized list of resources
typedef struct {
	unordered_map_str_t* dataCache;
	unordered_map_str_t* textCache;
	unordered_map_str_t* textureCache;
	unordered_map_str_t* waveCache;
	unordered_map_str_t* fontCache;
	unordered_map_str_t* meshCache;
	char* resourceFile;
	char* resourcePassword;
	rresCentralDir directory;
} ResourceList;

typedef struct {
	const char* resourceFile;
	const char* resourcePassword;
} ResourceListDesc;

/// @brief Create a new resource list.
/// @param _desc Resource list descriptor
/// @return Resource list pointer
ResourceList* CreateResourceList(ResourceListDesc _desc);

/// @brief Deallocate a resource list.
/// @param _list Resource list pointer
void DestroyResourceList(ResourceList* _list);

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
/// @return Status code
int GetData(ResourceList* _list, char* _filename, void** _data, size_t* _size);

/// @brief Retrieve a text file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Text file name
/// @param _text Pointer to destination buffer contents
/// @param _size Pointer to destination buffer size
/// @return Status code
int GetText(ResourceList* _list, char* _filename, char** _text, size_t* _size);

/// @brief Retrieve a texture file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Texture file name
/// @param _texture Pointer to destination variable
/// @return Status code
int GetTexture(ResourceList* _list, char* _filename, Texture2D* _texture);

/// @brief Retrieve a wave file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Wave file name
/// @param _wave Pointer to destination variable
/// @return Status code
int GetWave(ResourceList* _list, char* _filename, Wave* _wave);

/// @brief Retrieve a font file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Font file name
/// @param _font Pointer to destination variable
/// @return Status code
int GetFont(ResourceList* _list, char* _filename, Font* _font);

/// @brief Retrieve a mesh file from the resource list, loading from disk if it isn't cached.
/// @param _list Resource list pointer
/// @param _filename Mesh file name
/// @param _mesh Pointer to destination variable
/// @return Status code
int GetMesh(ResourceList* _list, char* _filename, Mesh* _mesh);

#endif