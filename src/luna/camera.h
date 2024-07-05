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
	Vector2 target;		/* Point in worldspace the camera should follow */
	Vector2 offset;		/* Offset of cameras target in screenspace */
	float rotation;		/* Angle of rotation */
	float zoom;			/* Zoom level */
} CameraDesc;

/// @brief Organized list of camera objects
typedef struct {
	unordered_map_t* cameraIndices;
	free_list_t* cameras;
	CameraID active;
} CameraList;

/// @brief Create a new camera list.
/// @return Camera list pointer
CameraList* _CreateCameraList();

/// @brief Deallocate a camera list.
/// @param _list Camera list pointer
void _DestroyCameraList(CameraList* _list);

/// @brief Create a new 2D camera object and add it to the list.
/// @param _list Camera list pointer
/// @param _desc Camera descriptor
/// @return Camera id
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