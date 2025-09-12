#include "trees.h"
#include "utils.h"

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
        symbol : /[a-z]+/ | ':' ;\
        exps : '(' <exp>* ')';\
        expq : '{' <exp>* '}';\
        exp : <float> | <number> | <symbol> | <exps> | <expq> ;\
        trees : /^/ <exp>* /$/ ;a\
      ",
    Number, Float, Symbol, ExpS, ExpQ, Exp, Trees);

  Env* e = genenv();
  initenv(e);

  puts("\nTREES 0.8");
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
  if(first->type != VALUE_FUNCTION){
    destroyval(first);
    destroyval(ans);
    return valerr("Unbegun EXPS.");
  }
  
  Value* res = call(e,first,ans);
  
  destroyval(first);
  return res;
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



