# Second PROJECT

# ARP-Assignment2 Submitted for the 14th February exam of ARP

This project is using shared memory, which is a type of inter-process communication (IPC) mechanism in operating systems, which allows multiple processes to access a common region of memory. This provides a way for processes to exchange data and synchronize their activities without requiring the overhead of system calls or network communication.

In shared memory, a portion of system memory is set aside as a shared region, which can be accessed by multiple processes. Each process can read and write to this region, and changes made by one process are immediately visible to all other processes accessing the same shared memory. The operating system is responsible for managing access to the shared memory and ensuring that data consistency is maintained, even in the presence of concurrent access.

Shared memory is a fast IPC mechanism because it avoids the overhead of system calls or network communication. However, it also requires coordination between processes to ensure that data consistency is maintained, which can be complex and error-prone. Additionally, shared memory can lead to race conditions and other synchronization problems if not used correctly.

Overall, shared memory is an important concept in operating systems and is widely used in a variety of applications, including database management systems, multi-threaded programs, and scientific simulations.

Base repository for the second ARP assignment.
The two processes involved in the simulation of the vision system, namely processA and processB, are implemented as simple ncurses windows. The development of the inter-process communication pipeline, that is the shared memory.

You also find a master process, responsible of spawning the entire simulation.


## PA
The code creates a shared memory object, a bitmap, and initializes a console UI. It enters an infinite loop where it waits for user input. If the user resizes the screen, the UI is redrawn. If the user presses the print button, a bitmap image is saved and the print_flag is set to true. If the user presses an arrow key, the position of a circle on the bitmap is changed and the bitmap is recreated. The update_shared_memory() function updates the shared memory object with the green value of each pixel in the bitmap. The program ends when the user closes the console UI.
     

## PB


## Compiling and running **processA** and **processB**
The two processes have been implemented as UIs using the ncurses library. Therefore, it is necessary to compile their source files by linking the shared library through -lncurses. Similar to the first assignment, use the resize event of the windows to handle situations where the graphical elements do not spawn correctly.


## How Run Code**
```
clear
gcc ./src/master.c -lm -lrt  -o ./bin/master
gcc ./src/processA.c -lbmp -lncurses -lm -lrt -o ./bin/processA
gcc ./src/processB.c -lbmp -lncurses -lm -lrt -o ./bin/processB
./bin/master
```



