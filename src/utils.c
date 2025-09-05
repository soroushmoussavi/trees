#include "utils.h"

Value* pop(Value* ans, int u){

  Value* removed = ans->cell[u];

  memmove(&ans->cell[u],&ans->cell[u+1],sizeof(Value*)*(ans->count-u-1));

  ans->count--;
  ans->cell = realloc(ans->cell,sizeof(Value*)*(ans->count));

  return removed;
}

Value* take(Value* ans, int u){
  Value* removed = pop(ans,u);
  destroyval(ans);
  return removed;
}

Value* copy(Value* ans){
  Value* res = (Value*) malloc(sizeof(Value));
  res->type = ans->type;

  switch(res->type){
    case VALUE_INT: res->i = ans->i; break;
    case VALUE_FLOAT: res->f = ans->f; break;
    case VALUE_SYM: 
      res->sym = (char*) malloc(strlen(ans->sym)+1);
      strcpy(res->sym,ans->sym);
      break;
    case VALUE_DEF: res->def = ans->def; break;
    case VALUE_EXPS:
    case VALUE_EXPQ:
      res->count = ans->count;
      res->cell = (Value**) malloc(sizeof(Value*)*res->count);
      for(int i = 0; i < res->count; i++){
        res->cell[i] = copy(ans->cell[i]);
      }
      break;
    case VALUE_ERROR: 
      res->err = (char*) malloc(strlen(ans->err)+1);
      strcpy(res->err,ans->err);
      break;
  }

}

void addval(Value* ans, Value* x){
  ans->count++;
  ans->cell = (Value**) realloc(ans->cell,ans->count * sizeof(Value*));
  ans->cell[ans->count-1] = x;
}

void destroyval(Value* ans){
  switch(ans->type){
    case VALUE_INT: break;
    case VALUE_FLOAT: break;
    case VALUE_SYM: free(ans->sym); break;
    case VALUE_DEF: break;
    case VALUE_EXPS:
    case VALUE_EXPQ: 
      for(int i = 0; i<ans->count; i++){
        destroyval(ans->cell[i]);
      }
      free(ans->cell);
      break;
    case VALUE_ERROR: free(ans->err); break;
  }
  free(ans);
}

Env* genenv(){
  Env* e = (Env*) malloc(sizeof(Env));
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

void destroyenv(Env* e){
  for(int i = 0; i < e->count; i++){
    free(e->syms[i]);
    destroyenv(e->vals[i]);
  }
  free(e->syms); free(e->vals); free(e);
}


Value* valint(int i){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_INT;
  ans->i = i;
  return ans;
}

Value* valfloat(double f){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_FLOAT;
  ans->f = f;
  return ans;
}

Value* valsym(char* s, SYM_TYPE st){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_SYM;
  ans->sym = malloc(strlen(s) + 1);
  ans->symtype = st;
  strcpy(ans->sym,s);
  return ans;
}

Value* valdef(standard def){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_DEF;
  ans->def = def;
  return ans;
}

Value* valexps(){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_EXPS;
  ans->count = 0;
  ans->cell = NULL;
  return ans;
}

Value* valexpq(){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_EXPQ;
  ans->count = 0;
  ans->cell = NULL;
  return ans;
}

Value* valerr(char* e){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_ERROR;
  ans->err = malloc(strlen(e) + 1);
  strcpy(ans->err,e);
  return ans;
}

void printexp(Value* ans, char open, char close){
  putchar(open);
  for(int i = 0; i < ans->count; i++){
    printval(ans->cell[i]);
    if(i != ans->count-1) putchar(' ');
  }
  putchar(close);
}

void printval(Value* ans){
  switch(ans->type){
    case VALUE_INT: printf("%i",ans->i); break;
    case VALUE_FLOAT: printf("%f",ans->f); break;
    case VALUE_SYM: printf("%s",ans->sym); break;
    case VALUE_DEF: printf("<def>"); break;
    case VALUE_EXPS: printexp(ans,'(',')'); break;
    case VALUE_EXPQ: printexp(ans,'{','}'); break;
    case VALUE_ERROR: printf("Error: %s",ans->err); break;
  }
}

void printlnval(Value* ans) { printval(ans); putchar('\n'); }

