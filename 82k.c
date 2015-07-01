#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define DEFAULT_CAPACITY 128

typedef struct _bignum {
    unsigned char *data;
    size_t        size;
    size_t        cap;
} bignum;

void bignum_init_cap(bignum *n, size_t cap) {
    n->data = malloc(cap * sizeof(unsigned char));
    n->size = 0;
    n->cap = cap;
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

void bignum_print_int(bignum *n) {
    if (n->size > 4) {
        printf("<Inf>\n");
        return;
    }
    unsigned int num =
           n->data[0]
        | (n->data[1] << 8)
        | (n->data[2] << 16)
        | (n->data[3] << 24);
    printf("%u\n", num);
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

char* limited_precision_base_conv(long int number, size_t base) {
    char base_digits[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    static char buff[64] = "\0";
    int converted_number[64];
    int next_digit, index=0;

    // convert to the indicated base
    while (number != 0) {
        converted_number[index] = number % base;
        number = number / base;
        ++index;
    }

    // now print the result in reverse order
    --index;  // back up to last entry in the array
    int i = 0;
    for (; index >= 0; --index, ++i) {
        buff[i] = base_digits[converted_number[index]];
    }
    buff[i] = '\0';
    return buff;
}

void test_bignum_from_string_binary() {
    bignum fs;
    bignum_init(&fs);
    bignum_from_string_binary(&fs, "10100000001010000", 2);
    assert(fs.size == 3);
    assert(fs.data[0] == 80);
    assert(fs.data[1] == 64);
    assert(fs.data[2] == 1);
    bignum_from_string_binary(&fs, "11011111001", 3);
    assert(fs.size == 3);
    assert(fs.data[0] == 80);
    assert(fs.data[1] == 64);
    assert(fs.data[2] == 1);
    bignum_from_string_binary(&fs, "110001100", 4);
    assert(fs.size == 3);
    assert(fs.data[0] == 80);
    assert(fs.data[1] == 64);
    assert(fs.data[2] == 1);
    bignum_from_string_binary(&fs, "10111000", 5);
    assert(fs.size == 3);
    assert(fs.data[0] == 80);
    assert(fs.data[1] == 64);
    assert(fs.data[2] == 1);
    bignum_free(&fs);
}

void test() {
    bignum n;
    bignum_init(&n);
    bignum_dump(&n);
    int arr[] = {42, 255, 256, 257, 258, 65535+17};
    for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); ++i) {
        bignum_from_int(&n, arr[i]);
        printf("%d = ", arr[i]);
        bignum_dump(&n);
    }
    bignum_from_int(&n, 1);
    bignum_inc(&n);
    assert(n.size == 1);
    assert(n.data[0] == 2);
    assert(n.data[1] == 0);
    bignum_inc(&n);
    assert(n.size == 1);
    assert(n.data[0] == 3);
    assert(n.data[1] == 0);
    bignum_from_int(&n, 255);
    bignum_inc(&n);
    assert(n.size == 2);
    assert(n.data[0] == 0);
    assert(n.data[1] == 1);
    bignum_inc(&n);
    assert(n.size == 2);
    assert(n.data[0] == 1);
    assert(n.data[1] == 1);
    bignum_from_int(&n, 65536);
    for (int i = 0; i < 22; ++i) {
        bignum_inc(&n);
        bignum_bprint(&n);
    }
    bignum_from_int(&n, 82000);
    bignum_bprint(&n);
    bignum_free(&n);
    int m = 82000;
    for (int b = 2; b < 7; ++b) {
        printf("%d (base %d) = %s\n", m, b, limited_precision_base_conv(m, b));
    }
    bignum small;
    bignum_init_cap(&small, 1);
    bignum_from_char(&small, 254);
    assert(small.size == 1);
    assert(small.cap == 1);
    bignum_inc(&small);
    assert(small.size == 1);
    assert(small.cap == 1);
    bignum_inc(&small);
    assert(small.size == 2);
    assert(small.cap == 2);
    bignum_free(&small);
    bignum a, b;
    bignum_init(&a);
    bignum_init(&b);
    bignum_from_int(&a, 82);
    bignum_from_int(&b, 250);
    bignum_dump(&a);
    bignum_dump(&b);
    bignum_add(&a, &b);
    bignum_dump(&a);
    assert(a.size == 2);
    assert(a.data[0] == 76);
    assert(a.data[1] == 1);
    bignum_from_int(&a, 82000);
    bignum_from_int(&b, 150000);
    bignum_dump(&a);
    bignum_dump(&b);
    bignum_add(&a, &b);
    bignum_dump(&a);
    assert(a.size == 3);
    assert(a.data[0] == 64);
    assert(a.data[1] == 138);
    assert(a.data[2] == 3);
    bignum_from_int(&a, 15);
    bignum_from_int(&b, 232000);
    bignum_dump(&a);
    bignum_dump(&b);
    bignum_add(&a, &b);
    bignum_dump(&a);
    assert(a.size == 3);
    assert(a.data[0] == 79);
    assert(a.data[1] == 138);
    assert(a.data[2] == 3);
    bignum_free(&a);
    bignum_free(&b);
    bignum x;
    bignum_init(&x);
    bignum_from_int(&x, 17);
    bignum_mul_int(&x, 3);
    assert(x.size == 1);
    assert(x.data[0] == 51);
    assert(x.data[1] == 0);
    bignum_free(&x);
    test_bignum_from_string_binary();
    bignum bb;
    bignum_init(&bb);
    bignum_from_int(&bb, 82000);
    bignum_print_int(&bb);
    bignum_from_int(&bb, 149327);
    bignum_print_int(&bb);
    bignum_from_int(&bb, 4294967295);
    bignum_print_int(&bb);
    bignum_inc(&bb);
    bignum_print_int(&bb);
    bignum_free(&bb);
    bignum bn;
    bignum bn2;
    bignum_init(&bn);
    bignum_init(&bn2);
    bignum_from_int(&bn, 1047);
    bignum_from_bignum(&bn2, &bn, 2);
    assert(bn2.size == 2);
    assert(bn2.data[0] == 23);
    assert(bn2.data[1] == 4);
    bignum_free(&bn);
    bignum_free(&bn2);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && 0 == strcmp(argv[1], "-t")) {
        test();
    }
    return 0;
}
