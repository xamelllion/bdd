#include "hashmap.h"

#ifndef HASHMAP_DEBUG
    #include <linux/slab.h>
#else
    #include <stdio.h>
    #include <stdlib.h>

    #define kfree free
    #define GFP_KERNEL 1
    #define kzalloc calloc
    #define pr_info printf
#endif

hashmap* hashmap_init() {
    hashmap* map = (hashmap*) kzalloc(sizeof(hashmap), GFP_KERNEL);
    map->size = DEFAULT_MAP_SIZE;
    map->array = (hashmap_value*) kzalloc(DEFAULT_MAP_SIZE*sizeof(hashmap_value), GFP_KERNEL);
    return map;
}

void hashmap_free(hashmap* map) {
    kfree(map->array);
    kfree(map);
}

uint64_bdd _hash_function(hashmap* map, uint64_bdd x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x % map->size;
}

void _increase_map_size(hashmap* map) {
    int prev_size = map->size;

    hashmap* new_map = (hashmap*) kzalloc(sizeof(hashmap), GFP_KERNEL);
    new_map->size = map->size + map->size * INCREASE_FACTOR;
    new_map->array = (hashmap_value*) kzalloc(new_map->size*sizeof(hashmap_value), GFP_KERNEL);
    new_map->used_size = 0;

    for (int i = 0; i < prev_size; i++) {
        if (map->array[i].has_value) {
            _hashmap_insert_value(new_map, map->array[i].key, map->array[i].value);
        }
    }

    kfree(map->array);
    map->size = new_map->size;
    map->used_size = new_map->used_size;
    map->array = new_map->array;
}

void _hashmap_insert_value(hashmap* map, uint64_bdd key, uint64_bdd value) {
    uint64_bdd index = _hash_function(map, key);
    hashmap_value val = {
        .has_value = 1,
        .key = key,
        .value = value
    };
    if (!map->array[index].has_value) {
        map->array[index] = val;
    } else {
        // solve collision
        int j = index + 1;
        for (int i = 0; i < map->size; i++) {
            j = j % map->size;
            if (!map->array[j].has_value) {
                map->array[j] = val;
                break;
            }
            j++;
        }
    }
    map->used_size++;
}

void hashmap_put(hashmap* map, uint64_bdd key, uint64_bdd value) {
    _hashmap_insert_value(map, key, value);
    if (map->size * FILL_FACTOR <= map->used_size) {
        _increase_map_size(map);
    }
}

hashmap_value hashmap_get(hashmap* map, uint64_bdd key) {
    uint64_bdd index = _hash_function(map, key);
    if (map->array[index].key == key) {
        return map->array[index];
    } else {
        // in case collision
        int j = index + 1;
        for (int i = 0; i < map->size; i++) {
            j = j % map->size;
            if (map->array[j].key == key)
                return map->array[j];
            j++;
        }
    }
    hashmap_value val = {
        .has_value = 0
    };
    return val;
}

void hashmap_remove(hashmap* map, int key) {
    uint64_bdd index = _hash_function(map, key);
    if (map->array[index].key == key) {
        map->array[index].has_value = 0;
        map->used_size--;
    } else {
        // in case collision
        int j = index + 1;
        for (int i = 0; i < map->size; i++) {
            j = j % map->size;
            if (map->array[j].key == key) {
                map->array[j].has_value = 0;
                map->used_size--;
            }
            j++;
        }
    }
}

void hashmap_print(hashmap* map) {
    pr_info("Table state:\n");
    for (int i = 0; i < map->size; i++) {
        if (map->array[i].has_value)
            pr_info("%2lu : %2lu\n", map->array[i].key, map->array[i].value);
    }
}
