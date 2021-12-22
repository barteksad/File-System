#ifndef TREE_C
#define TREE_C

#include <errno.h>
#include <regex.h>
#include <errno.h>
#include <stdlib.h>

#include "Tree.h"
#include "HashMap.h"


struct Tree
{
    HashMap *tree_map;
};

Tree* tree_new()
{
    Tree *t = malloc(sizeof(Tree));
    if (!t)
        return NULL;
    
    t->tree_map = hmap_new();
}

#endif