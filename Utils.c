#include <pthread.h>
#include <errno.h>

#include "err.h"
#include "Utils.h"

struct ReadWrite
{
    pthread_mutex_t lock;
    pthread_cond_t readers;
    pthread_cond_t writers;
    pthread_cond_t eraser; // only for folders not buckets
    int rcount, wcount, rwait, wwait;
    int change;
    int to_erase; // only for folders not buckets
};

int rw_init(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_init(&rw->lock, 0)) != 0)
        return err; // syserr (err, "mutex init failed");
    if ((err = pthread_cond_init(&rw->readers, 0)) != 0)
        return err; // syserr (err, "cond init readers failed");
    if ((err = pthread_cond_init(&rw->writers, 0)) != 0)
        return err; // syserr (err, "cond init writers failed");
    if ((err = pthread_cond_init(&rw->eraser, 0)) != 0)
        return err; // syserr (err, "cond init writers failed");

    rw->rcount = 0;
    rw->wcount = 0;
    rw->rwait = 0;
    rw->wwait = 0;

    rw->change = 0;
    rw->to_erase = 0;

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
    if ((err = pthread_mutex_destroy(&rw->eraser)) != 0)
        return err; // syserr (err, "mutex rw_destroy failed");

    return 0;
}

int rw_start_read(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_lock(&rw->lock)) != 0)
        return err;

    if (rw->to_erase == 1)
    {
        if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
            return err;
        return ENOENT;
    }

    while (rw->wcount + rw->wwait && rw->change == 0 && rw->to_erase == 0)
    {
        rw->rwait++;
        if ((err = pthread_cond_wait(&rw->readers, &rw->lock)) != 0)
            return err;
        rw->rwait--;
    }

    if (rw->to_erase == 1)
    {
        if ((err = pthread_cond_signal(&rw->readers)) != 0)
            return err;
        if ((err = pthread_cond_signal(&rw->writers)) != 0)
            return err;
        if (rw->rcount + rw->wcount + rw->rwait + rw->wwait == 0)
            if ((err = pthread_cond_signal(&rw->eraser)) != 0)
                return err;
        if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
            return err;
        return ENOENT;
    }

    if (rw->rwait > 0 && rw->change == 1)
    {
        if ((err = pthread_cond_signal(&rw->readers)) != 0)
            return err; // syserr (err, "cond signal rwait failed");
    }
    else
        rw->change = 0;

    rw->rcount++;

    if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
        return err;

    return 0;
}

int rw_end_read(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_lock(&rw->lock)) != 0)
        return err;

    if (rw->to_erase == 1)
    {
        if ((err = pthread_cond_signal(&rw->readers)) != 0)
            return err;
        if ((err = pthread_cond_signal(&rw->writers)) != 0)
            return err;
        if (rw->rcount + rw->wcount + rw->rwait + rw->wwait == 0)
            if ((err = pthread_cond_signal(&rw->eraser)) != 0)
                return err;
        if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
            return err;
        return ENOENT;
    }

    if (rw->rcount == 0)
        if ((err = pthread_cond_signal(&rw->writers)) != 0)
            return err; // syserr (err, "cond signal rwait failed");

    if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
        return err; // syserr (err, "unlock failed");

    return 0;
}

int rw_start_write(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_lock(&rw->lock)) != 0)
        return err;

    if (rw->to_erase == 1)
    {
        if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
            return err;
        return ENOENT;
    }

    while (rw->rcount > 0 && rw->to_erase == 0)
    {
        rw->wwait++;
        if ((err = pthread_cond_wait(&rw->writers, &rw->lock)) != 0)
            return err;
        rw->wwait--;
    }

    if (rw->to_erase == 1)
    {
        if ((err = pthread_cond_signal(&rw->readers)) != 0)
            return err;
        if ((err = pthread_cond_signal(&rw->writers)) != 0)
            return err;
        if (rw->rcount + rw->wcount + rw->rwait + rw->wwait == 0)
            if ((err = pthread_cond_signal(&rw->eraser)) != 0)
                return err;
        if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
            return err;
        return ENOENT;
    }

    rw->wcount++;

    return 0;
}

int rw_end_write(ReadWrite *rw)
{
    int err;

    if ((err = pthread_mutex_lock(&rw->lock)) != 0)
        return err;

    rw->wcount--;

    if (rw->to_erase == 1)
    {
        if ((err = pthread_cond_signal(&rw->readers)) != 0)
            return err;
        if ((err = pthread_cond_signal(&rw->writers)) != 0)
            return err;
        if (rw->rcount + rw->wcount + rw->rwait + rw->wwait == 0)
            if ((err = pthread_cond_signal(&rw->eraser)) != 0)
                return err;
        if ((err = pthread_mutex_unlock(&rw->lock)) != 0)
            return err;
        return ENOENT;
    }

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

int rw_remove(ReadWrite *rw) {
    int err;

    if ((err = pthread_mutex_lock(&rw->lock)) != 0)
        return err;

    rw->to_erase = 1;

    while (rw->rcount + rw->wcount + rw->rwait + rw->wwait > 0) {
        if ((err = pthread_cond_wait(&rw->eraser , &rw->lock)) != 0)
            return err;
    }

    return 0;
}
