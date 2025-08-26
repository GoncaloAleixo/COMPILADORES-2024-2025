#pragma once

#include <cdk/ast/typed_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/types/basic_type.h>

namespace udf {

  class variable_declaration_node: public cdk::typed_node {
    int _qualifier;
    std::vector<std::string> _identifiers;
    cdk::expression_node *_initializer;

  public:
    variable_declaration_node(int lineno, int qualifier, std::shared_ptr<cdk::basic_type> type, std::vector<std::string> identifiers,
                              cdk::expression_node *initializer) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifiers(identifiers), _initializer(initializer) { _type = type;
    }

    int qualifier() {
      return _qualifier;
    }
    std::vector<std::string> identifiers() {
      return _identifiers;
    }
    cdk::expression_node *initializer() {
      return _initializer;
    }
    
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_variable_declaration_node(this, level);
    }

  };

}  // udf
