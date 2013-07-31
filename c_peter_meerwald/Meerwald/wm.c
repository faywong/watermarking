#include "wm.h"

#ifdef __MINGW32_VERSION
void bzero(char *b, size_t length) {
  int i;
  for (i=0; i<length; i++) { *b=0; b++; }
}
#endif

void set_in_binary() {
#if defined(EMX)
  _fsetmode(in, "b");
#elif defined(MINGW)
  setmode(STDIN_FILENO, O_BINARY);
#endif
}

void set_out_binary() {
#if defined(EMX)
  _fsetmode(out, "b");
#elif defined(MINGW)
  setmode(STDOUT_FILENO, O_BINARY);
#endif
}

void wm_init2() {
  set_in_binary();
}

void wm_init1() {
  set_out_binary();
}

void wm_init() {
  set_in_binary();
  set_out_binary();
}

