#include "gc.h"

size_t gc_collected_count = 0;

#define OPCODE (_code->bytecode[ip+1])

INTERNAL void gc_mark_env_content(env_t* _env);

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
    
    // Free type-specific resources
    switch (_obj->type) {
        case OBJECT_TYPE_STRING:
            free(_obj->value.opaque);
            break;
        case OBJECT_TYPE_ARRAY:
            array_free((array_t*)_obj->value.opaque);
            break;
        case OBJECT_TYPE_RANGE:
            range_free((range_t*)_obj->value.opaque);
            break;
        case OBJECT_TYPE_ITERATOR:
            iterator_free((iterator_t*)_obj->value.opaque);
            break;
        case OBJECT_TYPE_OBJECT:
            hashmap_free((hashmap_t*)_obj->value.opaque);
            break;
        case OBJECT_TYPE_PROMISE:
            free(_obj->value.opaque);
            break;
        // Other types don't need special cleanup
    }
    
    // Free the object itself
    free(_obj);
}

INTERNAL void gc_mark_object(object_t* _obj) {
    if (_obj == NULL) {
        return;
    }

    // Mark the object itself
    _obj->marked = true;

    // Handle different object types
    switch (_obj->type) {
        case OBJECT_TYPE_ARRAY: {
            array_t* array = (array_t*)_obj->value.opaque;
            size_t length = array_length(array);
            for (size_t i = 0; i < length; i++) {
                gc_mark_object(array_get(array, i));
            }
            break;
        }
        case OBJECT_TYPE_ITERATOR: {
            iterator_t* iterator = (iterator_t*)_obj->value.opaque;
            gc_mark_object(iterator->obj);
            break;
        }
        case OBJECT_TYPE_OBJECT: {
            hashmap_t* hashmap = (hashmap_t*)_obj->value.opaque;
            for (size_t i = 0; i < hashmap->bucket_count; i++) {
                hashmap_node_t* node = hashmap->buckets[i];
                while (node) {
                    gc_mark_object(node->key);
                    gc_mark_object(node->value);
                    node = node->next;
                }
            }
            break;
        }
        case OBJECT_TYPE_USER_TYPE: {
            user_type_t* user = (user_type_t*)_obj->value.opaque;
            gc_mark_object(user->super);
            gc_mark_object(user->prototype);
            break;
        }
        case OBJECT_TYPE_USER_TYPE_INSTANCE: {
            user_type_instance_t* instance = (user_type_instance_t*)_obj->value.opaque;
            gc_mark_object(instance->constructor);
            gc_mark_object(instance->object);
            break;
        }
        case OBJECT_TYPE_FUNCTION: {
            code_t* code = (code_t*)_obj->value.opaque;
            if (code->environment != NULL) {
                gc_mark_env_content(code->environment);
            }
            break;
        }
        case OBJECT_TYPE_ERROR:
            gc_mark_object((object_t*)_obj->value.opaque);
            break;
        case OBJECT_TYPE_PROMISE:
            gc_mark_object(((async_promise_t*)_obj->value.opaque)->value);
            break;
        default:
            // Other types don't have references to mark
            break;
    }
}

INTERNAL void gc_mark_vm_content(vm_t* _vm) {
    gc_mark_object(_vm->tobj);
    gc_mark_object(_vm->fobj);
    gc_mark_object(_vm->null);

    for (size_t i = 0; i < _vm->sp; i++) {
        gc_mark_object(_vm->evaluation_stack[i]);
    }

    for (size_t i = 0; i < _vm->aq; i++) {
        async_t* async = _vm->queque[i];
        gc_mark_object(async->promise);
        gc_mark_env_content(async->env);
    }
}

INTERNAL void gc_mark_env_content(env_t* _env) {
    env_t* current = _env;
    // First mark all objects in the current environment chain
    while (current != NULL) {
        // Get and mark all objects in the current environment
        object_t** objects = env_get_object_list(current);
        for (size_t i = 0; objects[i] != NULL; i++) {
            gc_mark_object(objects[i]);
        }
        free(objects);
        current = current->parent;
    }
    
    // Then handle parent and closure environments if they exist
    // Note: This is recursive and could be optimized with a non-recursive approach
    // but keeping the recursive structure as instructed
    if (_env->parent != NULL) {
        gc_mark_env_content(_env->parent);
    }
    
    if (_env->closure != NULL) {
        gc_mark_env_content(_env->closure);
    }
}

INTERNAL void gc_sweep(vm_t* _vm, bool _free_all) {
    // Sweep through objects, freeing unmarked ones
    object_t** current = &_vm->root;
    
    while (*current != NULL) {
        object_t* obj = *current;
        
        if (!obj->marked) {
            // Object not marked, collect it
            ++gc_collected_count;
            *current = obj->next;  // Remove from linked list
            gc_free_object(obj);   // Free the object
        } else {
            // Reset mark for next collection cycle
            obj->marked = false;
            current = &obj->next;  // Move to next object
        }
    }
    
    // Early return if we're not freeing everything
    if (!_free_all) return;
    
    // Free the function table when doing a full cleanup
    for (size_t i = 0; i < _vm->function_table_size; i++) {
        // Free environment first, then the code object
        env_free(_vm->function_table_item[i]->environment);
        code_free(_vm->function_table_item[i]);
    }
    free(_vm->function_table_item);
}

void gc_collect_all(vm_t* _vm) {
    gc_mark_vm_content(_vm);
    gc_sweep(_vm, true);
}

void gc_collect(vm_t* _vm, env_t* _env) {
    size_t total_allocated = gc_total_objects(_vm->root);
    // mark the evaluation stack
    gc_mark_vm_content(_vm);

    // mark the env
    if (_env != NULL) gc_mark_env_content(_env);

    // collect the garbage
    gc_sweep(_vm, false);

    // printf("collected %d objects\n", gc_collected_count);
    gc_collected_count = 0;

    // reset the allocation counter
    _vm->allocation_counter = 0;
}