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

#define VALASSERT(args, cond, err) \
  if (!(cond)) { destroyval(args); return valerr(err); }


typedef enum {VALUE_INT, VALUE_FLOAT, VALUE_SYM, VALUE_DEF, VALUE_EXPS, VALUE_EXPQ, VALUE_ERROR} VAL_TYPE;

typedef struct Value Value;
typedef struct Env Env;

typedef Value* (*definition) (Env*,Value*); 

struct Value{
    VAL_TYPE type;
    union {
        int i;
        double f;
        char* sym;
        definition def;
        struct {
            int count;
            struct Value** cell;
        };
        char* err;
    };
};

struct Env{
    int count;
    char** syms;
    Value** vals;
};

Value* read(mpc_ast_t*);
Value* readint(mpc_ast_t*);
Value* readfloat(mpc_ast_t*);

Value* eval(Env*,Value*);
Value* evalexps(Env*,Value*);
Value* qop(Env*, Value*,char*);

void joinlink(Value*, Value*);

#endif