#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "bignum.h"

static size_t mul_lut_size = 0;
static bignum *mul_lut = NULL;

void bignum_init_cap(bignum *n, size_t cap) {
    n->data = malloc(cap * sizeof(unsigned char));
    n->size = 0;
    n->cap = cap;
    n->negative = false;
}

void bignum_init(bignum *n) {
    bignum_init_cap(n, DEFAULT_CAPACITY);
}

void bignum_resize(bignum *n) {
    n->cap *= 2;
    n->data = realloc(n->data, n->cap);
}

void bignum_free(bignum *n) {
    n->size = 0;
    n->cap = 0;
    free(n->data);
    n->data = NULL;
}

void bignum_copy(bignum *dest, bignum *src) {
    dest->cap = src->cap;
    dest->size = src->size;
    free(dest->data);
    dest->data = malloc(src->cap * sizeof(unsigned char));
    memcpy(dest->data, src->data, src->cap);
}

bool bignum_is_zero(bignum *n) {
    return n->size == 0 || (n->size == 1 && n->data[0] == 0);
}

void bignum_dump(bignum *n) {
    printf("{%zu (%zu): [", n->size, n->cap);
    if (n->size == 0) {
        printf("]}\n");
        return;
    }
    size_t end = n->size - 1;
    //end = n->cap - 1;
    for (int i = 0; i < end; ++i) {
        printf("%d, ", n->data[i]);
    }
    printf("%d], [", n->data[end]);
    for (int i = 0; i < end; ++i) {
        printf("%x, ", n->data[i]);
    }
    printf("%x]}\n", n->data[end]);
}

void bignum_bprint(bignum *n) {
    char *nibbles[] = {
        "0000",
        "0001",
        "0010",
        "0011",
        "0100",
        "0101",
        "0110",
        "0111",
        "1000",
        "1001",
        "1010",
        "1011",
        "1100",
        "1101",
        "1110",
        "1111",
    };
    for (int i = n->size - 1; i >= 0; --i) {
        char upper = (n->data[i] & 0xf0) >> 4;
        char lower = n->data[i] & 0x0f;
        printf("%s %s", nibbles[upper], nibbles[lower]);
        if (i == 0) {
            printf("\n");
        } else {
            printf("  ");
        }
    }
}

int bignum_to_int(bignum *n) {
    unsigned int num =
           n->data[0]
        | (n->data[1] << 8)
        | (n->data[2] << 16)
        | (n->data[3] << 24);
    return num;
}

void bignum_print_int(bignum *n) {
    if (n->size > 4) {
        printf("<Inf>\n");
        return;
    }
    printf("%u\n", bignum_to_int(n));
}

void bignum_from_char(bignum *n, unsigned char s) {
    n->data[0] = s;
    n->size = 1;
}

void bignum_from_int(bignum *n, int s) {
    assert(n->cap >= 4);
    unsigned char b1 =  s & 0x000000ff;
    unsigned char b2 = (s & 0x0000ff00) >> 8;
    unsigned char b3 = (s & 0x00ff0000) >> 16;
    unsigned char b4 = (s & 0xff000000) >> 24;
    n->data[0] = b1;
    n->data[1] = b2;
    n->data[2] = b3;
    n->data[3] = b4;
    n->size = 4;
    n->negative = false;
    for (int i = 3; i >= 0 && n->data[i] == 0; --i) {
        n->size -= 1;
    }
}

void bignum_inc(bignum *n) {
    bool carry = false;
    int i = 0;
    do {
        if (i >= n->cap) {
            bignum_resize(n);
        }
        carry = n->data[i] + 1 > 255;
        n->data[i] += 1;
        ++i;
    } while (carry);
    if (i > n->size) {
        n->size = i;
    }
}

// a += b
void bignum_add(bignum *a, bignum *b) {
    bool carry = false;
    int this = 0;
    int i = 0;
    do {
        if (i >= a->cap) {
            bignum_resize(a);
        }
        this = b->data[i] + carry;
        if (i < a->size) { // avoid summing garbage
            this += a->data[i];
        }
        carry = this > 255;
        if (carry) {
            this -= 256;
        }
        a->data[i] = this;
        ++i;
    } while (i < b->size);
    while (i < a->size && carry) {
        ++a->data[i];
        carry = a->data[i] == 0;
        ++i;
    }
    if (carry) {
        a->data[i] = 1;
        ++a->size; // XXX: resize?
    }
    if (i > a->size) {
        a->size = i; // XXX: resize?
    }
}

// a -= b
// If a turns out negative, only a->negative is set to true, but otherwise
// result is undefined
void bignum_sub(bignum* a, bignum *b) {
    if (b->size > a->size) {
        a->negative = true;
        return;
    }
    int carry = 0;
    for (int i = 0; i < a->size; ++i) {
        int difference = a->data[i] - b->data[i] - carry;
        if (difference < 0) {
            if (i == a->size - 1) {
                a->negative = true;
            }
            difference += 256;
            carry = 1;
        } else {
            carry = 0;
        }
        a->data[i] = difference;
    }
    while (a->size > 0 && a->data[a->size - 1] == 0) {
        --a->size;
    }
    if (a->size == 0) {
        a->size = 1;
    }
}

// a *= b
void bignum_mul_int_silly_loop(bignum *a, unsigned int b) {
    bignum tmp;
    bignum_init(&tmp);
    bignum_copy(&tmp, a);
    --b;
    // XXX: this loop is probably quite inefficient, think of something better
    while (b > 0) {
        bignum_add(a, &tmp);
        --b;
    }
    bignum_free(&tmp);
}

// a *= b
void bignum_mul_int(bignum *a, unsigned int b) {
    bignum tmp;
    bignum_init(&tmp);
    unsigned char b_bytes[] = {
         b & 0x000000ff,
        (b & 0x0000ff00) >> 8,
        (b & 0x00ff0000) >> 16,
        (b & 0xff000000) >> 24,
    };
    int b_size = 3;
    while (b_size > 1 && b_bytes[b_size] == 0) --b_size;
    for (int bi = 0; bi < b_size; ++bi) {
        int carry = 0;
        for (int ai = 0; ai < a->size; ++ai) {
            int prod = carry + a->data[ai] * b_bytes[bi];
            carry = prod / 256;
            tmp.data[ai + bi] = prod % 256;
        }
        tmp.data[bi + a->size] = carry;
    }
    tmp.size = a->size + b_size;
    while (tmp.size > 0 && tmp.data[tmp.size - 1] == 0) {
        --tmp.size;
    }
    if (tmp.size == 0) {
        tmp.size = 1;
    }
    bignum_copy(a, &tmp);
    bignum_free(&tmp);
}

bool bignum_lte(bignum *a, bignum *b) {
    if (a->size < b->size) {
        return true;
    } else if (a->size > b->size) {
        return false;
    }
    for (int i = a->size - 1; i >= 0; --i) {
        if (a->data[i] > b->data[i]) {
            return false;
        }
    }
    return true;
}

#define LUTSZ 10000
#define NDIVISORS 11
unsigned int lut[NDIVISORS][LUTSZ][2];
void init_div_mod_int_lut() {
    for (int b = 2; b < NDIVISORS; ++b) {
        for (unsigned int temp = 0; temp < LUTSZ; ++temp) {
            lut[b][temp][0] = temp / b;
            lut[b][temp][1] = lut[b][temp][0] * b;
        }
    }
}

// Algorithm taken from:
// http://stackoverflow.com/questions/10522379/bignum-division-with-an-unsigned-8-bit-integer-c
void bignum_div_mod_int(bignum *a, int b, int *remainder) {
    int i = a->size;
    unsigned int temp = 0;
    assert(b < NDIVISORS);
    while (i > 0) {
        --i;
        temp <<= 8;
        temp |= a->data[i];
        assert(temp < LUTSZ);
        a->data[i] = lut[b][temp][0];
        temp -= lut[b][temp][1];
    }
    if (remainder) {
        *remainder = temp;
    }
    while (a->size > 0 && a->data[a->size - 1] == 0) {
        --a->size;
    }
    if (a->size == 0) {
        a->size = 1;
    }
}

// remainder is optional
void bignum_div_mod(bignum *a, bignum *b, bignum *remainder) {
    int i = a->size;
    unsigned int temp = 0;
    int bb = bignum_to_int(b);
    while (i > 0) {
        --i;
        temp <<= 8;
        temp |= a->data[i];
        a->data[i] = temp / bb;
        temp -= a->data[i] * bb;
    }
    if (remainder) {
        bignum_from_int(remainder, temp);
    }
    while (a->size > 0 && a->data[a->size - 1] == 0) {
        --a->size;
    }
    if (a->size == 0) {
        a->size = 1;
    }
}

// a /= b
void bignum_div(bignum *a, bignum *b) {
    bignum_div_mod(a, b, NULL);
}

// a %= b
void bignum_mod(bignum *a, bignum *b) {
    bignum tmp;
    bignum_init(&tmp);
    bignum_div_mod(a, b, &tmp);
    bignum_copy(a, &tmp);
    bignum_free(&tmp);
}

void bignum_init_base_convert(size_t size, int base) {
    bignum multiplier;
    bignum_init(&multiplier);
    bignum_from_int(&multiplier, 1);
    mul_lut = malloc(size * sizeof(bignum));
    mul_lut_size = size;
    for (int i = 0; i < size; ++i) {
        bignum_init(&mul_lut[i]);
        bignum_copy(&mul_lut[i], &multiplier);
        bignum_mul_int(&multiplier, base);
    }
    bignum_free(&multiplier);
}

void bignum_free_base_convert_lut() {
    for (int i = 0; i < mul_lut_size; ++i) {
        bignum_free(&mul_lut[i]);
    }
    free(mul_lut);
    mul_lut = NULL;
    mul_lut_size = 0;
}

// assign n from s, treat s as being in base 'base'
void bignum_base_convert(bignum *n, bignum* s) {
    bignum_from_int(n, 0);
    int m = 0;
    for (int i = 0; i < s->size; ++i) {
        for (unsigned char mask = 1; mask != 0; mask <<= 1) {
            if (s->data[i] & mask) {
                bignum_add(n, &mul_lut[m]);
            }
            ++m;
        }
    }
}

/*
Examples:
82000 (base 3) = 11011111001 =
1*3**0 + 0*3**1 + 0*3**2 + 1*3**3 +3**4 +3**5 +3**6 +3**7 +0*3**8 +3**9 +3**10

82000 (base 5) = 10111000 =
0*5**0 + 0*5**1 + 0*5**2 + 1*5**3 + 1*5**4 + 1*5**5 + 0*5**6 + 1*5**7
*/
void bignum_from_string_binary(bignum *n, char const* s, size_t base) {
    char const *end_of_s = s + strlen(s) - 1;
    bignum_from_int(n, 0);
    bignum multiplier;
    bignum_init(&multiplier);
    bignum_from_int(&multiplier, 1);
    while (end_of_s != s) {
        assert((*end_of_s == '1') || (*end_of_s == '0'));
        if (*end_of_s == '1') {
            bignum_add(n, &multiplier);
        }
        bignum_mul_int(&multiplier, base);
        --end_of_s;
    }
    bignum_add(n, &multiplier);
    bignum_free(&multiplier);
}
