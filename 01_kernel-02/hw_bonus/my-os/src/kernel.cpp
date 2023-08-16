
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
// #define WAITPID_TEST

#define PRINT_MODE

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;

void printf(char* str);
bool myos::common::readIOFlag = false;
bool myos::common::criticalFlag = false;

class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        if (readIOFlag)
        {
            char* foo = " ";
            foo[0] = c;

            printf(foo);
            
        }

    }
};

GlobalDescriptorTable gdt;

TaskManager taskManager(&gdt);
InterruptManager interrupts(0x20, &gdt, &taskManager);
SyscallHandler syscalls(&interrupts, 0x80);
PrintfKeyboardEventHandler kbhandler;
KeyboardDriver keyboard(&interrupts, &kbhandler);


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
    // printf("sysfork() called\n");
    
    return taskManager.ForkProcess(cpu);
}

uint32_t fork2(CPUState* cpu)
{
    // printf("sysfork2() called\n");
    
    return taskManager.ForkProcess2(cpu);
}

void wait(CPUState* cpu)
{
    // printf("syswait() called\n");
    taskManager.WaitProcess(cpu);
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
    // interrupts.Deactivate();
    criticalFlag = true;
}

void exitCritical()
{
    // interrupts.Activate();
    criticalFlag = false;
}

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
void fork(int *pid)
{
    asm("int $0x80" : "=c" (*pid) : "a" (2));
}

// fork_exec() system call
int fork_exec(void entrypoint())
{
    int ret;
    asm("int $0x80" : "=a" (ret) : "a" (15), "b" ((uint32_t)entrypoint));
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

// waitpid() system call
void syswaitpid(uint32_t pid)
{
    asm("int $0x80" : : "a" (7), "b" (pid));
}

// exit() system call
void sysexit(int* ret)
{
    taskManager.TerminateProcess(ret);
}

// sysread() system call
void sysread(char* str, uint32_t size)
{
    keyboard.SetReadBytes(size);
    readIOFlag = true;

    // wait for keyboard interrupt to finish
    while(readIOFlag);

    // copy the string from keyboard buffer to str
    for (int i = 0; i < size; i++)
        str[i] = keyboard.ReadBuffer(i);

    str[size] = '\0';
    keyboard.ResetBuffer();

    readIOFlag = false;
}

bool readInt(int* num)
{
    char str[10];
    sysread(str, 10);
    *num = 0;
    for (int i = 0; i < 10; i++)
    {
        if (str[i] == '\0' || str[i] == '\n')
            break;

        if (str[i] < '0' || str[i] > '9')
            return false;

        *num *= 10;
        *num += str[i] - '0';
    }

    return true;
}

void taskRead()
{
    char str[10];
    while(true)
    {
        sysread(str, 10);
        printf(str);
        printf("\n");
    }
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

    while(true)
    {
        // printf("B");
    }
}

void taskA()
{
    int pid;
    pid = sysgetpid();
    printf("TASKA\n");
    printf("PID: ");
    printfHex32(pid);
    printf("\n");

    while(true)
    {
        // printf("A");
    }
}

void execHere()
{
    int ret = 0;
    printf("success!\n");

    sysexit(&ret);
}
void taskExecve()
{
    printf("old\n");
    sysexecve(&execHere);
    printf("here\n");


    while(true);
}

void taskD()
{
    int pid;

    pid = fork_exec(&execHere);
    syswaitpid(pid);

    int ret = 0;
    sysexit(&ret);
}

void taskWait()
{
    int pid;
    int ret = 0;
    pid = fork_exec(&taskD);
    syswaitpid(pid);

    sysexit(&ret);
}

void taskFork()
{
    int pid = 0;
    fork(&pid);

    if(pid == 0) {
            printf("\nID: ");
            printfHex32(pid);
            printf(" CHILD\n");
            sysexecve(&taskExecve);
    }
    else {
            printf("\nID: ");
            printfHex32(taskManager.GetCurrentPid());
            printf(" PARENT\n");
    }

    sysexit(&pid);
    // while(true);
}


void binarySearch ()
{   
    enterCritical();
    int arr[15];
    printf("\nPID: ");
    printfDec(sysgetpid());
    printf(": ");
    printf("bs - enter array size (max 15): ");
    int size;
    bool valid = readInt(&size);
    while(!valid || size > 15)
    {
        printf("\ninvalid input!\n");
        printf("enter array size (max 15): ");
        valid = readInt(&size);
    }

    for (int i = 0; i < size; i++)
    {
        printf("\nPID: ");
        printfDec(sysgetpid());
        printf(": ");
        printf("index ");
        printfDec(i);
        printf(": ");
        valid = readInt(&arr[i]);
        while(!valid)
        {
            printf("\ninvalid input!\n");
            printf("index: ");
            printfDec(i);
            printf(": ");
            valid = readInt(&arr[i]);
        }
    }

    printf("\nenter target: ");
    int target;
    valid = readInt(&target);
    while(!valid)
    {
        printf("\ninvalid input!\n");
        printf("enter target: ");
        valid = readInt(&target);
    }

    exitCritical();

    int low = 0;
    int high = size - 1;
    int mid;
    int fail = -1;

    while (low <= high) {
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
    enterCritical();
    int arr[15];
    printf("\nPID: ");
    printfDec(sysgetpid());
    printf(": ");
    printf("ls - enter array size (max 15): ");
    int size;
    bool valid = readInt(&size);
    while(!valid || size > 15)
    {
        printf("\ninvalid input!\n");
        printf("enter array size (max 15): ");
        valid = readInt(&size);
    }

    for (int i = 0; i < size; i++)
    {
        printf("\nPID: ");
        printfDec(sysgetpid());
        printf(": ");
        printf("index ");
        printfDec(i);
        printf(": ");
        valid = readInt(&arr[i]);
        while(!valid)
        {
            printf("\ninvalid input!\n");
            printf("index: ");
            printfDec(i);
            printf(": ");
            valid = readInt(&arr[i]);
        }
    }

    printf("\nenter target: ");
    int target;
    valid = readInt(&target);
    while(!valid)
    {
        printf("\ninvalid input!\n");
        printf("enter target: ");
        valid = readInt(&target);
    }

    exitCritical();
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

void collatzHelper(int n)
{
    bool printFlag = false;

    #ifdef PRINT_MODE
        printFlag = true;
    #endif

    if(printFlag) {
        printfDec(n);
        printf(": ");
    }

    while (n != 1) 
    {
        // if it is even
        if (n % 2 == 0)
            n = n / 2;
        else
            n = 3 * n + 1;

        if(printFlag) {
            printfDec(n);
            printf("  ");
        }
    }

    if(printFlag)
        printf("\n");
}

void collatz ()
{
    enterCritical();
    int success = 0;
    int num = 1;

    printf("\nPID: ");
    printfDec(sysgetpid());
    printf(": ");
    printf("enter number of collatz sequences(max 30): ");
    bool valid = readInt(&num);
    while(!valid || num > 30)
    {
        printf("\ninvalid input!\n");
        printf("enter number of collatz sequences(max 30): ");
        valid = readInt(&num);
    }
    exitCritical();

    for (int i = 0; i < num; i++)
        collatzHelper(i + 1);
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


void InitProcess() {
    printf("Init process started\n");
    
    // first strategy, load 3 processes
    #ifdef MICROKERNEL1
        void (*process[])() = {binarySearch, linearSearch, collatz};

        fork_exec(process[0]);
        fork_exec(process[1]);
        fork_exec(process[2]);
    #endif

    // second strategy, pick one process randomly, and load it 10 times
    #ifdef MICROKERNEL2
        void (*process[])() = {binarySearch, linearSearch, collatz};

        uint32_t time;

        // get the time
        asm volatile("rdtsc" : "=a" (time) : : "edx");
        
        if (time < 0)
            time = -time;

        int index = time % 3;

        for (int i = 0; i < 10; ++i)
            fork_exec(process[index]);
    #endif

    // third strategy, pick 2 out of 3 randomly, and load each one 3 times
    #ifdef MICROKERNEL3
    
        void (*process[])() = {binarySearch, linearSearch, collatz};

        uint32_t time;

        // get the time
        asm volatile("rdtsc" : "=a" (time) : : "edx");
        
        if (time < 0)
            time = -time;

        int index1 = time % 3;
        int index2 = (time + 1) % 3;

        for (int i = 0; i < 3; ++i) {
            fork_exec(process[index1]);
            fork_exec(process[index2]);
        }
    #endif

    #ifdef FORK_TEST
        fork_exec(&taskFork);
    #endif

    #ifdef EXECVE_TEST
        fork_exec(&taskExecve);
    #endif

    #ifdef GETPID_TEST
        fork_exec(&taskA);
        fork_exec(&taskB);
    #endif

    #ifdef MULTIPROGRAMMING_TEST
        fork_exec(&taskA);
        fork_exec(&taskB);
        fork_exec(&taskC);
    #endif

    #ifdef WAITPID_TEST
        int pid = fork_exec(&taskWait);
        syswaitpid(pid);
    #endif

    while (true);
}

void init()
{
    Task task_init(&gdt, &InitProcess);
    taskManager.AddTask(&task_init);
}



extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("hello\n");

    init();

    // printf("Initializing Hardware, Stage 1\n");        
    DriverManager drvManager;


    drvManager.AddDriver(&keyboard);
    
    MouseToConsole mousehandler;
    MouseDriver mouse(&interrupts, &mousehandler);
    drvManager.AddDriver(&mouse);
    
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&drvManager, &interrupts);
        
    // printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();

    interrupts.Activate();
    
    while(1);
}
