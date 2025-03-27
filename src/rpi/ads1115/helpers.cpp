#include "adc/helpers.hpp"

#include <cmath>

namespace helpers
{

namespace fp
{

double trim(double value, uint32_t prec)
{
    auto ratio = helpers::tr::pow(10, prec);
    return std::round(ratio * value) / ratio;
}

} // namespace fp

} // namespace helpers
