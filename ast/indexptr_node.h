#pragma once

#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>

namespace udf {

  /**
   * Class for describing index nodes (for pointer indexing).
   */
  class indexptr_node : public cdk::lvalue_node {
    cdk::expression_node *_base, *_index;

  public:
    indexptr_node(int lineno, cdk::expression_node *base,
               cdk::expression_node *index)
        : cdk::lvalue_node(lineno), _base(base), _index(index) {}

    cdk::expression_node *base()   { return _base; }
    cdk::expression_node *index() { return _index; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_indexptr_node(this, level);
    }
  };

} // udf
