#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "../kernel/proc.h"

uint64
sys_exit(void)
{
  int n;
  char exit_msg[32];
  argint(0, &n);
  argstr(1, exit_msg, sizeof(exit_msg));
  struct proc *p = myproc();
  safestrcpy(p->exit_msg, exit_msg, sizeof(p->exit_msg));
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void){
  uint64 addr;
  uint64 msg_addr;
  
  argaddr(0, &addr);
  argaddr(1, &msg_addr);
  
  return wait(addr, msg_addr);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_memsize(void)
{
  struct proc *p = myproc();
  return p->sz;
}
uint64
sys_forkn(void)
{
    struct proc *p = myproc();
    int n = p->trapframe->a0;
    uint64 pids_uaddr = p->trapframe->a1;
    // Call the helper function from proc.c
    return forkn_user(n, pids_uaddr, p->pagetable);
}

uint64
sys_waitall(void)
{
    struct proc *p = myproc();
    uint64 n_uaddr = p->trapframe->a0;
    uint64 statuses_uaddr = p->trapframe->a1;
    // Call the helper function from proc.c
    return waitall_user(n_uaddr, statuses_uaddr, p->pagetable);
}

uint64
sys_peterson_create(void)
{
  return peterson_create();
}

uint64
sys_peterson_acquire(void)
{
  int lock_id, role;
  
  argint(0, &lock_id);
  argint(1, &role);
    
  return peterson_acquire(lock_id, role);
}

uint64
sys_peterson_release(void)
{
  int lock_id, role;
  
  argint(0, &lock_id);
  argint(1, &role);
    
  return peterson_release(lock_id, role);
}

uint64
sys_peterson_destroy(void)
{
  int lock_id;
  
  argint(0, &lock_id);
    
  return peterson_destroy(lock_id);
}