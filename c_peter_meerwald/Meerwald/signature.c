#include "signature.h"

void init_signature_bits() {
  bzero(signature, sizeof(signature));
}

void init_signature1_bits() {
  bzero(signature1, sizeof(signature1));
}

void init_signature2_bits() {
  bzero(signature2, sizeof(signature2));
}

int _get_signature_bit(char *s, int lim, int n) {
  int byte = n >> 3;
  int bit = n & 7;

#ifdef DEBUG
  if (byte < 0 || byte >= lim)
    fprintf(stderr, "get_signature_bit?(): index out of range\n");
#endif

  return (s[byte] & (1 << bit)) >> bit;
}

int get_signature_bit(int n) {
  return _get_signature_bit(signature, NSIGNATURE, n);
}

int get_signature1_bit(int n) {
  return _get_signature_bit(signature1, NSIGNATURE, n);
}

int get_signature2_bit(int n) {
  return _get_signature_bit(signature2, NSIGNATURE, n);
}

void _set_signature_bit(char *s, int limit, int n, int v) {
  int byte = n >> 3;
  int bit = n & 7;

#ifdef DEBUG
  if (byte < 0 || byte >= limit / 8)
    fprintf(stderr, "get_signature_bit?(): index out of range\n");
#endif

  if (v)
    s[byte] |= (1 << bit);
  else
    s[byte] &= ~(1 << bit);
}

void set_signature_bit(int n, int v) {
  _set_signature_bit(signature, NSIGNATURE, n, v);
}

void set_signature1_bit(int n, int v) {
  _set_signature_bit(signature1, NSIGNATURE, n, v);
}

void set_signature2_bit(int n, int v) {
  _set_signature_bit(signature2, NSIGNATURE, n, v);
}

int _binstr_to_sig(const char *binstr, char *sig, int *bytes, int *bits) {
  int n = strlen(binstr);
  int i;

  for (i = 0; i < n; i++) {
    if (binstr[i] == '0')
      _set_signature_bit(sig, NSIGNATURE, i, 0);
    else if (binstr[i] == '1')
      _set_signature_bit(sig, NSIGNATURE, i, 1);
    else
      return 0;
  }

  *bytes = (n % 8 > 0) ? n / 8 + 1 : n / 8;
  *bits = n;

  return 1;  
}

int binstr_to_sig(const char *binstr) {
  return _binstr_to_sig(binstr, signature, &n_signature, &nbit_signature);
}

int binstr_to_sig1(const char *binstr) {
  return _binstr_to_sig(binstr, signature1, &n_signature1, &nbit_signature1);
}

int binstr_to_sig2(const char *binstr) {
  return _binstr_to_sig(binstr, signature2, &n_signature2, &nbit_signature2);
}

int _sig_to_binstr(char *binstr, char *sig, int bits) {
  int i;

  for (i = 0; i < bits; i++)
    binstr[i] = _get_signature_bit(sig, NSIGNATURE, i) ? '1' : '0';

  binstr[bits] = '\0';

  return 1;
}

int sig_to_binstr(char *binstr) {
  return _sig_to_binstr(binstr, signature, nbit_signature);
}

int sig1_to_binstr(char *binstr) {
  return _sig_to_binstr(binstr, signature1, nbit_signature1);
}

int sig2_to_binstr(char *binstr) {
  return _sig_to_binstr(binstr, signature2, nbit_signature2);
}
