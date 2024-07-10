#include "luna/camera.h"

CameraList* _CreateCameraList() {
	CameraList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate camera list!");
		return NULL; 
	}
	list->_cameraIndices = unordered_map_create(size_t);
	list->_cameras = free_list_create(Camera2D);
	list->_active = ID_NULL;
	if (!list->_cameras || !list->_cameraIndices) {
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate camera list containers!");
		_DestroyCameraList(list);
		return NULL;
	}
	return list;
}

void _DestroyCameraList(CameraList* _list) {
	if (_list) {
		unordered_map_destroy(_list->_cameraIndices);
		free_list_destroy(_list->_cameras);
		free(_list);
	}
}

size_t GetCameraListSize(CameraList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid camera list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return 0;
	}
	return free_list_size(_list->_cameras);
}

CameraListIt* CameraListItBegin(CameraList* _list) {
	LUNA_RETURN_CLEAR;
	// Error check
	CameraListIt* it = NULL;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid camera list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto camera_list_it_begin_fail;
	}
	if (free_list_size(_list->_cameras) == 0) { 
		goto camera_list_it_begin_fail; 
	}

	// Create map iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate camera list iterator!");
		goto camera_list_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = unordered_map_it(_list->_cameraIndices);
	if (!it->_ptr) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate camera list index map iterator!");
		goto camera_list_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DEBUG_WARN("Camera list index map iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto camera_list_it_begin_fail; 
	}

	// Retrieve camera data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->_cameras, *idx);
	it->id = (CameraID)it->_ptr->key;
	if (!it->data) { 
		LUNA_DEBUG_WARN("Camera list iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto camera_list_it_begin_fail; 
	}

	return it;
camera_list_it_begin_fail:
	free(it);
	return NULL;
}

CameraListIt* CameraListItNext(CameraListIt* _it) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_it) { return NULL; }

	// Advance to next valid element
	unordered_map_it_next(_it->_ptr);
	if (_it->_ptr) {
		// Update iterator contents
		size_t* idx = _it->_ptr->data;
		_it->data = free_list_get(_it->_list->_cameras, *idx);
		_it->id = (CameraID)_it->_ptr->key;
		if (!_it->data) { 
			LUNA_DEBUG_WARN("Next iterator position yielded invalid camera data!");
			LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
			free(_it);
			return NULL;
		}
	}
	else {
		// Deallocate iterator
		free(_it);
		return NULL;
	}
	return _it;
}

CameraID CreateCamera(CameraList* _list, CameraDesc _desc) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid camera list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
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
	if (!free_list_insert(_list->_cameras, &idx, &camera)) {
		LUNA_DEBUG_WARN("Failed to add camera to list!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->_cameraIndices, id, &idx)) {
		LUNA_DEBUG_WARN("Failed to add camera to index map!");
		free_list_remove(_list->_cameras, idx);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		return ID_NULL;
	}

	// Set as default
	if (free_list_size(_list->_cameras) == 1) {
		_list->_active = id;
	}
	return id;
}

void DestroyCamera(CameraList* _list, CameraID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid camera list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_cameraIndices, _id);
	if (idxPtr) {
		free_list_remove(_list->_cameras, *idxPtr);
		unordered_map_delete(_list->_cameraIndices, _id);
		if (_list->_active == _id) {
			_list->_active = ID_NULL;
		}
	}
	else {
		LUNA_DEBUG_WARN("Invalid camera id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

void SetActiveCamera(CameraList* _list, CameraID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid camera list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_cameraIndices, _id);
	if (idxPtr) {
		_list->_active = _id;
	}
	else {
		LUNA_DEBUG_WARN("Invalid camera id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return;
	}
	return;
}

Camera2D* GetCamera(CameraList* _list, CameraID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid camera list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return NULL; 
	}

	size_t* idxPtr = unordered_map_find(_list->_cameraIndices, _id);
	if (idxPtr) {
		Camera2D* camera = free_list_get(_list->_cameras, *idxPtr);
		return camera;
	}
	else {
		LUNA_DEBUG_WARN("Invalid camera id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return NULL;
}

Camera2D* GetActiveCamera(CameraList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid camera list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return NULL; 
	}

	CameraID id = GetActiveCameraID(_list);
	size_t* idxPtr = unordered_map_find(_list->_cameraIndices, id);
	if (idxPtr) {
		Camera2D* camera = free_list_get(_list->_cameras, *idxPtr);
		return camera;
	}
	else {
		LUNA_DEBUG_WARN("Invalid camera id (%d)!", (int)id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return NULL;
	}
}

CameraID GetActiveCameraID(CameraList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid camera list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return ID_NULL; 
	}
	return _list->_active;
}