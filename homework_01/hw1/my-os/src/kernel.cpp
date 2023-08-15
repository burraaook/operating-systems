
#include <common/types.h>
#include <gdt.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>
#include <syscalls.h>

#define MICROKERNEL1
// #define MICROKERNEL2
// #define MICROKERNEL3

// #define MULTIPROGRAMMING_TEST
// #define FORK_TEST
// #define EXECVE_TEST
// #define GETPID_TEST

// #define PRINT_MODE

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;

GlobalDescriptorTable gdt;

TaskManager taskManager;
InterruptManager interrupts(0x20, &gdt, &taskManager);
SyscallHandler syscalls(&interrupts, 0x80);


void enterCritical();
void exitCritical();

void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}


uint32_t fork(CPUState* cpu)
{
    printf("sysfork() called\n");
    taskManager.ForkProcess(&gdt, cpu);
    return (uint32_t) cpu;
}

void execve(CPUState* cpu)
{
    printf("execve() called\n");
    taskManager.ExecProcess(cpu);
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}

void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex(key & 0xFF);
}

void printfHex32(uint32_t key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex(key & 0xFF);
}

void printfChar(char c)
{
    char* foo = " ";
    foo[0] = c;
    printf(foo);
}

// print a decimal number
void printfDec(int num)
{
    if (num == 0)
    {
        printf("0");
        return;
    }

    char buffer[32];
    int i = 0;
    bool negativeFlag = false;

    if (num < 0)
    {
        num = -num;
        negativeFlag = true;
    }

    while (num != 0)
    {
        // convert number to string
        buffer[i] = "0123456789"[num % 10];
        num /= 10;
        i++;
    }

    if (negativeFlag)
    {
        // put a negative sign to end of the string
        buffer[i] = '-';
        i++;
    }

    buffer[i] = 0;

    char temp;
    
    // set cursors to beginning and end of the string
    int j = 0;
    i--;

    // reverse the string
    while (j < i)
    {
        temp = buffer[j];
        buffer[j++] = buffer[i];
        buffer[i--] = temp;
    }

    printf(buffer);
}

void enterCritical()
{
    interrupts.Deactivate();
}

void exitCritical()
{
    interrupts.Activate();
}

class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }
    
};

// write system call
void sysprintf(char* str)
{
    asm("int $0x80" : : "a" (4), "b" (str));
}

// fork() system call
int sysfork()
{
    int ret;
    asm("int $0x80" : "=a" (ret) : "a" (2));
    asm volatile("int $0x20");
    return ret;
}

// execve() system call
void sysexecve(void entry())
{
    asm("int $0x80" : : "a" (11), "b" (entry));
}

// getpid() system call, no interrupt for this one
int sysgetpid()
{
    int ret;

    enterCritical();
    ret = taskManager.GetCurrentPid();
    exitCritical();

    return ret;
}

// exit() system call
void sysexit(int* ret)
{
    taskManager.TerminateProcess(ret);
}

void InitProcess() {
    printf("Init process started\n");
    while (true);
}

void taskC()
{
    int pid;
    pid = sysgetpid();
    printf("TASKC\n");
    printf("PID: ");
    printfHex32(pid);
    printf("\n");

    while(true);
}

void taskB()
{
    int pid;
    pid = sysgetpid();
    printf("TASKB\n");
    printf("PID: ");
    printfHex32(pid);
    printf("\n");

    while(true);
}

void taskA()
{
    int pid;
    pid = sysgetpid();
    printf("TASKA\n");
    printf("PID: ");
    printfHex32(pid);
    printf("\n");

    while(true);
}

void execHere()
{
    int ret = 0;
    printf("success!\n");

    sysexit(&ret);
    // while(true);
}
void taskExecve()
{
    printf("old\n");
    sysexecve(&execHere);
    printf("here\n");
    while(true);
}

void taskFork()
{
    int pid = sysfork();

    if(pid == 0) {
            printf("\nID: ");
            printfHex32(pid);
            printf(" CHILD\n");
    }
    else {
            printf("\nID: ");
            printfHex32(taskManager.GetCurrentPid());
            printf(" PARENT\n");
    }


    sysexit(&pid);
    while(true);
}


void binarySearch ()
{   
    int arr[] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
    int size = 10;
    int target = 100;

    int low = 0;
    int high = size - 1;
    int mid;
    int fail = -1;
    while(low <= high)
    {
        mid = (low + high) / 2;
        if (arr[mid] == target) {

            #ifdef PRINT_MODE
                printf("ID: ");
                printfHex32(taskManager.GetCurrentPid());
                printf(" OUTPUT: ");
                printfDec(mid);
                printf("\n");
            #endif
            sysexit(&mid);
        }
            
        else if (arr[mid] < target)
            low = mid + 1;
        else
            high = mid - 1;
    }

    #ifdef PRINT_MODE
        printf("ID: ");
        printfHex32(taskManager.GetCurrentPid());
        printf(" OUTPUT: ");
        printfDec(fail);
        printf("\n");
    #endif

    sysexit(&fail);

 //   while(true);
}

void linearSearch ()
{
    int arr[] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
    int target = 175;
    int size = 10;
    int fail = -1;


    for (int i = 0; i < size; i++) {
        if (arr[i] == target) {
            
            #ifdef PRINT_MODE            
                printf("ID: ");
                printfHex32(taskManager.GetCurrentPid());
                printf(" OUTPUT: ");
                printfDec(i);
                printf("\n");
            #endif
            sysexit(&i);
        }
    }

    #ifdef PRINT_MODE
        printf("ID: ");
        printfHex32(taskManager.GetCurrentPid());
        printf(" OUTPUT: ");
        printfDec(fail);
        printf("\n");
    #endif

    sysexit(&fail);

   // while(true);
}

void collatz ()
{
    // 2d array to store the sequence
    int sequence[25][50];
    
    // initialize all elements to 0
    for (int i = 0; i < 25; i++) 
        for (int j = 0; j < 50; j++) 
            sequence[i][j] = 0;

    // 1's sequence is 1
    sequence[0][0] = 1;

    int success;

    for (int i = 0; i < 25; i++) {
        int num = i + 1;

        int j = 0;

        // calculate till num becomes 1
        while (num != 1) 
        {
            // if it is even
            if (num % 2 == 0)
                num = num / 2;
            else
                num = 3 * num + 1;

            sequence[i][j] = num;
            j++;
        }
    }

    #ifdef PRINT_MODE
        enterCritical();
        for (int i = 0; i < 25; i++) {
            int j = 0;
            printfDec(i + 1);
            printf(": ");
            while (sequence[i][j] != 0) {
                printfDec(sequence[i][j]);
                printf(" ");
                j++;
            }
            printf("\n");
        }
        exitCritical();
    #endif
    
    sysexit(&success);
}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

void init()
{
    Task task_init(&gdt, &InitProcess);
    // first strategy, load 3 processes
    #ifdef MICROKERNEL1
        Task task_binarySearch(&gdt, &binarySearch);
        Task task_linearSearch(&gdt, &linearSearch);
        Task task_collatz(&gdt, &collatz);

        taskManager.AddTask(&task_init);
        taskManager.AddTask(&task_linearSearch);
        taskManager.AddTask(&task_binarySearch);
        taskManager.AddTask(&task_collatz);
    #endif

    // second strategy, pick one process randomly, and load it 10 times
    #ifdef MICROKERNEL2
        // array of processes
        Task tasks[] = {Task(&gdt), Task(&gdt), Task(&gdt), Task(&gdt), 
                        Task(&gdt), Task(&gdt), Task(&gdt), Task(&gdt), 
                        Task(&gdt), Task(&gdt)}; 

        uint32_t time;

        // get the time
        asm volatile("rdtsc" : "=a" (time) : : "edx");
        
        if (time < 0)
            time = -time;

        // pick a random process
        uint32_t random = time % 3;

        // load the process 10 times
        for (int i = 0; i < 10; i++) {
            if (random == 0)
                tasks[i].setEntryPoint(&binarySearch);
            else if (random == 1)
                tasks[i].setEntryPoint(&linearSearch);
            else if (random == 2)
                tasks[i].setEntryPoint(&collatz);
        }

        // add the processes to the task manager
        taskManager.AddTask(&task_init);
        for (int i = 0; i < 10; i++) {
            taskManager.AddTask(&tasks[i]);
        }
    #endif

    // third strategy, pick 2 out of 3 randomly, and load each one 3 times
    #ifdef MICROKERNEL3
    
            // array of processes
            Task tasks[] = {Task(&gdt), Task(&gdt), Task(&gdt),
                            Task(&gdt), Task(&gdt), Task(&gdt)};
            
            uint32_t time;
            // get the time
            asm volatile("rdtsc" : "=a" (time) : : "edx");
            
            if (time < 0)
                time = -time;
            
            // pick 2 random processes
            uint32_t random1 = time % 3;
            uint32_t random2 = (random1+1) % 3;

            // load the processes 3 times
            for (int i = 0; i < 3; i++) {
                if (random1 == 0)
                    tasks[i].setEntryPoint(&binarySearch);
                else if (random1 == 1)
                    tasks[i].setEntryPoint(&linearSearch);
                else if (random1 == 2)
                    tasks[i].setEntryPoint(&collatz);
            }
    
            for (int i = 3; i < 6; i++) {
                if (random2 == 0)
                    tasks[i].setEntryPoint(&binarySearch);
                else if (random2 == 1)
                    tasks[i].setEntryPoint(&linearSearch);
                else if (random2 == 2)
                    tasks[i].setEntryPoint(&collatz);
            }
    
            // add processes through the task manager
            taskManager.AddTask(&task_init);
            for (int i = 0; i < 6; i++) {
                taskManager.AddTask(&tasks[i]);
            }            
    #endif

    #ifdef FORK_TEST
        Task task_fork(&gdt, &taskFork);
        taskManager.AddTask(&task_init);
        taskManager.AddTask(&task_fork);
    #endif

    #ifdef EXECVE_TEST
        Task task_exec(&gdt, &taskExecve);
        taskManager.AddTask(&task_init);
        taskManager.AddTask(&task_exec);
    #endif

    #ifdef GETPID_TEST
        Task task_a(&gdt, &taskA);
        Task task_b(&gdt, &taskB);
        
        taskManager.AddTask(&task_init);
        taskManager.AddTask(&task_a);
        taskManager.AddTask(&task_b);
    #endif

    #ifdef MULTIPROGRAMMING_TEST
        Task task_a(&gdt, &taskA);
        Task task_b(&gdt, &taskB);
        Task task_c(&gdt, &taskC);

        taskManager.AddTask(&task_init);
        taskManager.AddTask(&task_a);
        taskManager.AddTask(&task_b);
        taskManager.AddTask(&task_c);
    #endif
}

extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("hello\n");

    
    init();

    printf("Initializing Hardware, Stage 1\n");        
    DriverManager drvManager;

    PrintfKeyboardEventHandler kbhandler;
    KeyboardDriver keyboard(&interrupts, &kbhandler);
    drvManager.AddDriver(&keyboard);
    
    MouseToConsole mousehandler;
    MouseDriver mouse(&interrupts, &mousehandler);
    drvManager.AddDriver(&mouse);
    
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&drvManager, &interrupts);
        
    printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();

    interrupts.Activate();
    
    while(1);
}
