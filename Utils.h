#pragma once

typedef struct ReadWrite
{
    pthread_mutex_t lock;
    pthread_cond_t readers;
    pthread_cond_t writers;
    pthread_cond_t eraser; // only for folders not buckets
    int rcount, wcount, rwait, wwait;
    int change;
    int to_erase; // only for folders not buckets
} ReadWrite;

typedef struct Pair{
    void* value;
    ReadWrite* pair_guard;
};
int rw_init(struct ReadWrite *);

int rw_destroy(struct ReadWrite *);

int rw_start_read(ReadWrite *rw);

int rw_end_read(ReadWrite *rw);

int rw_start_write(ReadWrite *rw);

int rw_end_write(ReadWrite *rw);

int rw_remove(ReadWrite *rw);

