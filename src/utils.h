#ifndef UTILS
#define UTILS

#include "trees.h"

Value* pop(Value*,int);
Value* take(Value*,int);
Value* copy(Value*);
void addval(Value*,Value*); 
void destroyval(Value*);

void initenv(Env*);
void adddefenv(Env*,char*,definition);
Value* envget(Env*,Value*);
void envput(Env*,Value*,Value*);
Env* genenv();
void destroyenv(Env*);

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
Value* cncst(Env*,Value*);
Value* lenst(Env*,Value*);

Value* valint(int);
Value* valfloat(double);
Value* valsym(char*);
Value* valdef(definition);
Value* valexps();
Value* valexpq();
Value* valerr(char*);

void printexp(Value*,char,char);
void printval(Value*);
void printlnval(Value*);

#endif