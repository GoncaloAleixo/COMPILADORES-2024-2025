#pragma once

#include "targets/basic_ast_visitor.h"
#include <cdk/types/primitive_type.h>
#include <cdk/types/reference_type.h>
#include <cdk/types/structured_type.h>

namespace udf {

  /**
   * Print nodes as XML elements to the output stream.
   */
  class type_checker: public basic_ast_visitor {
    cdk::symbol_table<udf::symbol> &_symtab;
    std::shared_ptr<udf::symbol> _function;
    basic_ast_visitor *_parent;
    bool _hasReturn = false;

  public:
    type_checker(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<udf::symbol> &symtab,std::shared_ptr<udf::symbol> function, basic_ast_visitor *parent) :
        basic_ast_visitor(compiler), _symtab(symtab),_function(function), _parent(parent){
    }

  public:
    ~type_checker() {
      os().flush();
    }

  protected:
    void processUnaryExpression(cdk::unary_operation_node *const node, int lvl);
    void processBinaryPIDExpression(cdk::binary_operation_node *const node, int lvl);
    void processBinaryIDExpression(cdk::binary_operation_node *const node, int lvl);
    void processBinaryIExpression(cdk::binary_operation_node *const node, int lvl);
    void checkPointersTypes(std::shared_ptr<cdk::reference_type> rPointer, std::shared_ptr<cdk::reference_type> lPointer) ;
    void checkArgs(std::shared_ptr<udf::symbol> previous,std::shared_ptr<udf::symbol> function);
    void checkGenComps(std::shared_ptr<cdk::structured_type> gen,std::shared_ptr<cdk::structured_type> fun);
    void process_unary_expr(cdk::unary_operation_node *const node, int lvl, bool accept_doubles); 
    template<typename T>
    void process_literal(cdk::literal_node<T> *const node, int lvl) {
    }

  public:
    // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
    // do not edit these lines: end

  };

  
  //---------------------------------------------------------------------------
  //     HELPER MACRO FOR TYPE CHECKING
  //---------------------------------------------------------------------------
  
  #define CHECK_TYPES(compiler, symtab, function ,node) { \
    try { \
      udf::type_checker checker(compiler, symtab, function, this); \
      (node)->accept(&checker, 0); \
    } \
    catch (const std::string &problem) { \
      std::cerr << (node)->lineno() << ": " << problem << std::endl; \
      return; \
    } \
  }
  
  #define ASSERT_SAFE_EXPRESSIONS CHECK_TYPES(_compiler, _symtab,_function , node)
  
} // udf