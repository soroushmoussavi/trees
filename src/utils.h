#ifndef UTILS
#define UTILS

#include "trees.h"

#define VALASSERT(args, cond, err, ...) \
  if (!(cond)) { \
    Value* errorval = valerr(err,##__VA_ARGS__);\
    destroyval(args); \
    return errorval; \
    }

Value* pop(Value*,int);
Value* take(Value*,int);
Value* copy(Value*);
void addval(Value*,Value*); 
void destroyval(Value*);

void adddefenv(Env*,char*,definition);
Value* envget(Env*,Value*);
void envput(Env*,Value*,Value*);
Env* genenv();
void destroyenv(Env*);

void printexp(Value*,char,char);
void printval(Value*);
void printlnval(Value*);
char* printtype(VAL_TYPE);

void initenv(Env*);

Value* defst(Env*,Value*);

Value* addst(Env*,Value*);
Value* subst(Env*,Value*);
Value* mltst(Env*,Value*);
Value* divst(Env*,Value*);
Value* modst(Env*,Value*);
Value* fdvst(Env*,Value*);
Value* expst(Env*,Value*);

Value* heast(Env*,Value*);
Value* inist(Env*,Value*);
Value* finst(Env*,Value*);
Value* taist(Env*,Value*);
Value* lisst(Env*,Value*);
Value* evast(Env*,Value*);
Value* joist(Env*,Value*);
Value* prpst(Env*,Value*);
Value* appst(Env*,Value*);
Value* lenst(Env*,Value*);

Value* valint(int);
Value* valfloat(double);
Value* valsym(char*);
Value* valdef(definition);
Value* valexps();
Value* valexpq();
Value* valerr(char*, ...);

#endif