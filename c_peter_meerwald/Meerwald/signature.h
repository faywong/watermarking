#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "wm.h"

#define NSIGNATURE 4096
#define NBITSIGNATURE (NSIGNATURE * 8)

int n_signature;
int nbit_signature;
int n_signature1;
int nbit_signature1;
int n_signature2;
int nbit_signature2;

char signature[NSIGNATURE];
char signature1[NSIGNATURE];
char signature2[NSIGNATURE];

void init_signature_bits();
int get_signature_bit(int n);
void set_signature_bit(int n, int v);

void init_signature1_bits();
int get_signature1_bit(int n);
void set_signature1_bit(int n, int v);

void init_signature2_bits();
int get_signature2_bit(int n);
void set_signature2_bit(int n, int v);

int binstr_to_sig(const char *binstr);
int binstr_to_sig1(const char *binstr);
int binstr_to_sig2(const char *binstr);
int sig_to_binstr(char *binstr);
int sig1_to_binstr(char *binstr);
int sig2_to_binstr(char *binstr);

#endif
