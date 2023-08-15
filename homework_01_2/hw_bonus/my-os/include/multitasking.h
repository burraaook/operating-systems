 
#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H

#include <common/types.h>
#include <gdt.h>

namespace myos
{
    struct CPUState
    {
        common::uint32_t eax;
        common::uint32_t ebx;
        common::uint32_t ecx;
        common::uint32_t edx;

        common::uint32_t esi;
        common::uint32_t edi;
        common::uint32_t ebp;

        common::uint32_t error;

        common::uint32_t eip;
        common::uint32_t cs;
        common::uint32_t eflags;
        common::uint32_t esp;
        common::uint32_t ss;        
    } __attribute__((packed));
    
    
    class Task
    {
    friend class TaskManager;
    private:
        common::uint8_t stack[4096]; // 4 KiB
        CPUState* cpustate;

        // process ID
        common::uint32_t pid;
    public:
        Task(GlobalDescriptorTable *gdt, void entrypoint());

        Task(GlobalDescriptorTable *gdt);

        Task();
        void setEntryPoint(void entrypoint());
        ~Task();

        // get entry point
        void* GetEntryPoint();

        // set the task
        void Set(Task* task);
    };
    
    // process states
    enum ProcessState {
        RUNNING,
        BLOCKED,
        READY,
        TERMINATED
    };

    // process class
    class Process {
    
    friend class TaskManager;
    
    private:
        // process ID
        common::uint32_t pid;

        // parent process ID
        common::uint32_t ppid;

        // process waiting for
        common::uint32_t waitPid;
    
        // child processes
        common::uint32_t children[256];

        // return value
        int* retval;

        // process state
        ProcessState state;

        // number of children
        common::uint32_t numChildren;
    public:
        Process(common::uint32_t pid, common::uint32_t ppid);
        Process();
        ~Process();

        // Add child process
        void AddChild(common::uint32_t pid);

        // Remove child process
        void RemoveChild(common::uint32_t pid);

        bool IsChild(common::uint32_t pid);

        // getters
        common::uint32_t GetPid();
        common::uint32_t GetPpid();
        int* GetReturnValue();
        ProcessState GetState();
        common::uint32_t GetNumChildren();

        // setters
        void SetPid(common::uint32_t pid);
        void SetPpid(common::uint32_t ppid);
        void SetReturnValue(int* retval);
        void SetState(ProcessState state);
        void SetNumChildren(common::uint32_t numChildren);

    };

    // holds space for all processes
    // pid = 0 means inactive process
    struct ProcessTable
    {
        Process processes[256];
        int numProcesses;
    };

    class TaskManager
    {
    private:
        Task tasks[256];
        int numTasks;
        int currentTask;

        // number of terminated tasks
        int terminatedTask;

        // process table
        ProcessTable processTable;
    public:
        TaskManager(GlobalDescriptorTable *gdt);
        ~TaskManager();
        bool AddTask(Task* task);
        
        CPUState* Schedule(CPUState* cpustate);
        
        void PrintProcessTable();

        // take process to terminated state
        void TerminateProcess(int* returnVal);

        common::uint32_t GetCurrentPid();

        // system call implementations
        common::uint32_t ForkProcess(CPUState* cpustate);
        common::uint32_t ForkProcess2(CPUState* cpustate);
        void ExecProcess(CPUState* cpustate);
        void WaitProcess(CPUState* cpustate);

    };
}


#endif