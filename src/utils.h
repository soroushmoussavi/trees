#ifndef UTILS
#define UTILS

#include "trees.h"

Value* pop(Value*,int);
Value* take(Value*,int);
Value* copy(Value*);
void addval(Value*,Value*); 
void destroyval(Value*);
Env* genenv();
void destroyenv(Env*);

Value* valint(int);
Value* valfloat(double);
Value* valsym(char*,SYM_TYPE);
Value* valdef(standard);
Value* valexps();
Value* valexpq();
Value* valerr(char*);

void printexp(Value*,char,char);
void printval(Value*);
void printlnval(Value*);

#endif