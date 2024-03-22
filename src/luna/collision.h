#ifndef LUNA_COLLISION_H
#define LUNA_COLLISION_H
/**
 * collision.h
 * 2D Collision objects.
*/
#include "luna/common.h"

/// @brief Unique collision identifier
typedef _LUNA_ID_TYPE CollisionID;

#ifndef LUNA_COLLISION_BUCKET_SIZE
#define LUNA_COLLISION_BUCKET_SIZE 33
#endif
#define _coll_fnv_offset 2166136261U;
#define _coll_fnv_prime 16777619U;

/// @brief Single collision object
typedef struct {
	Rectangle boundingBox;
	CollisionID id;
	bool rounded;
} Collision;

/// @brief Descriptor for creating a collision object
typedef struct {
	Rectangle boundingBox;
	bool rounded;
} CollisionDesc;

/// @brief Single cell in space containing collision objects
typedef struct {
	unordered_map_t* contents;
} CollisionBucket;

/// @brief Spatial hash containing cells of collision objects
typedef struct {
	unordered_map_t* collisions;
	unordered_map_t* buckets;
} CollisionList;

/// @brief Create a new 2D collision list.
/// @return Collision list pointer
CollisionList* CreateCollisionMap();

/// @brief Deallocate a 2D collision list.
/// @param _list Collision List pointer
void DestroyCollisionMap(CollisionList* _list);

/// @brief Create a collision object in the map.
/// @param _list Collision List pointer
/// @param _desc Collision descriptor
/// @return Collision id
CollisionID CreateCollision(CollisionList* _list, CollisionDesc _desc);

/// @brief Remove the collision object from the map.
/// @param _list Collision List pointer
/// @param _id Collision id
void DestroyCollision(CollisionList* _list, CollisionID _id);

/// @brief Change the position of the collision object in the map.
/// @param _list Collision List pointer
/// @param _id Collision id
/// @param _position Updated position
void SetCollisionPosition(CollisionList* _list, CollisionID _id, Vector2 _position);

/// @brief Get a list of all collision objects overlapping with the given collision object.
/// @param _list Collision List pointer
/// @param _id Collision id
/// @param _results Array pointer (will contain an allocated array of collision ids. Must be deallocated later)
/// @param _numResults Array size (will contain the number of valid elements in the results array. Note that the actual allocated size of the array may be larger than this)
void GetCollisionsAtPlace(CollisionList* _list, CollisionID _id, CollisionID* _results, size_t* _numResults);

/// @brief Check if the two given collision objects are overlapping.
/// @param _list Collision List pointer
/// @param _id1 Collision id 1
/// @param _id2 Collision id 2
/// @return True if overlapping
bool CheckCollision(CollisionList* _list, CollisionID _id1, CollisionID _id2);

/// @brief Get all buckets that overlap the given rectangle.
/// @param _list Collision List pointer
/// @param _boundingBox Collision rectangle
vector_t* _GetBuckets(CollisionList* _list, Rectangle _boundingBox);

uint32_t _HashVector2i(Vector2i _v);

#endif