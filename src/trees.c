#include "trees.h"
#include "utils.h"

/*
        symq : \"hea\" | \"ini\" | \"fin\" | \"tai\" | \"lis\" | \"joi\" | \"eva\" | \"con\" | \"len\" ; \
        symmath : \"add\" | \"sub\" | \"mlt\" | \"div\" | \"mod\" | \"fdv\" | \"exp\" ;\
*/

int main(int argc, char** argv){

  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Float = mpc_new("float");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* ExpS = mpc_new("exps");
  mpc_parser_t* ExpQ = mpc_new("expq");
  mpc_parser_t* Exp = mpc_new("exp");
  mpc_parser_t* Trees = mpc_new("trees");
  
  mpca_lang(MPCA_LANG_DEFAULT,
      "\
        number : /-?[0-9]+/ ;\
        float : /(-?[0-9]+)(\\.[0-9]+)/ ; \
        symbol : /[a-z]+/ ;\
        exps : '(' <exp>* ')';\
        expq : '{' <exp>* '}';\
        exp : <float> | <number> | <symbol> | <exps> | <expq> ;\
        trees : /^/ <exp>* /$/ ;a\
      ",
    Number, Float, Symbol, ExpS, ExpQ, Exp, Trees);

  Env* e = genenv();
  initenv(e);

  puts("\nTREES 0.0.0.0.1");
  puts("Press Ctrl+C to Exit\n");

  while(1){
    
      char* input = readline("trees : ");
      add_history(input);

      mpc_result_t r;

      if(mpc_parse("<stdin>", input, Trees, &r)){

        Value* ans = eval(e,read(r.output));
        printlnval(ans);

        destroyval(ans);
        mpc_ast_delete(r.output);

      } else {
        mpc_err_print(r.error); mpc_err_delete(r.error);
      }
      
      free(input);
  }

  mpc_cleanup(7, Float, Number, Symbol, ExpS, ExpQ, Exp, Trees);
  destroyenv(e);

  return 0;
}

Value* read(mpc_ast_t* t){

  if(strstr(t->tag, "number")) return readint(t); 
  if(strstr(t->tag, "float")) return readfloat(t); 
  if(strstr(t->tag, "symbol")) return valsym(t->contents);
   
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

Value* eval(Env* e, Value* ans){
  if(ans->type == VALUE_SYM){
    Value* res = envget(e,ans);
    destroyval(ans);
    return res;
  }
  if(ans->type == VALUE_EXPS) return evalexps(e,ans);
  return ans;
}

Value* evalexps(Env* e, Value* ans){

  for(int i = 0; i < ans->count; i++){
    ans->cell[i] = eval(e,ans->cell[i]);
  }

  for(int i = 0; i < ans->count; i++){
    if(ans->cell[i]->type == VALUE_ERROR) return take(ans,i);
  }

  if(ans->count == 0) return ans;
  if(ans->count == 1) return take(ans,0);

  Value* first = pop(ans,0);
  if(first->type != VALUE_DEF){
    destroyval(first);
    destroyval(ans);
    return valerr("Unbegun EXPS.");
  }
  
  Value* res = first->def(e,ans);
  destroyval(first);
  return res;
}

Value* mathop(Env* e, Value* ans, char* sym){

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

Value* qop(Env* e, Value* ans, char* sym){
  
  if(!strcmp(sym,"hea")){
    VALASSERT(ans, ans->count == 1, "One Argument Required 'hea'.");
    VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'hea'.");
    VALASSERT(ans, ans->cell[0]->count != 0, "Empty Argument 'hea'.");

    Value* res = take(ans,0);
    while(res->count > 1){destroyval(pop(res,1));}
    return res;
  }

  if(!strcmp(sym,"ini")){
    VALASSERT(ans, ans->count == 1, "One Argument Required 'ini'.");
    VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'ini'.");
    VALASSERT(ans, ans->cell[0]->count != 0, "Empty Argument 'ini'.");

    Value* res = take(ans,0);
    destroyval(pop(res,res->count-1));
    return res;
  }

    if(!strcmp(sym,"fin")){
    VALASSERT(ans, ans->count == 1, "One Argument Required 'fin'.");
    VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'fin'.");
    VALASSERT(ans, ans->cell[0]->count != 0, "Empty Argument 'fin'.");

    Value* res = take(ans,0);
    while(res->count > 1){destroyval(pop(res,0));}
    return res;
  }

  if(!strcmp(sym,"tai")){
    VALASSERT(ans, ans->count == 1, "One Argument Required 'tai'.");
    VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'tai'.");
    VALASSERT(ans, ans->cell[0]->count != 0, "Empty Argument 'tai'.");

    Value* res = take(ans,0);
    destroyval(pop(res,0));
    return res;
  }

  if(!strcmp(sym,"lis")){
    ans->type = VALUE_EXPQ;
    return ans;
  }

  if(!strcmp(sym,"eva")){
    VALASSERT(ans, ans->count == 1, "One Argument Required 'eva'.");
    VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'eva'.");
    
    Value* res = take(ans,0);
    res->type = VALUE_EXPS;
    return eval(e,res);
  }

  if(!strcmp(sym,"joi")){
    for(int i = 0; i < ans->count; i++) VALASSERT(ans,ans->cell[i]->type == VALUE_EXPQ, "Incorrect Argument Type 'joi'.");
    
    Value* x = pop(ans,0);
    while(ans->count) joinlink(x,pop(ans,0));
    return x;
  }

  if(!strcmp(sym,"cnc")){
    VALASSERT(ans,ans->count == 2, "Two Arguments Required 'con");
    VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ && ans->cell[1]->type != VALUE_ERROR, "Incorrect Argument Type 'con'.");
    
    Value* x = valexpq();
    addval(x,pop(ans,1));
    Value* y = take(ans,0);
    joinlink(x,y);
    return x;
  }

  if(!strcmp(sym,"len")){
    VALASSERT(ans, ans->count == 1, "One Argument Required 'len'.");
    VALASSERT(ans, ans->cell[0]->type == VALUE_EXPQ, "Incorrect Argument Type 'len'.");
    
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



