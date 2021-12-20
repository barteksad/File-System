#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../err.h"
#include "seq_ok1.c"

int main(void) {
    if (seq_ok1() != 0)
        syserr("error seq_ok1!");

    printf("\n\nTEST PASSED!\n\n");
    return 0;
}