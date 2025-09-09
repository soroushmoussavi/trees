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

typedef enum {VALUE_INT, VALUE_FLOAT, VALUE_SYM, VALUE_FUNCTION, VALUE_EXPS, VALUE_EXPQ, VALUE_ERROR} VAL_TYPE;
typedef enum {ST, NV} SYM_KIND;

typedef struct Value Value;
typedef struct Env Env;

typedef Value* (*transform) (Env*,Value*); 

struct Value{
    VAL_TYPE type;
    union {
        int i;
        double f;
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

Value* eval(Env*,Value*);
Value* evalexps(Env*,Value*);

#endif