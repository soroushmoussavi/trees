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
  return res;
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

void adddefenv(Env* e, char* sym, definition def){
    Value* symval = valsym(sym);
    Value* defval = valdef(def);
    envput(e,symval,defval);
    destroyval(symval); destroyval(defval);
}

Value* envget(Env* e, Value* symval){
    for(int i = 0; i < e->count; i++){
        if(!strcmp(e->syms[i],symval->sym)) return copy(e->vals[i]);
    }
    return valerr("Undefined.");
}

void envput(Env* e, Value* symval, Value* x){

    for(int i = 0; i < e->count; i++){
        if(!strcmp(e->syms[i],symval->sym)){
            destroyval(e->vals[i]);
            e->vals[i] = copy(x);
            return;
        }
    } 

    e->count++;
    e->syms = (char**) realloc(e->syms,sizeof(char*) * e->count);
    e->vals = (Value**) realloc(e->vals,sizeof(Value*) * e->count);

    e->syms[e->count-1] = malloc(strlen(symval->sym)+1);
    strcpy(e->syms[e->count-1],symval->sym);
    e->vals[e->count-1] = copy(x);
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
    destroyval(e->vals[i]);
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

Value* valsym(char* s){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_SYM;
  ans->sym = malloc(strlen(s) + 1);
  strcpy(ans->sym,s);
  return ans;
}

Value* valdef(definition def){
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

void inttofloat(Value* ans){
  ans->type = VALUE_FLOAT;
  ans->f = ans->i;
}

void floattoint(Value* ans){
  ans->type = VALUE_INT;
  ans->i = ans->f;
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

void initenv(Env* e){
    /* INITIALIZE DEF STANDARD */
    adddefenv(e,"def",defst);

    /* INITIALIZE MATH STANDARD */
    adddefenv(e,"add",addst);
    adddefenv(e,"sub",subst);
    adddefenv(e,"mlt",mltst);
    adddefenv(e,"div",divst);
    adddefenv(e,"mod",modst);
    adddefenv(e,"fdv",fdvst);
    adddefenv(e,"exp",expst);
    /* INITIALIZE Q STANDARD */
    adddefenv(e,"hea",heast);
    adddefenv(e,"ini",inist);
    adddefenv(e,"fin",finst);
    adddefenv(e,"tai",taist);
    adddefenv(e,"lis",lisst);
    adddefenv(e,"eva",evast);
    adddefenv(e,"joi",joist);
    adddefenv(e,"cnc",cncst);
    adddefenv(e,"len",lenst);
}

Value* defst(Env* e, Value* ans){
    VALASSERT(ans,ans->cell[0]->type == VALUE_EXPQ,"Incorrect argument type 'def'.");
    Value* symlist = pop(ans,0);
    for(int i = 0; i < symlist->count; i++){
        VALASSERT(ans,symlist->cell[i]->type == VALUE_SYM,"Incorrect argument type 'def'.");
    }
    VALASSERT(ans,symlist->count == ans->count, "Misaligned Definitions.");
    for(int i = 0; i < symlist->count; i++){
        envput(e,symlist->cell[i],ans->cell[i]);
    }
    destroyval(symlist);
    destroyval(ans);
    return valexpq();
}

Value* addst(Env* e, Value* ans){

  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion.");

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f += y->f;
      else x->f += y->i;
    } else x->i += y->i; 
    destroyval(y);
  }

  return x;
}

Value* subst(Env* e, Value* ans){
  
  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion.");

  Value* x = pop(ans,0);

  if(ans->count == 0){
    if(x->type == VALUE_FLOAT) x->f *= -1;
    else x->i *= -1;
  }

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f -= y->f;
      else x->f -= y->i;
    } else x->i -= y->i; 
    destroyval(y);
  }

  return x;
}


Value* mltst(Env* e, Value* ans){
  
  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion.");

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f *= y->f;
      else x->f *= y->i;
    } else x->i *= y->i; 
    destroyval(y);
  }

  return x; 
}

Value* divst(Env* e, Value* ans){
  
  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion.");

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if((y->type == VALUE_FLOAT && y->f == 0) || (y->type == VALUE_INT && y->i == 0)){
      destroyval(ans); destroyval(x); destroyval(y);
      return valerr("Division by Zero.");
    }
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f /= y->f;
      else x->f /= y->i;
    } else {
      if(x->i % y->i == 0) x->i /= y->i; 
      else{
        inttofloat(x);
        x->f = (float) x->i / y->i;
      }
    }
    destroyval(y);
  }

  return x; 
}

Value* modst(Env* e, Value* ans){

  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion.");

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f = x->f - y->f * floor(x->f / y->f);
      else x->f = x->f - y->i * floor(x->f / y->i);
    } else x->i %= y->i; 
    destroyval(y);
  }

  return x; 
}

Value* fdvst(Env* e, Value* ans){
    
  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion.");

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if((y->type == VALUE_FLOAT && y->f == 0) || (y->type == VALUE_INT && y->i == 0)){
      destroyval(ans); destroyval(x); destroyval(y);
      return valerr("Division by Zero.");
    }
    if(x->type == VALUE_FLOAT) {
      x->type = VALUE_INT;
      if(y->type == VALUE_FLOAT) x->i = (int) floor(x->f/y->f);
      else x->i = (int) floor(x->f/y->i);
    } else {
      if(y->type == VALUE_FLOAT) x->i = (int) floor(x->i/y->f);
      else x->i = (int) floor(x->i/y->i);
    }
    destroyval(y);
  }

  return x; 
}

Value* expst(Env* e, Value* ans){

  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion.");

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f = pow(x->f,y->f);
      else x->f = pow(x->f,y->i);
    } else x->i = (int) pow(x->i,y->i);
    destroyval(y);
  }

  return x;
}

Value* heast(Env* e, Value* ans){ return qop(e,ans,"hea"); }
Value* inist(Env* e, Value* ans){ return qop(e,ans,"ini"); }
Value* finst(Env* e, Value* ans){ return qop(e,ans,"fin"); }
Value* taist(Env* e, Value* ans){ return qop(e,ans,"tai"); }
Value* lisst(Env* e, Value* ans){ return qop(e,ans,"lis"); }
Value* evast(Env* e, Value* ans){ return qop(e,ans,"eva"); }
Value* joist(Env* e, Value* ans){ return qop(e,ans,"joi"); }
Value* cncst(Env* e, Value* ans){ return qop(e,ans,"cnc"); }
Value* lenst(Env* e, Value* ans){ return qop(e,ans,"len"); }


