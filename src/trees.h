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

#define ASSERTQ(args, cond, err) \
  if (!(cond)) { destroyval(args); return valerr(err); }

typedef enum {VALUE_INT, VALUE_FLOAT, VALUE_SYM, VALUE_DEF, VALUE_EXPS, VALUE_EXPQ, VALUE_ERROR} VAL_TYPE;
typedef enum {SYM_Q,SYM_MATH} SYM_TYPE;

struct Value;
struct Env;
typedef struct Value Value;
typedef struct Env Env;

typedef Value* (*standard) (Env*,Value*); 

typedef struct Value{
    VAL_TYPE type;
    union {
        int i;
        double f;
        struct {
            char* sym;
            SYM_TYPE symtype;
        }; 
        standard def;
        struct {
            int count;
            struct Value** cell;
        };
        char* err;
    };
} Value;

typedef struct Env{
    int count;
    char** syms;
    Value** vals;
} Env;

Value* read(mpc_ast_t*);
Value* readint(mpc_ast_t*);
Value* readfloat(mpc_ast_t*);

Value* eval(Value*);
Value* evalexps(Value*);
Value* mathop(Value*,char*);
Value* qop(Value*,char*);

void mathlink(Value*, char*, Value*);
void joinlink(Value*, Value*);

#endif