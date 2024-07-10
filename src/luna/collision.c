#include "luna/collision.h"

CollisionList* _CreateCollisionList() {
	CollisionList* list = calloc(1, sizeof *list);
	if (!list) {
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate collision list!");
		return NULL;
	}

	list->_collisions = unordered_map_create(Collision);
	list->_buckets = unordered_map_create(CollisionBucket);
	if (!list->_collisions || !list->_buckets) {
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate collision list containers!");
		_DestroyCollisionList(list);
		return NULL;
	}
	return list;
}

void _DestroyCollisionList(CollisionList* _list) {
	if (_list) {
		unordered_map_destroy(_list->_collisions);
		for(unordered_map_it_t* it = unordered_map_it(_list->_buckets); it; unordered_map_it_next(it)) {
			CollisionBucket* bucket = it->data;
			unordered_map_destroy(bucket->_contents);
		}
		unordered_map_destroy(_list->_buckets);
		free(_list);
	}
}

size_t GetCollisionListSize(CollisionList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid collision list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return 0; 
	}
	return unordered_map_size(_list->_collisions);
}

CollisionListIt* CollisionListItBegin(CollisionList* _list) {
	LUNA_RETURN_CLEAR;
	// Error check
	CollisionListIt* it = NULL;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid collision list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto collision_list_it_begin_fail;
	}
	if (unordered_map_size(_list->_collisions) == 0) { 
		goto collision_list_it_begin_fail; 
	}

	// Create map iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate collision list iterator!");
		goto collision_list_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = unordered_map_it(_list->_collisions);
	if (!it->_ptr) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate collision list index map iterator!");
		goto collision_list_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DEBUG_WARN("Collision list index map iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto collision_list_it_begin_fail; 
	}

	// Retrieve collision data from list
	it->data = (Collision*)it->_ptr->data;
	it->id = (CollisionID)it->_ptr->key;
	if (!it->data) { 
		LUNA_DEBUG_WARN("Collision list iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto collision_list_it_begin_fail; 
	}

	return it;
collision_list_it_begin_fail:
	free(it);
	return NULL;
}

CollisionListIt* CollisionListItNext(CollisionListIt* _it) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_it) { return NULL; }

	// Advance to next valid element
	unordered_map_it_next(_it->_ptr);
	if (_it->_ptr) {
		// Update iterator contents
		_it->data = (Collision*)_it->_ptr->data;
		_it->id = (CollisionID)_it->_ptr->key;
		if (!_it->data) { 
			LUNA_DEBUG_WARN("Next iterator position yielded invalid collision data!");
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

CollisionID CreateCollision(CollisionList* _list, CollisionDesc _desc) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid collision list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return ID_NULL; 
	}

	// Register collision
	CollisionID id = _luna_id_generate();
	Collision collision = {
		._boundingBox = _desc.boundingBox,
		._id = id,
		._rounded = _desc.rounded
	};
	unordered_map_insert(_list->_collisions, id, &collision);

	// Add to buckets
	vector_t* buckets = _GetBuckets(_list, collision._boundingBox);
	if (!buckets) {
		LUNA_DEBUG_WARN("Failed to retrieve buckets for collision box! (x: %f, y: %f, w: %f, h: %f)", 
			collision._boundingBox.x, 
			collision._boundingBox.y, 
			collision._boundingBox.width, 
			collision._boundingBox.height
		);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		unordered_map_delete(_list->_collisions, id);
		return ID_NULL;
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->_buckets, hash);
		unordered_map_insert(bucket->_contents, id, 0);
	}
	vector_destroy(buckets);
	return id;
}

void DestroyCollision(CollisionList* _list, CollisionID _id) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid collision list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	// Get collision
	Collision* collision = unordered_map_find(_list->_collisions, _id);
	if (!collision) { 
		LUNA_DEBUG_WARN("Invalid collision id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return; 
	}

	// Remove from buckets
	vector_t* buckets = _GetBuckets(_list, collision->_boundingBox);
	if (!buckets) { 
		LUNA_DEBUG_WARN("Failed to retrieve buckets for collision id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		return; 
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->_buckets, hash);
		unordered_map_delete(bucket->_contents, _id);

		// Cull empty buckets
		if (unordered_map_size(bucket->_contents) == 0) {
			unordered_map_destroy(bucket->_contents);
			unordered_map_delete(_list->_buckets, hash);
		}
	}
	vector_destroy(buckets);

	// Unregister collision
	unordered_map_delete(_list->_collisions, _id);
}

void SetCollisionPosition(CollisionList* _list, CollisionID _id, Vector2 _position) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid collision list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	// Get collision
	Collision* collision = unordered_map_find(_list->_collisions, _id);
	if (!collision) {
		LUNA_DEBUG_WARN("Invalid collision id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return;
	}

	// Skip idempotent updates
	if ((collision->_boundingBox.x == _position.x && collision->_boundingBox.y == _position.y)) { return; }

	// Remove from buckets
	vector_t* buckets = _GetBuckets(_list, collision->_boundingBox);
	if (!buckets) { 
		LUNA_DEBUG_WARN("Failed to retrieve buckets for collision id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		return; 
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->_buckets, hash);
		unordered_map_delete(bucket->_contents, _id);
	}
	vector_destroy(buckets);

	// Update property
	collision->_boundingBox.x = _position.x;
	collision->_boundingBox.y = _position.y;

	// Add to new buckets
	buckets = _GetBuckets(_list, collision->_boundingBox);
	if (!buckets) { 
		LUNA_DEBUG_WARN("Failed to move buckets for collision id (%d), deleting to be safe...", _id);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		unordered_map_delete(_list->_collisions, _id);
		return;
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->_buckets, hash);
		unordered_map_insert(bucket->_contents, _id, 0);
	}
	vector_destroy(buckets);
}

size_t GetCollisionsAtPlace(CollisionList* _list, CollisionID _id, CollisionID* _results) {
	LUNA_RETURN_CLEAR;
	// Error check
	size_t numResults = 0;
	vector_t* buckets = NULL;
	unordered_map_t* candidateIds = NULL;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid collision list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto get_collisions_at_place_fail;
	}

	// Get collision
	Collision* collision = unordered_map_find(_list->_collisions, _id);
	if (!collision) { 
		LUNA_DEBUG_WARN("Invalid collision id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		goto get_collisions_at_place_fail; 
	}

	// Get buckets & merge contents
	candidateIds = unordered_map_create(unsigned char);
	buckets = _GetBuckets(_list, collision->_boundingBox);
	if (!buckets) { 
		LUNA_DEBUG_WARN("Failed to retrieve buckets for collision id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto get_collisions_at_place_fail; 
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->_buckets, hash);
		for(unordered_map_it_t* it = unordered_map_it(bucket->_contents); it; unordered_map_it_next(it)) {
			CollisionID candidateId = (CollisionID)it->key;
			if (!unordered_map_insert(candidateIds, candidateId, 0)) {
				LUNA_DEBUG_WARN("Failed to merge buckets for collision id (%d)!", (int)_id);
				LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
				goto get_collisions_at_place_fail;
			}
		}
	}

	// Copy into results array
	numResults = unordered_map_size(candidateIds);
	if (numResults == 0) { goto get_collisions_at_place_fail; }
	_results = calloc(numResults, sizeof *_results);
	if (!_results) { goto get_collisions_at_place_fail; }
	size_t idx = 0;
	for(unordered_map_it_t* it = unordered_map_it(candidateIds); it; unordered_map_it_next(it)) {
		CollisionID candidateId = (CollisionID)it->key;
		if (CheckCollision(_list, _id, candidateId)) {
			_results[idx++] = (CollisionID)it->key;
		}
	}

	vector_destroy(buckets);
	unordered_map_destroy(candidateIds);
	return numResults;
get_collisions_at_place_fail:
	vector_destroy(buckets);
	unordered_map_destroy(candidateIds);
	free(_results);
	return 0;
}

bool CheckCollision(CollisionList* _list, CollisionID _id1, CollisionID _id2) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid collision list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return false;
	}
	
	Collision* c1 = unordered_map_find(_list->_collisions, _id1);
	Collision* c2 = unordered_map_find(_list->_collisions, _id2);
	if (!c1) {
		LUNA_DEBUG_WARN("Invalid collision id (%d)!", (int)_id1);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return false;
	}
	if (!c2) {
		LUNA_DEBUG_WARN("Invalid collision id (%d)!", (int)_id2);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return false;
	}
	return CheckCollisionRecs(c1->_boundingBox, c2->_boundingBox);
}

vector_t* _GetBuckets(CollisionList* _list, Rectangle _boundingBox) {
	vector_t* results = NULL;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid collision list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto get_buckets_fail;
	}

	// Scan along x coordinates
	results = vector_create(uint32_t);
	int32_t xmin = (int32_t)(_boundingBox.x / LUNA_COLLISION_BUCKET_SIZE);
	int32_t xmax = (int32_t)((_boundingBox.x + _boundingBox.width) / LUNA_COLLISION_BUCKET_SIZE);
	for(int32_t x=xmin; x<=xmax; ++x) {
		// Scan along y coordinates
		int32_t ymin = (int32_t)(_boundingBox.y / LUNA_COLLISION_BUCKET_SIZE);
		int32_t ymax = (int32_t)((_boundingBox.y + _boundingBox.height) / LUNA_COLLISION_BUCKET_SIZE);
		for(int32_t y=ymin; y<=ymax; ++y) {
			// Hash bucket coordinates
			Vector2i vec = {x, y};
			uint32_t hash = _HashVector2i(vec);
			vector_push_back(results, &hash);

			// Create bucket if it doesn't already exist
			if (!unordered_map_find(_list->_buckets, hash)) {
				CollisionBucket bucket = {
					._contents = unordered_map_create(unsigned char)
				};
				if (!unordered_map_insert(_list->_buckets, hash, &bucket)) {
					LUNA_DEBUG_WARN("Failed to create collision bucket!");
					LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
					goto get_buckets_fail; 
				}
			}
		}
	}

	return results;
get_buckets_fail:
	// Delete any empty buckets that were created during the search
	for(size_t i=0; i<vector_size(results); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(results, i);
		CollisionBucket* bucket = unordered_map_find(_list->_buckets, hash);
		if (bucket && unordered_map_size(bucket->_contents) == 0) {
			unordered_map_destroy(bucket->_contents);
			unordered_map_delete(_list->_buckets, hash);
		}
	}
	return NULL;
}

uint32_t _HashVector2i(Vector2i _v) {
	uint32_t seed = _coll_fnv_offset;
	uint32_t h1 = _coll_fnv_offset;
	uint32_t h2 = _coll_fnv_offset;

	h1 ^= (_v.x & 0xFF);
	h1 *= _coll_fnv_prime;
	h1 ^= ((_v.x >> 8) & 0xFF);
	h1 *= _coll_fnv_prime;
	h1 ^= ((_v.x >> 16) & 0xFF);
	h1 *= _coll_fnv_prime;
	h1 ^= ((_v.x >> 24) & 0xFF);
	h1 *= _coll_fnv_prime;
	seed ^= h1 + 0x9E3779B9 + (seed << 6) + (seed >> 2);

	h2 ^= (_v.y & 0xFF);
	h2 *= _coll_fnv_prime;	
	h2 ^= ((_v.y >> 8) & 0xFF);
	h2 *= _coll_fnv_prime;
	h2 ^= ((_v.y >> 16) & 0xFF);
	h2 *= _coll_fnv_prime;
	h2 ^= ((_v.y >> 24) & 0xFF);
	h2 *= _coll_fnv_prime;
	seed ^= h2 + 0x9E3779B9 + (seed << 6) + (seed >> 2);

	return seed;
}