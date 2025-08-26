// udf/ast/tensor_contract_node.h
#pragma once
#include <cdk/ast/expression_node.h>
#include <cdk/ast/binary_operation_node.h>

namespace udf {
  /**
   * Node for tensor contraction (t1 ** t2).
   */
  class tensor_contract_node : public cdk::binary_operation_node{
  public:
    tensor_contract_node(int lineno, cdk::expression_node *l, cdk::expression_node *r)
      : cdk::binary_operation_node(lineno, l, r) {}

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_contract_node(this, level);
    }
  };
} // udf
