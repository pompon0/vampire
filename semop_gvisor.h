#include <sys/syscall.h>
#include <sys/sem.h>

static inline int semop_gvisor(int semid, struct sembuf *sops, size_t nsops) {
  return syscall(SYS_semop,semid,sops,nsops);
}
  
