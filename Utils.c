#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "err.h"
#include "HashMap.h"
#include "Utils.h"

int rw_init(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_init(&rw->lock, 0)) != 0)
        return err; // syserr (err, "mutex init failed");
    if ((err = pthread_cond_init(&rw->readers, 0)) != 0)
        return err; // syserr (err, "cond init readers failed");
    if ((err = pthread_cond_init(&rw->writers, 0)) != 0)
        return err; // syserr (err, "cond init writers failed");

    rw->rcount = 0;
    rw->wcount = 0;
    rw->rwait = 0;
    rw->wwait = 0;

    rw->change = 0;

    return 0;
}

int rw_destroy(ReadWrite *rw)
{
    int err;

    if ((err = pthread_cond_destroy(&rw->readers)) != 0)
        return err; // syserr (err, "cond readers destroy 1 failed");
    if ((err = pthread_cond_destroy(&rw->writers)) != 0)
        return err; // syserr (err, "cond writers rw_destroy failed");
    if ((err = pthread_mutex_destroy(&rw->lock)) != 0)
        return err; // syserr (err, "mutex rw_destroy failed");

    return 0;
}

int rw_start_read(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_lock(&rw->lock)) != 0)
        return err;

    while (rw->wcount || (rw->wwait > 0 && rw->change == 0))
    {
        rw->rwait++;
        if ((err = pthread_cond_wait(&rw->readers, &rw->lock)) != 0)
            return err;
        rw->rwait--;
    }

    rw->rcount++;
    
    if (rw->rwait > 0 && rw->change == 1)
    {
        if ((err = pthread_cond_signal(&rw->readers)) != 0)
            return err; // syserr (err, "cond signal rwait failed");
    }
    else
        rw->change = 0;


    if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
        return err;

    return 0;
}

int rw_end_read(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_lock(&rw->lock)) != 0)
        return err;

    rw->rcount--;

    if (rw->rcount == 0)
    {

        if (rw->wwait > 0)
        {
            if ((err = pthread_cond_signal(&rw->writers)) != 0)
                return err;
        }
        else
        {
            if ((err = pthread_cond_signal(&rw->readers)) != 0)
                return err;
        }
    }

    if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
        return err; // syserr (err, "unlock failed");

    return 0;
}

int rw_start_write(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_lock(&rw->lock)) != 0)
        return err;

    while (rw->rcount > 0 || rw->wcount > 0 || rw->change == 1)
    {
        rw->wwait++;
        if ((err = pthread_cond_wait(&rw->writers, &rw->lock)) != 0)
            return err;
        rw->wwait--;
    }

    rw->wcount++;

    if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
        return err; // syserr (err, "unlock failed");

    return 0;
}

int rw_end_write(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_lock(&rw->lock)) != 0)
        return err;

    rw->wcount--;

    if (rw->rwait > 0)
    {
        rw->change = 1;

        if ((err = pthread_cond_signal(&rw->readers)) != 0)
            return err;
    }
    else
    {
        if ((err = pthread_cond_signal(&rw->writers)) != 0)
            return err;
    }

    if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
        return err;

    return 0;
}

int rw_action_wrapper(ReadWrite *rw, AccessType a_type)
{
    if(!rw)
        return ENOENT;
        
    switch (a_type)
    {
    case START_READ:
        return rw_start_read(rw);
        break;
    case END_READ:
        return rw_end_read(rw);
        break;
    case START_WRITE:
        return rw_start_write(rw);
        break;
    case END_WRITE:
        return rw_end_write(rw);
        break;
    case NONE:
        return 0;
        break;
    default:
        syserr("switch action_wrapper broken");
        break;
    }

    return EINVAL;
}

void get_shared_path(const char *source, const char *target, char *shared, char* source_rest, char *target_rest)
{
    size_t source_len = strlen(source);
    size_t target_len = strlen(target);
    
    size_t i = 0;
    size_t last_slash = 0;
    while(i < source_len && i < target_len && source[i] == target[i])
    {
        if(source[i] == '/')
            last_slash = i;
        i++;
    }

    strcpy(shared, source);
    shared[last_slash+1] ='\0';
    strcpy(source_rest, source + last_slash);
    strcpy(target_rest, target + last_slash);
}
