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
	Rectangle _boundingBox;			// Position & size of collision box
	const CollisionID _id;			// Unique collision ID
	bool _rounded;					// If true, use a circular (or elliptical) collision boundary
} Collision;

/// @brief Descriptor for creating a collision object
typedef struct {
	Rectangle boundingBox;			// Position & size of collision box
	bool rounded;					// If true, use a circular (or elliptical) collision boundary
} CollisionDesc;

/// @brief Single cell in space containing collision objects
typedef struct {
	unordered_map_t* _contents;		// Collision objects inside this bucket
} CollisionBucket;

/// @brief Spatial hash containing cells of collision objects
typedef struct {
	unordered_map_t* _collisions;	// Container of collision data
	unordered_map_t* _buckets;		// Container of collision buckets
} CollisionList;

/// @brief Iterator for collision objects
typedef struct {
	CollisionList* _list;			// Collision list to iterate through
	unordered_map_it_t* _ptr;		// Internal iterator reference
	CollisionID id;					// Unique collision ID
	Collision* data;				// Collision data structure
} CollisionListIt;

/// @brief Create a new 2D collision list.
/// @return Collision list pointer
CollisionList* _CreateCollisionList();

/// @brief Deallocate a 2D collision list.
/// @param _list Collision list pointer
void _DestroyCollisionList(CollisionList* _list);

/// @brief Get the number of collisions in the list.
/// @param _list Collision list pointer
/// @return Number of collisions
size_t GetCollisionListSize(CollisionList* _list);

/// @brief Get an iterator for the list of collisions.
/// @param _list Collision list pointer
/// @return Collision iterator
CollisionListIt* CollisionListItBegin(CollisionList* _list);

/// @brief Move the iterator to the next element.
/// @param _it Collision iterator
/// @return Updated collision iterator (or NULL)
CollisionListIt* CollisionListItNext(CollisionListIt* _it);

/// @brief Create a collision object in the map.
/// @param _list Collision list pointer
/// @param _desc Collision descriptor
/// @return Collision id (or ID_NULL on error)
CollisionID CreateCollision(CollisionList* _list, CollisionDesc _desc);

/// @brief Remove the collision object from the map.
/// @param _list Collision list pointer
/// @param _id Collision id
void DestroyCollision(CollisionList* _list, CollisionID _id);

/// @brief Change the position of the collision object in the map.
/// @param _list Collision list pointer
/// @param _id Collision id
/// @param _position Updated position
void SetCollisionPosition(CollisionList* _list, CollisionID _id, Vector2 _position);

/// @brief Get a list of all collision objects overlapping with the given collision object.
/// @param _list Collision list pointer
/// @param _id Collision id
/// @param _results Array pointer (will contain an allocated array of collision ids. Must be deallocated later)
/// @return Array size (will contain the number of valid elements in the results array. Note that the actual allocated size of the array may be larger than this)
size_t GetCollisionsAtPlace(CollisionList* _list, CollisionID _id, CollisionID* _results);

/// @brief Check if the two given collision objects are overlapping.
/// @param _list Collision list pointer
/// @param _id1 Collision id 1
/// @param _id2 Collision id 2
/// @return True if overlapping
bool CheckCollision(CollisionList* _list, CollisionID _id1, CollisionID _id2);

/// @brief Get all buckets that overlap the given rectangle.
/// @param _list Collision list pointer
/// @param _boundingBox Collision rectangle
vector_t* _GetBuckets(CollisionList* _list, Rectangle _boundingBox);

/// @brief Hash bucket coordinates into map key.
/// @param _v Bucket coordinates
/// @return Map key
uint32_t _HashVector2i(Vector2i _v);

#endif