#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

char* unlimited_precision_base_conv(bignum *number, size_t base) {
    static char base_digits[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    bignum work;
    bignum_init(&work);
    bignum_copy(&work, number);
    // allocate enough to print binary:
    char *buff = malloc((work.size * 8 + 1) * sizeof(char));
    int *converted_number = malloc((work.size * 8) * sizeof(int));
    int digit = 0;
    // convert to the indicated base
    while (!bignum_is_zero(&work)) {
        bignum_div_mod_int(&work, base, &(converted_number[digit]));
        ++digit;
    }
    // now print the result in reverse order
    --digit;  // back up to last entry in the array
    int i = 0;
    while (digit >= 0) {
        buff[i] = base_digits[converted_number[digit]];
        --digit;
        ++i;
    }
    buff[i] = '\0';
    bignum_free(&work);
    free(converted_number);
    return buff;
}

bool check_base(bignum *n, int base) {
    bignum work;
    bignum_init(&work);
    bignum_copy(&work, n);
    int converted_digit = 0;
    while (!bignum_is_zero(&work)) {
        bignum_div_mod_int(&work, base, &converted_digit);
        if (converted_digit > 1) {
            bignum_free(&work);
            return false;
        }
    }
    bignum_free(&work);
    return true;
}

void search() {
    bignum n5;
    bignum n;
    bignum tmp;
    bignum_init(&n5);
    bignum_init(&n);
    bignum_init(&tmp);
    int base_cap = 4;
    bignum_from_int(&n5, 1);
    int last_size = n5.size;
    bignum_init_base_convert(40*8, 5);
    while (n5.size < 4) {
        bignum_base_convert(&n, &n5);
        int base = base_cap;
        while (base > 2) {
            if (!check_base(&n, base)) {
                break;
            }
            --base;
        }
        if (base == 2) {
            bignum_base_convert(&tmp, &n5);
            printf("covers all bases from 2 to %d: ", base_cap + 1);
            bignum_print_int(&tmp);
        }
        bignum_inc(&n5);
        if (n5.size > last_size) {
            printf("b5: ");
            bignum_dump(&n5);
            bignum_base_convert(&tmp, &n5);
            char* b = unlimited_precision_base_conv(&tmp, 10);
            printf("b10: %s\n", b);
            free(b);
            last_size = n5.size;
        }
    }
    bignum_free_base_convert_lut();
    bignum_free(&n5);
    bignum_free(&n);
    bignum_free(&tmp);
}

int main(int argc, char *argv[]) {
    init_div_mod_int_lut();
    if (argc > 1 && 0 == strcmp(argv[1], "-e")) {
        eyeball_tests();
        test();
        return 0;
    }
    if (argc > 1 && 0 == strcmp(argv[1], "-t")) {
        test();
        return 0;
    }
    search();
    return 0;
}

/*
82000 (base 2) = 10100000001010000
82000 (base 3) = 11011111001
82000 (base 4) = 110001100
82000 (base 5) = 10111000
82000 (base 6) = 1431344


covers all bases from 2 to 4:
1
4
81
84
85
256
273
324
325
336
337
1089
1092
1093
20496
20497
20736
20737
20740
65620
65856
65857
81921
81984
81985
82000
86032
86277
86292
86293
86356

real    15m11.751s
user    14m48.010s
sys     0m0.440s
*/
