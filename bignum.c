#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "bignum.h"

void bignum_init_cap(bignum *n, size_t cap) {
    n->data = malloc(cap * sizeof(unsigned char));
    n->size = 0;
    n->cap = cap;
    n->negative = false;
    for (int i = 0; i < cap; ++i) {
        n->data[i] = 0;
    }
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
    for (int i = 0; i < n->size - 1; ++i) {
        printf("%d, ", n->data[i]);
    }
    printf("%d]}\n", n->data[n->size - 1]);
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
        this = a->data[i] + b->data[i] + carry;
        carry = this > 255;
        if (carry) {
            this -= 256;
        }
        a->data[i] = this;
        ++i;
    } while (carry > 0 || i < b->size);
    if (i > a->size) {
        a->size = i;
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
void bignum_mul_int(bignum *a, unsigned int b) {
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

void bignum_div_mod_int(bignum *a, int b, int *remainder) {
    bignum bb;
    bignum_init(&bb);
    bignum_from_int(&bb, b);
    if (!remainder) {
        bignum_div_mod(a, &bb, NULL);
    } else {
        bignum rr;
        bignum_init(&rr);
        bignum_div_mod(a, &bb, &rr);
        *remainder = bignum_to_int(&rr);
        bignum_free(&rr);
    }
    bignum_free(&bb);
}

// remainder is optional
// 7 / 2
// 0;
// while (2 < 7)
//   1; 5
//   2; 3
//   3; 1
void bignum_div_mod(bignum *a, bignum *b, bignum *remainder) {
    bignum result;
    bignum_init(&result);
    while (bignum_lte(b, a)) {
        bignum_inc(&result);
        bignum_sub(a, b);
    }
    if (remainder) {
        bignum_copy(remainder, a);
    }
    bignum_copy(a, &result);
    bignum_free(&result);
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

// assign n from s, treat s as being in base 'base'
void bignum_from_bignum(bignum *n, bignum* s, size_t base) {
    bignum multiplier;
    bignum_from_int(n, 0);
    bignum_init(&multiplier);
    bignum_from_int(&multiplier, 1);
    for (int i = 0; i < s->size; ++i) {
        for (unsigned char mask = 1; mask != 0; mask <<= 1) {
            if (s->data[i] & mask) {
                bignum_add(n, &multiplier);
            }
            bignum_mul_int(&multiplier, base);
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
