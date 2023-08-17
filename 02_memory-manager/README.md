
# General
- Simulating a virtual memory management system using various page replacement algorithms in C++. Testing the manager with vector multiplication and summation using threads. 


Compilation
```
make
```  

Run
```
./operateArrays <frame_size> <physical_frames> <virtual_frames> <pr_algorithm> <print_table_count> <disk_file_name>
```   
  
# Some implementation details
- Inputs are power of 2.
- Page replacement algorithms are Second Chance, Least Recently Used, and WSClock.
- MMU is used for address translation.
- Page frames that are not in the physical memory are stored in the disk file.
- Implementation details in the doc.pdf file.  
