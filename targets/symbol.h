#pragma once

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace udf {

  class symbol {
    std::shared_ptr<cdk::basic_type> _type;
    std::string _name;
    long _value;  
    int _qualifier;
    bool _initialized;
    int _offset = 0;
    bool _function;
    bool _forward = false;
    bool _hasReturn = false;
    std::vector<std::shared_ptr<cdk::basic_type>> _arguments;

  public:
    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, bool initialized, bool function, int qualifier, bool forward = false ) :
        _type(type), _name(name), _value(0), _qualifier(qualifier), 
        _initialized(initialized), _function(function),_forward(forward), _hasReturn(false)
        {
    }

    virtual ~symbol() {
      // EMPTY
    }

    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    void type(std::shared_ptr<cdk::basic_type> t) {
        _type = t;
    }
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    const std::string &name() const {
      return _name;
    }
    long value() const {
      return _value;
    }
    long value(long v) {
      return _value = v;
    }
    int qualifier() const {
        return _qualifier;
    }
    bool initialized() const {
        return _initialized;
    }
    int offset() const {
        return _offset;
    }
    void set_offset(int offset)  {
        _offset = offset;
    }
    bool isFunction() const {
        return _function;
    }
    void set_return(bool ret)  {
        _hasReturn = ret;
    }
    bool hasReturn() const {
        return _hasReturn;
    }
    bool global() const {
        return _offset == 0;
    }
    bool isVariable() const {
        return !_function;
    }
    bool forward() const {
        return _forward;
    }
    std::vector<std::shared_ptr<cdk::basic_type>> arguments() {
        return _arguments;
    }
    void add_args(std::shared_ptr<cdk::basic_type> arg) {
        _arguments.push_back(arg);
    }
  };

} // udf
