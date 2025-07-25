#include "api/core/global.h"
#include "api/core/object.h"
#include "internal.h"

#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct hashmap_node_struct hashmap_node_t;
typedef struct hashmap_node_struct {
    object_t* key;
    object_t* value;
    hashmap_node_t* next;
} hashmap_node_t;

typedef struct hashmap_struct {
    hashmap_node_t** buckets;
    size_t bucket_count;
    size_t size;
} hashmap_t;

/*
 * Create a new hashmap.
 *
 * @return The new hashmap.
 */
hashmap_t* hashmap_new();

/*
 * Free the hashmap.
 *
 * @param _hashmap The hashmap.
 */
void hashmap_free(hashmap_t* _hashmap);

/*
 * Check if the hashmap has a key.
 *
 * @param _hashmap The hashmap.
 * @param _key The key.
 * @return True if the hashmap has the key, false otherwise.
 */
bool hashmap_has(hashmap_t* _hashmap, object_t* _key);

/*
 * Check if the hashmap has a key.
 *
 * @param _hashmap The hashmap.
 * @param _key The key.
 * @return True if the hashmap has the key, false otherwise.
 */
bool hashmap_has_string(hashmap_t* _hashmap, char* _key);

/*
 * Put a key-value pair into the hashmap.
 *
 * @param _hashmap The hashmap.
 * @param _key The key.
 * @param _value The value.
 */
void hashmap_put(hashmap_t* _hashmap, object_t* _key, object_t* _value);

/*
 * Get a value from the hashmap.
 *
 * @param _hashmap The hashmap.
 * @param _key The key.
 * @return The value.
 */
object_t* hashmap_get(hashmap_t* _hashmap, object_t* _key);

/*
 * Get a value from the hashmap.
 *
 * @param _hashmap The hashmap.
 * @param _key The key.
 * @return The value.
 */
object_t* hashmap_get_string(hashmap_t* _hashmap, char* _key);

/*
 * Extend the hashmap with another hashmap.
 *
 * @param _hashmap The hashmap.
 * @param _other The other hashmap.
 */
void hashmap_extend(hashmap_t* _hashmap, hashmap_t* _other);

/*
 * Get the size of the hashmap.
 *
 * @param _hashmap The hashmap.
 * @return The size of the hashmap.
 */
size_t hashmap_size(hashmap_t* _hashmap);

#endif