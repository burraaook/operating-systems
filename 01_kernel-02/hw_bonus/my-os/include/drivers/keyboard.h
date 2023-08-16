
#ifndef __MYOS__DRIVERS__KEYBOARD_H
#define __MYOS__DRIVERS__KEYBOARD_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>

namespace myos
{
    namespace drivers
    {
    
        class KeyboardEventHandler
        {
        public:
            KeyboardEventHandler();

            virtual void OnKeyDown(char);
            virtual void OnKeyUp(char);
        };
        
        class KeyboardDriver : public myos::hardwarecommunication::InterruptHandler, public Driver
        {
            myos::hardwarecommunication::Port8Bit dataport;
            myos::hardwarecommunication::Port8Bit commandport;
            
            KeyboardEventHandler* handler;

        protected:
            char keyBuffer[256];
            myos::common::uint32_t keyBufferIndex;
            myos::common::uint32_t bytesToRead; 
        public:
            KeyboardDriver(myos::hardwarecommunication::InterruptManager* manager, KeyboardEventHandler *handler);
            ~KeyboardDriver();
            virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
            virtual void Activate();


            char ReadBuffer(myos::common::uint32_t index);
            void ResetBuffer();
            void SetReadBytes(myos::common::uint32_t bytes);
        };

    }
}
    
#endif