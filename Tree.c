#ifndef TREE_C
#define TREE_C

#include <errno.h>
#include <regex.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>


#include "Tree.h"
#include "HashMap.h"

struct Tree
{
    HashMap *tree_map;
};

void* get_tree_map(Tree *tree)
{
    HashMap* map = tree->tree_map;
    return map;
}

typedef struct PathGetter
{
    char* path;
    ReadWrite *guards[2047];
    HashMap *map;
    size_t guard_write_pos;

}PathGetter;

static PathGetter* pg_create(char* _path, HashMap *_map)
{
    PathGetter* pg = malloc(sizeof(PathGetter));

    if(!pg)
        return NULL;

    pg->map = _map;
    
    pg->path = _path;
    pg->guard_write_pos = 0;

    return pg;
}

void tree_free(Tree* tree)
{
    hmap_free(tree->tree_map);
    free(tree);
}

static int pg_free(PathGetter * pg)
{
    int err;

    for(int i = pg->guard_write_pos - 1; i >= 0; i--)
    {
        if((err = rw_action_wrapper(pg->guards[i], END_READ)))
            return err;
    }

    free(pg);
    return 0;
}

static HashMap* pg_get(PathGetter * pg)
{
	char delim[] = "/";
    char *path = strtok(pg->path, delim);

    HashMap *tmp = pg->map;
    Pair *p;

    while(path != NULL)
	{
        p = hmap_get(tmp, path, START_READ);
        if(!p || !p->value)
        {
            pg_free(pg);
            if(p)
            {
                rw_action_wrapper(p->bucket_guard, END_READ);
                free(p);
            }
            return NULL;
        }
        else
        {
            pg->guards[pg->guard_write_pos++] = p->bucket_guard;
            tmp = p->value;
        }
		path = strtok(NULL, delim);
	}

    return tmp;
}

Tree* tree_new()
{
    Tree *t = malloc(sizeof(Tree));
    if (!t)
        return NULL;
    
    t->tree_map = hmap_new();

    return t;
}

int tree_create(Tree* tree, const char* path)
{
    if(!is_path_valid(path))
        return EINVAL;

    char *dirc, *basec, *bname, *dname;

    dirc = strdup(path);
    basec = strdup(path);
    dname = dirname(dirc);
    bname = basename(basec);

    bool swap_bd_name=false;;
    if(strcmp(dname, "/") == 0)
        swap_bd_name = true;

    HashMap *map = tree->tree_map;
    PathGetter *pg = NULL;

    if(!swap_bd_name)
    {
        pg = pg_create(dname, map);
        map  = pg_get(pg);
    }

    if(!map)
        return errno;

    HashMap *new_folder = hmap_new();
    int err = hmap_insert(map, bname, new_folder, false);
    if(err)
        hmap_free(new_folder);

    if(pg)
        pg_free(pg);

    free(dirc);
    free(basec);

    return err;
}

int tree_remove(Tree* tree, const char* path)
{
    if(!is_path_valid(path))
        return EINVAL;

    char *dirc, *basec, *bname, *dname;

    dirc = strdup(path);
    basec = strdup(path);
    dname = dirname(dirc);
    bname = basename(basec);

    bool swap_bd_name=false;;
    if(strcmp(dname, "/") == 0)
        swap_bd_name = true;

    HashMap *map = tree->tree_map;
    PathGetter *pg = NULL;

    if(!swap_bd_name)
    {
        pg = pg_create(bname, map);
        map  = pg_get(pg);
    }

    if(!map)
        return errno;

    Pair *p = hmap_remove(map, bname);

    int err = 0;

    if (p)
    {
        hmap_free(p->value);
        err = rw_action_wrapper(p->bucket_guard, END_WRITE);
        free(p);
    }
    else
        err = errno;
    
    if(pg)
        pg_free(pg);

    free(dirc);
    free(basec);

    return err;
}

#endif