#include "../HashMap.h"
#include "../Tree.h"

#include "test_utils.c"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

int seq_ok3()
{
    for (int i = 0; i < 200; i++)
    {

        Tree *t = tree_new();
        char *l = tree_list(t, "/");
        free(l);

        tree_create(t, "/a/");
        tree_create(t, "/a/c/");
        tree_move(t, "/a/", "/b/");

        l = tree_list(t, "/");
        assert(strcmp(l, "b") == 0);
        free(l);

        l = tree_list(t, "/b/");
        assert(strcmp(l, "c") == 0);
        free(l);

        tree_create(t, "/a/");
        tree_create(t, "/a/c/");
        tree_create(t, "/a/d/");
        tree_create(t, "/a/g/");

        tree_move(t, "/a/c/", "/a/e/");

        l = tree_list(t, "/");
        assert(strcmp(l, "a,b") == 0);
        free(l);

        l = tree_list(t, "/a/");
        assert(strcmp(l, "d,e,g") == 0);
        free(l);

        tree_create(t, "/a/e/xxx/");

        l = tree_list(t, "/a/e/");
        assert(strcmp(l, "xxx") == 0);
        free(l);

        tree_create(t, "/a/e/xxx/d/");
        tree_create(t, "/a/e/xxx/d/e/");
        tree_create(t, "/a/e/xxx/d/e/f/");
        tree_create(t, "/a/e/xxx/d/e/f/g/");

        tree_move(t, "/a/e/xxx/d/e/f/g/", "/a/e/xxx/d/e/f/h/");

        l = tree_list(t, "/a/e/xxx/d/e/f/");
        assert(strcmp(l, "h") == 0);
        free(l);

        tree_create(t, "/x/");
        tree_create(t, "/x/y/");
        tree_create(t, "/x/y/z/");

        tree_create(t, "/j/");
        tree_create(t, "/j/k/");
        tree_create(t, "/j/k/l/");

        tree_move(t, "/x/y/z/", "/j/k/l/x/");

        l = tree_list(t, "/x/y/");
        assert(strcmp(l, "") == 0);
        free(l);

        l = tree_list(t, "/j/k/l/");
        assert(strcmp(l, "x") == 0);
        free(l);

        tree_free(t);
        t = tree_new();

        tree_create(t, "/a/");
        tree_create(t, "/a/b/");
        tree_create(t, "/a/b/c/");
        tree_create(t, "/a/b/c/d/");
        tree_create(t, "/a/b/c/d/e/");
        tree_create(t, "/a/b/c/d/e/f/");
        tree_create(t, "/a/b/c/d/e/f/a/");
        tree_create(t, "/a/b/c/d/e/f/a/b/");

        tree_create(t, "/a/b/c/d/e/f/x/");
        tree_create(t, "/a/b/c/d/e/f/x/y/");

        tree_move(t, "/a/b/c/d/e/f/a/b/", "/a/b/c/d/e/f/x/y/z/");

        l = tree_list(t, "/a/b/c/d/e/f/a/");
        assert(strcmp(l, "") == 0);
        free(l);

        l = tree_list(t, "/a/b/c/d/e/f/x/y/");
        assert(strcmp(l, "z") == 0);
        free(l);

        l = tree_list(t, "/a/b/c/d/e/f/");
        assert(strcmp(l, "a,x") == 0);
        free(l);

        tree_move(t, "/a/b/c/d/e/f/x/y/z/", "/a/b/c/d/e/f/x/y/y/");

        l = tree_list(t, "/a/b/c/d/e/f/x/y/");
        assert(strcmp(l, "y") == 0);
        free(l);

        tree_free(t);
    }
    return 0;
}