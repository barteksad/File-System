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
    ReadWrite *guards[2050];
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
    // pg->guards = malloc(2047 * sizeof (ReadWrite*));
    if(!pg->guards)
        syserr("dup");

    // printf("init: %s : %ld\n", _path, strlen(_path));
    pg->path = (char *) malloc(sizeof(char) * (MAX_PATH_LENGTH + 1));
    if(!pg->path)
        syserr("dup");

    strcpy(pg->path, _path);
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
    if(pg == NULL)
        syserr("dup");

    int err = 0;

    for (int i = pg->guard_write_pos - 1; i >= 0; i--)
    {
        if (i == 0)
        {
            if(pg->first_acces != NONE)
                err = rw_action_wrapper(pg->guards[i], pg->first_acces + 1);
        }
        else
            err = rw_action_wrapper(pg->guards[i], END_READ);
        if (err != 0)
            return err;
    }
    // if(pg->path)    
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
                errno = ENOENT;
                if (pg->guard_write_pos == 0 && first_acces != NONE)
                    rw_action_wrapper(p->bucket_guard, first_acces + 1);
                else
                    rw_action_wrapper(p->bucket_guard, END_READ);
                free(p);
            }
            else
                errno = -1;
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
        if(pg)
            map = pg_get(pg, START_READ);
        else
            map= NULL;    
    }

    if (!map)
    {
        if(pg)
            pg_free(pg);
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
        if(pg)
            map = pg_get(pg, START_READ);
        else
            map = NULL;
    }

    if (!map)
    {
        if(pg)
            pg_free(pg);
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
        if(pg)
            map = pg_get(pg, START_READ);
        else
            map = NULL;    
    }

    if (!map)
    {
        if(pg)
            pg_free(pg);
        return NULL;
    }

    char *list = map_list(map);
    // char *list = make_map_contents_string(map);

    if (pg)
        pg_free(pg);

    return list;
}

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
    err = get_shared_path(source, target, &shared, &source_rest, &target_rest);

    HashMap *shared_map = tree->tree_map;
    PathGetter *pg_shared = NULL;

    if (strcmp(shared, "/") != 0)
    {
        pg_shared = pg_create(shared, shared_map);
        if(pg_shared)
            shared_map = pg_get(pg_shared, START_READ);
        else
            shared_map = NULL;
    }

    free(shared);

    if (!shared_map)
    {
        if(pg_shared)
            pg_free(pg_shared);
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

    // if (strcmp(source_dirc, "/") == 0)
        source_dname = make_path_to_parent(source_rest, source_bname);
    // else
    //     source_dname = make_path_to_parent(source_dirc, source_bname);

    // if (strcmp(target_dirc, "/") == 0)
        target_dname = make_path_to_parent(target_rest, target_bname);
    // else
    //     target_dname = make_path_to_parent(target_dirc, target_bname);

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

        p = hmap_get(source_map, source_first, shared_atype);

        if (!p)
        {
            if(pg_shared)
                pg_free(pg_shared);
            return -1;
        }
        if (!p->value)
        {
            if(pg_shared)
                pg_free(pg_shared);
            rw_action_wrapper(p->bucket_guard, shared_atype + 1);
            free(p);
            free(source_bname);
            free(target_bname);
            free(source_first);
            free(target_first);
            free(source_dname);
            free(target_dname);
            return ENOENT;
        }

        pg_source = pg_create(source_dname, source_map);
        if(pg_source)
            source_map = pg_get(pg_source, NONE);
        else
            source_map = NULL;

        pg_target = pg_create(target_dname, target_map);
        if(pg_target)
            target_map = pg_get(pg_target, NONE);
        else
            target_map = NULL;

        if (!source_map || !target_map)
        {
            if(pg_source)
                pg_free(pg_source);
            if(pg_target)
                pg_free(pg_target);
            rw_action_wrapper(p->bucket_guard, shared_atype + 1);
            if (pg_shared)
                pg_free(pg_shared);
            free(p);
            free(source_bname);
            free(target_bname);
            free(source_first);
            free(target_first);
            free(source_dname);
            free(target_dname);
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

        if (!source_p || !target_p || !source_p->value || target_p->value)
        {
            if (strcmp(source_dname, "/") != 0 && source_p)
                rw_action_wrapper(source_p->bucket_guard, END_WRITE);

            if (strcmp(target_dname, "/") != 0 && target_p)
                rw_action_wrapper(target_p->bucket_guard, END_WRITE);

            if (pg_source)
                pg_free(pg_source);
            if (pg_target)
                pg_free(pg_target);
            rw_action_wrapper(p->bucket_guard, shared_atype + 1);
            free(p);
            if (pg_shared)
                pg_free(pg_shared);
            free(source_bname);
            free(target_bname);
            free(source_first);
            free(target_first);
            free(source_dname);
            free(target_dname);

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
            if (strcmp(source_dname, "/") != 0)
                rw_action_wrapper(source_p->bucket_guard, END_WRITE);
            // if(strcmp(target_dname, "/") != 0 && target_p)
            //     rw_action_wrapper(target_p->bucket_guard, END_WRITE);
        }

        if (pg_source)
            pg_free(pg_source);
        if (pg_target)
            pg_free(pg_target);
        if (strcmp(target_dname, "/") != 0)
            rw_action_wrapper(p->bucket_guard, shared_atype + 1);
        if (pg_shared)
            pg_free(pg_shared);
        free(p);
        free(source_p_removed);
        free(source_bname);
        free(target_bname);
        free(source_first);
        free(target_first);
        free(source_dname);
        free(target_dname);
        free(source_p);
        free(target_p);

        return err;
    }
    else
    {
        free(source_rest);
        free(target_rest);

        pg_source = pg_create(source_dname, source_map);
        if(pg_source)
            source_map = pg_get(pg_source, START_READ);
        else
            source_map = NULL;

        pg_target = pg_create(target_dname, target_map);
        if(pg_target)
            target_map = pg_get(pg_target, START_READ);
        else
            target_map = NULL;

        if (!source_map || !target_map)
        {
            if (pg_source)
                pg_free(pg_source);
            if (pg_target)
                pg_free(pg_target);
            if (pg_shared)
                pg_free(pg_shared);
            free(source_first);
            free(target_first);
            free(source_bname);
            free(target_bname);
            free(source_dname);
            free(target_dname);
            return ENOENT;
        }

        Pair *source_p;
        Pair *target_p;

        source_p = hmap_get(source_map, source_bname, START_WRITE);
        target_p = hmap_get(target_map, target_bname, START_WRITE);

        if (!source_p || !target_p || !source_p->value|| target_p->value)
        {
            if (pg_source)
                pg_free(pg_source);
            if (pg_target)
                pg_free(pg_target);
            if (pg_shared)
                pg_free(pg_shared);
            free(source_first);
            free(target_first);
            free(source_bname);
            free(target_bname);
            free(source_dname);
            free(target_dname);

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
            pg_free(pg_source);
        if (pg_target)
            pg_free(pg_target);
        if (pg_shared)
            pg_free(pg_shared);
        free(source_p_removed);
        free(source_first);
        free(target_first);
        free(source_bname);
        free(target_bname);
        free(source_dname);
        free(target_dname);
        free(source_p);
        free(target_p);

        return err;
    }
}

#endif