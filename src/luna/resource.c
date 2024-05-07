#include "luna/resource.h"

ResourceList* CreateResourceList(ResourceListDesc _desc) {
	// Create resource container
	ResourceList* list = calloc(1, sizeof *list);
	if (!list) { return NULL; }
	list->dataCache = unordered_map_str_create(_lunaDataResource);
	list->textCache = unordered_map_str_create(char*);
	list->textureCache = unordered_map_str_create(Texture2D);
	list->waveCache = unordered_map_str_create(Wave);
	list->fontCache = unordered_map_str_create(Font);
	list->meshCache = unordered_map_str_create(Mesh);
	if (!list->dataCache || !list->textCache || !list->textureCache ||
		!list->waveCache || !list->fontCache || !list->meshCache) {
		DestroyResourceList(list);
		return NULL;
	}

	// Load resource file
	size_t fileLen = strlen(_desc.resourceFile);
	list->resourceFile = calloc(fileLen + 1, 1);
	if (!list->resourceFile) {
		DestroyResourceList(list);
		return NULL;
	}
	strcpy_s(list->resourceFile, fileLen + 1, _desc.resourceFile);
	if (_desc.resourcePassword) {
		size_t passLen = strlen(_desc.resourcePassword);
		list->resourcePassword = calloc(passLen + 1, 1);
		if (!list->resourcePassword) {
			DestroyResourceList(list);
			return NULL;
		}
		strcpy_s(list->resourcePassword, passLen + 1, _desc.resourcePassword);
	}
	list->directory = rresLoadCentralDirectory(list->resourceFile);
	return list;
}

void DestroyResourceList(ResourceList* _list) {
	if (_list) {
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->textureCache); it; unordered_map_str_it_next(it)) {
			Texture2D* rsc = it->data;
			UnloadTexture(*rsc);
		}
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->waveCache); it; unordered_map_str_it_next(it)) {
			Wave* rsc = it->data;
			UnloadWave(*rsc);
		}
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->fontCache); it; unordered_map_str_it_next(it)) {
			Font* rsc = it->data;
			UnloadFont(*rsc);
		}
		for(unordered_map_str_it_t* it = unordered_map_str_it(_list->meshCache); it; unordered_map_str_it_next(it)) {
			Mesh* rsc = it->data;
			UnloadMesh(*rsc);
		}
		unordered_map_str_destroy(_list->dataCache);
		unordered_map_str_destroy(_list->textCache);
		unordered_map_str_destroy(_list->textureCache);
		unordered_map_str_destroy(_list->waveCache);
		unordered_map_str_destroy(_list->fontCache);
		unordered_map_str_destroy(_list->meshCache);
		rresUnloadCentralDirectory(_list->directory);
		free(_list->resourceFile);
		free(_list->resourcePassword);

		free(_list);
	}
}

void ClearDataCache(ResourceList* _list) {
	if (_list) {
		unordered_map_str_clear(_list->dataCache);
	}
}

void ClearTextCache(ResourceList* _list) {
	if (_list) {
		unordered_map_str_clear(_list->textCache);
	}
}

void ClearTextureCache(ResourceList* _list) {
	if (_list) {
		unordered_map_str_clear(_list->textureCache);
	}
}

void ClearWaveCache(ResourceList* _list) {
	if (_list) {
		unordered_map_str_clear(_list->waveCache);
	}
}

void ClearFontCache(ResourceList* _list) {
	if (_list) {
		unordered_map_str_clear(_list->fontCache);
	}
}

void ClearMeshCache(ResourceList* _list) {
	if (_list) {
		unordered_map_str_clear(_list->meshCache);
	}
}

int GetData(ResourceList* _list, char* _filename, void** _data, size_t* _size) {
	// Error check
	if (!_list || !_filename) { goto LoadDataFail; }

	// Check cache
	_lunaDataResource* rsc = unordered_map_str_find(_list->dataCache, _filename);
	if (rsc) {
		*_size = rsc->size;
		*_data = rsc->data;
		return 0;
	}

	// Load resource
	if (_list->resourcePassword) {
		rresSetCipherPassword(_list->resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->directory, _filename);
	rresResourceChunk chunk = rresLoadResourceChunk(_list->resourceFile, id);
	int result = UnpackResourceChunk(&chunk);
	if (result == RRES_SUCCESS) {
		unsigned int dataSize = 0;
		*_data = LoadDataFromResource(chunk, &dataSize);
		*_size = (size_t)dataSize;
		rresUnloadResourceChunk(chunk);
	}
	else {
		LUNA_DBG_ERR("(GetData): Failed to load data block (%s) [Error code %d]", _filename, result);
		rresUnloadResourceChunk(chunk);
		goto LoadDataFail;
	}

	// Save to cache
	_lunaDataResource cache = {
		.data = *_data,
		.size = *_size
	};
	if (!unordered_map_str_insert(_list->dataCache, _filename, &cache)) {
		goto LoadDataFail;
	}
	return 0;

LoadDataFail:
	*_size = 0;
	*_data = NULL;
	return -1;
}

int GetText(ResourceList* _list, char* _filename, char** _text, size_t* _size) {
	// Error check
	if (!_list || !_filename) { goto LoadTextFail; }

	// Check cache
	_lunaTextResource* rsc = unordered_map_str_find(_list->textCache, _filename);
	if (rsc) {
		*_size = rsc->size;
		*_text = rsc->text;
	}

	// Load resource
	if (_list->resourcePassword) {
		rresSetCipherPassword(_list->resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->directory, _filename);
	rresResourceChunk chunk = rresLoadResourceChunk(_list->resourceFile, id);
	int result = UnpackResourceChunk(&chunk);
	if (result == RRES_SUCCESS) {
		*_text = LoadTextFromResource(chunk);
		*_size = strlen(*_text) + 1;
		rresUnloadResourceChunk(chunk);
	}
	else {
		LUNA_DBG_ERR("(GetText): Failed to load text block (%s) [Error code %d]", _filename, result);
		rresUnloadResourceChunk(chunk);
		goto LoadTextFail;
	}

	// Save to cache
	_lunaTextResource cache = {
		.text = *_text,
		.size = *_size
	};
	if (!unordered_map_str_insert(_list->textCache, _filename, &cache)) {
		goto LoadTextFail;
	}
	return 0;

LoadTextFail:
	*_size = 0;
	*_text = NULL;
	return -1;
}

int GetTexture(ResourceList* _list, char* _filename, Texture2D* _texture) {
	// Error check
	if (!_list || !_filename) { return -1; }

	// Check cache
	Texture2D* rsc = unordered_map_str_find(_list->textureCache, _filename);
	if (rsc) {
		*_texture = *rsc;
		return 0;
	}

	// Load resource
	if (_list->resourcePassword) {
		rresSetCipherPassword(_list->resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->directory, _filename);
	rresResourceChunk chunk = rresLoadResourceChunk(_list->resourceFile, id);
	int result = UnpackResourceChunk(&chunk);
	if (result == RRES_SUCCESS) {
		Image img = LoadImageFromResource(chunk);
		*_texture = LoadTextureFromImage(img);
		UnloadImage(img);
		rresUnloadResourceChunk(chunk);
	}
	else {
		LUNA_DBG_ERR("(GetTexture): Failed to load texture (%s) [Error code %d]", _filename, result);
		rresUnloadResourceChunk(chunk);
		return -1;
	}

	// Save to cache
	if (!unordered_map_str_insert(_list->textureCache, _filename, _texture)) {
		return -1;
	}
	return 0;
}

int GetWave(ResourceList* _list, char* _filename, Wave* _wave) {
	// Error check
	if (!_list || !_filename) { return -1; }

	// Check cache
	Wave* rsc = unordered_map_str_find(_list->waveCache, _filename);
	if (rsc) {
		*_wave = *rsc;
		return 0;
	}

	// Load resource
	if (_list->resourcePassword) {
		rresSetCipherPassword(_list->resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->directory, _filename);
	rresResourceChunk chunk = rresLoadResourceChunk(_list->resourceFile, id);
	int result = UnpackResourceChunk(&chunk);
	if (result == RRES_SUCCESS) {
		*_wave = LoadWaveFromResource(chunk);
		rresUnloadResourceChunk(chunk);
	}
	else {
		LUNA_DBG_ERR("(GetWave): Failed to load wave (%s) [Error code %d]", _filename, result);
		rresUnloadResourceChunk(chunk);
		return -1;
	}

	// Save to cache
	if (!unordered_map_str_insert(_list->waveCache, _filename, _wave)) {
		return -1;
	}
	return 0;
}

int GetFont(ResourceList* _list, char* _filename, Font* _font) {
	// Error check
	if (!_list || !_filename) { return -1; }

	// Check cache
	Font* rsc = unordered_map_str_find(_list->fontCache, _filename);
	if (rsc) {
		*_font = *rsc;
		return 0;
	}

	// Load resource
	if (_list->resourcePassword) {
		rresSetCipherPassword(_list->resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->directory, _filename);
	rresResourceMulti chunk = rresLoadResourceMulti(_list->resourceFile, id);
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
		LUNA_DBG_ERR("(GetFont): Failed to load font (%s) [Error code %d]", _filename, result);
		rresUnloadResourceMulti(chunk);
		return -1;
	}

	// Save to cache
	if (!unordered_map_str_insert(_list->fontCache, _filename, _font)) {
		return -1;
	}
	return 0;
}

int GetMesh(ResourceList* _list, char* _filename, Mesh* _mesh) {
	// Error check
	if (!_list || !_filename) { return -1; }

	// Check cache
	Mesh* rsc = unordered_map_str_find(_list->meshCache, _filename);
	if (rsc) {
		*_mesh = *rsc;
		return 0;
	}

	// Load resource
	if (_list->resourcePassword) {
		rresSetCipherPassword(_list->resourcePassword);
	}
	unsigned int id = rresGetResourceId(_list->directory, _filename);
	rresResourceMulti chunk = rresLoadResourceMulti(_list->resourceFile, id);
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
		LUNA_DBG_ERR("(GetMesh): Failed to load mesh (%s) [Error code %d]", _filename, result);
		rresUnloadResourceMulti(chunk);
		return -1;
	}

	// Save to cache
	if (!unordered_map_str_insert(_list->meshCache, _filename, _mesh)) {
		return -1;
	}
	return 0;
}