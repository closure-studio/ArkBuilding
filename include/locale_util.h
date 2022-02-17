#pragma once

#include "primitive_types.h"

namespace albc
{
    // convert from UTF-8 to OS charset
string toOSCharset(const string &src);
}