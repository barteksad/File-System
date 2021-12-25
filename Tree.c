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
#include "err.h"
#include "HashMap.h"
#include "path_utils.h"

struct Tree
{
    HashMap *tree_map;
};

typedef struct PathGetter
{
    char *path;
    ReadWrite *guards[2047];
    HashMap *map;
    size_t guard_write_pos;
    AccessType first_acces;

} PathGetter;

static PathGetter *pg_create(const char *_path, HashMap *_map)
{
    PathGetter *pg = malloc(sizeof(PathGetter));

    if (!pg)
        return NULL;

    pg->map = _map;

    pg->path = strdup(_path);
    pg->guard_write_pos = 0;

    return pg;
}

void tree_free(Tree *tree)
{
    hmap_free(tree->tree_map);
    free(tree);
}

static int pg_free(PathGetter *pg)
{
    int err;

    for (int i = pg->guard_write_pos - 1; i >= 0; i--)
    {
        if (i == 0)
            err = rw_action_wrapper(pg->guards[i], pg->first_acces + 1);
        else
            err = rw_action_wrapper(pg->guards[i], END_READ);
        if (err != 0)
            return err;
    }

    if (pg->path)
        free(pg->path);
    free(pg);
    return 0;
}

static HashMap *pg_get(PathGetter *pg, AccessType first_acces)
{
    pg->first_acces = first_acces;

    char delim[] = "/";
    char *path = strtok(pg->path, delim);

    HashMap *tmp = pg->map;
    Pair *p;

    while (path != NULL)
    {
        if (pg->guard_write_pos == 0)
            p = hmap_get(tmp, path, first_acces);
        else
            p = hmap_get(tmp, path, START_READ);
        if (!p || !p->value)
        {
            if (p)
            {
                if (pg->guard_write_pos == 0 && first_acces != NONE)
                    rw_action_wrapper(p->bucket_guard, first_acces + 1);
                else
                    rw_action_wrapper(p->bucket_guard, END_READ);
                free(p);
            }
            pg_free(pg);
            return NULL;
        }
        else
        {
            pg->guards[pg->guard_write_pos++] = p->bucket_guard;
            tmp = p->value;
        }

        free(p);
        path = strtok(NULL, delim);
    }

    return tmp;
}

Tree *tree_new()
{
    Tree *t = malloc(sizeof(Tree));
    if (!t)
        return NULL;

    t->tree_map = hmap_new();

    return t;
}

int tree_create(Tree *tree, const char *path)
{
    if (!is_path_valid(path))
        return EINVAL;

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

    HashMap *map = tree->tree_map;
    PathGetter *pg = NULL;

    if (!swap_bd_name)
    {
        pg = pg_create(dname, map);
        map = pg_get(pg, START_READ);
    }

    if (!map)
    {
        free(dirc);
        free(basec);
        return errno;
    }

    HashMap *new_folder = hmap_new();
    int err = hmap_insert(map, bname, new_folder, false);
    if (err)
        hmap_free(new_folder);

    if (pg)
        pg_free(pg);

    free(dirc);
    free(basec);

    return err;
}

int tree_remove(Tree *tree, const char *path)
{
    if (!is_path_valid(path))
        return EINVAL;

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

    HashMap *map = tree->tree_map;
    PathGetter *pg = NULL;

    if (!swap_bd_name)
    {
        pg = pg_create(bname, map);
        map = pg_get(pg, START_READ);
    }

    if (!map)
    {
        free(dirc);
        free(basec);
        return errno;
    }

    Pair *p = hmap_remove(map, bname, false, true);

    int err = 0;

    if (p)
    {
        hmap_free(p->value);
        err = rw_action_wrapper(p->bucket_guard, END_WRITE);
        free(p);
    }
    else
        err = errno;

    if (pg)
        pg_free(pg);

    free(dirc);
    free(basec);

    return err;
}

char *tree_list(Tree *tree, const char *path)
{
    if (!is_path_valid(path))
        return NULL;

    HashMap *map = tree->tree_map;
    PathGetter *pg = NULL;

    if (strcmp(path, "/") != 0)
    {
        pg = pg_create(path, map);
        map = pg_get(pg, START_READ);
    }

    if (!map)
        return NULL;

    char *list = map_list(map);

    if (pg)
        pg_free(pg);

    return list;
}

// int tree_move(Tree* tree, const char* source, const char* target)
// {
//     if (!is_path_valid(source) || !is_path_valid(target))
//         return EINVAL;
//     if(strcmp(source, "/") == 0)
//         return EBUSY;

//     if(strcmp(source, target) == 0)
//         return EBUSY;

//     // get source
//     char *source_dirc, *source_basec, *source_bname, *source_dname;
//     char *target_dirc, *target_basec, *target_bname, *target_dname;

//     source_dirc = strdup(source);
//     source_basec = strdup(source);
//     if(!source_dirc || !source_basec)
//         return -1;

//     target_dirc = strdup(target);
//     target_basec = strdup(target);
//     if(!target_dirc || !target_basec)
//         return -1;

//     source_dname = dirname(source_dirc);
//     source_bname = basename(source_basec);

//     target_dname = dirname(target_dirc);
//     target_bname = basename(target_basec);

//     bool same_dname = (strcmp(source_dname, target_dname) == 0);

//     bool swap_source_bd_name = false;
//     if (strcmp(source_dname, "/") == 0)
//         swap_source_bd_name = true;

//     HashMap *source_map = tree->tree_map;
//     PathGetter *source_pg = NULL;

//     if (!swap_source_bd_name)
//     {
//         source_pg = pg_create(source_bname, source_map);
//         source_map = pg_get(source_pg);
//     }

//     if (!source_map)
//     {
//         free(source_dirc);
//         free(source_basec);
//         return ENOENT;
//     }

//     bool swap_target_bd_name = false;
//     if (strcmp(target_dname, "/") == 0)
//         swap_target_bd_name = true;

//     HashMap *target_map = tree->tree_map;
//     PathGetter *target_pg = NULL;

//     if (!swap_target_bd_name)
//     {
//         target_pg = pg_create(target_bname, target_map);
//         target_map = pg_get(target_pg);
//     }

//     if (!target_map)
//     {
//         free(target_dirc);
//         free(target_basec);
//         return ENOENT;
//     }

//     int err;
//     bool same_bucket = false;
//     Pair * source_p;
//     Pair * target_p = hmap_get(target_map, target_bname, START_WRITE);
//     if(same_dname)
//     {
//         unsigned int h1 = get_hash(target_bname);
//         unsigned int h2 = get_hash(source_bname);
//         if(h1 == h2)
//             same_bucket = true;
//     }
//     if(same_bucket)
//         source_p = hmap_get(source_map, source_bname, NONE);
//     else
//         source_p = hmap_get(source_map, source_bname, START_WRITE);

//     if(target_p->value != NULL || source_p->value == NULL)
//     {
//         pg_free(target_pg);
//         pg_free(source_pg);

//         free(source_dirc);
//         free(source_basec);
//         free(target_dirc);
//         free(target_basec);

//         if(target_p->value != NULL)
//             err = EEXIST;
//         else
//             err = ENOENT;

//         rw_action_wrapper(target_p->bucket_guard, END_READ);
//         if(!same_bucket)
//             rw_action_wrapper(source_p->bucket_guard, END_READ);

//         free(target_p);
//         free(source_p);

//         return err;
//     }

//     Pair *source_removed = hmap_remove(source_map, source_bname, true, false);
//     if(!source_removed)
//         return errno;

//     err = hmap_insert(target_map, target_bname, source_removed->value, true);

//     if(!same_bucket)
//         rw_action_wrapper(source_p->bucket_guard, END_READ);

//     if(target_pg)
//         pg_free(target_pg);
//     if(source_pg)
//         pg_free(source_pg);

//     free(source_dirc);
//     free(source_basec);
//     free(target_dirc);
//     free(target_basec);

//     if(target_p->value != NULL)
//         err = EEXIST;
//     else
//         err = ENOENT;

//     // rw_action_wrapper(target_p->bucket_guard, END_READ);
//     // rw_action_wrapper(source_p->bucket_guard, END_READ);

//     free(target_p);
//     free(source_p);
//     free(source_removed);
// }

int tree_move(Tree *tree, const char *source, const char *target)
{
    if (!is_path_valid(source) || !is_path_valid(target))
        return EINVAL;

    if (strcmp(source, "/") == 0)
        return EBUSY;

    if (strstr(source, target) != NULL)
        return -2; // przenoszenie drzewa do poddrzewa

    int err = 0;

    char *shared, *source_rest, *target_rest;
    get_shared_path(source, target, &shared, &source_rest, &target_rest);

    HashMap *shared_map = tree->tree_map;
    PathGetter *pg_shared = NULL;

    if (strcmp(shared, "/") != 0)
    {
        pg_shared = pg_create(shared, shared_map);
        shared_map = pg_get(pg_shared, START_READ);
    }

    if (!shared_map)
    {
        free(shared);
        free(source_rest);
        free(target_rest);
        return ENOENT;
    }

    char *source_dirc, *source_first, *source_bname, *source_dname;
    char *target_dirc, *target_first, *target_bname, *target_dname;

    source_first = (char *)malloc(sizeof(char) * (MAX_FOLDER_NAME_LENGTH + 1));
    target_first = (char *)malloc(sizeof(char) * (MAX_FOLDER_NAME_LENGTH + 1));
    source_bname = (char *)malloc(sizeof(char) * (MAX_FOLDER_NAME_LENGTH + 1));
    target_bname = (char *)malloc(sizeof(char) * (MAX_FOLDER_NAME_LENGTH + 1));
    if (!source_first || !target_first || !source_bname || !target_bname)
        syserr("malloc failed!");

    source_dirc = split_path(source_rest, source_first);
    target_dirc = split_path(target_rest, target_first);

    if (strcmp(source_first, "") == 0)
        strcpy(source_first, source_rest);
    if (strcmp(target_rest, "") == 0)
        strcpy(target_first, target_rest);

    if (strcmp(source_dirc, "/") == 0)
        source_dname = make_path_to_parent(source_rest, source_bname);
    else
        source_dname = make_path_to_parent(source_dirc, source_bname);

    if (strcmp(target_dirc, "/") == 0)
        target_dname = make_path_to_parent(target_rest, target_bname);
    else
        target_dname = make_path_to_parent(target_dirc, target_bname);

    if (source_dirc == NULL)
        source_dirc = source_rest;

    if (target_dirc == NULL)
        target_dirc = target_rest;

    Pair *p = NULL;
    AccessType shared_atype;

    HashMap *source_map = shared_map;
    PathGetter *pg_source = NULL;

    HashMap *target_map = shared_map;
    PathGetter *pg_target = NULL;

    if (get_hash(source_first) == get_hash(target_first))
    {

        if (strcmp(source_dirc, "/") == 0 || strcmp(target_dirc, "/") == 0)
            shared_atype = START_WRITE;
        else
        {
            source_dname = make_path_to_parent(source_rest, NULL);
            target_dname = make_path_to_parent(target_rest, NULL);
            shared_atype = START_READ;
        }

        p = hmap_get(source_map, source_first, shared_atype);

        if (!p)
            return -1;
        if (!p->value)
        {
            pg_free(pg_shared);
            rw_action_wrapper(p->bucket_guard, shared_atype + 1);
            free(p);
            return ENOENT;
        }

        pg_source = pg_create(source_dname, source_map);
        source_map = pg_get(pg_source, NONE);

        pg_target = pg_create(target_dname, target_map);
        target_map = pg_get(pg_target, NONE);

        if (!source_map || !target_map)
        {
            // pg_free(pg_source);
            // pg_free(pg_target);
            rw_action_wrapper(p->bucket_guard, shared_atype + 1);
            free(p);
            free(source_first);
            free(target_first);
            free(source_bname);
            free(target_bname);
            if (pg_shared)
                pg_free(pg_shared);
            free(shared);
            free(source_rest);
            free(target_rest);
            return ENOENT;
        }

        Pair *source_p;
        Pair *target_p;

        if (strcmp(source_dname, "/") == 0)
            source_p = hmap_get(source_map, source_bname, NONE);
        else
            source_p = hmap_get(source_map, source_bname, START_WRITE);

        if (strcmp(target_dname, "/") == 0)
            target_p = hmap_get(target_map, target_bname, NONE);
        else
            target_p = hmap_get(target_map, target_bname, START_WRITE);

        if (!source_p || !source_p->value || !target_p || target_p->value)
        {
            if (strcmp(source_dname, "/") != 0 && source_p)
                rw_action_wrapper(source_p->bucket_guard, END_WRITE);

            if (strcmp(target_dname, "/") != 0 && target_p)
                rw_action_wrapper(target_p->bucket_guard, END_WRITE);

            if (pg_source)
                free(pg_source);
            if (pg_target)
                free(pg_target);
            rw_action_wrapper(p->bucket_guard, shared_atype + 1);
            free(p);
            free(source_first);
            free(target_first);
            free(source_bname);
            free(target_bname);
            if (pg_shared)
                pg_free(pg_shared);
            free(shared);
            free(source_rest);
            free(target_rest);

            if (source_p && !source_p->value)
                err = ENOENT;
            if (target_p && target_p->value)
                err = EEXIST;

            free(source_p);
            free(target_p);

            return err;
        }

        Pair *source_p_removed = hmap_remove(source_map, source_bname, true, false);
        err = hmap_insert(target_map, target_bname, source_p_removed->value, true);

        if (source_p_removed)
        {
            if (strcmp(source_dname, "/") != 0 && source_p)
                rw_action_wrapper(source_p->bucket_guard, END_WRITE);
            // if(strcmp(target_dname, "/") != 0 && target_p)
            //     rw_action_wrapper(target_p->bucket_guard, END_WRITE);
        }

        if (pg_source)
            free(pg_source);
        if (pg_target)
            free(pg_target);
        if (strcmp(target_dname, "/") != 0 && target_p)
            rw_action_wrapper(p->bucket_guard, shared_atype + 1);
        free(p);
        free(source_first);
        free(target_first);
        free(source_bname);
        free(target_bname);
        if (pg_shared)
            pg_free(pg_shared);
        free(shared);
        free(source_rest);
        free(target_rest);
        free(source_p_removed);
        free(source_p);
        free(target_p);

        return err;
    }
    else
    {
        pg_source = pg_create(source_dname, source_map);
        source_map = pg_get(pg_source, START_READ);

        pg_target = pg_create(target_dname, target_map);
        target_map = pg_get(pg_target, START_READ);

        if (!source_map || !target_map)
        {
            if (pg_source)
                free(pg_source);
            if (pg_target)
                free(pg_target);
            free(source_first);
            free(target_first);
            free(source_bname);
            free(target_bname);
            if (pg_shared)
                pg_free(pg_shared);
            free(shared);
            free(source_rest);
            free(target_rest);
            return ENOENT;
        }

        Pair *source_p;
        Pair *target_p;

        source_p = hmap_get(source_map, source_bname, START_WRITE);
        target_p = hmap_get(target_map, target_bname, START_WRITE);

        if (!source_p || !source_p->value || !target_p || target_p->value)
        {
            if (pg_source)
                free(pg_source);
            if (pg_target)
                free(pg_target);
            free(source_first);
            free(target_first);
            free(source_bname);
            free(target_bname);
            if (pg_shared)
                pg_free(pg_shared);
            free(shared);
            free(source_rest);
            free(target_rest);

            if (source_p)
                rw_action_wrapper(source_p->bucket_guard, END_WRITE);
            if (target_p)
                rw_action_wrapper(target_p->bucket_guard, END_WRITE);

            if (source_p && !source_p->value)
                err = ENOENT;
            if (target_p && target_p->value)
                err = EEXIST;

            free(source_p);
            free(target_p);

            return err;
        }

        Pair *source_p_removed = hmap_remove(source_map, source_bname, true, false);
        err = hmap_insert(target_map, target_bname, source_p_removed->value, true);

        if (source_p_removed)
        {
            rw_action_wrapper(source_p->bucket_guard, END_WRITE);
            // rw_action_wrapper(target_p->bucket_guard, END_WRITE);
        }

        if (pg_source)
            free(pg_source);
        if (pg_target)
            free(pg_target);
        free(source_first);
        free(target_first);
        free(source_bname);
        free(target_bname);
        if (pg_shared)
            pg_free(pg_shared);
        free(shared);
        free(source_rest);
        free(target_rest);
        free(source_p_removed);
        free(source_p);
        free(target_p);

        return err;
    }
}

#endif