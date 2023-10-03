
#ifndef STI_UTILS_MIXEDVALUE_FWD_H
#define STI_UTILS_MIXEDVALUE_FWD_H

#include <vector>

namespace STI
{
/// Utils
namespace Utils
{

class MixedValue;

typedef std::vector<MixedValue> MixedValueVector;

enum class MixedValueType { Empty, Boolean, Int, Double, String, Vector, VectorInt, Binary, File, Image, Number, Any};


} //Utils
} //STI

#endif
