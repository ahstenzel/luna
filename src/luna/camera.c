#include "luna/camera.h"

CameraList* _CreateCameraList() {
	CameraList* list = calloc(1, sizeof *list);
	if (!list) { return NULL; }
	list->cameraIndices = unordered_map_create(size_t);
	list->cameras = free_list_create(Camera2D);
	list->active = ID_NULL;
	if (!list->cameras || !list->cameraIndices) {
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

CameraID CreateCamera(CameraList* _list, CameraDesc _desc) {
	if (!_list) { return -1; }

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
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->cameraIndices, id, &idx)) {
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
	if (!_list) { return; }

	size_t* idxPtr = unordered_map_find(_list->cameraIndices, _id);
	if (idxPtr) {
		free_list_remove(_list->cameras, *idxPtr);
		unordered_map_delete(_list->cameraIndices, _id);
		if (_list->active == _id) {
			_list->active = ID_NULL;
		}
	}
}

void SetActiveCamera(CameraList* _list, CameraID _id) {
	if (!_list) { return; }

	size_t* idxPtr = unordered_map_find(_list->cameraIndices, _id);
	if (idxPtr) {
		_list->active = _id;
	}
}

Camera2D* GetCamera(CameraList* _list, CameraID _id) {
	if (!_list) { return NULL; }

	size_t* idxPtr = unordered_map_find(_list->cameraIndices, _id);
	if (idxPtr) {
		Camera2D* camera = free_list_get(_list->cameras, *idxPtr);
		return camera;
	}
	return NULL;
}

Camera2D* GetActiveCamera(CameraList* _list) {
	if (!_list) { return NULL; }

	size_t* idxPtr = unordered_map_find(_list->cameraIndices, GetActiveCameraID(_list));
	if (idxPtr) {
		Camera2D* camera = free_list_get(_list->cameras, *idxPtr);
		return camera;
	}
	return NULL;
}

CameraID GetActiveCameraID(CameraList* _list) {
	if (!_list) { return ID_NULL; }
	return _list->active;
}