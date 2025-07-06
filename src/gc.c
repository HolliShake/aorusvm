#include "gc.h"

size_t gc_collected_count = 0;

INTERNAL size_t gc_total_objects(object_t* root) {
    if (root == NULL) {
        return 0;
    }

    size_t count = 1;
    object_t* current = root->next;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

INTERNAL void gc_free_object(object_t* _obj) {
    if (_obj == NULL) {
        return;
    }
    if (OBJECT_TYPE_STRING(_obj)) {
        free(_obj->value.opaque);
    } else if (OBJECT_TYPE_ARRAY(_obj)) {
        array_free((array_t*) _obj->value.opaque);
    } else if (OBJECT_TYPE_OBJECT(_obj)) {
        hashmap_free((hashmap_t*) _obj->value.opaque);
    }
    free(_obj);
}

INTERNAL void gc_mark_object(object_t* _obj) {
    if (_obj == NULL) {
        return;
    }

    object_t* current = _obj;
    current->marked = true;
    if (OBJECT_TYPE_ARRAY(_obj)) {
        array_t* array = (array_t*) _obj->value.opaque;
        for (size_t i = 0; i < array_length(array); i++) {
            gc_mark_object(array_get(array, i));
        }
    } else if (OBJECT_TYPE_OBJECT(_obj)) {
        hashmap_t* hashmap = (hashmap_t*) _obj->value.opaque;
        for (size_t i = 0; i < hashmap->bucket_count; i++) {
            if (hashmap->buckets[i] == NULL) {
                continue;
            }
            hashmap_node_t* node = hashmap->buckets[i];
            while (node) {
                hashmap_node_t* next = node->next;
                gc_mark_object(node->key);
                gc_mark_object(node->value);
                node = next;
            }
        }
    }
}

INTERNAL void gc_mark_vm_content(vm_t* _vm) {
    gc_mark_object(_vm->tobj);
    gc_mark_object(_vm->fobj);
    gc_mark_object(_vm->null);

    for (size_t i = 0; i < _vm->sp; i++) {
        gc_mark_object(_vm->evaluation_stack[i]);
    }
}

INTERNAL void gc_mark_env_content(env_t* _env) {
    env_t* current = _env;
    while (current != NULL) {
        object_t** list = env_get_object_list(current);
        for (size_t i = 0; list[i] != NULL; i++) {
            gc_mark_object(list[i]);
        }
        free(list);
        current = current->parent;
    }
}

INTERNAL void gc_sweep(vm_t* vm, bool _collect_all) {
    object_t** current = &vm->root;

    while (*current != NULL) {
        object_t* obj = *current;

        if (!obj->marked || _collect_all) {
            *current = obj->next; // unlink
            gc_free_object(obj);
        } else {
            obj->marked = false;
            current = &obj->next;
        }
    }
}

void gc_collect_all(vm_t* _vm, env_t* _env) {
    gc_sweep(_vm, true);
}

void gc_collect(vm_t* _vm, env_t* _env) {
    size_t total_allocated = gc_total_objects(_vm->root);
    // mark the evaluation stack
    gc_mark_vm_content(_vm);

    // mark the env
    gc_mark_env_content(_env);

    // collect the garbage
    gc_sweep(_vm, false);

    // reset the allocation counter
    _vm->allocation_counter = 0;
}