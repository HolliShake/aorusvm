#include "api/ast/position.h"
#include "api/core/global.h"
#include "error.h"
#include "position.h"

DLLEXPORT position_t* position_new(int _line_start, int _colm_start, int _line_ended, int _colm_ended) {
    position_t* position = (position_t*) malloc(sizeof(position_t));
    ASSERTNULL(position, "failed to allocate memory for position");
    position->line_start = _line_start;
    position->colm_start = _colm_start;
    position->line_ended = _line_ended;
    position->colm_ended = _colm_ended;
    return position;
}

DLLEXPORT position_t* position_from_line_and_colm(int _line, int _colm) {
    position_t* position = (position_t*) malloc(sizeof(position_t));
    ASSERTNULL(position, "failed to allocate memory for position");
    position->line_start = _line;
    position->colm_start = _colm;
    position->line_ended = _line;
    position->colm_ended = _colm;
    return position;
}

DLLEXPORT position_t* position_merge(position_t* _position0, position_t* _position1) {
    position_t* position = (position_t*) malloc(sizeof(position_t));
    ASSERTNULL(position, "failed to allocate memory for position");
    position->line_start = _position0->line_start;
    position->colm_start = _position0->colm_start;
    position->line_ended = _position1->line_ended;
    position->colm_ended = _position1->colm_ended;
    return position;
}
