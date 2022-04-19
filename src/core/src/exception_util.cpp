#include "exception_util.h"

#ifdef ALBC_HAVE_BACKWARD
#include "backward.hpp"

namespace backward
{
static backward::SignalHandling sh; // NOLINT(cert-err58-cpp)
}
#endif