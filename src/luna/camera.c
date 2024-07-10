#include "luna/camera.h"

CameraList* _CreateCameraList() {
	CameraList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate camera list!");
		return NULL; 
	}
	list->cameraIndices = unordered_map_create(size_t);
	list->cameras = free_list_create(Camera2D);
	list->active = ID_NULL;
	if (!list->cameras || !list->cameraIndices) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate camera list containers!");
		_DestroyCameraList(list);
		return NULL;
	}
	return list;
}

void _DestroyCameraList(CameraList* _list) {
	if (_list) {
		unordered_map_destroy(_list->cameraIndices);
		free_list_destroy(_list->cameras);
		free(_list);
	}
}

size_t GetCameraListSize(CameraList* _list) {
	if (!_list) {
		LUNA_DBG_WARN("Invalid camera list reference!");
		return 0;
	}
	return free_list_size(_list->cameras);
}

CameraListIt* CameraListItBegin(CameraList* _list) {
	// Error check
	CameraListIt* it = NULL;
	if (!_list) {
		LUNA_DBG_WARN("Invalid camera list reference!");
		goto camera_list_it_begin_fail;
	}
	if (free_list_size(_list->cameras) == 0) { 
		goto camera_list_it_begin_fail; 
	}

	// Create map iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate camera list iterator!");
		goto camera_list_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = unordered_map_it(_list->cameraIndices);
	if (!it->_ptr) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate camera list index map iterator!");
		goto camera_list_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DBG_WARN("Camera list index map iterator points to invalid data!");
		goto camera_list_it_begin_fail; 
	}

	// Retrieve camera data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->cameras, *idx);
	it->id = (CameraID)it->_ptr->key;
	if (!it->data) { 
		LUNA_DBG_WARN("Camera list iterator points to invalid data!");
		goto camera_list_it_begin_fail; 
	}

	return it;
camera_list_it_begin_fail:
	free(it);
	return NULL;
}

void CameraListItNext(CameraListIt** _it) {
	// Error check
	if (!_it || !(*_it)) { return; }

	// Advance to next valid element
	CameraListIt* it = *_it;
	unordered_map_it_next(it->_ptr);
	if (it->_ptr) {
		// Update iterator contents
		size_t* idx = it->_ptr->data;
		it->data = free_list_get(it->_list->cameras, *idx);
		it->id = (CameraID)it->_ptr->key;
		if (!it->data) { 
			LUNA_DBG_WARN("Next iterator position yielded invalid camera data!");
			free(it);
			(*_it) = NULL;
			return;
		}
	}
	else {
		// Deallocate iterator
		free(it);
		(*_it) = NULL;
		return;
	}
}

CameraID CreateCamera(CameraList* _list, CameraDesc _desc) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid camera list reference!");
		return ID_NULL; 
	}

	// Create object
	CameraID id = _luna_id_generate();
	Camera2D camera = {
		.target = _desc.target,
		.offset = _desc.offset,
		.rotation = _desc.rotation,
		.zoom = _desc.zoom
	};

	// Add to list
	size_t idx = 0;
	if (!free_list_insert(_list->cameras, &idx, &camera)) {
		LUNA_DBG_WARN("Failed to add camera to list!");
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->cameraIndices, id, &idx)) {
		LUNA_DBG_WARN("Failed to add camera to index map!");
		free_list_remove(_list->cameras, idx);
		return ID_NULL;
	}

	// Set as default
	if (free_list_size(_list->cameras) == 1) {
		_list->active = id;
	}
	return id;
}

void DestroyCamera(CameraList* _list, CameraID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid camera list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->cameraIndices, _id);
	if (idxPtr) {
		free_list_remove(_list->cameras, *idxPtr);
		unordered_map_delete(_list->cameraIndices, _id);
		if (_list->active == _id) {
			_list->active = ID_NULL;
		}
	}
	else {
		LUNA_DBG_WARN("Invalid camera id (%d)!", (int)_id);
	}
}

void SetActiveCamera(CameraList* _list, CameraID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid camera list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->cameraIndices, _id);
	if (idxPtr) {
		_list->active = _id;
	}
	else {
		LUNA_DBG_WARN("Invalid camera id (%d)!", (int)_id);
	}
}

Camera2D* GetCamera(CameraList* _list, CameraID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid camera list reference!");
		return NULL; 
	}

	size_t* idxPtr = unordered_map_find(_list->cameraIndices, _id);
	if (idxPtr) {
		Camera2D* camera = free_list_get(_list->cameras, *idxPtr);
		return camera;
	}
	else {
		LUNA_DBG_WARN("Invalid camera id (%d)!", (int)_id);
	}
	return NULL;
}

Camera2D* GetActiveCamera(CameraList* _list) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid camera list reference!");
		return NULL; 
	}

	size_t* idxPtr = unordered_map_find(_list->cameraIndices, GetActiveCameraID(_list));
	if (idxPtr) {
		Camera2D* camera = free_list_get(_list->cameras, *idxPtr);
		return camera;
	}
	else {
		LUNA_DBG_WARN("Invalid camera id!");
	}
	return NULL;
}

CameraID GetActiveCameraID(CameraList* _list) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid camera list reference!");
		return ID_NULL; 
	}
	return _list->active;
}