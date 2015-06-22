#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef struct _bignum {
    unsigned char *data;
    size_t        size;
    size_t        cap;
} bignum;

void bignum_init(bignum *n) {
    int cap = 128;
    n->data = (unsigned char*) malloc(cap * sizeof(unsigned char));
    n->size = 0;
    n->cap = cap;
    for (int i = 0; i < cap; ++i) {
        n->data[i] = 0;
    }
}

void bignum_free(bignum *n) {
    n->size = 0;
    n->cap = 0;
    free(n->data);
    n->data = NULL;
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

void bignum_from_int(bignum *n, int s) {
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
        carry = n->data[i] + 1 > 255;
        n->data[i] += 1;
        ++i;
    } while (carry);
    if (i > n->size) {
        n->size = i;
    }
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
}

int main(int argc, char *argv[]) {
    if (argc > 1 && 0 == strcmp(argv[1], "-t")) {
        test();
    }
    return 0;
}
