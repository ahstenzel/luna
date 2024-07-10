#ifndef LUNA_CAMERA_H
#define LUNA_CAMERA_H
/**
 * camera.h
 * 2D Camera object.
*/
#include "luna/common.h"

typedef _LUNA_ID_TYPE CameraID;

/// @brief Descriptor for creating a 2D camera object
typedef struct {
	Vector2 target;						// Point in worldspace the camera should follow
	Vector2 offset;						// Offset of cameras target in screenspace
	float rotation;						// Rotation angle (degrees)
	float zoom;							// Zoom level
} CameraDesc;

/// @brief Organized list of camera objects
typedef struct {
	unordered_map_t* _cameraIndices;	// Map camera IDs to container indices
	free_list_t* _cameras;				// Container of camera data
	CameraID _active;					// Which camera is currently being drawn
} CameraList;

/// @brief Iterator for camera objects
typedef struct {
	CameraList* _list;					// Camera list to iterate through
	unordered_map_it_t* _ptr;			// Internal iterator reference
	CameraID id;						// Unique camera ID
	Camera2D* data;						// Camera data structure
} CameraListIt;

/// @brief Create a new camera list.
/// @return Camera list pointer
CameraList* _CreateCameraList();

/// @brief Deallocate a camera list.
/// @param _list Camera list pointer
void _DestroyCameraList(CameraList* _list);

/// @brief Get the number of cameras in the list.
/// @param _list Camera list pointer
/// @return Number of cameras
size_t GetCameraListSize(CameraList* _list);

/// @brief Get an iterator for the list of cameras.
/// @param _list Camera list pointer
/// @return Camera iterator
CameraListIt* CameraListItBegin(CameraList* _list);

/// @brief Move the iterator to the next element.
/// @param _it Pointer to camera iterator
void CameraListItNext(CameraListIt** _it);

/// @brief Create a new 2D camera object and add it to the list.
/// @param _list Camera list pointer
/// @param _desc Camera descriptor
/// @return Camera id (or ID_NULL on error)
CameraID CreateCamera(CameraList* _list, CameraDesc _desc);

/// @brief Remove the given camera object from the list.
/// @param _list Camera list pointer
/// @param _id Camera id
void DestroyCamera(CameraList* _list, CameraID _id);

/// @brief Set which camera in the list should be used for drawing.
/// @param _list Camera list pointer
/// @param _id Camera id (or ID_NULL to disable cameras)
void SetActiveCamera(CameraList* _list, CameraID _id);

/// @brief Get a raylib camera handle for the given camera object.
/// @param _list Camera list pointer
/// @param _id Camera id
/// @return Camera pointer
Camera2D* GetCamera(CameraList* _list, CameraID _id);

/// @brief Get a raylib camera handle for the active camera object.
/// @param _list Camera list pointer
/// @return Camera pointer
Camera2D* GetActiveCamera(CameraList* _list);

/// @brief Get the id of the active camera object.
/// @param _list Camera list pointer
/// @return Camera id
CameraID GetActiveCameraID(CameraList* _list);

#endif