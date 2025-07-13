#include "api/core/global.h"
#include "api/core/object.h"
#include "hashmap.h"
#include "type.h"

#ifndef ITERATOR_H
#define ITERATOR_H

typedef struct iterator_struct {
    object_t* obj;
    size_t    start;
    size_t    end;
    size_t    step;
    void*     next;
} iterator_t;

/*
 * @brief Create a new iterator object
 * 
 * @param _obj 
 * @return iterator_t* 
 */
iterator_t* iterator_new(object_t* _obj);

/*
 * @brief Check if an object is an iterator
 * 
 * @param _obj 
 * @return bool 
 */
bool iterator_has_next(object_t* _obj);

/*
 * @brief Get the next value from an iterator
 * 
 * @param _obj 
 * @return object_t* 
 */
object_t** iterator_next(object_t* _obj);

/*
 * @brief Free an iterator object
 * 
 * @param _iterator 
 */
void iterator_free(iterator_t* _iterator);


#endif