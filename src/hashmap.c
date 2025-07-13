#include "hashmap.h"

#define ENV_BUCKET_COUNT 16
#define LOAD_FACTOR_THRESHOLD 0.75

hashmap_t* hashmap_new() {
    hashmap_t* hashmap = malloc(sizeof(hashmap_t));
    ASSERTNULL(hashmap, "error allocating hashmap");
    hashmap->buckets = calloc(ENV_BUCKET_COUNT, sizeof(hashmap_node_t*));
    ASSERTNULL(hashmap->buckets, "error allocating buckets");
    hashmap->bucket_count = ENV_BUCKET_COUNT;
    hashmap->size = 0;
    return hashmap;
}

void hashmap_free(hashmap_t* _hashmap) {
    if (!_hashmap) return;

    // Free all nodes in each bucket
    for (size_t i = 0; i < _hashmap->bucket_count; i++) {
        hashmap_node_t* node = _hashmap->buckets[i];
        while (node) {
            hashmap_node_t* next = node->next;
            free(node->key);
            free(node);
            node = next;
        }
    }

    free(_hashmap->buckets);
    free(_hashmap);
}

INTERNAL void hashmap_rehash(hashmap_t* _hashmap) {
    size_t new_bucket_count = _hashmap->bucket_count * 2;
    hashmap_node_t** new_buckets = calloc(new_bucket_count, sizeof(hashmap_node_t*));
    ASSERTNULL(new_buckets, "error allocating buckets");

    for (size_t i = 0; i < _hashmap->bucket_count; i++) {
        hashmap_node_t* node = _hashmap->buckets[i];
        while (node) {
            hashmap_node_t* next = node->next;
            size_t index = object_hash(node->key) % new_bucket_count;
            node->next = new_buckets[index];
            new_buckets[index] = node;
            node = next;
        }
    }

    free(_hashmap->buckets);
    _hashmap->buckets = new_buckets;
    _hashmap->bucket_count = new_bucket_count;
}

bool hashmap_has(hashmap_t* _hashmap, object_t* _key) {
    ASSERTNULL(_hashmap, "hashmap is null");
    ASSERTNULL(_key, "key is null");
    
    size_t hash = object_hash(_key);
    hashmap_node_t* node = _hashmap->buckets[hash % _hashmap->bucket_count];
    while (node) {
        if (object_equals(node->key, _key)) return true;
        node = node->next;
    }
    return false;
}

void hashmap_put(hashmap_t* _hashmap, object_t* _key, object_t* _value) {
    ASSERTNULL(_hashmap, "hashmap is null");
    ASSERTNULL(_key, "key is null");
    ASSERTNULL(_value, "value is null");

    size_t hash = object_hash(_key);
    size_t index = hash % _hashmap->bucket_count;
    hashmap_node_t* node = _hashmap->buckets[index];

    // Check for existing entry
    while (node) {
        if (object_equals(node->key, _key)) {
            node->value = _value;
            return;
        }
        node = node->next;
    }

    // Create new node and insert at head of chain
    node = malloc(sizeof(hashmap_node_t));
    ASSERTNULL(node, "error allocating hashmap node");
    node->key = _key;
    node->value = _value;
    node->next = _hashmap->buckets[index];
    _hashmap->buckets[index] = node;
    _hashmap->size++;

    // Check load factor and rehash if needed
    if (_hashmap->size > _hashmap->bucket_count * LOAD_FACTOR_THRESHOLD) {
        hashmap_rehash(_hashmap);
    }
}

object_t* hashmap_get(hashmap_t* _hashmap, object_t* _key) {
    ASSERTNULL(_hashmap, "hashmap is null");
    ASSERTNULL(_key, "key is null");

    size_t hash = object_hash(_key);
    hashmap_node_t* node = _hashmap->buckets[hash % _hashmap->bucket_count];
    while (node) {
        if (object_equals(node->key, _key)) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

void hashmap_extend(hashmap_t* _hashmap, hashmap_t* _other) {
    ASSERTNULL(_hashmap, "hashmap is null");
    ASSERTNULL(_other, "other is null");
    
    // Early return if other hashmap is empty
    if (_other->size == 0) return;
    
    // Pre-allocate space for the combined size
    size_t combined_size = _hashmap->size + _other->size;
    if (combined_size > _hashmap->bucket_count * LOAD_FACTOR_THRESHOLD) {
        // Calculate new bucket count to avoid multiple rehashes
        size_t new_bucket_count = _hashmap->bucket_count;
        while (combined_size > new_bucket_count * LOAD_FACTOR_THRESHOLD) {
            new_bucket_count *= 2;
        }
        
        // Resize once before adding elements
        hashmap_rehash(_hashmap);
    }
    
    // Now add all elements without triggering additional rehashes
    for (size_t i = 0; i < _other->bucket_count; i++) {
        hashmap_node_t* node = _other->buckets[i];
        while (node) {
            hashmap_put(_hashmap, node->key, node->value);
            node = node->next;
        }
    }
}

size_t hashmap_size(hashmap_t* _hashmap) {
    ASSERTNULL(_hashmap, "hashmap is null");
    return _hashmap->size;
}