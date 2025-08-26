#pragma once
#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>

namespace udf {

  /**
   * Represents
   *   unless <condition> iterate <vector> for <count> using <function>;
   */
  class unless_node : public cdk::basic_node {
    cdk::expression_node *_condition;
    cdk::expression_node *_vector;
    cdk::expression_node *_count;
    cdk::expression_node *_function;

  public:
    unless_node(int lineno,
                cdk::expression_node *condition,
                cdk::expression_node *vector,
                cdk::expression_node *count,
                cdk::expression_node *function) :
        basic_node(lineno),
        _condition(condition),
        _vector(vector),
        _count(count),
        _function(function) {
    }

    cdk::expression_node* condition() const { return _condition; }
    cdk::expression_node* vector()    const { return _vector;    }
    cdk::expression_node* count()     const { return _count;     }
    cdk::expression_node* function()  const { return _function;  }

    void accept(basic_ast_visitor *visitor, int level) {
      visitor->do_unless_node(this, level);
    }
  };

} // namespace udf
