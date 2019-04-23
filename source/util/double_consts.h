#pragma once

#include <limits>

#define kDoubleNaN std::numeric_limits<double>::quiet_NaN()
#define kDoubleInf std::numeric_limits<double>::infinity()
#define kDoubleNInf -std::numeric_limits<double>::infinity()
#define kDoubleMin std::numeric_limits<double>::min()
#define kDoubleMax std::numeric_limits<double>::max()
