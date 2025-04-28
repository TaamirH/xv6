#include "kernel/types.h"
#include "user/user.h"

int 
main(int argc, char *argv[])
{
  // Default to 16 processes if no argument provided
  int n_processes = 16;
  
  if(argc > 1) {
    n_processes = atoi(argv[1]);
    // Check if it's a power of 2 and between 2 and 16
    int temp = n_processes;
    int valid = 1;
    
    if(n_processes < 2 || n_processes > 16)
      valid = 0;
      
    while(temp > 1) {
      if(temp % 2 != 0) {
        valid = 0;
        break;
      }
      temp /= 2;
    }
    
    if(!valid) {
      printf("Number of processes must be a power of 2 between 2 and 16\n");
      exit(1, "");
    }
  }
  
  // Create tournament tree
  int id = tournament_create(n_processes);
  
  if(id < 0) {
    printf("Failed to create tournament tree\n");
    exit(1, "");
  }
  
  // Acquire tournament lock
  if(tournament_acquire() < 0) {
    printf("Process %d: Failed to acquire tournament lock\n", getpid());
    exit(1, "");
  }
  
  // Critical section
  printf("Process %d: Tournament ID %d has acquired the lock\n", getpid(), id);
  sleep(2);  // Sleep to better demonstrate exclusion
  printf("Process %d: Tournament ID %d is releasing the lock\n", getpid(), id);
  
  // Release tournament lock
  if(tournament_release() < 0) {
    printf("Process %d: Failed to release tournament lock\n", getpid());
    exit(1, "");
  }
  // For parent process, wait for all children to finish
  if(id == 0) {
    for(int i = 0; i < n_processes - 1; i++) {
      wait(0, 0);
    }
  }
  
  exit(0, "");
}