#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NDEBUG
#undef NDEBUG
#endif

#include "../err.h"
#include "seq_ok1.c"
#include "seq_ok2.c"
#include "seq_ok3.c"
#include "con_ok1.c"
#include "con_ok2.c"
#include "con_ok3.c"
#include "path_ok1.c"

int main(void) {
    if (seq_ok1() != 0)
        syserr("error seq_ok1!");
    if (seq_ok2() != 0)
        syserr("error seq_ok2!");
    if (con_ok1() != 0)
        syserr("error con_ok1!");
    if (path_ok1() != 0)
        syserr("error path_ok1!");
    if (seq_ok3() != 0)
        syserr("error seq_ok3!");
    if (con_ok2() != 0)
        syserr("error con_ok1!");
    if (con_ok3() != 0)
        syserr("error con_ok3!");
    printf("\n\nTEST PASSED!\n\n");
    return 0;
}