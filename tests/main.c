#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../err.h"
#include "seq_ok1.c"
#include "seq_ok2.c"

int main(void) {
    if (seq_ok1() != 0)
        syserr("error seq_ok1!");
    if (seq_ok2() != 0)
        syserr("error seq_ok2!");

    printf("\n\nTEST PASSED!\n\n");
    return 0;
}