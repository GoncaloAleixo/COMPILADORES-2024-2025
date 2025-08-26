#pragma once

#include "targets/basic_ast_visitor.h"

#include <stack>
#include <set>
#include <sstream>
#include <cdk/emitters/basic_postfix_emitter.h>

namespace udf {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<udf::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;
    int _offset;
    std::shared_ptr<udf::symbol> _function;
    bool _inFunctionBody;
    bool _inFunctionArgs;
    bool _trash;
    bool _return;
    std::stack<int> _forIni, _forEnd, _forStep;
    std::set<std::string> _functions_to_declare;
    
 

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<udf::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf),
         _lbl(0),_offset(0), _function(nullptr),
          _inFunctionBody(false), _inFunctionArgs(false),
          _trash(false),_return(false){
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

  protected:
    void write_aux(std::shared_ptr<cdk::basic_type> type);

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // udf
