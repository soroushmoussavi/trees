#include "trees.h"
#include "utils.h"

int main(int argc, char** argv){

  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Float = mpc_new("float");
  mpc_parser_t* String = mpc_new("string");
  mpc_parser_t* Comment = mpc_new("comment");
  mpc_parser_t* Reference = mpc_new("reference");
  mpc_parser_t* Reveal = mpc_new("reveal");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* ExpS = mpc_new("exps");
  mpc_parser_t* ExpQ = mpc_new("expq");
  mpc_parser_t* Exp = mpc_new("exp");
  mpc_parser_t* Trees = mpc_new("trees");
  
  mpca_lang(MPCA_LANG_DEFAULT,
      "\
        number : /-?[0-9]+/ ;\
        float : /(-?[0-9]+)(\\.[0-9]+)/ ;\
        string  : /\"(\\\\.|[^\"])*\"/ ;\
        comment : /-[^\\r\\n]*/ ;\
        reference: /\\*[^\\r\\n]*/  ;\
        reveal: '!' <exps>+ ;\
        symbol : /[a-z]+/ | ':' ;\
        exps : '(' <exp>* ')';\
        expq : '{' <exp>* '}';\
        exp : <float> | <number> | <string> | <symbol> | <exps> | <expq>;\
        trees : /^/ ( <comment> | <reference> | <reveal> | <exp> )* /$/ ;a\
      ",
    Number, Float, String, Comment, Reference, Reveal, Symbol, ExpS, ExpQ, Exp, Trees);

  Env* e = genenv();
  initenv(e);
  
  if(argc > 1){
    for(int i = 1; i < argc; i++){
      Value* arg = valref(argv[i]);
      Value* x = reference(e,arg,Trees);
      if (x->type == VALUE_ERROR) { printlnval(x); }
      destroyval(x);
    }
  } else {
  
    puts("\nTREES 0.8");
    puts("Ctrl+C to Exit \n");

    while(1){
        char* input = readline("trees: ");
        add_history(input);

        mpc_result_t r;

        if(mpc_parse("<stdin>", input, Trees, &r)){

          Value* ans = read(r.output);
          if(ans->type != VALUE_REF) ans = eval(e,ans);
          else ans = reference(e,ans,Trees);
          if(ans->type != VALUE_NIL) printlnval(ans);

          destroyval(ans);
          mpc_ast_delete(r.output);

        } else {
          mpc_err_print(r.error); mpc_err_delete(r.error);
        }
        
        free(input);
    }
  }
  
  mpc_cleanup(11, Float, Number, String, Comment, Reference, Reveal, Symbol, ExpS, ExpQ, Exp, Trees);
  destroyenv(e);

  return 0;
}

Value* read(mpc_ast_t* t){

  if(strstr(t->tag, "number")) return readint(t); 
  if(strstr(t->tag, "float")) return readfloat(t); 
  if(strstr(t->tag, "string")) return readstr(t); 
  if(strstr(t->tag, "reference")) return valref(t->contents);
  if(strstr(t->tag, "symbol")) return valsym(t->contents);

  Value* ans = NULL;
  if(!strcmp(t->tag,">")) ans = valexps();
  if(strstr(t->tag,"exps")) ans = valexps();
  if(strstr(t->tag,"expq")) ans = valexpq();
  if(strstr(t->tag,"reveal")){
    ans = read(t->children[1]);
    ans->reveal = SHOW;
    return ans;
  }

  for(int i = 0; i<t->children_num; i++){
    if( 
      !strcmp(t->children[i]->contents,"(") ||
      !strcmp(t->children[i]->contents,")") || 
      !strcmp(t->children[i]->tag,"regex")  ||
      strstr(t->children[i]->tag, "comment") ||
      !strcmp(t->children[i]->contents,"{") ||
      !strcmp(t->children[i]->contents,"}")
    ) continue;

    addval(ans,read(t->children[i]));
  }

  if(ans->type == VALUE_EXPS){
    if(!ans->count) return valnil();
    if(ans->count == 1 && ans->cell[0]->type == VALUE_REF) return take(ans,0);
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

  VAL_REVEAL temp = ans->reveal;
  Value* res = call(e,first,ans);
  res->reveal = temp;

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

Value* readstr(mpc_ast_t* t){
  t->contents[strlen(t->contents)-1] = '\0';
  char* unescaped = (char*) malloc(sizeof(t->contents+1)+1);
  strcpy(unescaped,t->contents+1);
  unescaped = mpcf_unescape(unescaped);
  
  Value* stringval = valstr(unescaped);
  free(unescaped);
  return stringval;
}

Value* reference(Env* e, Value* ans, mpc_parser_t* p){
  
  VALASSERT(ans, ans->type == VALUE_REF, "REFERENCE Type Required '*'. Given %s.",printtype(ans->cell[0]->type));

  mpc_result_t r;

  if(mpc_parse_contents(ans->ref, p, &r)){
      Value* res = read(r.output);
      mpc_ast_delete(r.output);

      while(res->count){
        Value* x = NULL;
        if(res->cell[0]->type == VALUE_REF){x = reference(e,pop(res,0),p);}
        else {x = eval(e,pop(res,0));}
        if(x->reveal == SHOW || x->type == VALUE_ERROR) printlnval(x);
        destroyval(x);
      }
      destroyval(res); destroyval(ans);
      return valnil();
  } else {
    char* error = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    Value* errval = valerr("Improper Reference %s", error);
    free(error);
    destroyval(ans);

    return errval;
  }
}



