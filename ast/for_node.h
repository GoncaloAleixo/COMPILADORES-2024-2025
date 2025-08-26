#pragma once

#include <cdk/ast/basic_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {

  /**
   * Class for describing for-cycle nodes.
   */
  class for_node : public cdk::basic_node {
    cdk::sequence_node *_inits;                 // zero or more declarations or expressions
    cdk::sequence_node *_conditions;            // zero or more expressions
    cdk::sequence_node *_increments;            // zero or more expressions
    cdk::basic_node *_block;                   // body of the loop

  public:
    for_node(int lineno,
             cdk::sequence_node *inits,
             cdk::sequence_node *conditions,
             cdk::sequence_node *increments,
             cdk::basic_node *block)
        : cdk::basic_node(lineno), _inits(inits),
          _conditions(conditions), _increments(increments), _block(block) {
    }

    cdk::sequence_node *inits() const { return _inits; }
    cdk::sequence_node *conditions() const { return _conditions; }
    cdk::sequence_node *increments() const { return _increments; }
    cdk::basic_node *block() const { return _block; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_for_node(this, level);
    }
  };

} // udf
