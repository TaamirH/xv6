#include "../kernel/types.h"
#include "user.h"

int main (){
    printf("Initial memsize: %d\n", memsize());
    char *buffer =malloc (20000);
    if (buffer == 0){
        printf("malloc failed\n");
        exit(1,"");
    }
    printf("After malloc: %d\n", memsize());
    free(buffer);
    printf("After free: %d\n", memsize());
    exit(0,"");
}

/*1.How does calling a system call differ from calling a regular function?
How does it work?
answer : Calling a system call is different from calling a regular function because it requires a context switch from user space to kernel
space. When a system call is made, the user program executes a trap instruction, which causes the processor to switch to kernel mode
and start executing the system call handler. The system call handler then performs the requestedoperation and 
returns the result to the user program before switching back to user mode.

2. How are parameters passed to the system call function? How is the
return value returned to userspace?
answer: Parameters are passed to the system call function using the a0-a6 registers in the trapframe structure.
The system call handler reads the parameters from these registers and performs the requested operation.
The return value is stored in the a0 register of the trapframe structure before switching back to user mode.

3. What is the purpose of the usys.pl file?
answer: The usys.pl file is used to generate the user.h file,
which contains the declarations of the system call functions.
The usys.pl file contains a list of system call names and their corresponding numbers,
 which are used to generate the system call function declarations in the user.h file.

4. What is struct proc and where is it defined? Why do we need it?
Does a real-world operating system have a similar structure?
answer: struct proc is a data structure that represents a process in the xv6 operating system.
It is defined in the proc.h header file and contains information about the process,
such as its process ID, state, stack size, and page table.
We need the struct proc data structure to keep track of the state of each process in the system,
including its execution context, memory layout, and other relevant information.
Real-world operating systems have similar data structures to represent processes,
although they may contain additional fields and information depending on the operating system design.

5. How much memory does our program use before and after the allocation?
answer : 16384 before , 81920 after;
6. What is the difference between the memory size before and after
the release?
answer : 81920 - 16384 = 65536
7. Try to explain the difference before and after release. What could
cause this difference? (Advanced: look at the implementation of
malloc and free).
answer : this is because malloc allocates memory from the heap in chunks of a certain size and has some metadata in additional,
and the memory allocated by malloc is not released immediately when free is called.
Instead, the memory is returned to the free list and can be reused by other procs but does not return to the
operating system,its still reserved for the process.
*/