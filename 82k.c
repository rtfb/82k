#include <string.h>

#include "bignum.h"
#include "tests.h"

char* limited_precision_base_conv(long int number, size_t base) {
    static char base_digits[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    static char buff[64] = "\0";
    int converted_number[64];
    int index = 0;

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

bool check_base(bignum *n, int base) {
    // TODO
    return false;
}

void cover_all_bases() {
    bignum n;
    bignum_init(&n);
    int bn_as_int = 1;
    bignum_from_int(&n, bn_as_int);
    while (bn_as_int < 100000) {
        int base = 5;
        for (; base > 2; ++base) {
            if (!check_base(&n, base)) {
                break;
            }
        }
        if (base == 2) {
            printf("covers all bases from 2 to 5: ");
            bignum_print_int(&n);
        }
        bignum_inc(&n);
        bn_as_int = bignum_to_int(&n);
    }
    bignum_free(&n);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && 0 == strcmp(argv[1], "-e")) {
        eyeball_tests();
        test();
    }
    if (argc > 1 && 0 == strcmp(argv[1], "-t")) {
        test();
        cover_all_bases();
    }
    return 0;
}
