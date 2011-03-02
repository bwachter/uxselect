/*
 * shmdata.h
 * (c) 2011 Bernd Wachter bwachter-usenet@lart.info
 */

#ifndef _SHMDATA_H
#define _SHMDATA_H

#include <limits.h>

typedef struct _shm_exchange shm_exchange;
struct _shm_exchange {
    char user[255];
    char session[PATH_MAX];
};

#endif
