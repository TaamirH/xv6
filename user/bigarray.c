#include "kernel/types.h"
#include "user/user.h"

#define TOTAL_SIZE (1 << 16)  // 65536

int arr[TOTAL_SIZE];

int
main(void)
{
  printf("Starting execution of bigarray.c (PID: %d)...\n", getpid());
  for (int i = 0; i < TOTAL_SIZE; i++) {
    arr[i] = i;
    if (i % 16384 == 0) {
      printf("bigarray (PID: %d): Filling array: %d/%d\n", getpid(), i, TOTAL_SIZE);
    }
  }
  printf("bigarray (PID: %d): Array filling complete. Starting sum calculation...\n", getpid());
  int N_CHILDREN = 4;  // Can choose any number between 1-16
  int pipes[N_CHILDREN][2];

  // Create pipes
  for (int i = 0; i < N_CHILDREN; i++) {
    if (pipe(pipes[i]) < 0) {
      printf("bigarray (PID: %d): Failed to create pipe %d\n", getpid(), i);
      exit(1,"");
    }
  }

  int CHUNK_SIZE = TOTAL_SIZE / N_CHILDREN;
  int remainder = TOTAL_SIZE % N_CHILDREN;
  int pids[N_CHILDREN];
  printf("bigarray (PID: %d): Calling forkn with N_CHILDREN = %d\n", getpid(), N_CHILDREN);
  int rc = forkn(N_CHILDREN, pids);

  // Fail case
  if (rc < 0) {
    printf("bigarray (PID: %d): forkn failed\n", getpid());
    exit(1,"");
  } // Child process case
  else if (rc > 0) {
    int child_id = rc - 1;

    for (int i = 0; i < N_CHILDREN; i++) {
      if (i != child_id) {
        close(pipes[i][0]);
        close(pipes[i][1]);
      } else {
        close(pipes[i][0]);  
      }
    }

    int start = child_id * CHUNK_SIZE;
    int end = start + CHUNK_SIZE;
    if (child_id == N_CHILDREN - 1) {
      end += remainder;
    }
    int partial_sum = 0;
    int progress_interval = CHUNK_SIZE / 10;
    printf("bigarray (Child %d, PID=%d): Processing indices %d to %d\n", child_id + 1, getpid(), start, end - 1);

    for (int i = start; i < end; i++) {
      partial_sum += arr[i];
      if ((i - start) % progress_interval == 0 && i > start) {
        printf("bigarray (Child %d, PID=%d): Partial sum so far: %d\n", child_id + 1, getpid(), partial_sum);
      }
    }
    printf("bigarray (Child %d, PID=%d): Final partial sum = %d\n", child_id + 1, getpid(), partial_sum);
    write(pipes[child_id][1], &partial_sum, sizeof(partial_sum));
    close(pipes[child_id][1]);
    exit(0,"");
  } // Parent process
  else {
    for (int i = 0; i < N_CHILDREN; i++) {
      close(pipes[i][1]);  // Parent only reads
    }
    printf("bigarray (Parent PID=%d): Created children with PIDs:", getpid());

    for (int i = 0; i < N_CHILDREN; i++) {
      printf(" %d", pids[i]);
    }
    printf("\n");
    printf("bigarray (Parent PID=%d): Waiting for all children to complete...\n", getpid());

    int num_done = 0;
    int statuses[N_CHILDREN];
    if (waitall(&num_done, statuses) < 0) {
      printf("bigarray (Parent PID=%d): waitall failed\n", getpid());
      exit(1,"");
    }
    printf("bigarray (Parent PID=%d): All %d children have completed\n", getpid(), num_done);
    int child_sums[N_CHILDREN];

    for (int i = 0; i < N_CHILDREN; i++) {
      int sum = 0;
      // Get all the partial sums from children
      if (read(pipes[i][0], &sum, sizeof(sum)) != sizeof(sum)) { 
        printf("bigarray (Parent PID=%d): Failed to read from child %d pipe\n", getpid(), i + 1);
        sum = 0;
      }
      close(pipes[i][0]);
      child_sums[i] = sum;
    }

    long total_sum = 0;
    for (int i = 0; i < N_CHILDREN; i++) {
      printf("bigarray (Parent PID=%d): Child %d's partial sum = %d\n", getpid(), i + 1, child_sums[i]);
      total_sum += child_sums[i];
    }
    long expected_sum = (long)TOTAL_SIZE * (TOTAL_SIZE - 1) / 2;
    printf("bigarray (Parent PID=%d): Total sum = %l, Expected = %l\n", getpid(), total_sum, expected_sum);

    for (int i = 0; i < num_done; i++) {
      printf("bigarray (Parent PID=%d): Child %d returned status %d\n", getpid(), i + 1, statuses[i]);
    }
  }
  printf("bigarray (PID: %d): Exiting\n", getpid());
  exit(0,"");
}
