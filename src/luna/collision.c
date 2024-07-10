#include "luna/collision.h"

CollisionList* _CreateCollisionList() {
	CollisionList* list = calloc(1, sizeof *list);
	if (!list) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate collision list!");
		return NULL;
	}

	list->collisions = unordered_map_create(Collision);
	list->buckets = unordered_map_create(CollisionBucket);
	if (!list->collisions || !list->buckets) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate collision list containers!");
		_DestroyCollisionList(list);
		return NULL;
	}
	return list;
}

void _DestroyCollisionList(CollisionList* _list) {
	if (_list) {
		unordered_map_destroy(_list->collisions);
		for(unordered_map_it_t* it = unordered_map_it(_list->buckets); it; unordered_map_it_next(it)) {
			CollisionBucket* bucket = it->data;
			unordered_map_destroy(bucket->contents);
		}
		unordered_map_destroy(_list->buckets);
		free(_list);
	}
}

size_t GetCollisionListSize(CollisionList* _list) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid collision list reference!");
		return 0; 
	}
	return unordered_map_size(_list->collisions);
}

CollisionListIt* CollisionListItBegin(CollisionList* _list) {
	// Error check
	CollisionListIt* it = NULL;
	if (!_list) {
		LUNA_DBG_WARN("Invalid collision list reference!");
		goto collision_list_it_begin_fail;
	}
	if (unordered_map_size(_list->collisions) == 0) { 
		goto collision_list_it_begin_fail; 
	}

	// Create map iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate collision list iterator!");
		goto collision_list_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = unordered_map_it(_list->collisions);
	if (!it->_ptr) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate collision list index map iterator!");
		goto collision_list_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DBG_WARN("Collision list index map iterator points to invalid data!");
		goto collision_list_it_begin_fail; 
	}

	// Retrieve collision data from list
	it->data = (Collision*)it->_ptr->data;
	it->id = (CollisionID)it->_ptr->key;
	if (!it->data) { 
		LUNA_DBG_WARN("Collision list iterator points to invalid data!");
		goto collision_list_it_begin_fail; 
	}

	return it;
collision_list_it_begin_fail:
	free(it);
	return NULL;
}

void CollisionListItNext(CollisionListIt** _it) {
	// Error check
	if (!_it || !(*_it)) { return; }

	// Advance to next valid element
	CollisionListIt* it = *_it;
	unordered_map_it_next(it->_ptr);
	if (it->_ptr) {
		// Update iterator contents
		it->data = (Collision*)it->_ptr->data;
		it->id = (CollisionID)it->_ptr->key;
		if (!it->data) { 
			LUNA_DBG_WARN("Next iterator position yielded invalid collision data!");
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

CollisionID CreateCollision(CollisionList* _list, CollisionDesc _desc) {
	// Error check
	if (!_list) { 
		LUNA_DBG_WARN("Invalid collision list reference!");
		return ID_NULL; 
	}

	// Register collision
	CollisionID id = _luna_id_generate();
	Collision collision = {
		.boundingBox = _desc.boundingBox,
		.id = id,
		.rounded = _desc.rounded
	};
	unordered_map_insert(_list->collisions, id, &collision);

	// Add to buckets
	vector_t* buckets = _GetBuckets(_list, collision.boundingBox);
	if (!buckets) {
		LUNA_DBG_WARN("Failed to retrieve buckets for collision box! (x: %f, y: %f, w: %f, h: %f)", 
			collision.boundingBox.x, 
			collision.boundingBox.y, 
			collision.boundingBox.width, 
			collision.boundingBox.height
		);
		unordered_map_delete(_list->collisions, id);
		return ID_NULL;
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->buckets, hash);
		unordered_map_insert(bucket->contents, id, 0);
	}
	vector_destroy(buckets);
	return id;
}

void DestroyCollision(CollisionList* _list, CollisionID _id) {
	// Error check
	if (!_list) { 
		LUNA_DBG_WARN("Invalid collision list reference!");
		return; 
	}

	// Get collision
	Collision* collision = unordered_map_find(_list->collisions, _id);
	if (!collision) { 
		LUNA_DBG_WARN("Invalid collision id (%d)!", (int)_id);
		return; 
	}

	// Remove from buckets
	vector_t* buckets = _GetBuckets(_list, collision->boundingBox);
	if (!buckets) { 
		LUNA_DBG_WARN("Failed to retrieve buckets for collision id (%d)!", (int)_id);
		return; 
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->buckets, hash);
		unordered_map_delete(bucket->contents, _id);

		// Cull empty buckets
		if (unordered_map_size(bucket->contents) == 0) {
			unordered_map_destroy(bucket->contents);
			unordered_map_delete(_list->buckets, hash);
		}
	}
	vector_destroy(buckets);

	// Unregister collision
	unordered_map_delete(_list->collisions, _id);
}

void SetCollisionPosition(CollisionList* _list, CollisionID _id, Vector2 _position) {
	// Error check
	if (!_list) { 
		LUNA_DBG_WARN("Invalid collision list reference!");
		return; 
	}

	// Get collision
	Collision* collision = unordered_map_find(_list->collisions, _id);
	if (!collision) {
		LUNA_DBG_WARN("Invalid collision id (%d)!", (int)_id);
		return; 
	}

	// Don't update properties
	if ((collision->boundingBox.x == _position.x && collision->boundingBox.y == _position.y)) { return; }

	// Remove from buckets
	vector_t* buckets = _GetBuckets(_list, collision->boundingBox);
	if (!buckets) { 
		LUNA_DBG_WARN("Failed to retrieve buckets for collision id (%d)!", (int)_id);
		return; 
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->buckets, hash);
		unordered_map_delete(bucket->contents, _id);
	}
	vector_destroy(buckets);

	// Update property
	collision->boundingBox.x = _position.x;
	collision->boundingBox.y = _position.y;

	// Add to new buckets
	buckets = _GetBuckets(_list, collision->boundingBox);
	if (!buckets) { 
		LUNA_DBG_WARN("Failed to move buckets for collision id (%d), deleting to be safe...", _id);
		unordered_map_delete(_list->collisions, _id);
		return;
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->buckets, hash);
		unordered_map_insert(bucket->contents, _id, 0);
	}
	vector_destroy(buckets);

	return;
}

void GetCollisionsAtPlace(CollisionList* _list, CollisionID _id, CollisionID* _results, size_t* _numResults) {
	// Error check
	vector_t* buckets = NULL;
	unordered_map_t* candidateIds = NULL;
	if (!_list) { 
		LUNA_DBG_WARN("Invalid collision list reference!");
		goto get_collisions_at_place_fail;
	}

	// Get collision
	Collision* collision = unordered_map_find(_list->collisions, _id);
	if (!collision) { 
		LUNA_DBG_WARN("Invalid collision id (%d)!", (int)_id);
		goto get_collisions_at_place_fail; 
	}

	// Get buckets & merge contents
	candidateIds = unordered_map_create(unsigned char);
	buckets = _GetBuckets(_list, collision->boundingBox);
	if (!buckets) { 
		LUNA_DBG_WARN("Failed to retrieve buckets for collision id (%d)!", (int)_id);
		goto get_collisions_at_place_fail; 
	}
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->buckets, hash);
		for(unordered_map_it_t* it = unordered_map_it(bucket->contents); it; unordered_map_it_next(it)) {
			CollisionID candidateId = (CollisionID)it->key;
			if (!unordered_map_insert(candidateIds, candidateId, 0)) {
				LUNA_DBG_WARN("Failed to merge buckets for collision id (%d)!", (int)_id);
				goto get_collisions_at_place_fail;
			}
		}
	}

	// Copy into results array
	*_numResults = unordered_map_size(candidateIds);
	if (*_numResults == 0) { goto get_collisions_at_place_fail; }
	_results = calloc(*_numResults, sizeof *_results);
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
	return;
get_collisions_at_place_fail:
	vector_destroy(buckets);
	unordered_map_destroy(candidateIds);
	free(_results);
	_results = NULL;
	*_numResults = 0;
	return;
}

bool CheckCollision(CollisionList* _list, CollisionID _id1, CollisionID _id2) {
	// Error check
	if (!_list) { 
		LUNA_DBG_WARN("Invalid collision list reference!");
		return false;
	}
	
	Collision* c1 = unordered_map_find(_list->collisions, _id1);
	Collision* c2 = unordered_map_find(_list->collisions, _id2);
	if (!c1) {
		LUNA_DBG_WARN("Invalid collision id (%d)!", (int)_id1);
		return false;
	}
	if (!c2) {
		LUNA_DBG_WARN("Invalid collision id (%d)!", (int)_id2);
		return false;
	}
	return CheckCollisionRecs(c1->boundingBox, c2->boundingBox);
}

vector_t* _GetBuckets(CollisionList* _list, Rectangle _boundingBox) {
	vector_t* results = NULL;
	if (!_list) { 
		LUNA_DBG_WARN("Invalid collision list reference!");
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
			if (!unordered_map_find(_list->buckets, hash)) {
				CollisionBucket bucket = {
					.contents = unordered_map_create(unsigned char)
				};
				if (!unordered_map_insert(_list->buckets, hash, &bucket)) {
					LUNA_DBG_WARN("Failed to create collision bucket!");
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
		CollisionBucket* bucket = unordered_map_find(_list->buckets, hash);
		if (bucket && unordered_map_size(bucket->contents) == 0) {
			unordered_map_destroy(bucket->contents);
			unordered_map_delete(_list->buckets, hash);
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