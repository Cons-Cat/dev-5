#include <stdio.h>

#define success(x, str)            \
  if (!x) {                        \
    std::cout << str << std::endl; \
  }                                \
  /* assert(x, str); */

#define fn auto
