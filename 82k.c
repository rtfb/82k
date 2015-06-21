#include <stdio.h>
#include <malloc.h>
#include <string.h>

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
    printf("{%d (%d): [", n->size, n->cap);
    if (n->size == 0) {
        printf("]}\n");
        return;
    }
    for (int i = 0; i < n->size - 1; ++i) {
        printf("%d, ", n->data[i]);
    }
    printf("%d]}\n", n->data[n->size - 1]);
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
    bignum_free(&n);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && 0 == strcmp(argv[1], "-t")) {
        test();
    }
    return 0;
}
