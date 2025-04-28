#include "../kernel/types.h"
#include "../user/user.h"

int
main(int argc, char *argv[])
{
    int n = 4;  // Number of child processes to create
    int pids[16];  // Array to store child PIDs

    // Call forkn system call
    int result = forkn(n, pids);

    if (result < 0) {
        printf("forkn failed\n");
        exit(1,"");
    }

    // Parent process
    if (result == 0) {
        printf("Parent: Successfully created %d child processes\n", n);
        
        // Print out the PIDs
        printf("Child PIDs: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", pids[i]);
        }
        printf("\n");

        // Wait for all children to finish
        for (int i = 0; i < n; i++) {
            wait(0,0);
        }
    }
    // Child processes
    else {
        printf("Child %d (PID: %d) running\n", result, getpid());
        
        // Do some work or sleep to demonstrate
        for (int i = 0; i < result * 100000; i++) {
            // Some busy work to show different children behave differently
        }
        
        exit(0,"");
    }

    exit(0,"");
}