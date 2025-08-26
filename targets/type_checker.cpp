#include <string>
#include "targets/type_checker.h"
#include "targets/symbol.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>
#include <cdk/types/basic_type.h>
#include <cdk/types/reference_type.h>
#include "udf_parser.tab.h"

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void udf::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i<node->size(); i++) node->node(i)->accept(this,lvl);
}

void udf::type_checker::process_unary_expr(
    cdk::unary_operation_node *const node, int lvl, bool accept_doubles) {
    ASSERT_UNSPEC;

    node->argument()->accept(this, lvl);
    if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
        node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (!node->argument()->is_typed(cdk::TYPE_INT) &&
               !(accept_doubles &&
                 node->argument()->is_typed(cdk::TYPE_DOUBLE))) {
        throw std::string("wrong type in argument of unary expression");
    }

    node->type(node->argument()->type());
}

//---------------------------------------------------------------------------

void udf::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void udf::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}
void udf::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}
void udf::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  process_unary_expr(node, lvl, false);
}
void udf::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processBinaryIExpression(node,lvl);
}
void udf::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processBinaryIExpression(node,lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

//---------------------------------------------------------------------------

void udf::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in argument of unary expression");

  // in UDF, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void udf::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::processBinaryPIDExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  }
  else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  }
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  }
  else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
  }
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->type(node->right()->type());
  }
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }
  else
    throw std::string("wrong types in binary operation");

}

void udf::type_checker::processBinaryIDExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  }
  else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  }
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  }
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }
  else
    throw std::string("wrong types in binary operation");
}

void udf::type_checker::processBinaryIExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of binary expression");

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of binary expression");

  // in UDF, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryPIDExpression(node, lvl);
}
void udf::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryPIDExpression(node, lvl);
}
void udf::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryIDExpression(node, lvl);
}
void udf::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryPIDExpression(node, lvl);
}
void udf::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processBinaryIExpression(node, lvl);
}
void udf::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processBinaryIExpression(node, lvl);
}
void udf::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processBinaryIExpression(node, lvl);
}
void udf::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processBinaryIExpression(node, lvl);
}
void udf::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processBinaryIExpression(node, lvl);
}
void udf::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processBinaryIExpression(node, lvl);
}
void udf::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processBinaryIExpression(node, lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<udf::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void udf::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void udf::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;
  
  try {
    node->lvalue()->accept(this, lvl + 2);
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
  node->rvalue()->accept(this, lvl + 2);

  if (node->lvalue()->is_typed(cdk::TYPE_INT)) {
    if(node->rvalue()->is_typed(cdk::TYPE_INT)) {
        node->type(cdk::primitive_type::create(4,cdk::TYPE_INT));
    } else if(node->rvalue()->is_typed(cdk::TYPE_UNSPEC)){
        node->type(cdk::primitive_type::create(4,cdk::TYPE_INT));
        node->rvalue()->type(cdk::primitive_type::create(4,cdk::TYPE_INT));
    } else if(node->rvalue()->is_typed(cdk::TYPE_POINTER)){
        auto t = cdk::reference_type::cast(node->rvalue()->type());
        if(t->referenced()->name() != cdk::TYPE_UNSPEC)
          throw std::string("wrong assigment to integer.");
        else{
          node->type(cdk::primitive_type::create(4,cdk::TYPE_INT));
          node->rvalue()->type(cdk::primitive_type::create(4,cdk::TYPE_INT));
        }
    } else throw std::string("wrong assigment to integer.");
  } else if(node->lvalue()->is_typed(cdk::TYPE_DOUBLE)){
    if(node->rvalue()->is_typed(cdk::TYPE_INT) || node->rvalue()->is_typed(cdk::TYPE_DOUBLE)) {
        node->type(cdk::primitive_type::create(4,cdk::TYPE_DOUBLE));
    } else if(node->rvalue()->is_typed(cdk::TYPE_UNSPEC)){
        node->type(cdk::primitive_type::create(4,cdk::TYPE_DOUBLE));
        node->rvalue()->type(cdk::primitive_type::create(4,cdk::TYPE_DOUBLE));
    } 
    else throw std::string("wrong assigment to double.");
  } else if(node->lvalue()->is_typed(cdk::TYPE_STRING)) {

    if(node->rvalue()->is_typed(cdk::TYPE_STRING)) {
        node->type(cdk::primitive_type::create(4,cdk::TYPE_STRING));
    } else if(node->rvalue()->is_typed(cdk::TYPE_UNSPEC)){
        throw std::string("wrong assigment to string.");
    } 
    else throw std::string("wrong assigment to string.");
  } else if(node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    if(node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
        checkPointersTypes(cdk::reference_type::cast(node->lvalue()->type()),cdk::reference_type::cast(node->rvalue()->type()));
        node->rvalue()->type(node->lvalue()->type());
        node->type(node->lvalue()->type());
    } else throw std::string("wrong assigment to pointer.");
  } 
}

//---------------------------------------------------------------------------

void udf::type_checker::do_function_definition_node(udf::function_definition_node *const node, int lvl) {
  std::string id;

  // "fix" naming issues...
  if (node->identifier() == "udf")
    id = "_main";
  else if (node->identifier() == "_main")
    id = "._main";
  else
    id = node->identifier();
  std::shared_ptr<udf::symbol> function = std::make_shared<udf::symbol>\
  (node->type(),id,false, true,node->qualifier());



  if (node-> arguments()) {
    for (size_t i = 0; i < node->arguments()->size(); i++) {
      auto type = (dynamic_cast<cdk::typed_node *>(node->arguments()->node(i)))->type();
      if (type) {
        function->add_args(type);
      }
      
    }
  }
  // if(node->block()){
  //     auto b = dynamic_cast<udf::block_node*>(node->block());
  //     if(b->declarations())
  //       b->declarations()->accept(this, lvl + 2);
  //   }  
  
  set_new_symbol(function);
  

  std::shared_ptr<udf::symbol> previous = _symtab.find(function->name());
  
  if (previous) {
    if(previous->forward()){
      if (previous->qualifier() != tFORWARD && function->qualifier() != previous->qualifier()) {
          throw std::string("different qualifier in function '" + function->name() + "' redeclaration." );
      }
     
      if (function->type()->name() != previous->type()->name()) {
          throw std::string("different return type in function '" + function->name() + "' redeclaration." );
      }
      
      if (function->type()->name() == cdk::TYPE_POINTER){
          checkPointersTypes(cdk::reference_type::cast(previous->type()),cdk::reference_type::cast(function->type()));
      }

      checkArgs(previous,function);
      _symtab.replace(function->name(), function);
      _parent->set_new_symbol(function);
    } else throw std::string("conflicting definition for '" + function->name() + "'");
  }
  else {
    _symtab.insert(function->name(), function);
    _parent->set_new_symbol(function);
  }
}


void udf::type_checker::do_function_declaration_node(udf::function_declaration_node *const node, int lvl) {
  std::string id;

  if (node->identifier() == "udf")
    id = "_main";
  else if (node->identifier() == "_main")
    id = "._main";
  else
    id = node->identifier();

  std::shared_ptr<udf::symbol> function = std::make_shared<udf::symbol>\
  (node->type(),id,false, true,node->qualifier(), true);

  if (node-> arguments()) {
      for (size_t i = 0; i < node->arguments()->size(); i++) {
        auto type = (dynamic_cast<cdk::typed_node *>(node->arguments()->node(i)))->type();
        if (type) {
          function->add_args(type);
        }
    }
  }

  std::shared_ptr<udf::symbol> previous = _symtab.find(function->name());
  if (previous) {
    
    if (function->qualifier() != previous->qualifier()) {
        throw std::string("different qualifier in function '" + function->name() + "' redeclaration." );
    }
    if (function->type()->name() != previous->type()->name()) {
        throw std::string("different return type in function '" + function->name() + "' redeclaration." );
    }
    if (function->type()->name() == cdk::TYPE_POINTER){
        checkPointersTypes(cdk::reference_type::cast(previous->type()),cdk::reference_type::cast(function->type()));
    }
    checkArgs(previous,function);
  }
  else {
    _symtab.insert(function->name(), function);
    _parent->set_new_symbol(function);
  }

}

void udf::type_checker::do_evaluation_node(udf::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void udf::type_checker::do_write_node(udf::write_node *const node, int lvl) {
  node->arguments()->accept(this, lvl + 2);
   for(size_t i = 0; i < node->arguments()->size(); i++){
      cdk::expression_node* n = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i));
      if( n->is_typed(cdk::TYPE_UNSPEC)){
        n->type(cdk::primitive_type::create(4,cdk::TYPE_UNSPEC));
      }
      else if(n->is_typed(cdk::TYPE_POINTER))
          throw std::string("Invalid write argument type (pointer)"); 
   }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_input_node(udf::input_node *const node, int lvl) {
  try {
    ASSERT_UNSPEC;
    node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_for_node(udf::for_node *const node, int lvl) {
  if (node->inits()) node->inits()->accept(this, lvl + 4);
  if (node->conditions()) node->conditions()->accept(this, lvl + 4);
  if (node->increments()) node->increments()->accept(this, lvl + 4);
  if (node->block()) node->block()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_if_node(udf::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  node->block()->accept(this, lvl + 4);
}

void udf::type_checker::do_if_else_node(udf::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  node->thenblock()->accept(this, lvl + 4);
  node->elseblock()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_continue_node(udf::continue_node *const node, int lvl) {
   // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_sizeof_node(udf::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(cdk::primitive_type::create(4,cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_objects_node(udf::objects_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl +2);
  if (!node->argument()->is_typed(cdk::TYPE_INT))
    throw std::string("integer expression expected for memory allocation.");

  auto myType = cdk::reference_type::create(4,cdk::primitive_type::create(0,cdk::TYPE_UNSPEC));
  node->type(myType);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_break_node(udf::break_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_block_node(udf::block_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_return_node(udf::return_node *const node, int lvl) {
  if (node->retval()) {

    if (_function->is_typed(cdk::TYPE_VOID)) throw std::string("return specified for procedure.");
    
    node->retval()->accept(this, lvl + 2);
    if (_function->is_typed(cdk::TYPE_INT)) {
        if(node->retval()->is_typed(cdk::TYPE_UNSPEC))
          node->retval()->type(cdk::primitive_type::create(4,cdk::TYPE_INT));
        if (!node->retval()->is_typed(cdk::TYPE_INT))
            throw std::string("wrong type for return. Integer expected.");
    }
    else if (_function->is_typed(cdk::TYPE_DOUBLE)) {
      if(node->retval()->is_typed(cdk::TYPE_UNSPEC))
          node->retval()->type(cdk::primitive_type::create(8,cdk::TYPE_DOUBLE));
        if (!(node->retval()->is_typed(cdk::TYPE_INT) || \
        node->retval()->is_typed(cdk::TYPE_DOUBLE)))
            throw std::string("wrong type for return. Integer or double expected.");
    }
    else if (_function->is_typed(cdk::TYPE_STRING)) {
            if (!node->retval()->is_typed(cdk::TYPE_STRING))
                throw std::string("wrong type for return. String expected.");
    }
    else if (_function->is_typed(cdk::TYPE_POINTER)) {
        if (!node->retval()->is_typed(cdk::TYPE_POINTER))
          throw std::string("wrong type for return. String expected.");
        checkPointersTypes(cdk::reference_type::cast(_function->type()),cdk::reference_type::cast(node->retval()->type()));
    } else if(_function->is_typed(cdk::TYPE_UNSPEC)){
        _function->type(node->retval()->type());
    } else if(_function->is_typed(cdk::TYPE_STRUCT)){
        if(node->retval()->is_typed(cdk::TYPE_STRUCT))
          checkGenComps(cdk::structured_type::cast(node->retval()->type()),cdk::structured_type::cast(_function->type()));
        else
          throw std::string("wrong type for return. ");
    } else {
        throw std::string("unknown type for return.");
    }
  }
  _function->set_return(true);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_nullptr_node(udf::nullptr_node *const node, int lvl) {
  ASSERT_UNSPEC;
    node->type(cdk::reference_type::create(
        4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_indexptr_node(udf::indexptr_node *const node, int lvl) {
  ASSERT_UNSPEC;
  if (node->base()) {
    node->base()->accept(this, lvl + 2);
    if (!node->base()->is_typed(cdk::TYPE_POINTER)) throw std::string("pointer expression expected in left-index value.");
    }
    else  {
        if(!_function->is_typed(cdk::TYPE_POINTER))
            throw std::string("return pointer expression expected in left-value index.");
  }

  node->index()->accept(this, lvl + 2);
  if(!node->index()->is_typed(cdk::TYPE_INT)) throw std::string("integer expression expected in left-value index");
  
  node->type(cdk::reference_type::cast(node->base()->type())->referenced());
}

//---------------------------------------------------------------------------

void udf::type_checker::do_function_call_node(udf::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC;
  
  std::string id;
  if (node->identifier() == "udf")
    id = "_main";
  else if (node->identifier() == "_main")
    id = "._main";
  else
    id = node->identifier();

  std::shared_ptr<udf::symbol> symbol = _symtab.find(id);

  if (symbol == nullptr) throw std::string("symbol '" + id + "' is undeclared.");

  if (!symbol->isFunction()) throw std::string("symbol '" + id + "' is not a function.");

  
  if (node->arguments() && symbol->arguments().size() != node->arguments()->size())  std::cout<<"invalid number of arguments for function call" << std::endl;
  
  node->type(symbol->type());

  if ( node->arguments()) {
    node->arguments()->accept(this,lvl + 4);

    
    if(symbol->arguments().size() != node->arguments()->size()){
      throw std::string("Invalid number of function arguments");
    }
    
    for(size_t i = 0; i < node->arguments()->size(); i++){
      cdk::expression_node* n = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i));
      if(n->is_typed(cdk::TYPE_UNSPEC)){
        if(symbol->arguments().at(i)->name() == cdk::TYPE_INT || symbol->arguments().at(i)->name() == cdk::TYPE_DOUBLE) n->type(symbol->arguments().at(i));
        else throw std::string("invalid argument type in function call");
      }
      if(n->type() != symbol->arguments().at(i)) {
        if (n->is_typed(cdk::TYPE_INT) && symbol->arguments().at(i)->name() == cdk::TYPE_DOUBLE) {
          //n->type(cdk::make_primitive_type(8,cdk::TYPE_DOUBLE));
        }
        else throw std::string("invalid argument type in function call");
      }
    } 
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_address_of_node(udf::address_of_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->type(cdk::reference_type::create(4,node->lvalue()->type()));
}

//---------------------------------------------------------------------------
void udf::type_checker::do_variable_declaration_node(udf::variable_declaration_node *const node, int lvl) {
  // 1) Se vier com auto (tipo não especificado), infere do inicializador.
  if (!node->type() || node->type()->name() == cdk::TYPE_UNSPEC) {
    if (!node->initializer())
      throw std::string("auto requires an initializer");
    node->initializer()->accept(this, lvl + 2);
    auto init_t = node->initializer()->type();
    if (init_t->name() == cdk::TYPE_INT    ||
        init_t->name() == cdk::TYPE_DOUBLE ||
        init_t->name() == cdk::TYPE_STRING ||
        init_t->name() == cdk::TYPE_POINTER) {
      node->type(init_t);
      // Se for ponteiro genérico, manter como ptr<auto>  
    }
    else {
      throw std::string("cannot infer type for auto from initializer");
    }
  }

  // 2) Se houver initializer, cheque compatibilidade
  if (node->initializer()) {
    node->initializer()->accept(this, lvl + 2);
    auto declared = node->type()->name(), given = node->initializer()->type()->name();
    if (declared == cdk::TYPE_INT) {
      if (given == cdk::TYPE_UNSPEC) {
        // input → int
        node->initializer()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      }
      if (node->initializer()->type()->name() != cdk::TYPE_INT)
        throw std::string("wrong type for initializer (integer expected).");
    }
    else if (declared == cdk::TYPE_DOUBLE) {
      if (given == cdk::TYPE_UNSPEC) {
        node->initializer()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      }
      if (given != cdk::TYPE_DOUBLE && given != cdk::TYPE_INT)
        throw std::string("wrong type for initializer (integer or double expected).");
    }
    else if (declared == cdk::TYPE_STRING) {
      if (given != cdk::TYPE_STRING)
        throw std::string("wrong type for initializer (string expected).");
    }
    else if (declared == cdk::TYPE_POINTER) {
      if (given != cdk::TYPE_POINTER)
        throw std::string("wrong type for initializer (pointer expected).");
      checkPointersTypes(
        cdk::reference_type::cast(node->type()),
        cdk::reference_type::cast(node->initializer()->type())
      );
      // normalize RHS type
      node->initializer()->type(node->type());
    }
  }

  // 3) Insere símbolo (único identificador)
  auto id = node->identifiers().at(0);
  std::shared_ptr<udf::symbol> symbol = std::make_shared<udf::symbol>(
    node->type(), id, 
    /*is_lvalue=*/false, 
    /*has_initializer=*/(bool)node->initializer(),
    node->qualifier()
  );
  if (!_symtab.insert(id, symbol))
    throw std::string("variable '" + id + "' redeclared.");
  _parent->set_new_symbol(symbol);
}


//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_reshape_node(udf::tensor_reshape_node *const node, int lvl) {
  ASSERT_UNSPEC;

  // 1) Type‐check do tensor de entrada e dos argumentos de reshape
  node->tensor()->accept(this, lvl + 2);
  node->dimensions()->accept(this, lvl + 2);

  // 2) Verificar que o operando é um tensor
  if (!node->tensor()->is_typed(cdk::TYPE_TENSOR))
    throw std::string(std::to_string(node->lineno())
      + ": reshape operand must be tensor");

  auto tt = cdk::tensor_type::cast(node->tensor()->type());
  const auto &old_dims = tt->dims();

  // 3) Extrair novas dimensões e calcular capacidades
  std::vector<size_t> new_dims;
  new_dims.reserve(node->dimensions()->nodes().size());
  size_t new_cap = 1;
  for (auto *ch : node->dimensions()->nodes()) {
    auto *tn = dynamic_cast<cdk::typed_node*>(ch);
    // cada argumento deve ser int
    if (!tn->is_typed(cdk::TYPE_INT))
      throw std::string(std::to_string(node->lineno())
        + ": reshape dimensions must be integers");
    // deve ser literal positivo
    auto *lit = dynamic_cast<cdk::integer_node*>(ch);
    if (!lit)
      throw std::string(std::to_string(node->lineno())
        + ": reshape dimensions must be integer literals");
    int v = lit->value();
    if (v <= 0)
      throw std::string(std::to_string(node->lineno())
        + ": reshape dimensions must be positive");
    new_dims.push_back(static_cast<size_t>(v));
    new_cap *= static_cast<size_t>(v);
  }

  // 4) Calcular capacidade do tensor original
  size_t old_cap = 1;
  for (auto d : old_dims) old_cap *= d;

  // 5) Verificar igualdade de capacidade
  if (new_cap != old_cap)
    throw std::string(std::to_string(node->lineno())
      + ": total number of elements mismatch in reshape");

  // 6) Atribuir tipo resultante
  node->type(cdk::tensor_type::create(new_dims));
}


void udf::type_checker::do_tensor_contract_node(udf::tensor_contract_node *const node, int lvl) {
  ASSERT_UNSPEC;

  // 1) Type‐check dos operandos
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  // 2) Ambos têm de ser tensores
  if (!node->left()->is_typed(cdk::TYPE_TENSOR))
    throw std::string(std::to_string(node->lineno())
      + ": left operand of ** must be tensor");
  if (!node->right()->is_typed(cdk::TYPE_TENSOR))
    throw std::string(std::to_string(node->lineno())
      + ": right operand of ** must be tensor");

  // 3) Extrair dimensões
  auto t1 = cdk::tensor_type::cast(node->left()->type());
  auto t2 = cdk::tensor_type::cast(node->right()->type());
  const auto &d1 = t1->dims();
  const auto &d2 = t2->dims();

  // Só faz sentido se ambos tiverem pelo menos 1 dimensão
  if (d1.empty() || d2.empty())
    throw std::string(std::to_string(node->lineno())
      + ": cannot contract scalar tensors");

  // 4) Verificar última dimensão de t1 == primeira de t2
  if (d1.back() != d2.front())
    throw std::string(std::to_string(node->lineno())
      + ": incompatible tensor dimensions for contraction");

  // 5) Calcular dims do tensor resultante:
  //    { d1[0..n-2], d2[1..m-1] }
  std::vector<size_t> result_dims;
  result_dims.reserve(d1.size() + d2.size() - 2);
  for (size_t i = 0; i + 1 < d1.size(); ++i)
    result_dims.push_back(d1[i]);
  for (size_t j = 1; j < d2.size(); ++j)
    result_dims.push_back(d2[j]);

  // 6) Atribuir tipo tensor<result_dims>
  node->type(cdk::tensor_type::create(result_dims));
}


void udf::type_checker::do_tensor_node(udf::tensor_node *const node, int lvl) {
  ASSERT_UNSPEC;

  // 1) Type‐check each elemento do literal
  node->elements()->accept(this, lvl + 2);

  // 2) Recolher todos como typed_node*
  std::vector<cdk::typed_node*> elems;
  for (auto *ch : node->elements()->nodes()) {
    auto *tn = dynamic_cast<cdk::typed_node*>(ch);
    if (!tn)
      throw std::string("internal error: tensor child not typed_node");
    elems.push_back(tn);
  }

  // 3) Construir o shape: primeira dimensão = nº de elementos
  std::vector<size_t> shape{elems.size()};

  if (!elems.empty()) {
    // 3a) Validar que cada elemento é int, double ou tensor
    for (auto *tn : elems) {
      if (!(tn->is_typed(cdk::TYPE_INT) ||
            tn->is_typed(cdk::TYPE_DOUBLE) ||
            tn->is_typed(cdk::TYPE_TENSOR)))
        throw std::string(std::to_string(node->lineno())
                          + ": tensor literals must contain only scalars or tensors");
    }
    // 3b) Se for tensor, herdar as dims do primeiro elemento
    if (elems.front()->is_typed(cdk::TYPE_TENSOR)) {
      auto tt = cdk::tensor_type::cast(elems.front()->type());
      const auto &inner = tt->dims();  // vector<size_t>
      shape.insert(shape.end(), inner.begin(), inner.end());
    }
  }

  // 4) Atribuir o tipo tensor criado (baseType inferido internamente se for tensor)
  node->type(cdk::tensor_type::create(shape));
}


void udf::type_checker::do_tensor_rank_node(udf::tensor_rank_node *const node, int lvl) {
  ASSERT_UNSPEC;

  // 1) Type‐check do operando
  node->tensor()->accept(this, lvl + 2);

  // 2) Só faz sentido em tensor
  if (!node->tensor()->is_typed(cdk::TYPE_TENSOR))
    throw std::string(std::to_string(node->lineno())
      + ": operand of .rank must be a tensor");

  // 3) Resultado é sempre inteiro
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_tensor_capacity_node(udf::tensor_capacity_node *const node, int lvl) {
  ASSERT_UNSPEC;

  // 1) Type‐check do operando
  node->argument()->accept(this, lvl + 2);

  // 2) Só faz sentido em tensor
  if (!node->argument()->is_typed(cdk::TYPE_TENSOR))
    throw std::string(std::to_string(node->lineno())
      + ": operand of .capacity must be a tensor");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}


void udf::type_checker::do_tensor_index_node(udf::tensor_index_node *const node, int lvl) {
  ASSERT_UNSPEC;

  // 1) Type‐check o tensor e os índices
  node->tensor()->accept(this, lvl + 2);
  node->indexes()->accept(this, lvl + 2);

  // 2) O operando principal tem de ser tensor
  if (!node->tensor()->is_typed(cdk::TYPE_TENSOR))
    throw std::string(std::to_string(node->lineno())
      + ": tensor indexing applied to a non-tensor expression");  

  // 3) Extrair as dimensões originais
  auto tt = cdk::tensor_type::cast(node->tensor()->type());
  const auto &dims = tt->dims(); 

  // 4) Verificar número de índices ≤ rank
  size_t idxCount = node->indexes()->size();
  if (idxCount > dims.size())
    throw std::string(std::to_string(node->lineno())
      + ": too many indices for tensor of rank " + std::to_string(dims.size()));

  // 5) Cada índice deve ser int
  for (size_t i = 0; i < idxCount; ++i) {
    auto *expr = dynamic_cast<cdk::typed_node*>(node->indexes()->node(i));
    if (!expr || !expr->is_typed(cdk::TYPE_INT))
      throw std::string(std::to_string(node->lineno())
        + ": tensor index must be integer expression");
  }

  // 6) Inferir tipo resultante:
  //    - se idxCount == rank → escalar (double)
  //    - caso contrário → tensor com as dimensões restantes
  if (idxCount == dims.size()) {
    // acesso a elemento escalar; retornamos um double
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  }
  else {
    // slicing: tensor de rank menor
    std::vector<size_t> rem_dims(dims.begin() + idxCount, dims.end());
    node->type(cdk::tensor_type::create(rem_dims));
  }
}

void udf::type_checker::do_tensor_dims_node(udf::tensor_dims_node *const node, int lvl) {
  ASSERT_UNSPEC;

  // 1) Type‐check do tensor
  node->tensor()->accept(this, lvl + 2);

  // 2) Só faz sentido se for tensor
  if (!node->tensor()->is_typed(cdk::TYPE_TENSOR))
    throw std::string(std::to_string(node->lineno())
      + ": .dims applied to a non-tensor expression");

  // 3) .dims devolve um ponteiro para vector<int>, 
  //    representamos isso como pointer to INT
  node->type(
    cdk::reference_type::create(
      /* size */ 4,
      cdk::primitive_type::create(4, cdk::TYPE_INT)
    )
  );
}

void udf::type_checker::do_tensor_dim_node(udf::tensor_dim_node *const node, int lvl) {
  ASSERT_UNSPEC;

  // 1) Type‐check do tensor e do índice
  node->tensor()->accept(this, lvl + 2);
  node->index()->accept(this, lvl + 2);

  // 2) Verificações básicas
  if (!node->tensor()->is_typed(cdk::TYPE_TENSOR))
    throw std::string(std::to_string(node->lineno())
      + ": .dim applied to a non-tensor expression");
  auto idx = dynamic_cast<cdk::typed_node*>(node->index());
  if (!idx || !idx->is_typed(cdk::TYPE_INT))
    throw std::string(std::to_string(node->lineno())
      + ": index to .dim must be integer expression");

  // 3) (Opcional) verificar literal dentro do rank
  auto tt = cdk::tensor_type::cast(node->tensor()->type());
  const auto &dims = tt->dims();
  if (auto lit = dynamic_cast<cdk::integer_node*>(node->index())) {
    int v = lit->value();
    if (v < 0 || static_cast<size_t>(v) >= dims.size())
      throw std::string(std::to_string(node->lineno())
        + ": .dim index out of range");
  }

  // 4) Resultado é sempre inteiro
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_unless_node(udf::unless_node *const node, int lvl) {
  // primeiro, verifica recursivamente os sub-nós
  node->condition()->accept(this, lvl + 4);
  node->vector   ()->accept(this, lvl + 4);
  node->count    ()->accept(this, lvl + 4);
  node->function ()->accept(this, lvl + 4);

  // condição pode ser int _ou_ double
  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {
    // por omissão, deixar como int
    node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }
  else if (!node->condition()->is_typed(cdk::TYPE_INT) &&
           !node->condition()->is_typed(cdk::TYPE_DOUBLE)) {
    throw std::string(std::to_string(node->lineno()) +
      ": wrong type in condition of unless instruction (must be int or real)");
  }

  // vector deve ser ponteiro
  if (!node->vector()->is_typed(cdk::TYPE_POINTER)) {
    throw std::string(std::to_string(node->lineno()) +
      ": wrong type in vector of unless instruction");
  }

  // count deve ser int
  if (node->count()->is_typed(cdk::TYPE_UNSPEC)) {
    node->count()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }
  else if (!node->count()->is_typed(cdk::TYPE_INT)) {
    throw std::string(std::to_string(node->lineno()) +
      ": wrong type in count of unless instruction");
  }

  // extrai o nome da função
  std::string funcId;
  cdk::expression_node *fn = node->function();
  if (auto *fc = dynamic_cast<udf::function_call_node*>(fn)) {
    funcId = fc->identifier();
  }
  else if (auto *var = dynamic_cast<cdk::variable_node*>(fn)) {
    funcId = var->name();
  }
  else if (auto *rv = dynamic_cast<cdk::rvalue_node*>(fn)) {
    if (auto *lv = dynamic_cast<cdk::variable_node*>(rv->lvalue())) {
      funcId = lv->name();
    }
  }
  if (funcId.empty()) {
    throw std::string(std::to_string(node->lineno()) +
      ": invalid iteratee in 'using', expected a function name or call");
  }

  // normaliza nomes especiais
  if (funcId == "udf")      funcId = "_main";
  else if (funcId == "_main") funcId = "._main";

  // procura símbolo
  auto symbol = _symtab.find(funcId);
  if (!symbol || !symbol->isFunction()) {
    throw std::string(std::to_string(node->lineno()) +
      ": '" + funcId + "' is not a function");
  }

  // deve ter exatamente um argumento
  if (symbol->arguments().size() != 1) {
    throw std::string(std::to_string(node->lineno()) +
      ": iteratee function must take exactly one argument");
  }

  // tipo do parâmetro deve bater com tipo do elemento do vetor
  auto paramType = symbol->arguments()[0];
  auto elemType  = cdk::reference_type::cast(node->vector()->type())->referenced();
  if (!(paramType->name() == elemType->name() ||
        (paramType->name() == cdk::TYPE_DOUBLE && elemType->name() == cdk::TYPE_INT))) {
    throw std::string(std::to_string(node->lineno()) +
      ": function argument type does not match vector element type");
  }
}


//-----------------------------------------------------------------------
//------------------------------AUXILIARY METHODS------------------------

void udf::type_checker::checkPointersTypes(std::shared_ptr<cdk::reference_type> rPointer, std::shared_ptr<cdk::reference_type> lPointer) {
    int rLvl = 1;
    int lLvl = 1;

    auto rRef = rPointer->referenced();
    auto lRef = lPointer->referenced();

    if(rRef->name() == cdk::TYPE_UNSPEC)
      return;

    while(true){
      if(rRef->name() != cdk::TYPE_POINTER) break;
      rLvl++;
      rRef = cdk::reference_type::cast(rRef)->referenced();
    }

    while(true){
      if(lRef->name() != cdk::TYPE_POINTER) break;
      lLvl++;
      lRef = cdk::reference_type::cast(lRef)->referenced();
    }
    if (lRef->name() == cdk::TYPE_UNSPEC);
    else if(lLvl != rLvl) throw std::string("Non matching pointer levels");
    else if(rRef->name() != lRef->name()) throw std::string("Non matching pointer types");

}

void udf::type_checker::checkArgs(std::shared_ptr<udf::symbol> previous,std::shared_ptr<udf::symbol> function)
{
    if (previous->arguments().size() == function->arguments().size()){
        for (size_t i = 0; i < function->arguments().size(); i++) {
            if (previous->arguments().at(i)->name() != function->arguments().at(i)->name() ) {
                throw std::string("arguments in function '" + function->name() + "' do not matching previous ones.");
            }
            else if ( previous->arguments().at(i)->name() == cdk::TYPE_POINTER && function->arguments().at(i)->name() == cdk::TYPE_POINTER) {
                checkPointersTypes(cdk::reference_type::cast(previous->arguments().at(i)),cdk::reference_type::cast(function->arguments().at(i)));
            }
        }
    }
    else {
        throw std::string("different number of arguments in function'" + function->name() + "' declaration.");
    }
}

void udf::type_checker::checkGenComps(std::shared_ptr<cdk::structured_type> gen,std::shared_ptr<cdk::structured_type> fun){
  for(size_t i = 0; i < gen->length(); i++){
    if(fun->component(i)->name() == cdk::TYPE_DOUBLE && gen->component(i)->name() == cdk::TYPE_INT)
      continue;

    if(fun->component(i)->name() != gen->component(i)->name())
      throw std::string("gen types differ on return");

    if(fun->component(i)->name() == cdk::TYPE_STRUCT)
      checkGenComps(std::dynamic_pointer_cast<cdk::structured_type>(gen->component(i)),std::dynamic_pointer_cast<cdk::structured_type>(fun->component(i)));
  }
}