#ifndef UTILS
#define UTILS

#include <stdbool.h>
#include <pthread.h>

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

typedef enum AccessType {
    START_READ,
    END_READ, // = START_READ + 1
    START_WRITE,
    END_WRITE, // START_WRITE + 1
    REMOVE,
    NONE,
} AccessType;

int rw_init(struct ReadWrite *);

int rw_destroy(struct ReadWrite *);

int rw_start_read(ReadWrite *rw);

int rw_end_read(ReadWrite *rw);

int rw_start_write(ReadWrite *rw);

int rw_end_write(ReadWrite *rw);

int rw_remove(ReadWrite *rw);

int rw_action_wrapper(ReadWrite *rw, AccessType a_type);

void get_shared_path(const char *source, const char *target, char *shared, char* source_rest, char *target_rest);


#endif // define utils