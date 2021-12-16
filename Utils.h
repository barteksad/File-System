#pragma once

typedef struct ReadWrite ReadWrite;

int rw_init(struct ReadWrite *);

int rw_destroy(struct ReadWrite *);

int rw_start_read(ReadWrite *rw);

int rw_end_read(ReadWrite *rw);

int rw_start_write(ReadWrite *rw);

int rw_end_write(ReadWrite *rw);

int rw_remove(ReadWrite *rw);

