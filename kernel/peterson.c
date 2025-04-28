#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern struct petersonlock petersonlocks[NPETERSON];
extern struct spinlock petersonlock;

// Create a new Peterson lock
int
peterson_create(void)
{
  acquire(&petersonlock);
  
  // Find a free lock
  int i;
  for(i = 0; i < NPETERSON; i++) {
    if(petersonlocks[i].active == 0) {
      petersonlocks[i].active = 1;
      petersonlocks[i].flag[0] = 0;
      petersonlocks[i].flag[1] = 0;
      petersonlocks[i].turn = 0;
      release(&petersonlock);
      return i;  // Return lock id
    }
  }
  
  release(&petersonlock);
  return -1;  // No free locks
}

// Acquire a Peterson lock
int
peterson_acquire(int lock_id, int role)
{
  if(lock_id < 0 || lock_id >= NPETERSON || role < 0 || role > 1) {
    return -1;
  }
  
  // First ensure the lock is active
  acquire(&petersonlock);
  if(petersonlocks[lock_id].active == 0) {
    release(&petersonlock);
    return -1;
  }
  release(&petersonlock);
  
  // Set flag to indicate intention to enter critical section
  petersonlocks[lock_id].flag[role] = 1;
  __sync_synchronize();  // Memory barrier
  
  // Give priority to the other process
  petersonlocks[lock_id].turn = 1 - role;
  __sync_synchronize();  // Memory barrier
  
  // Wait while the other process wants to enter and it's the other's turn
  while(petersonlocks[lock_id].flag[1 - role] && 
        petersonlocks[lock_id].turn == 1 - role) {
    __sync_synchronize();  // Memory barrier
    yield();  // Give up CPU to avoid busy-waiting
  }
  
  return 0;
}

// Release a Peterson lock
int
peterson_release(int lock_id, int role)
{
  if(lock_id < 0 || lock_id >= NPETERSON || role < 0 || role > 1) {
    return -1;
  }
  
  acquire(&petersonlock);
  if(petersonlocks[lock_id].active == 0) {
    release(&petersonlock);
    return -1;
  }
  release(&petersonlock);
  
  // Reset flag
  petersonlocks[lock_id].flag[role] = 0;
  __sync_synchronize();  // Memory barrier
  
  return 0;
}

// Destroy a Peterson lock
int
peterson_destroy(int lock_id)
{
  if(lock_id < 0 || lock_id >= NPETERSON) {
    return -1;
  }
  
  acquire(&petersonlock);
  if(petersonlocks[lock_id].active == 0) {
    release(&petersonlock);
    return -1;
  }
  
  petersonlocks[lock_id].active = 0;
  petersonlocks[lock_id].flag[0] = 0;
  petersonlocks[lock_id].flag[1] = 0;
  petersonlocks[lock_id].turn = 0;
  
  release(&petersonlock);
  return 0;
}