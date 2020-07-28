
#ifndef STI_UTILS_MIXEDVALUE_FWD_H
#define STI_UTILS_MIXEDVALUE_FWD_H

#include <vector>

namespace STI
{
namespace Utils
{

class MixedValue;

typedef std::vector<MixedValue> MixedValueVector;

enum class MixedValueType { Boolean, Int, Double, String, Vector, Empty, File, Image, Any};


} //Utils
} //STI

#endif
