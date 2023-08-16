
#include <drivers/keyboard.h>

using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;


KeyboardEventHandler::KeyboardEventHandler()
{
}

void KeyboardEventHandler::OnKeyDown(char)
{
}

void KeyboardEventHandler::OnKeyUp(char)
{
}





KeyboardDriver::KeyboardDriver(InterruptManager* manager, KeyboardEventHandler *handler)
: InterruptHandler(manager, 0x21),
dataport(0x60),
commandport(0x64)
{
    this->handler = handler;
}

KeyboardDriver::~KeyboardDriver()
{
}

void printf(char*);
void printfHex(uint8_t);

void KeyboardDriver::Activate()
{
    while(commandport.Read() & 0x1)
        dataport.Read();
    commandport.Write(0xae); // activate interrupts
    commandport.Write(0x20); // command 0x20 = read controller command byte
    uint8_t status = (dataport.Read() | 1) & ~0x10;
    commandport.Write(0x60); // command 0x60 = set controller command byte
    dataport.Write(status);
    dataport.Write(0xf4);
}

uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp)
{
    uint8_t key = dataport.Read();
    
    if(handler == 0)
        return esp;
    
    bool keyPress = false;

    char readKey = '\0';

    if(key < 0x80)
    {
        keyPress = true;

        switch (key) {
            case 0x02: readKey = '1'; break;
            case 0x03: readKey = '2'; break;
            case 0x04: readKey = '3'; break;
            case 0x05: readKey = '4'; break;
            case 0x06: readKey = '5'; break;
            case 0x07: readKey = '6'; break;
            case 0x08: readKey = '7'; break;
            case 0x09: readKey = '8'; break;
            case 0x0A: readKey = '9'; break;
            case 0x0B: readKey = '0'; break;

            case 0x10: readKey = 'q'; break;
            case 0x11: readKey = 'w'; break;
            case 0x12: readKey = 'e'; break;
            case 0x13: readKey = 'r'; break;
            case 0x14: readKey = 't'; break;
            case 0x15: readKey = 'z'; break;
            case 0x16: readKey = 'u'; break;
            case 0x17: readKey = 'i'; break;
            case 0x18: readKey = 'o'; break;
            case 0x19: readKey = 'p'; break;

            case 0x1E: readKey = 'a'; break;
            case 0x1F: readKey = 's'; break;
            case 0x20: readKey = 'd'; break;
            case 0x21: readKey = 'f'; break;
            case 0x22: readKey = 'g'; break;
            case 0x23: readKey = 'h'; break;
            case 0x24: readKey = 'j'; break;
            case 0x25: readKey = 'k'; break;
            case 0x26: readKey = 'l'; break;

            case 0x2C: readKey = 'y'; break;
            case 0x2D: readKey = 'x'; break;
            case 0x2E: readKey = 'c'; break;
            case 0x2F: readKey = 'v'; break;
            case 0x30: readKey = 'b'; break;
            
            case 0x31: readKey = 'n'; break;
            case 0x32: readKey = 'm'; break;
            case 0x33: readKey = ','; break;
            case 0x34: readKey = '.'; break;
            case 0x35: readKey = '-'; break;

            case 0x1C: readKey = '\n'; break;
            case 0x39: readKey = ' '; break;

            default:
            {
                printf("KEYBOARD 0x");
                printfHex(key);
                break;
            }
        }
        handler->OnKeyDown(readKey);
    }

    if (readIOFlag)
    {
        // write to the keyboard buffer
        if (keyPress)
        {
            if (keyBufferIndex < 256 || readKey != '\n' || keyBufferIndex < bytesToRead)
                keyBuffer[keyBufferIndex++] = readKey;
            
            if (keyBufferIndex >= 256 || keyBufferIndex >= bytesToRead || readKey == '\n')
            {
                keyBufferIndex = 0;
                bytesToRead = 0;
                readIOFlag = false;
                keyPress = false;
            }
        }
    }

    if (keyPress && !readIOFlag && (readKey != '\n')) {
        printf("\nKey pressed! switching\n");

        esp = (uint32_t) interruptManager->ScheduleTransmitter((CPUState*) esp);
    }
        

    return esp;
}

char KeyboardDriver::ReadBuffer(uint32_t index) {
    if (index < 256)
        return keyBuffer[index];
    else
        return '\0';
}
void KeyboardDriver::ResetBuffer() {
    keyBufferIndex = 0;
    bytesToRead = 0;
    keyBuffer[0] = '\0';
}

void KeyboardDriver::SetReadBytes(uint32_t bytes) {
    bytesToRead = bytes;
}