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
    case VALUE_STRING:
      res->str = (char*) malloc(strlen(ans->str)+1);
      strcpy(res->str,ans->str);
      break;
    case VALUE_SYM: 
      res->sym = (char*) malloc(strlen(ans->sym)+1);
      strcpy(res->sym,ans->sym);
      break;
    case VALUE_FUNCTION:
      if (!ans->pro){
        res->pro = NULL;
        res->proname = NULL;
        res->env = envcopy(ans->env);
        res->args = copy(ans->args);
        res->body = copy(ans->body);
      } else {
        res->pro = ans->pro; 
        res->proname = (char*) malloc(strlen(ans->proname)+1);
        strcpy(res->proname,ans->proname);
      }
      break;
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
    case VALUE_STRING: free(ans->str); break;
    case VALUE_SYM: free(ans->sym); break;
    case VALUE_FUNCTION: 
      if(!ans->pro){
        destroyenv(ans->env);
        destroyval(ans->args);
        destroyval(ans->body);
      } else free(ans->proname); 
      break;
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

void addstdef(Env* e, char* sym, transform pro){
    Value* symval = valsym(sym);
    Value* proval = valpro(pro,printproname(sym));
    envdef(e,symval,proval,ST);
    destroyval(symval); destroyval(proval);
}

Value* envget(Env* e, Value* symval){
    for(int i = 0; i < e->count; i++){
        if(!strcmp(e->syms[i],symval->sym)) return copy(e->vals[i]);
    }
    if(e->parent){
      return envget(e->parent,symval);
    } else return valerr("Undefined '%s'.",symval->sym);
}

Value* envput(Env* e, Value* symval, Value* x, SYM_KIND k){
    for(int i = 0; i < e->count; i++){
        if(!strcmp(e->syms[i],symval->sym)){
            if(e->kinds[i] == ST) return valerr("Overriding the Standard.");
            destroyval(e->vals[i]);
            e->vals[i] = copy(x);
            return valexpq();
        }
    } 

    e->count++;
    e->syms = (char**) realloc(e->syms,sizeof(char*) * e->count);
    e->vals = (Value**) realloc(e->vals,sizeof(Value*) * e->count);
    e->kinds = (SYM_KIND*) realloc(e->kinds,sizeof(SYM_KIND) * e->count);

    e->syms[e->count-1] = malloc(strlen(symval->sym)+1);
    strcpy(e->syms[e->count-1],symval->sym);
    e->vals[e->count-1] = copy(x);
    e->kinds[e->count-1] = k;
    return valexpq();
}

Value* envdef(Env* e, Value* symval, Value* x, SYM_KIND k){
  while (e->parent) e = e->parent;
  return envput(e,symval,x,k);
}

Env* envcopy(Env* e){
  Env* new = genenv();
  new->parent = e->parent;
  new->count = e->count;
  new->syms = (char**) malloc(sizeof(char*) * new->count);
  new->vals = (Value**) malloc(sizeof(Value*) * new->count);
  new->kinds = (SYM_KIND*) malloc(sizeof(SYM_KIND) * new->count);

  for(int i = 0; i < new->count; i++){
    new->syms[i] = malloc(strlen(e->syms[i])+1);
    strcpy(new->syms[i],e->syms[i]);
    new->vals[i] = copy(e->vals[i]);
    new->kinds[i] = e->kinds[i];
  }

  return new;
}

Env* genenv(){
  Env* e = (Env*) malloc(sizeof(Env));
  e->count = 0;
  e->parent = NULL;
  e->syms = NULL;
  e->vals = NULL;
  e->kinds = NULL;
  return e;
}

void destroyenv(Env* e){
  for(int i = 0; i < e->count; i++){
    free(e->syms[i]);
    destroyval(e->vals[i]);
  }
  free(e->syms); free(e->vals); free(e->kinds); free(e);
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

Value* valstr(char* s){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_STRING;
  ans->str = malloc(strlen(s)+1);
  strcpy(ans->str,s);
  return ans;
}

Value* valsym(char* s){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_SYM;
  ans->sym = malloc(strlen(s) + 1);
  strcpy(ans->sym,s);
  return ans;
}

Value* valpro(transform pro, char* proname){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_FUNCTION;
  ans->pro = pro;
  ans->proname = (char*) malloc(strlen(proname) + 1);
  strcpy(ans->proname,proname);
  return ans;
}

Value* valone(Value* args, Value* body){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_FUNCTION;
  ans->pro = NULL;
  ans->env = genenv();
  ans->args = args;
  ans->body = body;
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

Value* valerr(char* e, ...){
  Value* ans = (Value*) malloc(sizeof(Value));
  ans->type = VALUE_ERROR;

  va_list va;
  va_start(va,e);
  ans->err = malloc(512);
  vsnprintf(ans->err,511,e,va);

  ans->err = realloc(ans->err,strlen(ans->err)+1);
  va_end(va);

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
    case VALUE_STRING: 
      {
        char* escaped = (char*) malloc(strlen(ans->str)+1);
        strcpy(escaped,ans->str);
        escaped = mpcf_escape(escaped);
        printf("\"%s\"", escaped);
        free(escaped);
        break; 
      }
    case VALUE_SYM: printf("%s",ans->sym); break;
    case VALUE_FUNCTION: 
      if(!ans->pro) {
        printf("(one "); 
        printval(ans->args);
        putchar(' ');
        printval(ans->body);
        putchar(')');
      } else printf("%s",ans->proname); 
      break;
    case VALUE_EXPS: printexp(ans,'(',')'); break;
    case VALUE_EXPQ: printexp(ans,'{','}'); break;
    case VALUE_ERROR: printf("Error: %s",ans->err); break;
  }
}

void printlnval(Value* ans) { printval(ans); putchar('\n'); }

char* printtype(VAL_TYPE t){
  switch(t){
    case VALUE_INT: return "INT"; 
    case VALUE_FLOAT: return "FLOAT";
    case VALUE_STRING: return "STRING";
    case VALUE_SYM: return "SYM";
    case VALUE_FUNCTION: return "FUNCTION";
    case VALUE_EXPS: return "EXPS";
    case VALUE_EXPQ: return "EXPQ";
    case VALUE_ERROR: return "ERROR";
  }
}

char* printproname(char* sym){
  /* MAY DEPART FROM THE sym+'st' CONVENTION */
  if(!strcmp(sym,"one")) return "onest";
  if(!strcmp(sym,"two")) return "twost";
  if(!strcmp(sym,"def")) return "defst";
  if(!strcmp(sym,"add")) return "addst";
  if(!strcmp(sym,"sub")) return "subst";
  if(!strcmp(sym,"mlt")) return "mltst";
  if(!strcmp(sym,"div")) return "divst";
  if(!strcmp(sym,"mod")) return "modst";
  if(!strcmp(sym,"fdv")) return "fdvst";
  if(!strcmp(sym,"exp")) return "expst";
  if(!strcmp(sym,"dsj")) return "dsjst";
  if(!strcmp(sym,"cnj")) return "cnjst";
  if(!strcmp(sym,"neg")) return "negst";
  if(!strcmp(sym,"grt")) return "grtst";
  if(!strcmp(sym,"gre")) return "grest";
  if(!strcmp(sym,"lst")) return "lstst";
  if(!strcmp(sym,"lse")) return "lsest";
  if(!strcmp(sym,"eqt")) return "eqtst";
  if(!strcmp(sym,"neq")) return "neqst";
  if(!strcmp(sym,"ter")) return "terst";
  if(!strcmp(sym,"hea")) return "heast";
  if(!strcmp(sym,"ini")) return "inist";
  if(!strcmp(sym,"fin")) return "finst";
  if(!strcmp(sym,"tai")) return "taist";
  if(!strcmp(sym,"lis")) return "lisst";
  if(!strcmp(sym,"eva")) return "evast";
  if(!strcmp(sym,"joi")) return "joist";
  if(!strcmp(sym,"prp")) return "prpst";
  if(!strcmp(sym,"app")) return "appst";
  if(!strcmp(sym,"len")) return "lenst";
  return "-----";
}

void initenv(Env* e){
    /* INITIALIZE DEF STANDARD */
    addstdef(e,"one",onest);
    addstdef(e,"two",twost);
    addstdef(e,"def",defst);
    /* INITIALIZE MATH STANDARD */
    addstdef(e,"add",addst);
    addstdef(e,"sub",subst);
    addstdef(e,"mlt",mltst);
    addstdef(e,"div",divst);
    addstdef(e,"mod",modst);
    addstdef(e,"fdv",fdvst);
    addstdef(e,"exp",expst);
    /* INITIALIZE LOGICAL STANDARD */
    addstdef(e,"dsj",dsjst);
    addstdef(e,"cnj",cnjst);
    addstdef(e,"neg",negst);
    /* INITIALIZE TER STANDARD */
    addstdef(e,"grt",grtst);
    addstdef(e,"gre",grest);
    addstdef(e,"lst",lstst);
    addstdef(e,"lse",lsest);
    addstdef(e,"eqt",eqtst);
    addstdef(e,"neq",neqst);
    addstdef(e,"ter",terst);    
    /* INITIALIZE Q STANDARD */
    addstdef(e,"hea",heast);
    addstdef(e,"ini",inist);
    addstdef(e,"fin",finst);
    addstdef(e,"tai",taist);
    addstdef(e,"lis",lisst);
    addstdef(e,"eva",evast);
    addstdef(e,"joi",joist);
    addstdef(e,"prp",prpst);
    addstdef(e,"app",appst);    
    addstdef(e,"len",lenst);
}

Value* call(Env* e, Value* function, Value* arguments){
  if(function->pro) return function->pro(e,arguments);

  int given = arguments->count;
  int expected = function->args->count;

  while(arguments->count){
    if(function->args->count == 0){
      destroyval(arguments);
      if(expected == 1) return valerr("Expected Max 1 Argument. Given %i.",expected,given);
      else return valerr("Expected Max %i Arguments. Given %i.",expected,given);
    }
    Value* symval = pop(function->args,0);

    if(!strcmp(symval->sym,":")){
      if(function->args->count != 1){
        destroyval(arguments);
        return valerr("One Variable Length Argument Required. Given %i.",function->args->count);
      }
      Value* varsym = pop(function->args,0);
      envput(function->env,varsym,lisst(e,arguments),NV);
      destroyval(symval); destroyval(varsym);
      break;
    }
    Value* x = pop(arguments,0);
    envput(function->env,symval,x,NV);
    destroyval(symval); destroyval(x);
  } destroyval(arguments);

  if(function->args->count > 0 && !strcmp(function->args->cell[0]->sym,":")){
      if(function->args->count != 2){
        destroyval(arguments);
        return valerr("One Variable Length Argument Required. Given %i.",function->args->count);
      }
      destroyval(pop(function->args,0));
      Value* symval = pop(function->args,0);
      Value* empty = valexpq();
      envput(function->env,symval,empty,NV); 
      destroyval(empty); destroyval(symval);
  }


  if(function->args->count == 0){
    function->env->parent = e;

    Value* holder = valexps();
    addval(holder,copy(function->body));
    return evast(function->env,holder);
  } else {
    return copy(function);
  }

}

Value* onest(Env* e, Value* ans){

  VALASSERT(ans, ans->count == 2, "Two Arguments Required 'one'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ && ans->cell[1]->type == VALUE_EXPQ, "EXPQ and EXPQ Types Required 'one'. Given %s and %s.",printtype(ans->cell[0]->type),printtype(ans->cell[1]->type));

  for(int i = 0; i < ans->cell[0]->count; i++){
    VALASSERT(ans,ans->cell[0]->cell[i]->type == VALUE_SYM, "SYM Types Required within First Argument 'one'. Found %s.",printtype(ans->cell[0]->cell[i]->type));
  }

  Value* args = pop(ans,0);
  Value* body = pop(ans,0);

  destroyval(ans);

  return valone(args,body);
}

Value* twost(Env* e, Value* ans){
  return var(e,ans,"two");
}

Value* defst(Env* e, Value* ans){
  return var(e,ans,"def");
}

Value* var(Env* e, Value* ans, char* sym){
    VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "EXPQ Type Required as First Argument 'def'. Given %s.",printtype(ans->cell[0]->type));

    Value* symlist = pop(ans,0);
    VALASSERT(ans,symlist->count > 0, "Empty Argument 'def'.");
    for(int i = 0; i < symlist->count; i++){
      VALASSERT(ans,symlist->cell[i]->type == VALUE_SYM, "SYM Types Required within First Argument 'def'. Found %s.",printtype(symlist->cell[i]->type));
    }
    VALASSERT(ans,symlist->count == ans->count, "Misaligned Definitions. Given %i SYM. Given %i Values.",symlist->count, ans->count);
    Value* empty = valexpq();

    for(int i = 0; i < symlist->count; i++){
      if(!strcmp(sym,"def")) {
        destroyval(empty);
        empty = envdef(e,symlist->cell[i],ans->cell[i],NV);
        if(empty->type == VALUE_ERROR) break;
      } else {
        envput(e,symlist->cell[i],ans->cell[i],NV);
      }
    }
    destroyval(symlist);
    destroyval(ans);
    return empty;
}

Value* addst(Env* e, Value* ans){

  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'add'. Found %s.",printtype(ans->cell[i]->type));

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      if(x->type == VALUE_INT) inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f += y->f;
      else x->f += y->i;
    } else x->i += y->i; 
    destroyval(y);
  }

  return x;
}

Value* subst(Env* e, Value* ans){
  
  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'sub'. Found %s.",printtype(ans->cell[i]->type));

  Value* x = pop(ans,0);

  if(ans->count == 0){
    if(x->type == VALUE_FLOAT) x->f *= -1;
    else x->i *= -1;
  }

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      if(x->type == VALUE_INT) inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f -= y->f;
      else x->f -= y->i;
    } else x->i -= y->i; 
    destroyval(y);
  }

  return x;
}


Value* mltst(Env* e, Value* ans){
  
  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'mlt'. Found %s.",printtype(ans->cell[i]->type));

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      if(x->type == VALUE_INT) inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f *= y->f;
      else x->f *= y->i;
    } else x->i *= y->i; 
    destroyval(y);
  }

  return x; 
}

Value* divst(Env* e, Value* ans){
  
  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'div'. Found %s.",printtype(ans->cell[i]->type));

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if((y->type == VALUE_FLOAT && y->f == 0) || (y->type == VALUE_INT && y->i == 0)){
      destroyval(ans); destroyval(x); destroyval(y);
      return valerr("Division by Zero.");
    }
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      if(x->type == VALUE_INT) inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f /= y->f;
      else x->f /= y->i;
    } else {
      if(x->i % y->i == 0) x->i /= y->i; 
      else{
        inttofloat(x);
        x->f = (double) x->f / y->i;
      }
    }
    destroyval(y);
  }

  return x; 
}

Value* modst(Env* e, Value* ans){

  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'mod'. Found %s.",printtype(ans->cell[i]->type));

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      if(x->type == VALUE_INT) inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f = x->f - y->f * floor(x->f / y->f);
      else x->f = x->f - y->i * floor(x->f / y->i);
    } else x->i %= y->i; 
    destroyval(y);
  }

  return x; 
}

Value* fdvst(Env* e, Value* ans){
    
  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'fdv'. Found %s.",printtype(ans->cell[i]->type));

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

  for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'exp'. Found %s.",printtype(ans->cell[i]->type));

  Value* x = pop(ans,0);

  while(ans->count > 0){
    Value* y = pop(ans,0);
    if(x->type == VALUE_FLOAT || y->type == VALUE_FLOAT) {
      if(x->type == VALUE_INT) inttofloat(x);
      if(y->type == VALUE_FLOAT) x->f = pow(x->f,y->f);
      else x->f = pow(x->f,y->i);
    } else x->i = (int) pow(x->i,y->i);
    destroyval(y);
  }

  return x;
}

Value* dsjst(Env* e, Value* ans){
  int res = 0;
  for(int i = 0; i < 2; i++){
    VALASSERT(ans,ans->cell[i]->type == VALUE_INT, "Non-INT Inclusion 'dsj'. Found %s.",printtype(ans->cell[i]->type));
    if(ans->cell[i]->i) { res = 1; break; }
  }
  destroyval(ans);
  return valint(res);
}

Value* cnjst(Env* e, Value* ans){
  int res = 1;
  for(int i = 0; i < 2; i++){
    VALASSERT(ans,ans->cell[i]->type == VALUE_INT, "Non-INT Inclusion 'dsj'. Found %s.",printtype(ans->cell[i]->type));
    if(!ans->cell[i]->i) { res = 0; break; }
  }
  destroyval(ans);
  return valint(res);
}

Value* negst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 1, "One Argument Required 'neg'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_INT, "INT Type Required 'neg'. Given %s.",printtype(ans->cell[0]->type));
  int res = !ans->cell[0]->i;
  destroyval(ans);
  return valint(res);
}

Value* grtst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 2, "Two Arguments Required 'grt'. Given %i.", ans->count);
  for(int i = 0; i < 2; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'grt'. Found %s.",printtype(ans->cell[i]->type));
  
  int res;
  if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_FLOAT) res = ans->cell[0]->f > ans->cell[1]->f;
  if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->f > (float) ans->cell[1]->i;
  if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_FLOAT) res = (float) ans->cell[0]->i > ans->cell[1]->f;
  if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->i > ans->cell[1]->i;
  destroyval(ans);
  return valint(res);
}

Value* grest(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 2, "Two Arguments Required 'gre'. Given %i.", ans->count);
  for(int i = 0; i < 2; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'grt'. Found %s.",printtype(ans->cell[i]->type));
  
  int res;
  if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_FLOAT) res = ans->cell[0]->f >= ans->cell[1]->f;
  if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->f >= (float) ans->cell[1]->i;
  if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_FLOAT) res = (float) ans->cell[0]->i >= ans->cell[1]->f;
  if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->i >= ans->cell[1]->i;
  destroyval(ans);
  return valint(res);
}


Value* lstst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 2, "Two Arguments Required 'lat'. Given %i.", ans->count);
  for(int i = 0; i < 2; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'grt'. Found %s.",printtype(ans->cell[i]->type));
  
  int res;
  if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_FLOAT) res = ans->cell[0]->f < ans->cell[1]->f;
  if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->f < (float) ans->cell[1]->i;
  if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_FLOAT) res = (float) ans->cell[0]->i < ans->cell[1]->f;
  if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->i < ans->cell[1]->i;
  destroyval(ans);
  return valint(res);
}

Value* lsest(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 2, "Two Arguments Required 'lse'. Given %i.", ans->count);
  for(int i = 0; i < 2; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_INT || ans->cell[i]->type == VALUE_FLOAT, "Non-numeric Inclusion 'grt'. Found %s.",printtype(ans->cell[i]->type));
  
  int res;
  if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_FLOAT) res = ans->cell[0]->f <= ans->cell[1]->f;
  if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->f <= (float) ans->cell[1]->i;
  if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_FLOAT) res = (float) ans->cell[0]->i <= ans->cell[1]->f;
  if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->i <= ans->cell[1]->i;
  destroyval(ans);
  return valint(res);
}

Value* eqtst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 2, "Two Arguments Required 'eqt'. Given %i.", ans->count);
  int res;
  if((ans->cell[0]->type == VALUE_FLOAT || ans->cell[0]->type == VALUE_INT) && (ans->cell[1]->type == VALUE_FLOAT || ans->cell[1]->type == VALUE_INT)){
    if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_FLOAT) res = ans->cell[0]->f == ans->cell[1]->f;
    if(ans->cell[0]->type == VALUE_FLOAT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->f == (float) ans->cell[1]->i;
    if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_FLOAT) res = (float) ans->cell[0]->i == ans->cell[1]->f;
    if(ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_INT) res = ans->cell[0]->i == ans->cell[1]->i;
    destroyval(ans); return valint(res);
  }

  if(ans->cell[0]->type != ans->cell[1]->type) {destroyval(ans); return valint(0);}
  switch(ans->cell[0]->type){
    case VALUE_SYM: res = !strcmp(ans->cell[0]->sym,ans->cell[1]->sym); break;
    case VALUE_STRING: res = !strcmp(ans->cell[0]->sym,ans->cell[1]->sym); break;
    case VALUE_FUNCTION:
      if(ans->cell[0]->pro || ans->cell[1]->pro) res = ans->cell[0]->pro == ans->cell[1]->pro;
      else{
        Value* bothargs = valexps(); addval(bothargs,copy(ans->cell[0]->args)); addval(bothargs,copy(ans->cell[1]->args));
        Value* bothbody = valexps(); addval(bothbody,copy(ans->cell[0]->body)); addval(bothbody,copy(ans->cell[1]->body));
        bothargs = eqtst(e,bothargs); bothbody = eqtst(e,bothbody);
        res = bothargs->i * bothbody->i; 
        destroyval(bothargs); destroyval(bothbody);
      }
      break;
    case VALUE_EXPS:
    case VALUE_EXPQ:
      if(ans->cell[0]->count != ans->cell[1]->count) {res = 0; break;}
      res = 1;
      int n = ans->cell[0]->count;
      for(int i = 0; i < n; i++){
        Value* both = valexps(); addval(both,pop(ans->cell[0],0)); addval(both,pop(ans->cell[1],0));
        both = eqtst(e,both);
        res *= both->i; 
        destroyval(both);
        if(!res) break;
      }
      break;
    case VALUE_ERROR: res = !strcmp(ans->cell[0]->err,ans->cell[1]->err); break;
    default: 
      res = 0;
      break;
  }
  destroyval(ans);
  return valint(res);
}

Value* neqst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 2, "Two Arguments Required 'neq'. Given %i.", ans->count);
  Value* res = eqtst(e,ans);
  res->i = !res->i;
  return res;
}

Value* terst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 3, "Three Arguments Required 'ter'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_INT && ans->cell[1]->type == VALUE_EXPQ && ans->cell[2]->type == VALUE_EXPQ, "INT, EXPQ and EXPQ Types Required 'ter'. Given %s, %s and %s.",printtype(ans->cell[0]->type),printtype(ans->cell[1]->type),printtype(ans->cell[2]->type));

  Value* res;
  ans->cell[1]->type = VALUE_EXPS;
  ans->cell[2]->type = VALUE_EXPS;

  if(ans->cell[0]->i) res = eval(e,pop(ans,1));
  else res = eval(e,pop(ans,2));

  destroyval(ans);
  return res;
}

Value* heast(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 1, "One Argument Required 'hea'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "EXPQ Type Required 'hea'. Given %s.",printtype(ans->cell[0]->type));
  VALASSERT(ans, ans->cell[0]->count != 0, "Empty Argument 'hea'.");

  Value* res = take(ans,0);
  while(res->count > 1){destroyval(pop(res,1));}
  return res;
}

Value* inist(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 1, "One Argument Required 'ini'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "EXPQ Type Required 'ini'. Given %s.",printtype(ans->cell[0]->type));
  VALASSERT(ans, ans->cell[0]->count != 0, "Empty Argument 'ini'.");

  Value* res = take(ans,0);
  destroyval(pop(res,res->count-1));
  return res;
}

Value* finst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 1, "One Argument Required 'fin'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "EXPQ Type Required 'fin'. Given %s.",printtype(ans->cell[0]->type));
  VALASSERT(ans, ans->cell[0]->count != 0, "Empty Argument 'fin'.");

  Value* res = take(ans,0);
  while(res->count > 1){destroyval(pop(res,0));}
  return res;
}

Value* taist(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 1, "One Argument Required 'tai'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "EXPQ Type Required 'tai'. Given %s.",printtype(ans->cell[0]->type));
  VALASSERT(ans, ans->cell[0]->count != 0, "Empty Argument 'tai'.");

  Value* res = take(ans,0);
  destroyval(pop(res,0));
  return res;
}

Value* lisst(Env* e, Value* ans){     
  ans->type = VALUE_EXPQ;
  return ans; 
}

Value* evast(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 1, "One Argument Required 'eva'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "EXPQ Type Required 'eva'. Given %s.",printtype(ans->cell[0]->type));
  
  Value* res = take(ans,0);
  res->type = VALUE_EXPS;
  return eval(e,res);
}

Value* joist(Env* e, Value* ans){
  for(int i = 0; i < ans->count; i++)  VALASSERT(ans, ans->cell[i]->type == VALUE_EXPQ, "EXPQ Types Required 'joi'. Found %s.",printtype(ans->cell[i]->type));
  
  Value* x = pop(ans,0);
  while(ans->count) {
    Value* y = pop(ans,0);
    while(y->count) addval(x,pop(y,0));
    destroyval(y);
  }
  return x;
}

Value* prpst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 2, "Two Arguments Required 'prp'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ && ans->cell[1]->type != VALUE_ERROR, "EXPQ and NON-ERROR Types Required 'prp'. Given %s and %s.",printtype(ans->cell[0]->type),printtype(ans->cell[1]->type));

  Value* x = valexpq();
  addval(x,pop(ans,1));
  Value* y = take(ans,0);
  while(y->count) addval(x,pop(y,0));
  destroyval(y);
  return x;
}

Value* appst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 2, "Two Arguments Required 'app'. Given %i.", ans->count);
  VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ && ans->cell[1]->type != VALUE_ERROR, "EXPQ and NON-ERROR Types Required 'app'. Given %s and %s.",printtype(ans->cell[0]->type),printtype(ans->cell[1]->type));

  Value* y = pop(ans,1);
  Value* x = take(ans,0);
  addval(x,y);
  return x;
}

Value* lenst(Env* e, Value* ans){
  VALASSERT(ans, ans->count == 1, "One Argument Required 'len'. Given %i.", ans->count, 1);
  VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "EXPQ Type Required 'len'. Given %s.",printtype(ans->cell[0]->type));
  
  int length = ans->cell[0]->count;
  destroyval(ans);
  return valint(length);
}



