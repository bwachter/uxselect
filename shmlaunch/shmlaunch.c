#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "../src/shmdata.h"

int main(){
  int shm_id;
  shm_exchange *shm;
  char shm_id_str[50];
  const int shm_size=sizeof(shm_exchange);
  pid_t pid;

  shm_id=shmget(IPC_PRIVATE, shm_size,
                IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  shm=(shm_exchange *) shmat(shm_id, 0, 0);
  printf("shared memory attached at address %p\n", shm);

  printf("SHM_ID: %i\n", shm_id);
  snprintf(shm_id_str, 50, "%i", shm_id);
  setenv("SHM_ID", shm_id_str, 1);
  setenv("UX_USER", "bwachter", 1);

  pid=fork();
  if (pid==0){
    execlp("uxselect", "uxselect", (char *)NULL);
    exit(-1);
  } else if (pid>0){
    waitpid(pid, 0, 0);
  }

  printf("SHM contents:\n user: %s,\n session: %s\n", shm->user, shm->session_path);
  shmctl(shm_id, IPC_RMID, 0);

  return 0;
}
