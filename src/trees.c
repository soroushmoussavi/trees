#include "trees.h"

int main(int argc, char** argv){

  mpc_parser_t* Float = mpc_new("float");
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* SymQ = mpc_new("symq");
  mpc_parser_t* SymMath = mpc_new("symmath");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* ExpS = mpc_new("exps");
  mpc_parser_t* ExpQ = mpc_new("expq");
  mpc_parser_t* Exp = mpc_new("exp");
  mpc_parser_t* Trees = mpc_new("trees");
  
  mpca_lang(MPCA_LANG_DEFAULT,
      "\
        number : /-?[0-9]+/ ;\
        float : /(-?[0-9]+)(\\.[0-9]+)/ ; \
        symq : \"hea\" | \"ini\" | \"fin\" | \"tai\" | \"lis\" | \"joi\" | \"eva\" | \"con\" | \"len\" ; \
        symmath : \"add\" | \"sub\" | \"mlt\" | \"div\" | \"mod\" | \"fdv\" | \"exp\" ;\
        symbol : <symq> | <symmath> ;\
        exps : '(' <exp>* ')';\
        expq : '{' <exp>* '}';\
        exp : <float> | <number> | <symbol> | <exps> | <expq> ;\
        trees : /^/ <exp>* /$/ ;a\
      ",
    Float, Number, SymQ, SymMath, Symbol, ExpS, ExpQ, Exp, Trees);

  puts("\nTREES 0.0.0.0.1");
  puts("Press Ctrl+C to Exit\n");

  while(1){
    
      char* input = readline("trees : ");
      add_history(input);

      mpc_result_t r;

      if(mpc_parse("<stdin>", input, Trees, &r)){

        Value* ans = eval(read(r.output));
        printlnval(ans);

        destroyval(ans);
        mpc_ast_delete(r.output);

      } else {
        mpc_err_print(r.error); mpc_err_delete(r.error);
      }
      
      free(input);
  }

  mpc_cleanup(9, Float, Number, SymQ, SymMath, Symbol, ExpS, ExpQ, Exp, Trees);

  return 0;
}

Value* read(mpc_ast_t* t){

  if(strstr(t->tag, "number")) return readint(t); 
  if(strstr(t->tag, "float")) return readfloat(t); 
  if(strstr(t->tag, "symbol")){
    if(strstr(t->tag, "symq")) return valsym(t->contents, SYM_Q);
    if(strstr(t->tag, "symmath")) return valsym(t->contents, SYM_MATH);
  }
   
  Value* ans = NULL;
  if(!strcmp(t->tag,">")) ans = valexps();
  if(strstr(t->tag,"exps")) ans = valexps();
  if(strstr(t->tag,"expq")) ans = valexpq();

  for(int i = 0; i<t->children_num; i++){
    if( 
      !strcmp(t->children[i]->contents,"(") ||
      !strcmp(t->children[i]->contents,")") || 
      !strcmp(t->children[i]->tag,"regex")  ||
      !strcmp(t->children[i]->contents,"{") ||
      !strcmp(t->children[i]->contents,"}") 
    ) continue;
    addval(ans,read(t->children[i]));
  }

  return ans;
}

Value* eval(Value* ans){
  if(ans->type == VALUE_EXPS) return evalexps(ans);
  return ans;
}

Value* evalexps(Value* ans){

  for(int i = 0; i < ans->count; i++){
    ans->cell[i] = eval(ans->cell[i]);
  }

  for(int i = 0; i < ans->count; i++){
    if(ans->cell[i]->type == VALUE_ERROR) return take(ans,i);
  }

  if(ans->count == 0) return ans;
  if(ans->count == 1) return take(ans,0);

  Value* first = pop(ans,0);
  if(first->type != VALUE_SYM){
    destroyval(first);
    destroyval(ans);
    return valerr("Unbegun EXPS.");
  }

  Value* res;
  switch(first->symtype){
    case SYM_Q: res = qop(ans,first->sym); break;
    case SYM_MATH: 
      res = mathop(ans,first->sym); 
      destroyval(ans);
      break;
  }

  destroyval(first);

  return res;
}

Value* mathop(Value* ans, char* sym){

  for(int i = 0; i < ans->count; i++){
    if(ans->cell[i]->type != VALUE_INT && ans->cell[i]->type != VALUE_FLOAT){
      return valerr("Non-numeric Inclusion.");
    }
  }

  Value* x = pop(ans,0);

  if(!strcmp(sym,"sub") && ans->count == 0){
    if(x->type == VALUE_INT) x->i *= -1;
    if(x->type == VALUE_FLOAT) x->f *= -1;
  }

  while(ans->count > 0){
    Value* y = pop(ans,0);
    mathlink(x,sym,y);
    destroyval(y);
  }

  return x;
}

Value* qop(Value* ans, char* sym){
  
  if(!strcmp(sym,"hea")){
    ASSERTQ(ans, ans->count == 1, "One Argument Required 'hea'.");
    ASSERTQ(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'hea'.");
    ASSERTQ(ans, ans->cell[0]->count != 0, "Empty Argument 'hea'.");

    Value* res = take(ans,0);
    while(res->count > 1){destroyval(pop(res,1));}
    return res;
  }

  if(!strcmp(sym,"ini")){
    ASSERTQ(ans, ans->count == 1, "One Argument Required 'ini'.");
    ASSERTQ(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'ini'.");
    ASSERTQ(ans, ans->cell[0]->count != 0, "Empty Argument 'ini'.");

    Value* res = take(ans,0);
    destroyval(pop(res,res->count-1));
    return res;
  }

    if(!strcmp(sym,"fin")){
    ASSERTQ(ans, ans->count == 1, "One Argument Required 'fin'.");
    ASSERTQ(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'fin'.");
    ASSERTQ(ans, ans->cell[0]->count != 0, "Empty Argument 'fin'.");

    Value* res = take(ans,0);
    while(res->count > 1){destroyval(pop(res,0));}
    return res;
  }

  if(!strcmp(sym,"tai")){
    ASSERTQ(ans, ans->count == 1, "One Argument Required 'tai'.");
    ASSERTQ(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'tai'.");
    ASSERTQ(ans, ans->cell[0]->count != 0, "Empty Argument 'tai'.");

    Value* res = take(ans,0);
    destroyval(pop(res,0));
    return res;
  }

  if(!strcmp(sym,"lis")){
    ans->type = VALUE_EXPQ;
    return ans;
  }

  if(!strcmp(sym,"eva")){
    ASSERTQ(ans, ans->count == 1, "One Argument Required 'eva'.");
    ASSERTQ(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'eva'.");
    
    Value* res = take(ans,0);
    res->type = VALUE_EXPS;
    return eval(res);
  }

  if(!strcmp(sym,"joi")){
    for(int i = 0; i < ans->count; i++) ASSERTQ(ans,ans->cell[i]->type == VALUE_EXPQ, "Incorrect Argument Type 'joi'.");
    
    Value* x = pop(ans,0);
    while(ans->count) joinlink(x,pop(ans,0));
    return x;
  }

  if(!strcmp(sym,"con")){
    ASSERTQ(ans,ans->count == 2, "Two Arguments Required 'con");
    ASSERTQ(ans, ans->cell[0]->type == VALUE_EXPQ && ans->cell[1]->type != VALUE_ERROR, "Incorrect Argument Type 'con'.");
    
    Value* x = valexpq();
    addval(x,pop(ans,1));
    Value* y = take(ans,0);
    joinlink(x,y);
    return x;
  }

  if(!strcmp(sym,"len")){
    ASSERTQ(ans, ans->count == 1, "One Argument Required 'len'.");
    ASSERTQ(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'len'.");
    
    int length = ans->cell[0]->count;
    destroyval(ans);
    return valint(length);
  }

  return ans;
}


void mathlink(Value* ans, char* op, Value* x){

  if(ans->type == VALUE_ERROR) return;
  char hasfloat = ans->type == VALUE_FLOAT || x->type  == VALUE_FLOAT;

  if(!strcmp(op,"add")){
    if(hasfloat){
      if(ans->type == VALUE_INT){
        ans->type = VALUE_FLOAT;
        ans->f = ans->i;
      } 
      if (x->type == VALUE_INT) ans->f += x->i;
      else ans->f += x->f;
    } else ans->i += x->i; 
  } 

  if(!strcmp(op,"sub")){
    if(hasfloat){
      if(ans->type == VALUE_INT){
        ans->type = VALUE_FLOAT;
        ans->f = ans->i;
      } 
      if (x->type == VALUE_INT) ans->f -= x->i;
      else ans->f -= x->f;
    } else ans->i -= x->i; 
  } 

  if(!strcmp(op,"mlt")){
    if(hasfloat){
      if(ans->type == VALUE_INT){
        ans->type = VALUE_FLOAT;
        ans->f = ans->i;
      } 
      if (x->type == VALUE_INT) ans->f *= x->i;
      else ans->f *= x->f;
    } else ans->i *= x->i; 
  } 

  if(!strcmp(op,"div")){
    
    if((x->type == VALUE_INT && x->i == 0) || (x->type == VALUE_FLOAT && x->f == 0)){
      destroyval(ans);
      ans = valerr("Division by Zero");
    }

    else if(hasfloat){
      if(ans->type == VALUE_INT){
        ans->type = VALUE_FLOAT;
        ans->f = ans->i;
      } 
      if (x->type == VALUE_INT) ans->f /= x->i;
      else ans->f /= x->f;
    } 
    else {
      if(ans->i%x->i == 0) ans->i /= x->i;
      else {
        ans->type = VALUE_FLOAT;
        ans->f = (float) ans->i / x->i;
      }
    } 
  } 

  if(!strcmp(op,"mod")){
    if(hasfloat){
      if(ans->type == VALUE_INT){
        ans->type = VALUE_FLOAT;
        ans->f = ans->i;
      } 
      if (x->type == VALUE_INT) ans->f = ans->f - x->i * floor(ans->f / x->i);
      else ans->f = ans->f - x->f * floor(ans->f / x->f);
    } else ans->i %= x->i;
  } 

  if(!strcmp(op,"fdv")){
      if(ans->type == VALUE_FLOAT){
        ans->type = VALUE_INT;
        if(x->type == VALUE_INT) ans->i = (int) floor(ans->f/x->i);
        else ans->i = (int) floor(ans->f/x->f);
      } else {
        if(x->type == VALUE_INT) ans->i = (int) floor(ans->i/x->i);
        else ans->i = (int) floor(ans->i/x->f);
      }
  }

  if(!strcmp(op,"exp")){
    if(hasfloat){
      if(ans->type == VALUE_INT){
        ans->type = VALUE_FLOAT;
        ans->f = ans->i;
      } 
      if (x->type == VALUE_INT) ans->f = pow(ans->f,x->i);
      else ans->f = pow(ans->f,x->f);
    } else ans->i = (int) pow(ans->i,x->i); 
  } 
  
}

void joinlink(Value* ans, Value* x){
  while(x->count) addval(ans,pop(x,0));
  destroyval(x);
}

Value* readint(mpc_ast_t* t){
  errno = 0;
  long li = strtol(t->contents,NULL,10);
  return (errno == ERANGE || li > INT_MAX || li < INT_MIN) ? valerr("Integer Exceeding.") : valint((int) li);
}

Value* readfloat(mpc_ast_t* t){
  errno = 0;
  double f = strtod(t->contents,NULL);
  return (errno == ERANGE) ? valerr("Float Exceeding.") : valfloat(f);
}

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




