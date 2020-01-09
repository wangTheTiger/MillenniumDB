#ifndef VISITORS__VALUE_GETTER_H
#define VISITORS__VALUE_GETTER_H

#include "base/graph/value/value.h"

#include <boost/variant.hpp>

#include <string>

namespace visitors {
    class getValue: public boost::static_visitor<Value*> {
    public:
        std::unique_ptr<Value> operator() (int const& n) const {
            return std::make_unique<ValueInt>(n);
        }
        std::unique_ptr<Value> operator() (float const& f) const {
            return std::make_unique<ValueFloat>(f);
        }
        std::unique_ptr<Value> operator() (bool const& b) const {
            return std::make_unique<ValueBool>(b);
        }
        std::unique_ptr<Value> operator() (std::string const& text) const {
            return std::make_unique<ValueString>(text);
        }
    };

}; // namespace visitors


#endif // VISITORS__VALUE_GETTER_H