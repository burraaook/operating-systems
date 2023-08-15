
#include <syscalls.h>



using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;

SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
:    InterruptHandler(interruptManager, InterruptNumber  + interruptManager->HardwareInterruptOffset())
{
}

SyscallHandler::~SyscallHandler()
{
}


void printf(char*);
uint32_t fork(CPUState* cpu);
void execve(CPUState* cpu);

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    
    switch(cpu->eax)
    {   
        case 1:
        // fork()
        case 2:
            fork(cpu);
            //cpu->eax = 0;
            break;

        // waitpid()
        case 7:
            // implement waitpid

            break;

        // execve()
        case 11:
            // implement execve
            execve(cpu);
            cpu->eax = 0;
            cpu->eip = cpu->ebx;
            esp = (uint32_t)cpu; 
            break;
        case 4:
            printf((char*)cpu->ebx);
            break;
            
        default:
            break;
    }

    return esp;
}
