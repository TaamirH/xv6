#include "../kernel/types.h"
#include "user.h"

int main() {
  printf("testing exit message\n");
    exit(0, "Goodbye World xv6");
    return 0;  // This line will never be reached
}

/*
What happened when we changed signatures:

All existing calls to exit() and wait() became incompatible
Compiler started generating errors due to mismatched function signatures
Requires updating all existing code to match the new function signatures
This is a classic example of how changing system call interfaces impacts the entire operating system


Why add a new field to the PCB for exit message:

Kernel needs a place to store the message during the transition from child to parent
The exit message needs to persist after the child process exits but before the parent reads it
Simply reading from the exiting process is risky because:

The process resources might be deallocated
Memory might be reused
Kernel needs a stable storage location for the message




When shell gets exit message:

Not immediately after exit
During the wait() system call
When the parent process explicitly calls wait() to collect child process information
This means the message is retrieved as part of the process cleanup and information transfer


If exit message is longer than 32 characters:

Message will be truncated to 32 characters
safestrcpy() ensures only 32 characters (including null terminator) are copied
Prevents buffer overflow
Prevents potential security vulnerabilities


If exit message is shorter than 32 characters:

Remaining space is filled with null bytes
memset() or safestrcpy() ensures this
Prevents reading uninitialized memory
Guarantees the message is null-terminated


Exit message copies:

Copied at least twice:

From user space to kernel space (argstr())
From system call handler to process PCB (safestrcpy())


Potentially a third time when copied to parent process during wait()


In sh.c, exit message is received in the main() function:
int status;
char exit_msg[32];
wait(&status, exit_msg);

// If message is not empty, print it
if(exit_msg[0] != '\0') {
  printf("Exit message: %s\n", exit_msg);
}

Creates a buffer for exit message
Calls wait() with status and message buffer
wait() fills the buffer with the child's exit message

If shell modifies exit message after receiving:

No impact on the original process
Message has already been copied
Modification only affects the shell's local copy
Original message in kernel remains unchanged
*/