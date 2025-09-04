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


typedef struct Env Env;
typedef enum {VALUE_INT, VALUE_FLOAT, VALUE_SYM, VALUE_DEF, VALUE_EXPS, VALUE_EXPQ, VALUE_ERROR} VAL_TYPE;
typedef enum {SYM_Q,SYM_MATH} SYM_TYPE;

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

void printexp(Value*,char,char);
void printval(Value*);
void printlnval(Value*);

Value* read(mpc_ast_t*);
Value* readint(mpc_ast_t*);
Value* readfloat(mpc_ast_t*);

Value* eval(Value*);
Value* evalexps(Value*);
Value* mathop(Value*,char*);
Value* qop(Value*,char*);

void mathlink(Value*, char*, Value*);
void joinlink(Value*, Value*);

Value* take(Value*,int);
Value* pop(Value*,int);
Value* copy(Value*);
void addval(Value*,Value*); 
void destroyval(Value*);

Value* valint(int);
Value* valfloat(double);
Value* valsym(char*,SYM_TYPE);
Value* valdef(standard);
Value* valexps();
Value* valexpq();
Value* valerr(char*);

#endif