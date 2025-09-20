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

void addstdef(Env*,char*,transform);
Value* envget(Env*,Value*);
Value* envput(Env*,Value*,Value*,SYM_KIND);
Value* envdef(Env*,Value*,Value*,SYM_KIND);
Env* envcopy(Env*);
Env* genenv();
void destroyenv(Env*);

void printexp(Value*,char,char);
void printval(Value*);
void printlnval(Value*);
char* printtype(VAL_TYPE);
char* printproname(char*);

void initenv(Env*);
Value* call(Env*,Value*,Value*);

Value* onest(Env*,Value*);

Value* defst(Env*,Value*);
Value* decst(Env*,Value*);

Value* var(Env*,Value*,char*);

Value* addst(Env*,Value*);
Value* subst(Env*,Value*);
Value* mltst(Env*,Value*);
Value* divst(Env*,Value*);
Value* modst(Env*,Value*);
Value* fdvst(Env*,Value*);
Value* expst(Env*,Value*);

Value* dsjst(Env*, Value*);
Value* cnjst(Env*, Value*);
Value* negst(Env*, Value*);

Value* grtst(Env*,Value*);
Value* grest(Env*,Value*);
Value* lstst(Env*,Value*);
Value* lsest(Env*,Value*);
Value* eqtst(Env*,Value*);
Value* neqst(Env*,Value*);
Value* terst(Env*,Value*);

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

Value* errst(Env*,Value*);

Value* valint(int);
Value* valfloat(double);
Value* valstr(char*);
Value* valref(char*);
Value* valsym(char*);
Value* valpro(transform,char*);
Value* valexps();
Value* valexpq();
Value* valerr(char*,...);
Value* valnil();

#endif