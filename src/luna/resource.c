#include "luna/resource.h"

ResourceList* _CreateResourceList(ResourceListDesc _desc) {
	// Create resource container
	ResourceList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate resource list!");
		return NULL; 
	}
	list->_dataCache = unordered_map_str_create(_lunaDataResource);
	list->_textCache = unordered_map_str_create(char*);
	list->_textureCache = unordered_map_str_create(Texture2D);
	list->_waveCache = unordered_map_str_create(Wave);
	list->_fontCache = unordered_map_str_create(Font);
	list->_meshCache = unordered_map_str_create(Mesh);
	if (!list->_dataCache || !list->_textCache || !list->_textureCache ||
		!list->_waveCache || !list->_fontCache || !list->_meshCache) {
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate resource list containers!");
		_DestroyResourceList(list);
		return NULL;
	}

	// Load resource file
	size_t fileLen = strlen(_desc.resourceFile);
	list->_resourceFile = calloc(fileLen + 1, 1);
	if (!list->_resourceFile) {
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate resource list name buffer!");
		_DestroyResourceList(list);
		return NULL;
	}
	strcpy_s(list->_resourceFile, fileLen + 1, _desc.resourceFile);
	if (_desc.resourcePassword) {
		size_t passLen = strlen(_desc.resourcePassword);
		list->_resourcePassword = calloc(passLen + 1, 1);
		if (!list->_resourcePassword) {
			LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate resource list password buffer!");
			_DestroyResourceList(list);
			return NULL;
		}
		strcpy_s(list->_resourcePassword, passLen + 1, _desc.resourcePassword);
	}
	list->_directory = rresLoadCentralDirectory(list->_resourceFile);
	return list;
}

void _DestroyResourceList(ResourceList* _list) {
	if (_list) {
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->_dataCache); it; unordered_map_str_it_next(it)) {
			_lunaDataResource* rsc = it->data;
			free(rsc->_data);
		}
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->_textCache); it; unordered_map_str_it_next(it)) {
			_lunaTextResource* rsc = it->data;
			free(rsc->_text);
		}
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->_textureCache); it; unordered_map_str_it_next(it)) {
			Texture2D* rsc = it->data;
			UnloadTexture(*rsc);
		}
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->_waveCache); it; unordered_map_str_it_next(it)) {
			Wave* rsc = it->data;
			UnloadWave(*rsc);
		}
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->_fontCache); it; unordered_map_str_it_next(it)) {
			Font* rsc = it->data;
			UnloadFont(*rsc);
		}
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->_meshCache); it; unordered_map_str_it_next(it)) {
			Mesh* rsc = it->data;
			UnloadMesh(*rsc);
		}
		unordered_map_str_destroy(_list->_dataCache);
		unordered_map_str_destroy(_list->_textCache);
		unordered_map_str_destroy(_list->_textureCache);
		unordered_map_str_destroy(_list->_waveCache);
		unordered_map_str_destroy(_list->_fontCache);
		unordered_map_str_destroy(_list->_meshCache);
		rresUnloadCentralDirectory(_list->_directory);
		free(_list->_resourceFile);
		free(_list->_resourcePassword);

		free(_list);
	}
}

void ClearDataCache(ResourceList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	unordered_map_str_clear(_list->_dataCache);
}

void ClearTextCache(ResourceList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	unordered_map_str_clear(_list->_textCache);
}

void ClearTextureCache(ResourceList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	unordered_map_str_clear(_list->_textureCache);
}

void ClearWaveCache(ResourceList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	unordered_map_str_clear(_list->_waveCache);
}

void ClearFontCache(ResourceList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	unordered_map_str_clear(_list->_fontCache);
}

void ClearMeshCache(ResourceList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	unordered_map_str_clear(_list->_meshCache);
}

void GetData(ResourceList* _list, char* _filename, void** _data, size_t* _size) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto get_data_fail;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		goto get_data_fail;
	}

	// Check cache
	_lunaDataResource* rsc = unordered_map_str_find(_list->_dataCache, _filename);
	if (rsc) {
		*_size = rsc->_size;
		*_data = rsc->_data;
		return;
	}

	// Load resource
	if (_list->_resourcePassword) {
		rresSetCipherPassword(_list->_resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->_directory, _filename);
	rresResourceChunk chunk = rresLoadResourceChunk(_list->_resourceFile, id);
	int result = UnpackResourceChunk(&chunk);
	if (result == RRES_SUCCESS) {
		unsigned int dataSize = 0;
		*_data = LoadDataFromResource(chunk, &dataSize);
		*_size = (size_t)dataSize;
		rresUnloadResourceChunk(chunk);
	}
	else {
		LUNA_DEBUG_WARN("Failed to load data block (%s)! [Error code %d]", _filename, result);
		LUNA_RETURN_SET(LUNA_RETURN_RESOURCE_FAILURE);
		rresUnloadResourceChunk(chunk);
		goto get_data_fail;
	}

	// Save to cache
	_lunaDataResource cache = {
		._data = *_data,
		._size = *_size
	};
	if (!unordered_map_str_insert(_list->_dataCache, _filename, &cache)) {
		LUNA_DEBUG_WARN("Failed to cache data block (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto get_data_fail;
	}
	return;

get_data_fail:
	*_size = 0;
	*_data = NULL;
	return;
}

void GetText(ResourceList* _list, char* _filename, char** _text, size_t* _size) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto get_text_fail;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		goto get_text_fail;
	}

	// Check cache
	_lunaTextResource* rsc = unordered_map_str_find(_list->_textCache, _filename);
	if (rsc) {
		*_size = rsc->_size;
		*_text = rsc->_text;
		return;
	}

	// Load resource
	if (_list->_resourcePassword) {
		rresSetCipherPassword(_list->_resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->_directory, _filename);
	rresResourceChunk chunk = rresLoadResourceChunk(_list->_resourceFile, id);
	int result = UnpackResourceChunk(&chunk);
	if (result == RRES_SUCCESS) {
		*_text = LoadTextFromResource(chunk);
		*_size = strlen(*_text) + 1;
		rresUnloadResourceChunk(chunk);
	}
	else {
		LUNA_DEBUG_WARN("Failed to load text block (%s)! [Error code %d]", _filename, result);
		LUNA_RETURN_SET(LUNA_RETURN_RESOURCE_FAILURE);
		rresUnloadResourceChunk(chunk);
		goto get_text_fail;
	}

	// Save to cache
	_lunaTextResource cache = {
		._text = *_text,
		._size = *_size
	};
	if (!unordered_map_str_insert(_list->_textCache, _filename, &cache)) {
		LUNA_DEBUG_WARN("Failed to cache text block (%s)!", _filename);
		
		goto get_text_fail;
	}
	return;

get_text_fail:
	*_size = 0;
	*_text = NULL;
	return;
}

void GetTexture(ResourceList* _list, char* _filename, Texture2D* _texture) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache
	Texture2D* rsc = unordered_map_str_find(_list->_textureCache, _filename);
	if (rsc) {
		*_texture = *rsc;
		return;
	}

	// Load resource
	if (_list->_resourcePassword) {
		rresSetCipherPassword(_list->_resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->_directory, _filename);
	rresResourceChunk chunk = rresLoadResourceChunk(_list->_resourceFile, id);
	int result = UnpackResourceChunk(&chunk);
	if (result == RRES_SUCCESS) {
		Image img = LoadImageFromResource(chunk);
		*_texture = LoadTextureFromImage(img);
		UnloadImage(img);
		rresUnloadResourceChunk(chunk);
	}
	else {
		LUNA_DEBUG_WARN("Failed to load texture (%s)! [Error code %d]", _filename, result);
		LUNA_RETURN_SET(LUNA_RETURN_RESOURCE_FAILURE);
		rresUnloadResourceChunk(chunk);
		return;
	}

	// Save to cache
	if (!unordered_map_str_insert(_list->_textureCache, _filename, _texture)) {
		LUNA_DEBUG_WARN("Failed to cache texture (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		return;
	}
}

void GetWave(ResourceList* _list, char* _filename, Wave* _wave) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache
	Wave* rsc = unordered_map_str_find(_list->_waveCache, _filename);
	if (rsc) {
		*_wave = *rsc;
		return;
	}

	// Load resource
	if (_list->_resourcePassword) {
		rresSetCipherPassword(_list->_resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->_directory, _filename);
	rresResourceChunk chunk = rresLoadResourceChunk(_list->_resourceFile, id);
	int result = UnpackResourceChunk(&chunk);
	if (result == RRES_SUCCESS) {
		*_wave = LoadWaveFromResource(chunk);
		rresUnloadResourceChunk(chunk);
	}
	else {
		LUNA_DEBUG_WARN("Failed to load wave (%s)! [Error code %d]", _filename, result);
		LUNA_RETURN_SET(LUNA_RETURN_RESOURCE_FAILURE);
		rresUnloadResourceChunk(chunk);
		return;
	}

	// Save to cache
	if (!unordered_map_str_insert(_list->_waveCache, _filename, _wave)) {
		LUNA_DEBUG_WARN("Failed to cache wave (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		return;
	}
}

void GetFont(ResourceList* _list, char* _filename, Font* _font) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache
	Font* rsc = unordered_map_str_find(_list->_fontCache, _filename);
	if (rsc) {
		*_font = *rsc;
		return;
	}

	// Load resource
	if (_list->_resourcePassword) {
		rresSetCipherPassword(_list->_resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->_directory, _filename);
	rresResourceMulti chunk = rresLoadResourceMulti(_list->_resourceFile, id);
	int result = RRES_SUCCESS;
	for(unsigned int i=0; i<chunk.count; ++i) {
		result = UnpackResourceChunk(&chunk.chunks[i]);
		if (result != RRES_SUCCESS) {
			break;
		}
	}
	if (result == RRES_SUCCESS) {
		*_font = LoadFontFromResource(chunk);
		rresUnloadResourceMulti(chunk);
	}
	else {
		LUNA_DEBUG_WARN("Failed to load font (%s)! [Error code %d]", _filename, result);
		LUNA_RETURN_SET(LUNA_RETURN_RESOURCE_FAILURE);
		rresUnloadResourceMulti(chunk);
		return;
	}

	// Save to cache
	if (!unordered_map_str_insert(_list->_fontCache, _filename, _font)) {
		LUNA_DEBUG_WARN("Failed to cache font (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		return;
	}
}

void GetMesh(ResourceList* _list, char* _filename, Mesh* _mesh) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache
	Mesh* rsc = unordered_map_str_find(_list->_meshCache, _filename);
	if (rsc) {
		*_mesh = *rsc;
		return;
	}

	// Load resource
	if (_list->_resourcePassword) {
		rresSetCipherPassword(_list->_resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->_directory, _filename);
	rresResourceMulti chunk = rresLoadResourceMulti(_list->_resourceFile, id);
	int result = RRES_SUCCESS;
	for(unsigned int i=0; i<chunk.count; ++i) {
		result = UnpackResourceChunk(&chunk.chunks[i]);
		if (result != RRES_SUCCESS) {
			break;
		}
	}
	if (result == RRES_SUCCESS) {
		*_mesh = LoadMeshFromResource(chunk);
		rresUnloadResourceMulti(chunk);
	}
	else {
		LUNA_DEBUG_WARN("Failed to load mesh (%s)! [Error code %d]", _filename, result);
		LUNA_RETURN_SET(LUNA_RETURN_RESOURCE_FAILURE);
		rresUnloadResourceMulti(chunk);
		return;
	}

	// Save to cache
	if (!unordered_map_str_insert(_list->_meshCache, _filename, _mesh)) {
		LUNA_DEBUG_WARN("Failed to cache mesh (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		return;
	}
}

void DeleteData(ResourceList* _list, char* _filename) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache and unload from memory
	_lunaDataResource* rsc = unordered_map_str_find(_list->_dataCache, _filename);
	if (rsc) {
		free(rsc->_data);
	}
	else {
		LUNA_DEBUG_WARN("Invalid data block (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Remove from cache
	unordered_map_str_delete(_list->_dataCache, _filename);
}

void DeleteText(ResourceList* _list, char* _filename) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache and unload from memory
	_lunaTextResource* rsc = unordered_map_str_find(_list->_textCache, _filename);
	if (rsc) {
		free(rsc->_text);
	}
	else {
		LUNA_DEBUG_WARN("Invalid text block (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Remove from cache
	unordered_map_str_delete(_list->_textCache, _filename);
}

void DeleteTexture(ResourceList* _list, char* _filename) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache and unload from memory
	Texture2D* rsc = unordered_map_str_find(_list->_textureCache, _filename);
	if (rsc) {
		UnloadTexture(*rsc);
	}
	else {
		LUNA_DEBUG_WARN("Invalid texture (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Remove from cache
	unordered_map_str_delete(_list->_textureCache, _filename);
}

void DeleteWave(ResourceList* _list, char* _filename) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache and unload from memory
	Wave* rsc = unordered_map_str_find(_list->_waveCache, _filename);
	if (rsc) {
		UnloadWave(*rsc);
	}
	else {
		LUNA_DEBUG_WARN("Invalid wave (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Remove from cache
	unordered_map_str_delete(_list->_waveCache, _filename);
}

void DeleteFont(ResourceList* _list, char* _filename) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache and unload from memory
	Font* rsc = unordered_map_str_find(_list->_fontCache, _filename);
	if (rsc) {
		UnloadFont(*rsc);
	}
	else {
		LUNA_DEBUG_WARN("Invalid font (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Remove from cache
	unordered_map_str_delete(_list->_fontCache, _filename);
}

void DeleteMesh(ResourceList* _list, char* _filename) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid resource list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}
	if (!_filename) {
		LUNA_DEBUG_WARN("Invalid filename!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Check cache and unload from memory
	Mesh* rsc = unordered_map_str_find(_list->_meshCache, _filename);
	if (rsc) {
		UnloadMesh(*rsc);
	}
	else {
		LUNA_DEBUG_WARN("Invalid mesh (%s)!", _filename);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
		return;
	}

	// Remove from cache
	unordered_map_str_delete(_list->_meshCache, _filename);
}