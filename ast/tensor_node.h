#pragma once

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {

  /**
   * Node for an explicitly declared tensor (tnode).
   */
  class tensor_node : public cdk::expression_node {
    cdk::sequence_node *_elements;

  public:
    tensor_node(int lineno, cdk::sequence_node *elements)
        : cdk::expression_node(lineno), _elements(elements) {}

    cdk::sequence_node *elements() const { return _elements; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_node(this, level);
    }
  };

} // udf
