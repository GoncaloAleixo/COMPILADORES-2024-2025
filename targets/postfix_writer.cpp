#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <functional>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated
#include "targets/frame_size_calculator.h"

#include "udf_parser.tab.h"

//---------------------------------------------------------------------------

void udf::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void udf::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}

void udf::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if (_inFunctionBody) {
    _pf.DOUBLE(node->value());
  }
  else {
    _pf.SDOUBLE(node->value());
  }
}

void udf::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(0);                           
  _pf.EQ();                             
}

void udf::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  int lbl = ++_lbl;
  node->left()->accept(this,lvl+2);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl));
  node->right()->accept(this,lvl+2);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}
void udf::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  int lbl = ++_lbl;
  node->left()->accept(this, lvl+2);
  _pf.DUP32();
  _pf.JNZ(mklbl(lbl));
  node->right()->accept(this,lvl + 2);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

void udf::postfix_writer::do_unless_node(udf::unless_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _symtab.push();

  int lineno = node->lineno();
  _pf.ALIGN();

  node->condition()->accept(this, lvl);
  int endLabel = ++_lbl;
  _pf.JZ(mklbl(endLabel));

  const std::string it_name = "_it";
  auto zero = new cdk::integer_node(lineno, 0);
  auto it_decl = new udf::variable_declaration_node(
      lineno, tPRIVATE,
      cdk::primitive_type::create(4, cdk::TYPE_INT),
      std::vector<std::string>{it_name}, zero);
  it_decl->accept(this, lvl + 2);

  const std::string cnt_name = "_count";
  auto cnt_decl = new udf::variable_declaration_node(
      lineno, tPRIVATE,
      cdk::primitive_type::create(4, cdk::TYPE_INT),
      std::vector<std::string>{cnt_name}, node->count());
  cnt_decl->accept(this, lvl + 2);

  auto it_var    = new cdk::variable_node(lineno, it_name);
  auto it_rvalue = new cdk::rvalue_node(lineno, it_var);
  auto cnt_var   = new cdk::variable_node(lineno, cnt_name);
  auto cnt_rvalue= new cdk::rvalue_node(lineno, cnt_var);

  auto idxptr  = new udf::indexptr_node(lineno, node->vector(), it_rvalue);
  auto elem_rv = new cdk::rvalue_node(lineno, idxptr);

  auto fn_rv   = dynamic_cast<cdk::rvalue_node*>(node->function());
  auto fn_var  = dynamic_cast<cdk::variable_node*>(fn_rv->lvalue());
  auto funcName= fn_var->name();
  auto call    = new udf::function_call_node(
      lineno, funcName,
      new cdk::sequence_node(lineno, elem_rv));
  auto eval_call = new udf::evaluation_node(lineno, call);

  auto one     = new cdk::integer_node(lineno, 1);
  auto sum     = new cdk::add_node(lineno, it_rvalue, one);
  auto assign  = new cdk::assignment_node(lineno, it_var, sum);
  auto eval_inc= new udf::evaluation_node(lineno, assign);

  auto cmp_lt  = new cdk::lt_node(lineno, it_rvalue, cnt_rvalue);

  auto for_nd = new udf::for_node(
      lineno,
      /*inits=*/ new cdk::sequence_node(lineno),             
      /*conds=*/ new cdk::sequence_node(lineno, cmp_lt),
      /*incrs=*/ new cdk::sequence_node(lineno, eval_inc),
      /*block=*/ new cdk::sequence_node(lineno, eval_call)
  );
  for_nd->accept(this, lvl + 2);

  _pf.ALIGN();
  _pf.LABEL(mklbl(endLabel));
  _symtab.pop();
}


//---------------------------------------------------------------------------

void udf::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (_inFunctionBody) {
    _pf.INT(node->value()); // integer literal is on the stack: push an integer
  } else {
    _pf.SINT(node->value()); // integer literal is on the DATA segment
  }
}

void udf::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl1;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value()); // output string characters

  if (_function) {
    // local variable initializer
    _pf.TEXT();
    _pf.ADDR(mklbl(lbl1));
  } else {
    // global variable initializer
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_unary_minus_node(cdk::unary_minus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

    node->argument()->accept(this, lvl);    // determine the value
    if (node->is_typed(cdk::TYPE_DOUBLE)) { // 2-complement
        _pf.DNEG();
    } else {
        _pf.NEG();
    }
}

void udf::postfix_writer::do_unary_plus_node(cdk::unary_plus_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);

  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(3);
    _pf.SHTL();
  }
  

  node->right()->accept(this, lvl + 2);
  
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(3);
    _pf.SHTL();
  } 

  if (node->is_typed(cdk::TYPE_DOUBLE)){
    _pf.DADD();
  }
  else
    _pf.ADD();
}
void udf::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(3);
    _pf.SHTL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DSUB();
  else
    _pf.SUB();
}
void udf::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DMUL();
  else
    _pf.MUL();
}
void udf::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DDIV();
  else
    _pf.DIV();
}
void udf::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}
void udf::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  _pf.LT();
}
void udf::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  _pf.LE();
}
void udf::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  _pf.GE();
}
void udf::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  _pf.GT();
}
void udf::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  _pf.NE();
}
void udf::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl + 2);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->right()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  _pf.EQ();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  auto var = _symtab.find(node->name());
  if(_inFunctionBody && var->offset() != 0) {
    _pf.LOCAL(var->offset());
  }
  else {
    _pf.ADDR(var->name());
    if(var->is_typed(cdk::TYPE_STRUCT)){
      _pf.DUP32();
      _trash = true;
    }
  }
}

void udf::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->lvalue()->accept(this, lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDDOUBLE();
  }
  else {
    // integers, pointers, and strings
    _pf.LDINT();
  }
} 

void udf::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->rvalue()->accept(this, lvl + 2);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT))
      _pf.I2D();
      _pf.DUP64();
  } else {
    _pf.DUP32();
  }

  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.STDOUBLE();
  } else {
    _pf.STINT();
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_function_definition_node(udf::function_definition_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  
  _function = new_symbol();
  _functions_to_declare.erase(_function->name());
  reset_new_symbol();

  _offset = 8;
  _symtab.push(); // arguments scope
  if(node->arguments()) {
    _inFunctionArgs = true;
    for (size_t ix = 0; ix < node-> arguments()->size(); ix++){
      cdk::basic_node *argument = node->arguments()->node(ix);
      if (!argument ) break;
      argument->accept(this,0);
      
    }
    _inFunctionArgs = false;
  }

  _pf.TEXT();
  _pf.ALIGN();
  if (node->qualifier() == tPUBLIC)_pf.GLOBAL(_function->name(), _pf.FUNC());
  _pf.LABEL(_function->name());

  frame_size_calculator lsc(_compiler, _symtab);
  node->accept(&lsc,lvl);
  _pf.ENTER(lsc.localsize());

  _inFunctionBody = true;

  if(_function->type()->size() == 0)
    _offset = -lsc.retsize();
  else
    _offset = -_function->type()->size();    

  if(node->block())
    node->block()->accept(this, lvl + 4);


  _inFunctionBody = false;

  if (!node->is_typed(cdk::TYPE_VOID) && !_function->hasReturn()) {
  std::cerr << "missing return statement on non void function" << std::endl;
  exit(1);
  }

  _symtab.pop();

  _pf.LEAVE();
  _pf.RET();


   if (node->identifier() == "udf") {
     for(std::string s: _functions_to_declare) {
       _pf.EXTERN(s);
     }
   }
}

void udf::postfix_writer::do_function_declaration_node(udf::function_declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if(!new_symbol()) return;

  std::shared_ptr<udf::symbol> function = new_symbol();
  _functions_to_declare.insert(function->name());
  reset_new_symbol();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_evaluation_node(udf::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->argument()->accept(this, lvl); 
  if (node->argument()->is_typed(cdk::TYPE_INT)) {
    _pf.TRASH(4); 
  } else if (node->argument()->is_typed(cdk::TYPE_STRING)) {
    _pf.TRASH(4); 
  } else if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.TRASH(8);
  } else if (node->argument()->is_typed(cdk::TYPE_VOID)) {
    _pf.TRASH(4);
  } else if (node->argument()->is_typed(cdk::TYPE_POINTER)) {
    _pf.TRASH(4);
  } else if (node->argument()->is_typed(cdk::TYPE_STRUCT)) {
    _pf.TRASH(4);
  } else {
    std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
    exit(1);
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_write_node(udf::write_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->arguments()->accept(this, lvl); // determine the values to print
  for(int i = node->arguments()->size() - 1; i >= 0 ; i--){
    auto n = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i));
    write_aux(n->type());
  }
    if(node->newline()) {
      _functions_to_declare.insert("println");
      _pf.CALL("println"); // print a newline
    }
}

void udf::postfix_writer::write_aux(std::shared_ptr<cdk::basic_type> type){
  if (type->name() == cdk::TYPE_INT) {
      _functions_to_declare.insert("printi");
      _pf.CALL("printi");
      _pf.TRASH(4); // delete the printed value
    } else if (type->name() == cdk::TYPE_STRING) {
      _functions_to_declare.insert("prints");
      _pf.CALL("prints");
      _pf.TRASH(4); // delete the printed value's address
    } else if (type->name() == cdk::TYPE_DOUBLE) {
      _functions_to_declare.insert("printd");
      _pf.CALL("printd");
      _pf.TRASH(8); 
    } else if(type->name() == cdk::TYPE_STRUCT){
      auto gen = cdk::structured_type::cast(type);
      for(size_t i = 0; i < gen->length(); i++){
        write_aux(gen->component(i));
      }
    } else {
      std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
      exit(1);
    }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_input_node(udf::input_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  _functions_to_declare.insert("readi");
  _pf.CALL("readi");
  _pf.LDFVAL32();
  _pf.STINT();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_for_node(udf::for_node * const node, int lvl) {
  int cond = ++_lbl, incr = ++_lbl, endfor = ++_lbl;

  _forIni.push(cond); // after init, before body
  _forStep.push(incr);// after intruction
  _forEnd.push(endfor);// after for

  if(node->inits())
    node->inits()->accept(this, lvl + 2);
  
  _pf.ALIGN();
  _pf.LABEL(mklbl(cond));

  if(node->conditions())
    node->conditions()->accept(this, lvl + 2);

  _pf.JZ(mklbl(endfor));

  if(node->block())
    node->block()->accept(this, lvl + 2);

   _pf.ALIGN();
  _pf.LABEL(mklbl(incr));

  if(node->increments())
    node->increments()->accept(this, lvl + 2);

  _pf.JMP(mklbl(cond));

  _pf.ALIGN();
  _pf.LABEL(mklbl(endfor));

  _forIni.pop();
  _forStep.pop();
  _forEnd.pop();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_if_node(udf::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_if_else_node(udf::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}

//---------------------------------------------------------------------------
void udf::postfix_writer::do_continue_node(udf::continue_node * const node, int lvl) {
  if (_forIni.size() != 0) {
    _pf.JMP(mklbl(_forStep.top()));
  } else
    std::cerr << node->lineno() << "'continue' outside loop" <<std::endl;
}

//---------------------------------------------------------------------------
void udf::postfix_writer::do_sizeof_node(udf::sizeof_node * const node, int lvl) {
  if (_inFunctionBody) {
    _pf.INT(node->argument()->type()->size()); // integer literal is on the stack: push an integer
  } else {
    _pf.SINT(node->argument()->type()->size()); // integer literal is on the DATA segment
  }
}


//---------------------------------------------------------------------------

void udf::postfix_writer::do_objects_node(udf::objects_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  node->argument()->accept(this, lvl);
  
  auto type = cdk::reference_type::cast(node->type());
  if(type->referenced()->name() == cdk::TYPE_DOUBLE)
    _pf.INT(3);
  else
    _pf.INT(2);
  _pf.SHTL(); 
  _pf.ALLOC();
  _pf.SP();
}


//---------------------------------------------------------------------------

void udf::postfix_writer::do_break_node(udf::break_node * const node, int lvl) {
  if (_forIni.size() != 0) {
    _pf.JMP(mklbl(_forEnd.top()));
  } else 
    std::cerr << node->lineno() << "'break' outside loop" <<std::endl;
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_block_node(udf::block_node * const node, int lvl) {
  _symtab.push();
  if (node->declarations()) node->declarations()->accept(this, lvl + 2);
  if (node->instructions()) node->instructions()->accept(this, lvl + 2);
  _symtab.pop();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_return_node(udf::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  if (!_function->is_typed(cdk::TYPE_VOID)) {
    _return = true;
    node->retval()->accept(this, lvl + 2);
    _return = false;

    if (_function->is_typed(cdk::TYPE_INT) || _function->is_typed(cdk::TYPE_STRING) ||
    _function->is_typed(cdk::TYPE_POINTER)) {
      _pf.STFVAL32();
    } else if (_function->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->retval()->is_typed(cdk::TYPE_INT)) _pf.I2D();
        _pf.STFVAL64();
    } else if (_function->is_typed(cdk::TYPE_STRUCT)) {

    } else {
      std::cerr << node->lineno() << "unknown return type" <<std::endl;
    }
  }

  _pf.LEAVE();
  _pf.RET();
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_nullptr_node(udf::nullptr_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if(_inFunctionBody) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_indexptr_node(udf::indexptr_node * const node, int lvl) {
  node->base()->accept(this, lvl);
  node->index()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE))
    _pf.INT(3);
  else 
    _pf.INT(2);
  _pf.SHTL();
  _pf.ADD(); // add pointer and index
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_function_call_node(udf::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  std::shared_ptr<udf::symbol> symbol = _symtab.find(node->identifier());

  size_t argsSize = 0;

  if(node->is_typed(cdk::TYPE_STRUCT)){
    _pf.INT(node->type()->size());
    _pf.ALLOC();
    _pf.SP();
  }
  if (node->arguments()) {
    for (int ax = node->arguments()->size(); ax > 0; ax--) {
      auto arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(ax - 1));
      arg->accept(this, lvl + 2);
      if(arg->is_typed(cdk::TYPE_INT) && symbol->arguments().at(ax - 1)->name() == cdk::TYPE_DOUBLE){
        _pf.I2D();
        argsSize += 4;
      }
      argsSize += arg->type()->size();
    }
  }
  
  _pf.CALL(symbol->name());
  
      
  if(argsSize != 0){
    _pf.TRASH(argsSize);
  }

  if (symbol->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
    } else {
    _pf.LDFVAL32();
  }
}


//---------------------------------------------------------------------------

void udf::postfix_writer::do_address_of_node(udf::address_of_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void udf::postfix_writer::do_variable_declaration_node(udf::variable_declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto id = node->identifiers().at(0);
  int typesize = node->type()->size();
  int offset = 0;

  if (_inFunctionArgs) {
    offset = _offset;
    _offset += typesize;
  }
  else if (_inFunctionBody) {
    _offset -= typesize;
    offset = _offset;
  }
  else {
    offset = 0; // variável global
  }

  std::shared_ptr<udf::symbol> symbol = new_symbol();
  symbol->set_offset(offset);
  reset_new_symbol();

  if (_inFunctionBody) {
    // variável local
    if (node->initializer()) {
      node->initializer()->accept(this, lvl);

      if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_INT))
          _pf.I2D();
        _pf.LOCAL(offset);
        _pf.STDOUBLE();
      }
      else if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER) || node->is_typed(cdk::TYPE_STRING)) {
        _pf.LOCAL(offset);
        _pf.STINT();
      }
      else {
        std::cerr << "Erro: tipo não suportado para inicialização local." << std::endl;
        exit(1);
      }
    }
  }
  else if (!_function) {
    // variável global
    if (!node->initializer()) {
      _pf.BSS();
      _pf.ALIGN();
      _pf.LABEL(id);
      _pf.SALLOC(typesize);
    } else {
      _pf.DATA();
      _pf.ALIGN();
      _pf.LABEL(id);

      if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER)) {
        node->initializer()->accept(this, lvl);
      }
      else if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_DOUBLE)) {
          node->initializer()->accept(this, lvl);
        } else if (node->initializer()->is_typed(cdk::TYPE_INT)) {
          cdk::integer_node *dclini = dynamic_cast<cdk::integer_node *>(node->initializer());
          cdk::double_node ddi(dclini->lineno(), dclini->value());
          ddi.accept(this, lvl);
        } else {
          std::cerr << node->lineno() << ": '" << id << "' tem inicializador inválido para double.\n";
          exit(1);
        }
      }
      else if (node->is_typed(cdk::TYPE_STRING)) {
        node->initializer()->accept(this, lvl);
      }
      else {
        std::cerr << node->lineno() << ": '" << id << "' tem tipo global inesperado.\n";
        exit(1);
      }
    }
  }
}


//---------------------------------------------------------------------------

void udf::postfix_writer::do_tensor_node(udf::tensor_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // 1) Inicializa a RTS (mem_init) apenas uma vez
  static bool initialized = false;
  if (!initialized) {
    _pf.CALL("mem_init");
    initialized = true;
  }

  // 2) Aloca o tensor: empilha dims (inverso), depois n_dims, e chama tensor_create
  auto &dims = cdk::tensor_type::cast(node->type())->dims();
  for (int i = int(dims.size()) - 1; i >= 0; --i)  // push dims[k-1] ... dims[0]
    _pf.INT(dims[i]);
  _pf.INT(dims.size());                              // push n_dims
  _pf.CALL("tensor_create");                         // → Tensor* em EAX
  _pf.LDFVAL32();                                    // empilha o ponteiro

  // 3) Popula cada elemento via tensor_set
  size_t total = 1;
  for (auto d : dims) total *= d;
  // calculamos índices em C++ (sabemos dims[] em compile time)
  std::vector<size_t> idx(dims.size());
 for (size_t flat = 0; flat < total; ++flat) {
    size_t rem = flat;
    for (int d = int(dims.size()) - 1; d >= 0; --d) {
      idx[d] = rem % dims[d];
      rem /= dims[d];
    }

    _pf.DUP32();                   // duplica o Tensor*
    // empilha o valor literal
    node->elements()->node(flat)
        ->accept(this, lvl + 2);
    // empilha cada índice
    for (auto v : idx) _pf.INT(v);
    // chama a RTS para armazenar
    _pf.CALL("tensor_set");
  }
}

void udf::postfix_writer::do_tensor_reshape_node(udf::tensor_reshape_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // 1) empilha o ponteiro para o tensor a reshapar
  node->tensor()->accept(this, lvl);
  // 2) empilha as novas dimensões, em ordem inversa
  auto *dims = node->dimensions();
  for (int i = (int)dims->size() - 1; i >= 0; --i)
    static_cast<cdk::expression_node*>(dims->node(i))->accept(this, lvl);
  // 3) empilha o número de novas dimensões
  _pf.INT(dims->size());
  // 4) chama tensor_reshape(tensor, new_n_dims, ...);
  _pf.CALL("tensor_reshape");  
  // 5) carrega o pointer retornado (32‐bit)
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_contract_node(udf::tensor_contract_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // 1) empilha segundo operando (B)
  node->right()->accept(this, lvl);
  // 2) empilha primeiro operando (A)
  node->left()->accept(this, lvl);
  // 3) chama tensor_matmul(A, B)
  _pf.CALL("tensor_matmul");
  // 4) carrega o ponteiro para o novo tensor
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_rank_node(udf::tensor_rank_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // empilha o tensor e chama tensor_get_n_dims
  node->tensor()->accept(this, lvl);
  _pf.CALL("tensor_get_n_dims");
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_capacity_node(udf::tensor_capacity_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // empilha o tensor e chama tensor_size
  node->argument()->accept(this, lvl);
  _pf.CALL("tensor_size");
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_index_node(udf::tensor_index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // 1) empilha índices em ordem inversa
  auto *idxs = node->indexes();
  for (int i = (int)idxs->size() - 1; i >= 0; --i)
    static_cast<cdk::expression_node*>(idxs->node(i))->accept(this, lvl);
  // 2) empilha o ponteiro para o tensor
  node->tensor()->accept(this, lvl);
  // 3) chama tensor_getptr(tensor, ...indices)
  _pf.CALL("tensor_getptr");
  // 4) carrega o ponteiro para o elemento (double*)
  _pf.LDFVAL32();
}

void udf::postfix_writer::do_tensor_dims_node(udf::tensor_dims_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // empilha o tensor e chama tensor_get_dims
  node->tensor()->accept(this, lvl);
  _pf.CALL("tensor_get_dims");
  _pf.LDFVAL32();  // devolve pointer para array de dims
}

void udf::postfix_writer::do_tensor_dim_node(udf::tensor_dim_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  // 1) Avaliar ponteiro para o tensor
  node->tensor()->accept(this, lvl + 2);
  // 2) Avaliar índice da dimensão
  node->index()->accept(this, lvl + 2);
  // 3) Chamar a função RTS que retorna o tamanho da dimensão
  _pf.CALL("tensor_get_dim_size"); 
}