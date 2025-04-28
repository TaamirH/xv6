#include "kernel/types.h"
#include "user/user.h"

// Tournament tree global variables
static int process_count = 0;
static int process_index = -1;
static int *lock_ids = 0;
static int levels = 0;

// Create a tournament tree
int tournament_create(int processes)
{
  // Check if processes is a power of 2 up to 16
  int temp = processes;
  while(temp > 1) {
    if(temp % 2 != 0)
      return -1;
    temp /= 2;
  }
  
  if(processes <= 0 || processes > 16)
    return -1;
    
  // Number of levels in the tree
  int lvls = 0;
  temp = processes;
  while(temp > 1) {
    lvls++;
    temp /= 2;
  }
  
  // Total number of internal nodes (locks) needed
  int total_locks = processes - 1;
  
  // Allocate memory for lock IDs
  lock_ids = malloc(total_locks * sizeof(int));
  if(!lock_ids)
    return -1;
  
  // Create all the locks needed
  for(int i = 0; i < total_locks; i++) {
    lock_ids[i] = peterson_create();
    if(lock_ids[i] < 0) {
      // Failed to create a lock
      for(int j = 0; j < i; j++) {
        peterson_destroy(lock_ids[j]);
      }
      free(lock_ids);
      lock_ids = 0;
      return -1;
    }
  }
  
  // Store the process count and levels
  process_count = processes;
  levels = lvls;
  
  // Fork all the needed processes
  for(int i = 0; i < processes - 1; i++) {
    int pid = fork();
    if(pid < 0) {
      // Fork failed, return error
      return -1;
    } else if(pid == 0) {
      // This is the child process
      process_index = i + 1; // Assign index (parent is 0)
      return process_index;  // Return index to child process
    }
    // No need to store PIDs in parent process
  }
  
  // This is the parent process (index 0)
  process_index = 0;
  return 0;
}

// Calculate lock index and role for a specific level
static void get_lock_info(int level, int *lock_idx, int *role) {
  int L = levels;
  int idx = process_index;
  
  // Formula for lock and role calculation from assignment
  // role at level l = (index & (1 << (L - l - 1))) >> (L - l - 1)
  *role = (idx & (1 << (L - level - 1))) >> (L - level - 1);
  
  // lock at level l = index >> (L - l)
  *lock_idx = idx >> (L - level);
  
  // Map to flat array:
  // locks[lockl + (1 << l) - 1]
  *lock_idx = *lock_idx + (1 << level) - 1;
}

// Acquire the tournament lock
int tournament_acquire(void) {
  if(process_index < 0 || lock_ids == 0)
    return -1;
    
  // Ascend the tree, acquiring locks at each level
  for(int level = levels - 1; level >= 0; level--) {
    int lock_idx, role;
    get_lock_info(level, &lock_idx, &role);
    
    if(lock_idx >= (process_count - 1)) {
      // Invalid lock index
      return -1;
    }
    
    // Acquire the lock at this level
    if(peterson_acquire(lock_ids[lock_idx], role) < 0) {
      // Failed to acquire lock, release all acquired locks
      for(int l = levels - 1; l > level; l--) {
        int prev_lock, prev_role;
        get_lock_info(l, &prev_lock, &prev_role);
        peterson_release(lock_ids[prev_lock], prev_role);
      }
      return -1;
    }
  }
  
  return 0;
}

// Release the tournament lock
int tournament_release(void) {
  if(process_index < 0 || lock_ids == 0)
    return -1;
    
  // Descend the tree, releasing locks in reverse order
  for(int level = 0; level < levels; level++) {
    int lock_idx, role;
    get_lock_info(level, &lock_idx, &role);
    
    // Release the lock at this level
    if(peterson_release(lock_ids[lock_idx], role) < 0)
      return -1;
  }
  
  return 0;
}