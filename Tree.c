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
#include "Utils.h"
#include "err.h"
#include "path_utils.h"

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

// error codes
#define MALLOC_FAILED -1
#define MOVE_SUBTREES -2

struct Tree
{
    HashMap *tree_map;
    ReadWrite rw;
};

typedef struct PathGetter
{
    char *path;
    ReadWrite *guards[2050];
    Tree *tree;
    size_t guard_write_pos;
    AccessType first_acces;
    AccessType last_access;
    bool last_used;

} PathGetter;

static PathGetter *pg_create(const char *_path, Tree *_tree)
{
    PathGetter *pg = NULL;
    pg = (PathGetter *)malloc(sizeof(PathGetter));

    if (!pg)
    {
        errno = MALLOC_FAILED;
        return NULL;
    }

    pg->tree = _tree;
    pg->path = NULL;
    pg->path = strdup(_path);
    if (!pg->path)
    {
        errno = MALLOC_FAILED;
        free(pg);
        return NULL;
    }

    pg->guard_write_pos = 0;
    pg->last_used = false;

    return pg;
}

void tree_free(Tree *tree)
{
    const char *key = NULL;
    void *value = NULL;
    HashMapIterator it = hmap_iterator(tree->tree_map);
    while (hmap_next(tree->tree_map, &it, &key, &value))
    {
        tree_free(value);
    }
    hmap_free(tree->tree_map);
    free(tree);
}

static int pg_free(PathGetter *pg)
{
    if (!pg)
        return 0;

    int err = 0;

    for (int i = pg->guard_write_pos - 1; i >= 0; i--)
    {
        AccessType current_access;
        if (pg->last_used)
        {
            pg->last_used = false;
            current_access = pg->last_access;
        }
        else if (i == 0)
            current_access = pg->first_acces;
        else
            current_access = START_WRITE;

        if (current_access != NONE)
            current_access += 1;

        err = rw_action_wrapper(pg->guards[i], current_access);

        if (err != 0)
            return err;
    }

    free(pg->path);

    free(pg);
    return 0;
}

static Tree *pg_get(PathGetter *pg, AccessType first_acces, AccessType last_access)
{
    pg->first_acces = first_acces;
    pg->last_access = last_access;

    char delim[] = "/";
    char *path = strtok(pg->path, delim);

    int err = 0;
    Tree *tmp = pg->tree;

    while (path != NULL)
    {
        ReadWrite *curr_rw = &tmp->rw;
        AccessType current_access;
        char *next_path = strtok(NULL, delim);

        if (!next_path && !(pg->guard_write_pos == 0 && first_acces == NONE))
        {
            pg->last_used = true;
            current_access = last_access;
        }
        else if (pg->guard_write_pos == 0)
            current_access = first_acces;
        else
            current_access = START_WRITE;

        if ((err = rw_action_wrapper(curr_rw, current_access)) != 0)
        {
            errno = err;
            return NULL;
        }
        pg->guards[pg->guard_write_pos++] = curr_rw;
        tmp = (Tree *)hmap_get(tmp->tree_map, path);

        if (!tmp)
        {
            errno = ENOENT;
            return NULL;
        }
        else
            path = next_path;
    }

    return tmp;
}

Tree *tree_new()
{
    Tree *t = NULL;
    t = (Tree *)malloc(sizeof(Tree));
    if (!t)
        syserr("Failed to create new tree!");

    if (rw_init(&t->rw) != 0)
        syserr("Failed to create new tree!");

    t->tree_map = hmap_new();
    if (!t->tree_map)
        syserr("Failed to create new tree!");

    return t;
}

int tree_create(Tree *tree, const char *path)
{
    if (!is_path_valid(path))
        return EINVAL;

    if (strcmp(path, "/") == 0)
        return EEXIST;

    int err = 0;
    char *dirc, *basec, *bname, *dname;

    dirc = strdup(path);
    basec = strdup(path);
    if (!dirc || !basec)
        return -1;

    dname = dirname(dirc);
    bname = basename(basec);

    bool swap_bd_name = false;
    if (strcmp(dname, "/") == 0)
        swap_bd_name = true;

    PathGetter *pg = NULL;

    if (!swap_bd_name)
    {
        pg = pg_create(dname, tree);
        if (pg)
            tree = pg_get(pg, START_READ, START_WRITE);
        else
            tree = NULL;
    }
    else
        err = rw_action_wrapper(&tree->rw, START_WRITE);

    if (!tree || err != 0)
    {
        if (pg)
            pg_free(pg);
        free(dirc);
        free(basec);
        return errno;
    }

    Tree *new_folder = tree_new();
    bool is_inserted = hmap_insert(tree->tree_map, bname, new_folder);

    if (!is_inserted)
        tree_free(new_folder);

    if (pg)
        pg_free(pg);
    else
        err = rw_action_wrapper(&tree->rw, END_WRITE);

    free(dirc);
    free(basec);

    if (err != 0)
        return err;
    else if (is_inserted)
        return 0;
    else
        return EEXIST;
}

int tree_remove(Tree *tree, const char *path)
{
    if (!is_path_valid(path))
        return EINVAL;

    if (strcmp(path, "/") == 0)
        return EBUSY;

    int err = 0;
    char *dirc, *basec, *bname, *dname;

    dirc = strdup(path);
    basec = strdup(path);
    if (!dirc || !basec)
        return -1;

    dname = dirname(dirc);
    bname = basename(basec);

    bool swap_bd_name = false;
    if (strcmp(dname, "/") == 0)
        swap_bd_name = true;

    PathGetter *pg = NULL;

    if (!swap_bd_name)
    {
        pg = pg_create(dname, tree);
        if (pg)
            tree = pg_get(pg, START_READ, START_WRITE);
        else
            tree = NULL;
    }
    else
        err = rw_action_wrapper(&tree->rw, START_WRITE);

    if (!tree || err != 0)
    {
        if (pg)
            pg_free(pg);
        free(dirc);
        free(basec);
        return errno;
    }

    Tree *to_be_removed = hmap_get(tree->tree_map, bname);
    if (!to_be_removed || hmap_size(to_be_removed->tree_map) != 0)
    {
        if (pg)
            pg_free(pg);
        else
            err = rw_action_wrapper(&tree->rw, END_WRITE);
        free(dirc);
        free(basec);

        if (!to_be_removed)
            return ENOENT;
        else
            return ENOTEMPTY;
    }

    bool is_removed = hmap_remove(tree->tree_map, bname);
    if (!is_removed)
        syserr("hmap broken!");

    tree_free(to_be_removed);

    if (pg)
        pg_free(pg);
    else
        err = rw_action_wrapper(&tree->rw, END_WRITE);

    free(dirc);
    free(basec);

    return err;
}

char *tree_list(Tree *tree, const char *path)
{
    if (!is_path_valid(path))
        return NULL;

    int err = 0;
    PathGetter *pg = NULL;

    if (strcmp(path, "/") != 0)
    {
        pg = pg_create(path, tree);
        if (pg)
            tree = pg_get(pg, START_READ, START_READ);
        else
            tree = NULL;
    }
    else
        err = rw_action_wrapper(&tree->rw, START_READ);

    if (!tree || err != 0)
    {
        if (pg)
            pg_free(pg);
        return NULL;
    }

    char *list = NULL;
    list = make_map_contents_string(tree->tree_map);

    if (pg)
        pg_free(pg);
    else
        err = rw_action_wrapper(&tree->rw, END_READ);

    if (err != 0)
    {
        if (list)
            free(list);
        return NULL;
    }

    return list;
}

int tree_move(Tree *tree, const char *source, const char *target)
{
    if (!is_path_valid(source) || !is_path_valid(target))
        return EINVAL;

    if (strcmp(source, "/") == 0)
        return EBUSY;

    if (strcmp(target, "/") == 0)
        return EEXIST;

    if ((strstr(target, source) == target || strstr(source, target) == source))
        return MOVE_SUBTREES; // moving tree into subtree or subtree into tree

    int err = 0;

    size_t source_len = strlen(source);
    size_t target_len = strlen(target);
    char *shared = NULL, *target_rest = NULL, *source_rest = NULL;
    shared = (char *)malloc(sizeof(char) * (max(source_len, target_len) + 1));
    source_rest = (char *)malloc(sizeof(char) * (source_len + 1));
    target_rest = (char *)malloc(sizeof(char) * (target_len + 1));
    if (!shared || !source_rest || !target_rest)
    {
        if (shared)
            free(shared);
        if (target_rest)
            free(target_rest);
        if (source_rest)
            free(source_rest);
        return MALLOC_FAILED;
    }

    get_shared_path(source, target, shared, source_rest, target_rest);

    char *source_dirc = NULL, *source_first = NULL, *source_bname = NULL, *source_dname = NULL;
    char *target_dirc = NULL, *target_first = NULL, *target_bname = NULL, *target_dname = NULL;

    source_first = (char *)malloc(sizeof(char) * (MAX_FOLDER_NAME_LENGTH + 1));
    target_first = (char *)malloc(sizeof(char) * (MAX_FOLDER_NAME_LENGTH + 1));
    source_bname = (char *)malloc(sizeof(char) * (MAX_FOLDER_NAME_LENGTH + 1));
    target_bname = (char *)malloc(sizeof(char) * (MAX_FOLDER_NAME_LENGTH + 1));
    if (!source_first || !target_first || !source_bname || !target_bname)
    {
        free(shared);
        free(target_rest);
        free(source_rest);

        if (source_first)
            free(source_first);
        if (target_first)
            free(target_first);
        if (source_bname)
            free(source_bname);
        if (target_bname)
            free(target_bname);

        return MALLOC_FAILED;
    }

    Tree *shared_tree = tree;
    PathGetter *pg_shared = NULL;

    if (strcmp(shared, "/") != 0)
    {
        pg_shared = pg_create(shared, shared_tree);
        if (pg_shared)
            shared_tree = pg_get(pg_shared, START_READ, START_READ);
        else
            shared_tree = NULL;
    }
   
    free(shared);

    if (!shared_tree)
    {
        pg_free(pg_shared);
        free(source_rest);
        free(target_rest);
        return ENOENT;
    }


    source_dirc = split_path(source_rest, source_first);
    target_dirc = split_path(target_rest, target_first);

    if (strcmp(source_first, "") == 0)
        strcpy(source_first, source_rest);
    if (strcmp(target_rest, "") == 0)
        strcpy(target_first, target_rest);

    source_dname = make_path_to_parent(source_rest, source_bname);
    target_dname = make_path_to_parent(target_rest, target_bname);

    if (source_dirc == NULL)
        source_dirc = source_rest;

    if (target_dirc == NULL)
        target_dirc = target_rest;

    AccessType shared_atype;

    Tree *source_tree = shared_tree;
    PathGetter *pg_source = NULL;

    Tree *target_tree = shared_tree;
    PathGetter *pg_target = NULL;


    if (strcmp(source_dirc, "/") == 0 || strcmp(target_dirc, "/") == 0)
    {
        shared_atype = START_WRITE;
        free(source_rest);
        free(target_rest);
    }
    else
    {
        free(source_dname);
        free(target_dname);
        shared_atype = START_READ;
        source_dname = make_path_to_parent(source_rest, NULL);
        target_dname = make_path_to_parent(target_rest, NULL);
        free(source_rest);
        free(target_rest);
    }

    err = rw_action_wrapper(&shared_tree->rw, shared_atype);

    if (err != 0)
    {
        pg_free(pg_shared);
        free(source_bname);
        free(target_bname);
        free(source_first);
        free(target_first);
        free(source_dname);
        free(target_dname);

        return err;
    }

    pg_source = pg_create(source_dname, source_tree);
    if (pg_source)
        source_tree = pg_get(pg_source, NONE, NONE);
    else
        source_tree = NULL;

    pg_target = pg_create(target_dname, target_tree);
    if (pg_target)
        target_tree = pg_get(pg_target, NONE, NONE);
    else
        target_tree = NULL;

    if (!source_tree || !target_tree)
    {
        pg_free(pg_source);
        pg_free(pg_target);
        rw_action_wrapper(&shared_tree->rw, shared_atype + 1);
        pg_free(pg_shared);
        free(source_bname);
        free(target_bname);
        free(source_first);
        free(target_first);
        free(source_dname);
        free(target_dname);
        return ENOENT;
    }

    Tree *source_to_remove = NULL;
    Tree *target_to_insert = 1;

    if (strcmp(source_dname, "/") == 0)
        source_to_remove = hmap_get(source_tree->tree_map, source_bname);
    else
    {
        err = rw_action_wrapper(&source_tree->rw, START_WRITE);
        if (err == 0)
            source_to_remove = hmap_get(source_tree->tree_map, source_bname);
    }

    if (strcmp(target_dname, "/") == 0)
        target_to_insert = hmap_get(target_tree->tree_map, target_bname);
    else
    {
        err = rw_action_wrapper(&target_tree->rw, START_WRITE);
        if (err == 0)
            target_to_insert = hmap_get(target_tree->tree_map, target_bname);
    }

    if (!source_to_remove || target_to_insert || err)
    {
        if (strcmp(source_dname, "/") != 0)
            rw_action_wrapper(&source_tree->rw, END_WRITE);

        if (strcmp(target_dname, "/") != 0)
            rw_action_wrapper(&target_tree->rw, END_WRITE);

        pg_free(pg_source);
        pg_free(pg_target);
        rw_action_wrapper(&shared_tree->rw, shared_atype + 1);
        pg_free(pg_shared);
        free(source_bname);
        free(target_bname);
        free(source_first);
        free(target_first);
        free(source_dname);
        free(target_dname);

        if (!source_to_remove)
            err = ENOENT;
        else if (target_to_insert)
            err = EEXIST;

        return err;
    }

    bool is_removed = hmap_remove(source_tree->tree_map, source_bname);
    if (!is_removed)
        syserr("rw lock broken!");
    bool is_inserted = hmap_insert(target_tree->tree_map, target_bname, source_to_remove);
    if (!is_inserted)
        syserr("rw lock broken!");

    if (strcmp(source_dname, "/") != 0)
        rw_action_wrapper(&source_tree->rw, END_WRITE);
    if (strcmp(target_dname, "/") != 0)
        rw_action_wrapper(&target_tree->rw, END_WRITE);
    pg_free(pg_source);
    pg_free(pg_target);
    rw_action_wrapper(&shared_tree->rw, shared_atype + 1);
    pg_free(pg_shared);
    free(source_bname);
    free(target_bname);
    free(source_first);
    free(target_first);
    free(source_dname);
    free(target_dname);

    return err;
}

#endif