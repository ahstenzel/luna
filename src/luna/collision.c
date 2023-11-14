#include "luna/collision.h"

CollisionList* CreateCollisionMap() {
	CollisionList* map = malloc(sizeof *map);
	if (!map) {
		return NULL;
	}
	map->collisions = unordered_map_create(Collision);
	map->buckets = unordered_map_create(CollisionBucket);
	if (!map->collisions || !map->buckets) {
		DestroyCollisionMap(map);
		return NULL;
	}
	return map;
}

void DestroyCollisionMap(CollisionList* _list) {
	if (_list) {
		unordered_map_destroy(_list->collisions);
		for(unordered_map_it_t* it = unordered_map_it(_list->buckets); it; unordered_map_it_next(it)) {
			CollisionBucket* bucket = it->data;
			unordered_map_destroy(bucket->contents);
		}
		unordered_map_destroy(_list->buckets);
	}
	free(_list);
}

CollisionID CreateCollision(CollisionList* _list, CollisionDesc _desc) {
	// Error check
	if (!_list) { return ID_NULL; }

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
	if (!_list) { return; }

	// Get collision
	Collision* collision = unordered_map_find(_list->collisions, _id);
	if (!collision) { return; }

	// Remove from buckets
	vector_t* buckets = _GetBuckets(_list, collision->boundingBox);
	if (!buckets) { return; }
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
	if (!_list) { return; }

	// Get collision
	Collision* collision = unordered_map_find(_list->collisions, _id);
	if (!collision || 
		(collision->boundingBox.x == _position.x && collision->boundingBox.y == _position.y)) { return; }

	// Remove from buckets
	vector_t* buckets = _GetBuckets(_list, collision->boundingBox);
	if (!buckets) { return; }
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
		LUNA_DBG_WARN("Failed to move collision buckets for collision id (%d), deleting to be safe...\n", _id);
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
	if (!_list) { goto collision_get_fail; }

	// Get collision
	Collision* collision = unordered_map_find(_list->collisions, _id);
	if (!collision) { goto collision_get_fail; }

	// Get buckets & merge contents
	candidateIds = unordered_map_create(unsigned char);
	buckets = _GetBuckets(_list, collision->boundingBox);
	if (!buckets) { goto collision_get_fail; }
	for(size_t i=0; i<vector_size(buckets); ++i) {
		uint32_t hash = *(uint32_t*)vector_get(buckets, i);
		CollisionBucket* bucket = unordered_map_find(_list->buckets, hash);
		for(unordered_map_it_t* it = unordered_map_it(bucket->contents); it; unordered_map_it_next(it)) {
			CollisionID candidateId = (CollisionID)it->key;
			if (!unordered_map_insert(candidateIds, candidateId, 0)) {
				goto collision_get_fail;
			}
		}
	}

	// Copy into results array
	*_numResults = unordered_map_size(candidateIds);
	if (*_numResults == 0) { goto collision_get_fail; }
	_results = calloc(*_numResults, sizeof *_results);
	if (!_results) { goto collision_get_fail; }
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
collision_get_fail:
	vector_destroy(buckets);
	unordered_map_destroy(candidateIds);
	free(_results);
	_results = NULL;
	*_numResults = 0;
	return;
}

bool CheckCollision(CollisionList* _list, CollisionID _id1, CollisionID _id2) {
	// Error check
	if (!_list) { return false; }
	
	Collision* c1 = unordered_map_find(_list->collisions, _id1);
	Collision* c2 = unordered_map_find(_list->collisions, _id2);
	if (!c1 || !c2) { return false; }
	return CheckCollisionRecs(c1->boundingBox, c2->boundingBox);
}

vector_t* _GetBuckets(CollisionList* _list, Rectangle _boundingBox) {
	vector_t* results = vector_create(uint32_t);
	if (!results) { return NULL; }

	// Scan along x coordinates
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
				if (!unordered_map_insert(_list->buckets, hash, &bucket)) { goto collision_get_buckets_fail; }
			}
		}
	}

	return results;
collision_get_buckets_fail:
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