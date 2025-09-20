#ifndef TREES
#define TREES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "mpc.h"

#include <editline/readline.h>
#ifdef __linux__ 
#include <editline/history.h>
#endif

typedef enum {VALUE_INT, VALUE_FLOAT, VALUE_STRING, VALUE_REF, VALUE_SYM, VALUE_FUNCTION, VALUE_EXPS, VALUE_EXPQ, VALUE_ERROR, VALUE_NIL} VAL_TYPE;
typedef enum {HIDE, SHOW} VAL_REVEAL;
typedef enum {ST, NV} SYM_KIND;

typedef struct Value Value;
typedef struct Env Env;

typedef Value* (*transform) (Env*,Value*); 

struct Value{
    VAL_TYPE type;
    VAL_REVEAL reveal;
    union {
        int i;
        double f;
        char* str;
        char* ref;
        char* sym;
        struct{
            transform pro;
            char* proname;
            Env* env;
            Value* args;
            Value* body;
        };
        struct {
            int count;
            struct Value** cell;
        };
        char* err;
    };
};

struct Env{
    Env* parent;
    int count;
    char** syms;
    Value** vals;
    SYM_KIND* kinds;
};

Value* read(mpc_ast_t*);
Value* readint(mpc_ast_t*);
Value* readfloat(mpc_ast_t*);
Value* readstr(mpc_ast_t*);

Value* eval(Env*,Value*);
Value* evalexps(Env*,Value*);

Value* reference(Env*,Value*,mpc_parser_t*);

#endif