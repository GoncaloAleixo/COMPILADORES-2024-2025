#pragma once

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {

  /**
   * Node for tensor reshaping 
   */
  class tensor_reshape_node : public cdk::expression_node {
    cdk::expression_node *_tensor;
    cdk::sequence_node *_dimensions;

  public:
    tensor_reshape_node(int lineno, cdk::expression_node *tensor, cdk::sequence_node *dimensions)
        : cdk::expression_node(lineno), _tensor(tensor), _dimensions(dimensions) {}

    cdk::expression_node *tensor() { return _tensor; }
    cdk::sequence_node *dimensions() { return _dimensions; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_reshape_node(this, level);
    }
  };

} // udf
