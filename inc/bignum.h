#ifndef BIGNUM_H__
#define BIGNUM_H__

#include <stdio.h> // this is for size_t. Should be a better one
#include <stdbool.h>

#define DEFAULT_CAPACITY 128

typedef struct _bignum {
    unsigned char *data;
    size_t        size;
    size_t        cap;
    bool          negative;
} bignum;

void bignum_init_cap(bignum *n, size_t cap);
void bignum_init(bignum *n);
void bignum_resize(bignum *n);
void bignum_free(bignum *n);
void bignum_copy(bignum *dest, bignum *src);
void bignum_dump(bignum *n);
void bignum_bprint(bignum *n);
char* bignum_sprint(bignum *n, size_t base);
int bignum_to_int(bignum *n);
void bignum_print_int(bignum *n);
void bignum_from_char(bignum *n, unsigned char s);
void bignum_from_int(bignum *n, int s);
void bignum_inc(bignum *n);
void bignum_add(bignum *a, bignum *b);
void bignum_sub(bignum* a, bignum *b);
void bignum_mul_int(bignum *a, unsigned int b);
bool bignum_lt(bignum *a, bignum *b);
void bignum_div_mod(bignum *a, bignum *b, bignum *remainder);
void bignum_div(bignum *a, bignum *b);
void bignum_mod(bignum *a, bignum *b);
void bignum_from_bignum(bignum *n, bignum* s, size_t base);
void bignum_from_string_binary(bignum *n, char const* s, size_t base);
char* limited_precision_base_conv(long int number, size_t base);

#endif
