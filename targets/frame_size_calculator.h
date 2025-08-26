#pragma once

#include "targets/basic_ast_visitor.h"

#include <string>
#include <iostream>
#include <sstream>
#include <stack>
#include <cdk/symbol_table.h>
#include "targets/symbol.h"

namespace udf {

  class frame_size_calculator: public basic_ast_visitor {
    cdk::symbol_table<udf::symbol> &_symtab;
    size_t _localsize;
    bool _ret;
    size_t _retsize;

  public:
    frame_size_calculator(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<udf::symbol> &symtab) :
        basic_ast_visitor(compiler), _symtab(symtab), _localsize(0), _ret(false),_retsize(0) {
    }

  public:
    ~frame_size_calculator();

  public:
    size_t localsize() const {
      return _localsize;
    }

    size_t retsize() const {
      return _retsize;
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // udf
