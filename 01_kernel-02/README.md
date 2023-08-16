# General
- Implementing operating system which has POSIX system calls, and microkernel structure on top of Victor Engelmann's make your own operating system tutorial. Additionally, previous implementation is improved, and additional features are added when mouse and keyboard interrupts happen.  


# installation
` make mykernel.iso `

- after this it must be installed to the virtualbox vm as Engelmann explained.

# compilation and run
` make run `

# test macros

## in kernel.cpp
MICROKERNEL1              -> test strategy 1  
MICROKERNEL2              -> test strategy 2  
MICROKERNEL3              -> test strategy 3  
MULTIPROGRAMMING_TEST     -> test multiprogramming, scheduling, and round robin  
FORK_TEST                 -> test fork  
EXECVE_TEST               -> test execve  
GETPID_TEST               -> test getpid  
WAITPID_TEST              -> test waitpid  
PRINT_MODE                -> enable printing binary search, linear search, and collatz sequence  
  
## in multitasking.cpp
SWITCH_PRINT_MODE         -> enable printing context switch  
PT_PRINT_MODE             -> enable printing process table  
  
- They are all explained in detail in the report. (5- Test Cases and Results section)  
- By default MICROKERNEL1, SWITCH_PRINT_MODE, and PT_PRINT_MODE are enabled.  
- In case you cannot compile, iso files for microkernel1, microkernel2, microkernel3 are provided.  