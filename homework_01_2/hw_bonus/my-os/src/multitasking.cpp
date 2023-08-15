
#include <multitasking.h>

using namespace myos;
using namespace myos::common;

void printf(char* str);
void printfHex32(uint32_t);
void printfHex16(uint16_t);
void printfDec(int);

// default return value
int default_val = 0;

#define SWITCH_PRINT_MODE
#define PT_PRINT_MODE


Task::Task(GlobalDescriptorTable *gdt, void entrypoint())
{
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    
    cpustate -> eax = 0;
    cpustate -> ebx = 0;
    cpustate -> ecx = 0;
    cpustate -> edx = 0;

    cpustate -> esi = 0;
    cpustate -> edi = 0;
    cpustate -> ebp = 0;
    
    cpustate -> eip = (uint32_t)entrypoint;
    cpustate -> cs = gdt->CodeSegmentSelector();

    cpustate -> eflags = 0x202;

    pid = 0;
}

// create uninitialized tasks
Task::Task(GlobalDescriptorTable *gdt)
{
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    
    cpustate -> eax = 0;
    cpustate -> ebx = 0;
    cpustate -> ecx = 0;
    cpustate -> edx = 0;

    cpustate -> esi = 0;
    cpustate -> edi = 0;
    cpustate -> ebp = 0;
    
    cpustate -> eip = 0;
    cpustate -> cs = gdt->CodeSegmentSelector();

    cpustate -> eflags = 0x202;

    pid = 0;
}

Task::Task()
{
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    
    cpustate -> eax = 0;
    cpustate -> ebx = 0;
    cpustate -> ecx = 0;
    cpustate -> edx = 0;

    cpustate -> esi = 0;
    cpustate -> edi = 0;
    cpustate -> ebp = 0;
    
    cpustate -> eip = 0;
    cpustate -> cs = 0;

    cpustate -> eflags = 0x202;

    // pid = 0 means that task is not initialized
    pid = 0;
}

Task::~Task()
{
}

// setter for task
void Task::Set(Task* task)
{
    cpustate->eax = task->cpustate->eax;
    cpustate->ebx = task->cpustate->ebx;
    cpustate->ecx = task->cpustate->ecx;
    cpustate->edx = task->cpustate->edx;

    cpustate->esi = task->cpustate->esi;
    cpustate->edi = task->cpustate->edi;
    cpustate->ebp = task->cpustate->ebp;

    cpustate->eip = task->cpustate->eip;
    cpustate->cs = task->cpustate->cs;
    cpustate->eflags = task->cpustate->eflags;

    pid = task->pid;
}

// setter for entry point of task
void Task::setEntryPoint(void entrypoint())
{
    cpustate -> eip = (uint32_t)entrypoint;
}

void* Task::GetEntryPoint()
{
    return (void*)cpustate->eip;
}

TaskManager::TaskManager(GlobalDescriptorTable *gdt)
{
    numTasks = 0;
    currentTask = -1;
    terminatedTask = 0;
    processTable.numProcesses = 0;
    for(int i = 0; i < 256; i++)
    {
        tasks[i].cpustate -> cs = gdt->CodeSegmentSelector();
    }

}


TaskManager::~TaskManager()
{
}

/*
* Add task to task manager
* Update process table and process information
*/
bool TaskManager::AddTask(Task* task)
{
    if(numTasks >= 256)
        return false;

    // assign minimum possible pid
    for (int i = 1; i < 256; i++)
    {
        if (processTable.processes[i].GetPid() == 0)
        {
            task->pid = i;
            break;
        }
    }

    // get process from process table
    Process* process = &(processTable.processes[task->pid]);
    
    // init is parent of all processes
    process->SetPid(task->pid);
    process->SetPpid(1);
    process->SetState(READY);
    process->SetNumChildren(0);

    // if not init, add to parent's children
    if (task->pid != 1)
    {
        Process* parent = &(processTable.processes[process->GetPpid()]);
        parent->AddChild(task->pid);
    }

    tasks[numTasks++].Set(task);
    return true;
}

/*
* Print process table
*/
void TaskManager::PrintProcessTable()
{
    printf("\nProcess table:\n");
    printf("\nTotal tasks: ");
    printfDec(numTasks - terminatedTask);
    printf("\n");
    for (int i = 0; i < 256; i++)
    {
        if (processTable.processes[i].GetPid() != 0)
        {
            printf("PID: ");
            printfDec(processTable.processes[i].GetPid());
            printf(" PPID: ");
            printfDec(processTable.processes[i].GetPpid());
            printf(" State: ");
            switch (processTable.processes[i].GetState())
            {
                case READY:
                    printf("READY");
                    break;
                case RUNNING:
                    printf("RUNNING");
                    break;
                case TERMINATED:
                    printf("TERMINATED");
                    break;
                case BLOCKED:
                    printf("BLOCKED");
                    break;
                default:
                    printf("UNKNOWN");
                    break;
            }
            printf(" Return Value: ");
            int* val = processTable.processes[i].GetReturnValue();
            if (*val < 0) 
            {
                printf("-");
                printfDec(*val * -1);
            }
            else
                printfDec(*val);
            printf(" Num Children: ");
            printfDec(processTable.processes[i].GetNumChildren());
            printf("\n");
        }
    }
    printf("\n");
}

/*
* Terminate current process
* Change its state to terminated
*/
void TaskManager::TerminateProcess(int* returnVal)
{
    // if it is init, do nothing
    if (tasks[currentTask].pid == 1)
        return;
    
    // if it is not init, set return value
    Process* process = &(processTable.processes[tasks[currentTask].pid]);
    process->SetReturnValue(returnVal);

    // set state to terminated
    process->SetState(TERMINATED);
    terminatedTask++;

    // // print termination message
    // printf("\nProcess: ");
    // printfDec(process->GetPid());
    // printf(" terminated with return value: ");
    // if (*returnVal < 0) 
    // {
    //     printf("-");
    //     printfDec(*returnVal * -1);
    // }
    // else
    //     printfDec(*returnVal);
    // printf("\n");

    while(true);
}

/*
* Schedule next task using round robin
* Terminated tasks are not scheduled
*/
CPUState* TaskManager::Schedule(CPUState* cpustate)
{

    // if there is no task, return
    if(numTasks - terminatedTask <= 0)
        return cpustate;

    // set current task's cpu state
    if(currentTask >= 0)
        tasks[currentTask].cpustate = cpustate;

    bool printFlag = false;

    #ifdef SWITCH_PRINT_MODE
        if (numTasks - terminatedTask > 1 && currentTask >= 0)
        {
            printf("Switching task from ");
            printfDec(tasks[currentTask].pid);
            printFlag = true;
        }
    #endif

    // current task is on the ready state
    if (tasks[currentTask].pid != 0)
    {
        Process* process = &(processTable.processes[tasks[currentTask].pid]);
        if (process->GetState() == RUNNING)
            process->SetState(READY);
    }

    ProcessState state = TERMINATED;
    // next task shouldn't be terminated
    do
    {   
        // round robin till next task is not terminated
        if(++currentTask >= numTasks)
            currentTask %= numTasks;

        // if it is blocked, check if waiting process is terminated
        if (processTable.processes[tasks[currentTask].pid].GetState() == BLOCKED)
        {
            Process* process = &(processTable.processes[tasks[currentTask].pid]);
            if (processTable.processes[process->waitPid].GetState() == TERMINATED)
            {
                process->SetState(READY);
                process->waitPid = 0;
            }
        }

        state = processTable.processes[tasks[currentTask].pid].GetState();
    } while (state == TERMINATED || state == BLOCKED);

    // print switching message
    #ifdef SWITCH_PRINT_MODE
    if (numTasks - terminatedTask > 1 && currentTask >= 0 && printFlag)
    {
        printf(" to ");
        printfDec(tasks[currentTask].pid);
        printf("\n");
        printFlag = false;
    }
    #endif

    // current task is on the running state
    Process* process = &(processTable.processes[tasks[currentTask].pid]);
    if(process->GetState() == READY)
        process->SetState(RUNNING);

    // print current process table
    #ifdef PT_PRINT_MODE
            PrintProcessTable();
    #endif
    return tasks[currentTask].cpustate;
}

common::uint32_t TaskManager::GetCurrentPid()
{
    return tasks[currentTask].pid;
}

common::uint32_t TaskManager::ForkProcess(CPUState* cpustate)
{
    // if memory is full, return
    if (numTasks >= 256)
        return 0;

    for (int i = 0; i < sizeof(tasks[currentTask].stack); i++)
    {
        tasks[numTasks].stack[i] = tasks[currentTask].stack[i];
    }
    tasks[numTasks].cpustate = (CPUState*)(tasks[numTasks].stack + 4096 - sizeof(CPUState));
    common::uint32_t currentTaskOffset = ((common::uint32_t)tasks[currentTask].cpustate) - ((common::uint32_t)cpustate);
    tasks[numTasks].cpustate = (CPUState*)(((common::uint32_t)tasks[numTasks].cpustate) - currentTaskOffset);
    // tasks[numTasks].cpustate-> cs = cpustate -> cs;

    // assign minimum possible pid
    for (int i = tasks[currentTask].pid + 1; i < 256; i++)
    {
        if (processTable.processes[i].GetPid() == 0)
        {
            tasks[numTasks].pid = i;
            break;
        }
    }

    tasks[numTasks].cpustate -> ecx = 0;

    // add child to parent
    Process* parent = &(processTable.processes[tasks[currentTask].pid]);
    parent->AddChild(tasks[numTasks].pid);

    // add parent to child
    Process* child = &(processTable.processes[tasks[numTasks].pid]);
    child->SetPpid(tasks[currentTask].pid);

    // add process to process table
    Process* process = &(processTable.processes[tasks[numTasks].pid]);
    process->SetPid(tasks[numTasks].pid);
    process->SetPpid(tasks[currentTask].pid);
    process->SetState(READY);

    numTasks++;

    // return child pid
    return tasks[numTasks-1].pid;
}

common::uint32_t TaskManager::ForkProcess2(CPUState* cpustate)
{
    // if memory is full, return
    if (numTasks >= 256)
        return 0;

    tasks[numTasks].cpustate->eip = cpustate->ebx;

    // assign minimum possible pid
    for (int i = tasks[currentTask].pid + 1; i < 256; i++)
    {
        if (processTable.processes[i].GetPid() == 0)
        {
            tasks[numTasks].pid = i;
            break;
        }
    }

    // add child to parent
    Process* parent = &(processTable.processes[tasks[currentTask].pid]);
    parent->AddChild(tasks[numTasks].pid);

    // add parent to child
    Process* child = &(processTable.processes[tasks[numTasks].pid]);
    child->SetPpid(tasks[currentTask].pid);

    // add process to process table
    Process* process = &(processTable.processes[tasks[numTasks].pid]);
    process->SetPid(tasks[numTasks].pid);
    process->SetPpid(tasks[currentTask].pid);
    process->SetState(READY);

    numTasks++;

    return tasks[numTasks-1].pid;
}

void TaskManager::WaitProcess(CPUState* cpustate)
{
    common::uint32_t pid = cpustate->ebx;

    // self waiting is not allowed
    if (tasks[currentTask].pid == cpustate->ebx || pid == 0 || pid == 1)
        return;

    // cannot wait for a process that is not a child
    if (!processTable.processes[tasks[currentTask].pid].IsChild(pid))
        return;

    Process* process = &(processTable.processes[tasks[currentTask].pid]);
    process->SetState(BLOCKED);
    process->waitPid = pid;
}

/*
* system call with function pointer
* gets the entry point from ebx register
*/
void TaskManager::ExecProcess(CPUState* cpustate)
{
    tasks[currentTask].cpustate = cpustate;
    tasks[currentTask].cpustate -> eip = cpustate -> ebx;

    // set process state to ready
    Process* process = &(processTable.processes[tasks[currentTask].pid]);
    process->SetState(READY);
}

Process::Process()
{
    this->pid = 0;
    this->ppid = 1;
    this->state = TERMINATED;
    this->retval = &default_val;
    this->waitPid = 0;
    // initialize children array
    for (int i = 0; i < 256; i++)
        this->children[i] = 0;
}

// create a new process with pid and ppid
Process::Process(common::uint32_t pid, common::uint32_t ppid) 
{
    this->pid = pid;
    this->ppid = ppid;
    this->state = READY;
    this->retval = &default_val;
    this->waitPid = 0;
    // initialize children array
    for (int i = 0; i < 256; i++)
        this->children[i] = 0;
}

Process::~Process() 
{
}

void Process::AddChild(uint32_t pid) 
{
    
    if (this->numChildren >= 256 || pid >= 256 || pid < 0) {
        return;
    }

    this->children[pid] = pid;
    this->numChildren++;
}

void Process::RemoveChild(uint32_t pid) 
{
    if (this->numChildren >= 256 || pid >= 256 || pid < 0) {
        return;
    }

    this->children[pid] = 0;
    if (this->numChildren > 0)
        this->numChildren--;
}

bool Process::IsChild(uint32_t pid) 
{
    if (this->numChildren >= 256 || pid >= 256 || pid < 0) {
        return false;
    }

    if (this->children[pid] == pid)
        return true;
    else
        return false;
}

common::uint32_t Process::GetPid() {
    return this->pid;
}

common::uint32_t Process::GetPpid() {
    return this->ppid;
}

common::uint32_t Process::GetNumChildren() {
    return this->numChildren;
}

ProcessState Process::GetState() {
    return this->state;
}

int* Process::GetReturnValue() {
    return this->retval;
}

void Process::SetPid(common::uint32_t pid) {
    this->pid = pid;
}

void Process::SetPpid(common::uint32_t ppid) {
    this->ppid = ppid;
}

void Process::SetNumChildren(common::uint32_t numChildren) {
    this->numChildren = numChildren;
}

void Process::SetState(ProcessState state) {
    this->state = state;
}

void Process::SetReturnValue(int* retval) {
    this->retval = retval;
}
