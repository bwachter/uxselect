/*
 * shmdata.h
 * (c) 2011 Bernd Wachter bwachter-usenet@lart.info
 */

#ifndef _SHMDATA_H
#define _SHMDATA_H

#include <limits.h>

#define UXLAUNCH_IPC_VERSION 1
#define UXLAUNCH_NAME_LIMIT 50

typedef struct _shm_exchange shm_exchange;
struct _shm_exchange {
    char user[255];
    char session_path[PATH_MAX];
    char session_name[UXLAUNCH_NAME_LIMIT];
};

#endif
