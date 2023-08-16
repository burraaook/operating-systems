
# General
- Implementing operating system which has POSIX system calls, and microkernel structure on top of Victor Engelmann's make your own operating system tutorial.  


# installation
```
make mykernel.iso
```
  
- after this it must be installed to the virtualbox vm as Engelmann explained.  
  
# compilation and run
```
make run
```   
  
# test macros

## in kernel.cpp
MICROKERNEL1              -> test strategy 1  
MICROKERNEL2              -> test strategy 2  
MICROKERNEL3              -> test strategy 3  
MULTIPROGRAMMING_TEST     -> test multiprogramming, scheduling, and round robin  
FORK_TEST                 -> test fork(incomplete)  
EXECVE_TEST               -> test execve  
GETPID_TEST               -> test getpid  
PRINT_MODE                -> enable printing binary search, linear search, and collatz sequence  

## in multitasking.cpp  
SWITCH_PRINT_MODE         -> enable printing context switch  
PT_PRINT_MODE             -> enable printing process table  
  

- Implementation details in the doc.pdf file.  
