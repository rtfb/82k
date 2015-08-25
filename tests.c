#include <assert.h>

#include "bignum.h"

void test_bignum_lte() {
    bignum a, b;
    bignum_init(&a);
    bignum_init(&b);
    bignum_from_int(&a, 0);
    bignum_from_int(&b, 0);
    assert(bignum_lte(&a, &b) == true);
    bignum_from_int(&b, 1);
    assert(bignum_lte(&a, &b) == true);
    bignum_from_int(&a, 2);
    bignum_from_int(&b, 2);
    assert(bignum_lte(&a, &b) == true);
    bignum_from_int(&a, 3);
    assert(bignum_lte(&a, &b) == false);
    bignum_free(&a);
    bignum_free(&b);
}

void test_bignum_sub() {
    bignum a, b;
    bignum_init(&a);
    bignum_init(&b);
    bignum_from_int(&a, 17);
    bignum_from_int(&b, 13);
    bignum_sub(&a, &b);
    assert(a.data[0] == 4);
    assert(a.negative == false);
    bignum_from_int(&a, 13);
    bignum_from_int(&b, 17);
    bignum_sub(&a, &b);
    assert(a.negative == true);
    bignum_from_int(&a, 13);
    bignum_from_int(&b, 13);
    bignum_sub(&a, &b);
    assert(a.negative == false);
    assert(a.data[0] == 0);
    assert(a.size == 1);
    // test size shrink
    bignum_from_int(&a, 13987654);
    bignum_from_int(&b, 13987651);
    bignum_sub(&a, &b);
    assert(a.negative == false);
    assert(a.data[0] == 3);
    assert(a.size == 1);
    //
    bignum_from_int(&a, 256);
    bignum_from_int(&b, 1);
    bignum_sub(&a, &b);
    assert(a.negative == false);
    assert(a.data[0] == 255);
    assert(a.size == 1);
    //
    bignum_from_int(&a, 65536);
    bignum_from_int(&b, 1);
    bignum_sub(&a, &b);
    assert(a.negative == false);
    assert(a.data[0] == 255);
    assert(a.data[1] == 255);
    assert(a.data[2] == 0);
    assert(a.size == 2);
    bignum_free(&a);
    bignum_free(&b);
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

void eyeball_tests() {
    bignum n;
    bignum_init(&n);
    bignum_dump(&n);
    int arr[] = {42, 255, 256, 257, 258, 65535+17};
    for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); ++i) {
        bignum_from_int(&n, arr[i]);
        printf("%d = ", arr[i]);
        bignum_dump(&n);
    }
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
}

void test_bignum_div_mod() {
    bignum n, b;
    bignum_init(&n);
    bignum_init(&b);
    bignum_from_int(&n, 256);
    bignum_from_int(&b, 2);
    bignum_div_mod(&n, &b, NULL);
    assert(n.data[0] == 128);
    assert(n.size == 1);
    bignum_free(&n);
    bignum_free(&b);
}

void test_bignum_div_mod_int() {
    bignum n;
    bignum_init(&n);
    bignum_from_int(&n, 256);
    bignum_div_mod_int(&n, 2, NULL);
    assert(n.data[0] == 128);
    assert(n.size == 1);
    bignum_from_int(&n, 8);
    bignum_div_mod_int(&n, 2, NULL);
    assert(n.data[0] == 4);
    bignum_div_mod_int(&n, 2, NULL);
    assert(n.data[0] == 2);
    bignum_div_mod_int(&n, 2, NULL);
    assert(n.data[0] == 1);
    bignum_div_mod_int(&n, 2, NULL);
    assert(n.data[0] == 0);
    bignum_from_int(&n, 82000);
    int remainder = 0;
    bignum_div_mod_int(&n, 2, &remainder);
    assert(bignum_to_int(&n) == 41000);
    bignum_free(&n);
}

void test_bignum_is_zero() {
    bignum n;
    bignum_init(&n);
    assert(bignum_is_zero(&n) == true);
    bignum_from_int(&n, 1);
    assert(bignum_is_zero(&n) == false);
    bignum_from_int(&n, 256);
    assert(bignum_is_zero(&n) == false);
    bignum_from_int(&n, 82000);
    assert(bignum_is_zero(&n) == false);
    bignum_free(&n);
}

void test_bignum_from_bignum() {
    bignum bn;
    bignum bn2;
    bignum_init(&bn);
    bignum_init(&bn2);
    bignum_init_base_convert(40, 2);
    bignum_from_int(&bn, 1047);
    bignum_base_convert(&bn2, &bn);
    assert(bn2.size == 2);
    assert(bn2.data[0] == 23);
    assert(bn2.data[1] == 4);
    bignum_free_base_convert_lut();
    //
    bignum_init_base_convert(40, 5);
    bignum_from_int(&bn, 1);
    bignum_base_convert(&bn2, &bn);
    assert(bn2.size == 1);
    assert(bn2.data[0] == 1);
    bignum_from_int(&bn, 2); // binary 10
    bignum_base_convert(&bn2, &bn);
    assert(bn2.size == 1);
    assert(bn2.data[0] == 5);
    bignum_from_int(&bn, 3); // binary 11
    bignum_base_convert(&bn2, &bn);
    assert(bn2.size == 1);
    assert(bn2.data[0] == 6);
    bignum_from_int(&bn, 5); // binary 101
    bignum_base_convert(&bn2, &bn);
    assert(bn2.size == 1);
    assert(bn2.data[0] == 26);
    bignum_free(&bn);
    bignum_free(&bn2);
    bignum_free_base_convert_lut();
}

void test_bignum_mul_int() {
    bignum x;
    bignum_init(&x);
    bignum_from_int(&x, 17);
    bignum_mul_int(&x, 3);
    assert(x.size == 1);
    assert(x.data[0] == 51);
    assert(x.data[1] == 0);
    //
    bignum_from_int(&x, 1);
    for (int i = 0; i < 16; ++i) {
        bignum_mul_int(&x, 5);
    }
    assert(x.size == 5);
    assert(x.data[0] == 0xc1);
    assert(x.data[1] == 0x6f);
    assert(x.data[2] == 0xf2);
    assert(x.data[3] == 0x86);
    assert(x.data[4] == 0x23);
    assert(x.data[5] == 0);
    //
    bignum_from_int(&x, 1220703125);
    bignum_mul_int(&x, 5);
    assert(x.size == 5);
    assert(x.data[0] == 0xe9);
    assert(x.data[1] == 0x41);
    assert(x.data[2] == 0xcc);
    assert(x.data[3] == 0x6b);
    assert(x.data[4] == 0x1);
    assert(x.data[5] == 0);
    bignum_free(&x);
}

void test_bignum_add() {
    bignum a, b;
    bignum_init(&a);
    bignum_init(&b);
    bignum_from_int(&a, 82);
    bignum_from_int(&b, 250);
    bignum_add(&a, &b);
    assert(a.size == 2);
    assert(a.data[0] == 76);
    assert(a.data[1] == 1);
    bignum_from_int(&a, 82000);
    bignum_from_int(&b, 150000);
    bignum_add(&a, &b);
    assert(a.size == 3);
    assert(a.data[0] == 64);
    assert(a.data[1] == 138);
    assert(a.data[2] == 3);
    bignum_from_int(&a, 15);
    bignum_from_int(&b, 232000);
    bignum_add(&a, &b);
    assert(a.size == 3);
    assert(a.data[0] == 79);
    assert(a.data[1] == 138);
    assert(a.data[2] == 3);
    bignum_from_int(&a, 120);
    bignum_from_int(&b, 140);
    a.data[1] = 7; // make sure there's garbage after data
    bignum_add(&a, &b);
    assert(a.size == 2);
    assert(a.data[0] == 4);
    assert(a.data[1] == 1);
    //
    bignum_from_int(&a, 500);
    bignum_from_int(&b, 125);
    bignum_add(&a, &b);
    assert(a.size == 2);
    assert(a.data[0] == 113);
    assert(a.data[1] == 2);
    bignum_free(&a);
    bignum_free(&b);
}

void test() {
    bignum n;
    bignum_init(&n);
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
    bignum_free(&n);
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
    test_bignum_add();
    test_bignum_mul_int();
    test_bignum_from_string_binary();
    test_bignum_from_bignum();
    test_bignum_lte();
    test_bignum_sub();
    test_bignum_div_mod();
    test_bignum_div_mod_int();
    test_bignum_is_zero();
    printf("Tests OK\n");
}
