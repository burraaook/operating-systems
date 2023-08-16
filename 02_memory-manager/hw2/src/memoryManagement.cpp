#include "../include/memoryManagement.h"

using namespace memorymanagement;

// write everything to disk with 50 characters per line
const int diskEntrySegmentSpace = 50;
size_t printCounter = 0;

virtualAddress::virtualAddress ()
{
    this->pageTableEntryNumber = 0;
    this->valueOffset = 0;
    this->pageFrameSize = 0;
    this->pageTableSize = 0;
}

virtualAddress::virtualAddress (size_t pageTableEntryNumber, size_t valueOffset, size_t pageFrameSize, size_t pageTableSize)
{
    this->pageTableEntryNumber = pageTableEntryNumber;
    this->valueOffset = valueOffset;
    this->pageFrameSize = pageFrameSize;
    this->pageTableSize = pageTableSize;
}

virtualAddress::~virtualAddress ()
{

}

// increment operators
virtualAddress& virtualAddress::operator++ ()
{
    // increment valueOffset
    valueOffset++;
    // check if valueOffset is greater than or equal to pageFrameSize
    if (valueOffset >= pageFrameSize)
    {
        // increment pageTableEntryNumber
        pageTableEntryNumber++;
        // set valueOffset to 0
        valueOffset = 0;
    }
    // check if pageTableEntryNumber is greater than or equal to pageTableSize
    if (pageTableEntryNumber >= pageTableSize)
    {
        // set pageTableEntryNumber to 0
        pageTableEntryNumber = 0;
    }
    return *this;
}

virtualAddress virtualAddress::operator++ (int)
{
    virtualAddress temp = *this;
    ++*this;
    return temp;
}

void virtualAddress::copy (virtualAddress *address)
{
    this->pageTableEntryNumber = address->pageTableEntryNumber;
    this->valueOffset = address->valueOffset;
    this->pageFrameSize = address->pageFrameSize;
    this->pageTableSize = address->pageTableSize;
}

void virtualAddress::print () const
{
    // print page table entry number, value offset, page frame size, page table size
    std::cout << "Page Table Entry Number: " << pageTableEntryNumber << std::endl;
    std::cout << "Value Offset: " << valueOffset << std::endl;
    std::cout << "Page Frame Size: " << pageFrameSize << std::endl;
    std::cout << "Page Table Size: " << pageTableSize << std::endl << std::endl;
}
physicalAddress::physicalAddress (const size_t *memorySize)
{
    this->frameNumber = 0;
    this->valueOffset = 0;
    this->pageFrameSize = 0;
    this->memorySize = *memorySize;
}

physicalAddress::physicalAddress (size_t frameNumber, size_t valueOffset, size_t pageFrameSize, size_t memorySize)
{
    this->frameNumber = frameNumber;
    this->valueOffset = valueOffset;
    this->pageFrameSize = pageFrameSize;
    this->memorySize = memorySize;
}

physicalAddress::~physicalAddress ()
{

}

// increment operators
physicalAddress& physicalAddress::operator++ ()
{
    // increment valueOffset
    valueOffset++;
    // check if valueOffset is greater than or equal to pageFrameSize
    if (valueOffset >= pageFrameSize)
    {
        // increment frameNumber
        frameNumber++;
        // set valueOffset to 0
        valueOffset = 0;
    }
    // check if frameNumber is greater than or equal to memorySize
    if (frameNumber >= memorySize)
    {
        // set frameNumber to 0
        frameNumber = 0;
    }
    return *this;
}

physicalAddress physicalAddress::operator++ (int)
{
    physicalAddress temp = *this;
    ++*this;
    return temp;
}

void physicalAddress::print ()
{
    // print frame number, value offset, page frame size, memory size
    std::cout << "Frame Number: " << frameNumber << std::endl;
    std::cout << "Value Offset: " << valueOffset << std::endl;
    std::cout << "Page Frame Size: " << pageFrameSize << std::endl;
    std::cout << "Memory Size: " << memorySize << std::endl << std::endl;
}

ArrayType::ArrayType (size_t size)
{
    this->size = size;
    this->addresses = new virtualAddress[size];
}

ArrayType::~ArrayType ()
{
    delete[] addresses;
}

// getters
size_t ArrayType::getSize ()
{
    return size;
}

virtualAddress ArrayType::getAddress (size_t index)
{
    if (index < size)
        return addresses[index];
    else
        return virtualAddress();
}

// setters
void ArrayType::setSize (size_t size)
{
    this->size = size;
}

PageFrame::PageFrame (size_t frameNumber, int processId, size_t frameSize)
{
    this->frameNumber = frameNumber;
    this->processId = processId;
    this->frameSize = frameSize;
    this->numValues = 0;
    this->full = false;
    this->values = new int[frameSize];
}

PageFrame::~PageFrame ()
{
    delete[] values;
}

// getters
size_t PageFrame::getFrameNumber ()
{
    return frameNumber;
}

int PageFrame::getProcessId ()
{
    return processId;
}

size_t PageFrame::getFrameSize ()
{
    return frameSize;
}

size_t PageFrame::getNumValues ()
{
    return numValues;
}

// setters
void PageFrame::setFrameNumber (size_t frameNumber)
{
    this->frameNumber = frameNumber;
}

void PageFrame::setProcessId (int processId)
{
    this->processId = processId;
}

void PageFrame::setFrameSize (size_t frameSize)
{
    this->frameSize = frameSize;
}

void PageFrame::setNumValues (size_t numValues)
{
    this->numValues = numValues;
}

// methods
bool PageFrame::addValue (int value)
{
    if (numValues < frameSize)
    {
        values[numValues] = value;
        numValues++;

        if (numValues == frameSize)
            full = true;
        return true;
    }
    else
    {
        // add to different page frame
        return false;
    }
}

int PageFrame::getValue (size_t index)
{
    // std::cout << "-Index: " << index << " NumValues: " << numValues << std::endl;
    // std::cout << "-Frame Size: " << frameSize << std::endl;
    if (index < frameSize)
        return values[index];
    else
        return -1;
}

bool PageFrame::setValue (size_t index, int value)
{
    // std::cout << "Index: " << index << " NumValues: " << numValues << std::endl;
    // std::cout << "Frame Size: " << frameSize << std::endl;
    if (index < frameSize)
    {
        values[index] = value;
        return true;
    }
    else
    {
        std::cout << "Index out of bounds" << std::endl;
        // segmentation fault
        return false;
    }
}

void PageFrame::print ()
{
    // print frame number, and process id, size
    std::cout << "Frame Number: " << frameNumber << std::endl;
    std::cout << "Process Id: " << processId << std::endl;
    std::cout << "Frame Size: " << frameSize << std::endl;
    // print values with index
    for (size_t i = 0; i < numValues; ++i)
    {
        std::cout << "Index: " << i << " Value: " << values[i] << std::endl;
    }
}

bool PageFrame::isFull ()
{
    return full;
}

PhysicalMemory::PhysicalMemory (size_t numFrames, size_t frameSize, size_t memorySize)
{
    this->numFrames = 0;
    this->frameSize = frameSize;
    this->memorySize = memorySize;
    this->frames = new PageFrame*[numFrames];
}

PhysicalMemory::~PhysicalMemory ()
{
    // remove all frames
    for (size_t i = 0; i < numFrames; ++i)
    {
        delete frames[i];
    }

    delete[] frames;

}

// getters
size_t PhysicalMemory::getNumFrames ()
{
    return numFrames;
}

size_t PhysicalMemory::getFrameSize ()
{
    return frameSize;
}

size_t PhysicalMemory::getMemorySize ()
{
    return memorySize;
}

// setters
void PhysicalMemory::setNumFrames (size_t numFrames)
{
    this->numFrames = numFrames;
}

void PhysicalMemory::setFrameSize (size_t frameSize)
{
    this->frameSize = frameSize;
}

// methods
bool PhysicalMemory::addFrame (PageFrame *frame)
{
    if (numFrames < memorySize)
    {
        frames[numFrames] = frame;
        numFrames++;
    }
    else
    {
        // memory full, remove frame
        return false;
    }

    return true;
}

bool PhysicalMemory::setFrame (PageFrame *frame)
{
    for (size_t i = 0; i < numFrames; i++)
    {
        if (frames[i]->getFrameNumber() == frame->getFrameNumber())
        {
            frames[i] = frame;
            return true;
        }
    }
    return false;
}

bool PhysicalMemory::setFrame (size_t index, PageFrame *frame)
{
    if (index < numFrames)
    {
        frames[index] = frame;
        return true;
    }
    else
    {
        return false;
    }
}

PageFrame *PhysicalMemory::getFrame (size_t index)
{

    if (index < numFrames)
        return frames[index];
    else
        return nullptr;
}

PageFrame *PhysicalMemory::getFrameWithFrameNumber (size_t frameNumber)
{
    // std::cout << "searching for frame number: " << frameNumber << std::endl;
    for (size_t i = 0; i < numFrames; i++)
    {
        // std::cout << "Frame Number: " << frames[i]->getFrameNumber() << std::endl;
        if (frames[i]->getFrameNumber() == frameNumber)
            return frames[i];
    }

    // std::cout << "-Frame not found" << std::endl;
    return nullptr;
}

void PhysicalMemory::print ()
{
    // print number of frames, frame size, memory size
    std::cout << "Number of Frames: " << numFrames << std::endl;
    std::cout << "Frame Size: " << frameSize << std::endl;
    std::cout << "Memory Size: " << memorySize << std::endl;
    // print frames
    for (size_t i = 0; i < numFrames; i++)
    {
        frames[i]->print();
    }
}

PageTableEntry::PageTableEntry ()
{
    this->pageNumber = 0;
    this->frameNumber = 0;
    this->processId = 0;
    this->pageFrameSize = 0;

    this->valid = false;
    this->dirty = false;
    this->referenced = false;
    this->present = false;
    this->modified = false;

    this->protection = 0;
    this->diskLineNumber = 0;
    this->lastReferenceTime = std::chrono::system_clock::now();
}

PageTableEntry::PageTableEntry (int pageNumber, size_t frameNumber, int processId, size_t pageFrameSize)
{
    this->pageNumber = pageNumber;
    this->frameNumber = frameNumber;
    this->processId = processId;
    this->pageFrameSize = pageFrameSize;

    this->valid = false;
    this->dirty = false;
    this->referenced = false;
    this->present = false;
    this->modified = false;

    this->protection = 0;
    this->diskLineNumber = 0;
    this->lastReferenceTime = std::chrono::system_clock::now();
}

PageTableEntry::~PageTableEntry ()
{

}

// getters
int PageTableEntry::getPageNumber ()
{
    return pageNumber;
}

size_t PageTableEntry::getFrameNumber ()
{
    return frameNumber;
}

int PageTableEntry::getProcessId ()
{
    return processId;
}

bool PageTableEntry::isValid ()
{
    return valid;
}

bool PageTableEntry::isDirty ()
{
    return dirty;
}

bool PageTableEntry::isReferenced ()
{
    return referenced;
}

bool PageTableEntry::isPresent ()
{
    return present;
}

bool PageTableEntry::isModified ()
{
    return modified;
}

int PageTableEntry::getProtection ()
{
    return protection;
}

size_t PageTableEntry::getPageFrameSize ()
{
    return pageFrameSize;
}

size_t PageTableEntry::getDiskLineNumber ()
{
    return diskLineNumber;
}

// setters
void PageTableEntry::setPageNumber (int pageNumber)
{
    this->pageNumber = pageNumber;
}

void PageTableEntry::setFrameNumber (size_t frameNumber)
{
    this->frameNumber = frameNumber;
}

void PageTableEntry::setProcessId (int processId)
{
    this->processId = processId;
}

void PageTableEntry::setValid (bool valid)
{
    this->valid = valid;
}

void PageTableEntry::setDirty (bool dirty)
{
    this->dirty = dirty;
}

void PageTableEntry::setReferenced (bool reference)
{
    this->referenced = reference;
}

void PageTableEntry::setPresent (bool present)
{
    this->present = present;
}

void PageTableEntry::setModified (bool modified)
{
    this->modified = modified;
}

void PageTableEntry::setProtection (int protection)
{
    this->protection = protection;
}

void PageTableEntry::setPageFrameSize (size_t pageFrameSize)
{
    this->pageFrameSize = pageFrameSize;
}

void PageTableEntry::setDiskLineNumber (size_t diskLineNumber)
{
    this->diskLineNumber = diskLineNumber;
}

// methods
void PageTableEntry::print ()
{
    // print page number, frame number, process id, page frame size
    std::cout << "Page Number: " << pageNumber << std::endl;
    std::cout << "Frame Number: " << frameNumber << std::endl;
    std::cout << "Process Id: " << processId << std::endl;
    std::cout << "Page Frame Size: " << pageFrameSize << std::endl;
    // print valid, dirty, referenced, present, modified
    std::cout << "Valid: " << valid << std::endl;
    std::cout << "Dirty: " << dirty << std::endl;
    std::cout << "Referenced: " << referenced << std::endl;
    std::cout << "Present: " << present << std::endl;
    std::cout << "Modified: " << modified << std::endl;
    // print protection
    std::cout << "Protection: " << protection << std::endl << std::endl;
}

PageTable::PageTable ()
{

}

PageTable::~PageTable ()
{

}

RegularPageTable::RegularPageTable (int processId, size_t pageFrameSize, size_t pageTableSize)
    : PageTable()
{
    this->numEntries = 0;
    this->processId = processId;
    this->pageFrameSize = pageFrameSize;
    this->pageTableSize = pageTableSize;
    this->entries = new PageTableEntry*[pageTableSize];
    this->type = PageTableType::REGULAR;

    // initialize entries
    for (size_t i = 0; i < pageTableSize; ++i)
    {
        entries[i] = new PageTableEntry(i, 0, processId, pageFrameSize);
    }
}

RegularPageTable::~RegularPageTable ()
{
    delete[] entries;
}

// getters
size_t RegularPageTable::getNumEntries (int processId)
{
    if (this->processId == processId)
        return numEntries;
    else
        return 0;
}

int RegularPageTable::getNumProcesses ()
{
    return 1;
}

size_t RegularPageTable::getPageFrameSize ()
{
    return pageFrameSize;
}

size_t RegularPageTable::getPageTableSize ()
{
    return pageTableSize;
}

PageTableType RegularPageTable::getType ()
{
    return type;
}

PageTableEntry *RegularPageTable::getEntry (int processId, int pageNumber)
{   
    if (this->processId == processId)
    {
        for (size_t i = 0; i < pageTableSize; ++i)
        {
            if (entries[i]->getPageNumber() == pageNumber)
                return entries[i];
        }
    }

    // std::cout << "Entry not found" << std::endl;
    return nullptr;
}


// setters
bool RegularPageTable::setEntry (int processId, int pageNumber, PageTableEntry *entry)
{
    if (this->processId == processId)
    {
        for (size_t i = 0; i < numEntries; ++i)
        {
            if (entries[i]->getPageNumber() == pageNumber)
            {
                // delete existing entry
                if (entries[i] != entry)
                    delete entries[i];

                // Update the existing entry
                entries[i] = entry;
                
                return true;
            }
        }
    }

    // entry not found
    return false;
}

// methods
void RegularPageTable::print ()
{
    // print number of entries, process id, page frame size, page table size
    std::cout << "Number of Entries: " << numEntries << std::endl;
    std::cout << "Process Id: " << processId << std::endl;
    std::cout << "Page Frame Size: " << pageFrameSize << std::endl;
    std::cout << "Page Table Size: " << pageTableSize << std::endl;
    // print entries
    for (size_t i = 0; i < numEntries; i++)
    {
        entries[i]->print();
    }

    std::cout << std::endl;
}

bool RegularPageTable::addEntry (int processId, PageTableEntry *entry)
{
    if (this->processId == processId)
    {
        if (numEntries < pageTableSize)
        {
            // check if entry already exists
            for (size_t i = 0; i < numEntries; ++i)
            {
                if (entries[i]->getPageNumber() == entry->getPageNumber())
                {
                    // delete existing entry
                    if (entries[i] != entry)
                        delete entries[i];

                    // Update the existing entry
                    entries[i] = entry;

                    return true;
                }
            }
            return true;
        }
        else
        {
            // page table full, remove entry
            return false;
        }
    }

    // page table full, remove entry
    return false;
}

bool RegularPageTable::addEntry (int processId, const size_t *pageFrameNumber, size_t *entryNumber)
{
    // check if processId is valid
    if (this->processId != processId)
        return false;

    if (numEntries >= pageTableSize)
        return false;

    // find an empty entry
    for (size_t i = 0; i < pageTableSize; ++i)
    {
        if (!entries[i]->isValid())
        {
            // set the entry
            *entryNumber = i;
            entries[i]->setPageNumber(i);
            entries[i]->setFrameNumber(*pageFrameNumber);
            entries[i]->setValid(true);
            entries[i]->setDirty(false);
            entries[i]->setReferenced(true);
            entries[i]->setPresent(true);
            entries[i]->setModified(false);
            entries[i]->setProtection(0);

            // increment numEntries
            numEntries++;

            return true;
        }
    }

    return false; // If no empty entry is found, return false
}

InvertedPageTable::InvertedPageTable (size_t pageFrameSize, size_t pageTableSize)
    : PageTable()
{
    this->numProcesses = 0;
    this->pageFrameSize = pageFrameSize;
    this->pageTableSize = pageTableSize;
    this->type = PageTableType::INVERTED;
    this->entries = new std::unordered_map<int, std::vector<PageTableEntry *>>();
}

InvertedPageTable::~InvertedPageTable()
{
    // remove all entries
    for (auto it = entries->begin(); it != entries->end(); ++it)
    {
        std::vector<PageTableEntry*>& entryList = it->second;
        for (PageTableEntry* entry : entryList)
        {
            delete entry;
        }
    }
    delete entries;
}


// getters
size_t InvertedPageTable::getNumEntries (int processId)
{
    auto it = entries->find(processId);
    if (it != entries->end())
    {
        const std::vector<PageTableEntry*>& entryList = it->second;
        return entryList.size();
    }

    return 0; // If processId does not exist, return 0
}

int InvertedPageTable::getNumProcesses ()
{
    this->numProcesses = entries->size();
    return numProcesses;
}

size_t InvertedPageTable::getPageFrameSize ()
{
    return pageFrameSize;
}

size_t InvertedPageTable::getPageTableSize ()
{
    return pageTableSize;
}

PageTableType InvertedPageTable::getType ()
{
    return type;
}

PageTableEntry* InvertedPageTable::getEntry (int processId, int pageNumber)
{
    auto it = entries->find(processId);
    if (it != entries->end())
    {
        const std::vector<PageTableEntry*>& entryList = it->second;
        for (PageTableEntry* entry : entryList)
        {
            if (entry->getPageNumber() == pageNumber)
            {
                return entry;
            }
        }
    }

    return nullptr; // If entry is not found, return nullptr
}

// setters
bool InvertedPageTable::setEntry(int processId, int pageNumber, PageTableEntry* entry)
{
    auto it = entries->find(processId);
    if (it != entries->end())
    {
        std::vector<PageTableEntry*>& entryList = it->second;
        for (PageTableEntry*& existingEntry : entryList)
        {
            if (existingEntry->getPageNumber() == pageNumber)
            {
                // delete existing entry
                if (existingEntry != entry)
                    delete existingEntry;

                // Update the existing entry
                existingEntry = entry;

                return true;
            }
        }
        // Entry not found, add it to the list
        entryList.push_back(entry);
        return true;
    }

    return false; // If entry list for processId doesn't exist
}

// methods
void InvertedPageTable::print ()
{
    // print number of entries, process id, page frame size, page table size
    std::cout << "Number of Processes: " << numProcesses << std::endl;
    std::cout << "Page Frame Size: " << pageFrameSize << std::endl;
    std::cout << "Page Table Size: " << pageTableSize << std::endl;
    // print entries for each process
    // iterate through the map, print entries
    for (auto it = entries->begin(); it != entries->end(); ++it)
    {
        std::cout << "Process Id: " << it->first << std::endl;
        std::vector<PageTableEntry*>& entryList = it->second;
        std::cout << "Number of Entries: " << entryList.size() << std::endl;
        for (PageTableEntry* entry : entryList)
        {
            entry->print();
        }
    }

    std::cout << std::endl;
}

bool InvertedPageTable::addEntry(int processId, PageTableEntry* entry)
{
    auto it = entries->find(processId);
    if (it != entries->end())
    {
        if (it->second.size() >= pageTableSize)
        {
            // page table full, remove entry
            return false;
        }
        else
        {
            // if entry already exists, update it
            for (PageTableEntry*& existingEntry : it->second)
            {
                if (existingEntry->getPageNumber() == entry->getPageNumber())
                {
                    // delete existing entry
                    if (existingEntry != entry)
                        delete existingEntry;

                    // Update the existing entry
                    existingEntry = entry;

                    return true;
                }
            }
            return true;
        }
    }
    else
    {
        std::vector<PageTableEntry*>* entryList = new std::vector<PageTableEntry*>();
        entryList->push_back(entry);
        entries->insert(std::make_pair(processId, *entryList));
        numProcesses++;
    }

    return true;
}

MMU::MMU (size_t pageFrameSize, size_t tlbSize)
{
    this->pageFrameSize = pageFrameSize;
    this->tlbSize = tlbSize;
    this->tlbs = new std::unordered_map<int, std::vector<PageTableEntry *>>();
}

MMU::~MMU ()
{
    // remove all entries
    // for (auto it = tlbs->begin(); it != tlbs->end(); ++it)
    // {
    //     std::vector<PageTableEntry*>& entryList = it->second;
    //     for (PageTableEntry* entry : entryList)
    //     {
    //         delete entry;
    //     }
    // }
    delete tlbs;
}

// getters

size_t MMU::getPageFrameSize ()
{
    return pageFrameSize;
}

size_t MMU::getTlbSize ()
{
    return tlbSize;
}

// setters
void MMU::setPageFrameSize (size_t pageFrameSize)
{
    this->pageFrameSize = pageFrameSize;
}

void MMU::setTlbSize (size_t tlbSize)
{
    this->tlbSize = tlbSize;
}

// methods
bool MMU::addEntry (int processId, PageTableEntry* entry)
{
    auto it = tlbs->find(processId);
    if (it != tlbs->end())
    {
        std::vector<PageTableEntry*>& entryList = it->second;
        if (entryList.size() >= tlbSize)
        {
            // TLB full, remove entry
            return false;
        }
        else
        {
            // if entry already exists, update it
            for (PageTableEntry*& existingEntry : entryList)
            {
                if (existingEntry->getPageNumber() == entry->getPageNumber())
                {
                    // delete existing entry
                    if (existingEntry != entry)
                        delete existingEntry;

                    // Update the existing entry
                    existingEntry = entry;

                    return true;
                }
            }

            // Entry not found, add it to the list
            entryList.push_back(entry);
            return true;
        }
    }
    else
    {
        std::vector<PageTableEntry*> *entryList = new std::vector<PageTableEntry*>();
        entryList->push_back(entry);
        tlbs->insert(std::make_pair(processId, *entryList));
        return true;
    }
}
PageTableEntry* MMU::getEntry (int processId, int pageNumber)
{
    auto it = tlbs->find(processId);
    if (it != tlbs->end())
    {
        std::vector<PageTableEntry*>& entryList = it->second;
        for (PageTableEntry* entry : entryList)
        {
            if (entry->getPageNumber() == pageNumber)
            {
                return entry;
            }
        }
    }
    return nullptr;
}


bool MMU::setEntry(int processId, int pageNumber, PageTableEntry* entry)
{
    auto it = tlbs->find(processId);
    if (it != tlbs->end())
    {
        std::vector<PageTableEntry*>& entryList = it->second;
        for (PageTableEntry*& existingEntry : entryList)
        {
            if (existingEntry->getPageNumber() == pageNumber)
            {
                // delete existing entry
                if (existingEntry != entry)
                    delete existingEntry;

                // Update the existing entry
                existingEntry = entry;

                return true;
            }
        }
    }
    return false;
}

bool MMU::addressTranslation (int processId, const virtualAddress *virtualAddress, physicalAddress *physicalAddress, const size_t *frameNumber)
{
    // check if processId is valid
    if (processId < 0)
        return false;

    // convert virtual address to physical address
    physicalAddress->frameNumber = *frameNumber;
    physicalAddress->valueOffset = virtualAddress->valueOffset;
    physicalAddress->pageFrameSize = pageFrameSize;

    return true;
}
MemoryManager::MemoryManager(int maxProcesses, size_t frameSize, size_t physicalMemorySize, size_t pageTableSize, PageAlgorithm pageAlgorithm,
                            bool invertedFlag, size_t printCount, std::string diskFileName)
    : numProcesses(1),
      diskFileName(std::move(diskFileName)), printCount(printCount),
      pageAlgorithm(pageAlgorithm),
      numReads(0), numWrites(0), numPageMisses(0), numPageReplacements(0),
      numDiskWrites(0), numDiskReads(0)
{
    this->maxProcesses = maxProcesses;

    // power of 2
    this->physicalMemorySize = 1 << physicalMemorySize;
    this->pageTableSize = 1 << pageTableSize;
    this->frameSize = 1 << frameSize;
    

    // Create the PhysicalMemory instance
    physicalMemory = new PhysicalMemory(this->physicalMemorySize, this->frameSize, this->physicalMemorySize);
    this->invertedFlag = false;
    invertedFlag = false;
    // Create the MMU instance
    mmu = new MMU(this->frameSize, 10);

    if (invertedFlag)
    {
        // Create a single InvertedPageTable for all processes
        pageTables = new PageTable *[1];
        pageTables[0] = new InvertedPageTable(this->frameSize, this->pageTableSize);
    }
    else
    {
        // Each process has its own RegularPageTable
        pageTables = new PageTable *[maxProcesses];
        for (int i = 0; i < maxProcesses; ++i)
            pageTables[i] = new RegularPageTable(i, this->frameSize, this->pageTableSize);
    }

    maxPageFrameNumber = 0;

    // create disk file
    std::ofstream diskFile;
    // if it exists, truncate it
    diskFile.open(this->diskFileName, std::ofstream::out | std::ofstream::trunc);
    diskFile.close();

    this->SCindex = 0;
    this->WSCLOCKindex = 0;
    this->threshold = std::chrono::milliseconds(10);
}

MemoryManager::~MemoryManager()
{
    // Delete the PhysicalMemory instance
    delete physicalMemory;

    // Delete the MMU instance
    delete mmu;

    // Delete the PageTable instances
    if (pageTables)
    {
        for (int i = 0; i < numProcesses; ++i)
            delete pageTables[i];
        delete[] pageTables;
    }
}

// getters
std::string MemoryManager::getDiskFileName ()
{
    return diskFileName;
}

bool MemoryManager::getInvertedFlag ()
{
    return invertedFlag;
}

size_t MemoryManager::getPrintCount ()
{
    return printCount;
}

PageAlgorithm MemoryManager::getPageAlgorithm ()
{
    return pageAlgorithm;
}

size_t MemoryManager::getFrameSize ()
{
    return frameSize;
}

size_t MemoryManager::getPhysicalMemorySize ()
{
    return physicalMemorySize;
}

size_t MemoryManager::getPageTableSize ()
{
    return pageTableSize;
}

int MemoryManager::getNumProcesses ()
{
    return numProcesses;
}

size_t MemoryManager::getPageMissCount ()
{
    return numPageMisses;
}

// returns with virtual address
bool MemoryManager::allocateMemory (int processId, virtualAddress* virtual_address)
{
    // Check if processId is valid
    if (processId < 0 || processId > numProcesses)
        return false;   
    
    if (!invertedFlag)
    {
        // Regular page table
        RegularPageTable* pageTable = (RegularPageTable*)pageTables[processId];
        bool found = false;

        if (pageTable->getNumEntries(processId) != 0)
        {
            // std::cout << "\nPage table not empty" << std::endl;
            // traverse entries to find an empty page frame
            for (size_t i = 0; i < pageTable->getPageTableSize(); ++i)
            {
                if (pageTable->getEntry(processId, i)->isPresent())
                {                    
                    for (size_t j = 0; j < physicalMemory->getNumFrames(); ++j)
                    {
                        if (physicalMemory->getFrame(j)->getProcessId() == processId)
                        {

                            // check frame number is equal to page table entry frame number
                            // std::cout << "-Frame number: " << physicalMemory->getFrame(j)->getFrameNumber() << std::endl;
                            // std::cout << "Page table entry frame number: " << pageTable->getEntry(processId, i)->getFrameNumber() << std::endl;
                            PageTableEntry* entry = pageTable->getEntry(processId, i);
                            // std::cout << "Entry frame number: " << entry->getFrameNumber() << std::endl;
                            // std::cout << "entry number " << entry->getPageNumber() << std::endl;
                            if (entry->getFrameNumber() != physicalMemory->getFrame(j)->getFrameNumber())
                                continue;

                            // std::cout << "entry found" << std::endl;
                            // check if frame is full
                            if (!physicalMemory->getFrame(j)->isFull())
                            {
                                // put the value in the frame
                                physicalMemory->getFrame(j)->addValue(0);

                                // set the page table entry
                                
                                entry->setModified(true);
                                entry->setReferenced(true);
                                entry->setPresent(true);
                                entry->setFrameNumber(physicalMemory->getFrame(j)->getFrameNumber());
                                // set the virtual address
                                virtual_address->pageTableEntryNumber = i;
                                virtual_address->valueOffset = physicalMemory->getFrame(j)->getNumValues() - 1;
                                found = true;

                                // print frame number
                                // std::cout << "Frame Number: " << physicalMemory->getFrame(j)->getFrameNumber() << std::endl;
                                // std::cout << "Entry Number: " << entry->getPageNumber() << std::endl;
                                
                                if (pageAlgorithm == PageAlgorithm::LRU)
                                    entry->lastReferenceTime = std::chrono::system_clock::now();

                                return true;
                            }
                        }
                    }                    
                }
            }

            // std::cout << "all frames full" << std::endl;
        }

            // if not found, allocate a new page frame
            if (!found)
            {
                // if page table is full, return false
                if (pageTable->getNumEntries(processId) >= pageTableSize)
                    return false;

                PageFrame* frame = new PageFrame(maxPageFrameNumber, processId, frameSize);
                this->maxPageFrameNumber++;

                size_t entryNumber = 0;
                // add frame to the one of the page table entries
                if (physicalMemory->addFrame(frame) == false)
                {
                    // std::cout << "Memory full" << std::endl;

                    // memory full, page replacement
                    this->numPageMisses++;
                    if (pageAlgorithm == PageAlgorithm::SC)
                    {
                            size_t diskPos;
                            // std::cout << "second chance" << std::endl;
                            if(!secondChance1(processId, pageTable, frame, &entryNumber, &diskPos, ReplaceMode::APPEND))
                                return false;
                            
                            // std::cout << "second chance done" << std::endl;
                            // write to virtual address
                            virtual_address->pageTableEntryNumber = entryNumber;
                            virtual_address->valueOffset = 0;
                            virtual_address->pageFrameSize = frameSize;
                            virtual_address->pageTableSize = pageTableSize;
                            
                            this->numPageReplacements++;
                            frame->addValue(0);
                            // std::cout << "virtual address: " << std::endl;
                            // virtual_address->print();
                            // std::cout << std::endl;
                            return true;
                    }
                    else if (pageAlgorithm == PageAlgorithm::LRU)
                    {
                        // it is lru
                        // std::cout << "\nlru " << std::endl;
                        // std::cout << "frame created: " << frame->getFrameNumber() << std::endl;
                        size_t diskPos;
                        if(!lru1(processId, pageTable, frame, &entryNumber, &diskPos, ReplaceMode::APPEND))
                            return false;

                        // std::cout << "lru done" << std::endl;
                        // write to virtual address
                        virtual_address->pageTableEntryNumber = entryNumber;
                        virtual_address->valueOffset = 0;
                        virtual_address->pageFrameSize = frameSize;
                        virtual_address->pageTableSize = pageTableSize;

                        this->numPageReplacements++;
                        frame->addValue(0);

                        // entry->lastReferenceTime = std::chrono::system_clock::now();
                        return true;
                    }

                    else if (pageAlgorithm == PageAlgorithm::WSCLOCK)
                    {
                        // it is wsclock
                        // std::cout << "wsclock" << std::endl;
                        size_t diskPos;

                        if(!wsclock1(processId, pageTable, frame, &entryNumber, &diskPos, ReplaceMode::APPEND))
                            return false;

                        // std::cout << "wsclock done" << std::endl;

                        // write to virtual address
                        virtual_address->pageTableEntryNumber = entryNumber;
                        virtual_address->valueOffset = 0;
                        virtual_address->pageFrameSize = frameSize;
                        virtual_address->pageTableSize = pageTableSize;

                        this->numPageReplacements++;
                        frame->addValue(0);

                        return true;
                    }

                    return false;
                }
                else
                {
                    // add value to the frame
                    frame->addValue(0);
                    // get frame number
                    size_t frameNumber = frame->getFrameNumber();
                    // set the page table entry
                    pageTable->addEntry(processId, &frameNumber, &entryNumber);

                    // set entry as present
                    PageTableEntry* entry = pageTable->getEntry(processId, entryNumber);
                    entry->setPresent(true);

                    // set the virtual address
                    virtual_address->pageTableEntryNumber = entryNumber;
                    virtual_address->valueOffset = 0;
                    virtual_address->pageFrameSize = frameSize;
                    virtual_address->pageTableSize = pageTableSize;
                    return true;
                }
            }
    }
    else
    {
        // get inverted Page table
        InvertedPageTable* invertedPageTable = (InvertedPageTable*)pageTables[0];

        // check if page table is full
        if (invertedPageTable->getNumEntries(processId) >= pageTableSize)
            return false;

        // std::cout << "inverted page table num entries: " << invertedPageTable->getNumEntries(processId) << std::endl;
        // std::cout << "num frames: " << physicalMemory->getNumFrames() << std::endl;
        // check if there are any page frame in memory
        if (physicalMemory->getNumFrames() == 0)
        {
            // memory is empty, allocate a new page frame
            PageFrame* frame = new PageFrame(maxPageFrameNumber, processId, frameSize);
            frame->full = false;
            this->maxPageFrameNumber++;

            // add frame to the physical memory
            physicalMemory->addFrame(frame);
            frame->addValue(0);
            std::cout << "frame full: " << frame->isFull() << std::endl;

            // add entry to the inverted page table
            size_t entryNumber = 0;
            PageTableEntry* entry = new PageTableEntry(entryNumber, maxPageFrameNumber - 1, processId, frameSize);
            entry->setPresent(true);

            invertedPageTable->addEntry(processId, entry);

            // set the virtual address
            virtual_address->pageTableEntryNumber = entryNumber;
            virtual_address->valueOffset = 0;
            virtual_address->pageFrameSize = frameSize;
            virtual_address->pageTableSize = pageTableSize;
            return true;
        }

        // memory is not empty, find an empty page frame
        // bool found = false;

        // traverse entries to find an empty page frame
        std::vector<PageTableEntry*> entryList = invertedPageTable->entries->at(processId);
        // std::cout << "entry list size: " << entryList.size() << std::endl;
        for (size_t i = 0; i < entryList.size(); ++i)
        {

            if (entryList[i]->isPresent())
            {
                // std::cout << "entry present" << std::endl;
                for (size_t j = 0; j < physicalMemory->getNumFrames(); ++j)
                {
                    // std::cout << "frame number: " << physicalMemory->getFrame(j)->getFrameNumber() << std::endl;
                    // std::cout << "entry frame number: " << entryList[i]->getFrameNumber() << std::endl;
                    if (physicalMemory->getFrame(j)->getProcessId() == processId)
                    {

                        // check frame number is equal to page table entry frame number
                        if (entryList[i]->getFrameNumber() != physicalMemory->getFrame(j)->getFrameNumber())
                            continue;

                        // std::cout << "frame full: " << physicalMemory->getFrame(j)->isFull() << std::endl;
                        // check if frame is full
                        if (!physicalMemory->getFrame(j)->isFull())
                        {
                            // put the value in the frame
                            physicalMemory->getFrame(j)->addValue(0);

                            // set the page table entry
                            entryList[i]->setModified(true);
                            entryList[i]->setReferenced(true);
                            entryList[i]->setPresent(true);
                            entryList[i]->setFrameNumber(physicalMemory->getFrame(j)->getFrameNumber());
                            // set the virtual address
                            virtual_address->pageTableEntryNumber = i;
                            virtual_address->valueOffset = physicalMemory->getFrame(j)->getNumValues() - 1;
                            // found = true;

                            if (pageAlgorithm == PageAlgorithm::LRU)
                                entryList[i]->lastReferenceTime = std::chrono::system_clock::now();

                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

// writes to the virtual address
bool MemoryManager::writeMemory (int processId, const virtualAddress *address, int value)
{
    // check if processId is valid
    if (processId < 0 || processId >= numProcesses)
        return false;

    printCounter++;
    if (printCounter >= printCount)
    {
        printCounter = 0;
        printPageTableInfo(processId);
    }



    physicalAddress* pAddress = new physicalAddress(&physicalMemorySize);
  
    // get page table
    PageTable* pageTable = pageTables[processId];

    if (!invertedFlag)
    {
        // Regular page table
        RegularPageTable* regularPageTable = (RegularPageTable*)pageTable;

        // get page table entry
        PageTableEntry* entry = regularPageTable->getEntry(processId, address->pageTableEntryNumber);
        if (entry == nullptr)
        {
            delete pAddress;
            return false;
        }

        // get frame number
        size_t frameNumber = entry->getFrameNumber();

        // translate virtual address to physical address
        if (mmu->addressTranslation(processId, address, pAddress, &frameNumber) == false)
        {
            delete pAddress;
            return false;
        }
        // std::cout << "\nvirtual address: " << std::endl;
        // address->print();

        // std::cout << "\nphysical address: " << std::endl;
        // pAddress->print();
        // check if page frame is in memory
        if (entry->isPresent())
        {
            // std::cout << "page frame is in memory" << std::endl;
            // std::cout << "frame number: " << pAddress->frameNumber << std::endl;
            // write to the physical address
            physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->setValue(pAddress->valueOffset, value);
            // std::cout << "Value written to frame " << pAddress->frameNumber << std::endl;
            // std::cout << "value written to entry " << entry->getPageNumber() << std::endl;
            numWrites++;

            // set the page table entry
            entry->setModified(true);
            entry->setReferenced(true);

            if (pageAlgorithm == PageAlgorithm::LRU)
            {
                entry->lastReferenceTime = std::chrono::system_clock::now();
            }

            delete pAddress;
            return true;
        }
        else
        {
            // page fault
            this->numPageMisses++;

            if (entry->isValid())
            {
                // page frame is in disk

                // new page frame
                PageFrame* frame = new PageFrame(pAddress->frameNumber, processId, frameSize);

                // page replacement

                if (pageAlgorithm == PageAlgorithm::SC)
                {
                    size_t diskPos = entry->getDiskLineNumber();
                    size_t entryNumber = entry->getPageNumber();
                    // std::cout << "second chance write memory" << std::endl;
                    // std::cout << "\n searching page frame: " << entry->getFrameNumber() << std::endl;

                    if(!secondChance1(processId, regularPageTable, frame, &entryNumber, &diskPos, ReplaceMode::OVERWRITE_AND_READ))
                        return false;

                    // std::cout << "second chance done write memory" << std::endl;

                    // set the page table entry
                    entry->setModified(true);
                    entry->setReferenced(true);
                    entry->setPresent(true);
                    entry->setFrameNumber(frame->getFrameNumber());

                    // std::cout << "entry set" << std::endl;
                    // std::cout << "frame number: " << frame->getFrameNumber() << std::endl;
                    // std::cout << "p frame number: " << pAddress->frameNumber << std::endl;
                    // write to the physical address
                    physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->setValue(pAddress->valueOffset, value);

                    delete pAddress;
                    numWrites++;
                    numPageReplacements++;
                    return true;
                }
                else if (pageAlgorithm == PageAlgorithm::LRU)
                {
                    // it is lru
                    // std::cout << "\nlru" << std::endl;
                    // std::cout << "page frame needed: " << entry->getFrameNumber() << std::endl;
                    size_t diskPos = entry->getDiskLineNumber();
                    size_t entryNumber = entry->getPageNumber();

                    if(!lru1(processId, regularPageTable, frame, &entryNumber, &diskPos, ReplaceMode::OVERWRITE_AND_READ))
                        return false;

                    // std::cout << "lru done" << std::endl;

                    // set the page table entry
                    entry->setModified(true);
                    entry->setReferenced(true);
                    entry->setPresent(true);
                    entry->setFrameNumber(frame->getFrameNumber());
                    entry->lastReferenceTime = std::chrono::system_clock::now();


                    // std::cout << "last reference time: " << std::chrono::duration_cast<std::chrono::milliseconds>(entry->lastReferenceTime.time_since_epoch()).count() << std::endl;
                    // std::cout << "entry set" << std::endl;

                    // write to the physical address
                    physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->setValue(pAddress->valueOffset, value);

                    delete pAddress;
                    numWrites++;
                    numPageReplacements++;
                    return true;
                }

                else if (pageAlgorithm == PageAlgorithm::WSCLOCK)
                {
                    // it is wsclock
                    // std::cout << "wsclock" << std::endl;
                    // std::cout << "page frame needed: " << entry->getFrameNumber() << std::endl;
                    size_t diskPos = entry->getDiskLineNumber();
                    size_t entryNumber = entry->getPageNumber();

                    if(!wsclock1(processId, regularPageTable, frame, &entryNumber, &diskPos, ReplaceMode::OVERWRITE_AND_READ))
                        return false;

                    // std::cout << "wsclock done" << std::endl;

                    // set the page table entry
                    entry->setModified(true);
                    entry->setReferenced(true);
                    entry->setPresent(true);
                    entry->setFrameNumber(frame->getFrameNumber());

                    // std::cout << "entry set" << std::endl;

                    // write to the physical address
                    physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->setValue(pAddress->valueOffset, value);

                    delete pAddress;
                    numWrites++;
                    numPageReplacements++;
                    return true;
                }
            }
            else
            {
                // invalid page table entry
                delete pAddress;
                return false;
            }
            delete pAddress;
            return false;
        }
    }
    else
    {
        // get inverted page table
        InvertedPageTable* invertedPageTable = (InvertedPageTable*)pageTables[0];

        // get page table entry
        PageTableEntry* entry = invertedPageTable->getEntry(processId, address->pageTableEntryNumber);

        // get frame number
        size_t frameNumber = entry->getFrameNumber();

        // translate virtual address to physical address
        if (mmu->addressTranslation(processId, address, pAddress, &frameNumber) == false)
        {
            delete pAddress;
            return false;
        }

        // check if page frame is in memory
        if (entry->isPresent())
        {
            // write to the physical address
            physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->setValue(pAddress->valueOffset, value);
            numWrites++;

            // set the page table entry
            entry->setModified(true);
            entry->setReferenced(true);

            if (pageAlgorithm == PageAlgorithm::LRU)
            {
                entry->lastReferenceTime = std::chrono::system_clock::now();
            }

            delete pAddress;
            return true;
        }
        else
        {
            // page fault
            this->numPageMisses++;

            if (entry->isValid())
            {
                // page frame is in disk

                // new page frame
                // PageFrame* frame = new PageFrame(pAddress->frameNumber, processId, frameSize);

                // page replacement


                if (pageAlgorithm == PageAlgorithm::SC)
                {
                    // it is second chance
                }
                else if (pageAlgorithm == PageAlgorithm::LRU)
                {
                    // it is lru
                }
                else if (pageAlgorithm == PageAlgorithm::WSCLOCK)
                {
                    // it is wsclock
                }
            }
        }
    }

    return false;
}

// reads from the virtual address
bool MemoryManager::readMemory (int processId, const virtualAddress *address, int *value)
{
    // check if processId is valid
    if (processId < 0 || processId >= numProcesses)
        return false;

    printCounter++;
    if (printCounter >= printCount)
    {
        printCounter = 0;
        printPageTableInfo(processId);
    }

    physicalAddress* pAddress = new physicalAddress(&physicalMemorySize);

    // get page table
    PageTable* pageTable = pageTables[processId];

    if (!invertedFlag)
    {
        // Regular page table
        RegularPageTable* regularPageTable = (RegularPageTable*)pageTable;

        // get page table entry
        PageTableEntry* entry = regularPageTable->getEntry(processId, address->pageTableEntryNumber);
        if (entry == nullptr)
        {
            delete pAddress;
            return false;
        }

        // get frame number
        size_t frameNumber = entry->getFrameNumber();

        // translate virtual address to physical address
        if (mmu->addressTranslation(processId, address, pAddress, &frameNumber) == false)
        {
            delete pAddress;
            return false;
        }

        // std::cout << "\nvirtual address: " << std::endl;
        // address->print();
        // std::cout << "\nphysical address: " << std::endl;
        // pAddress->print();

        // check if page frame is in memory
        if (entry->isPresent())
        {
            // read from the physical address
            *value = physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->getValue(pAddress->valueOffset);
            delete pAddress;
            numReads++;

            // set the page table entry
            entry->setReferenced(true);
            return true;
        }
        else
        {
            if (entry->isValid())
            {
                
                // page fault
                this->numPageMisses++;

                // page frame is in disk

                // new page frame
                PageFrame* frame = new PageFrame(pAddress->frameNumber, processId, frameSize);
                // std::cout << "--frame size: " << frame->getFrameSize() << std::endl;
                // page replacement

                if (pageAlgorithm == PageAlgorithm::SC)
                {
                    size_t diskPos = entry->getDiskLineNumber();
                    size_t entryNumber = entry->getPageNumber();
                    // std::cout << "second chance read memory" << std::endl;
                    // std::cout << "\n searching page frame: " << entry->getFrameNumber() << std::endl;

                    if(!secondChance1(processId, regularPageTable, frame, &entryNumber, &diskPos, ReplaceMode::OVERWRITE_AND_READ))
                        return false;

                    // std::cout << "2-frame size: " << frame->getFrameSize() << std::endl;

                    // std::cout << "second chance done read memory" << std::endl;

                    // set the page table entry
                    entry->setModified(false);
                    entry->setReferenced(true);
                    entry->setPresent(true);
                    entry->setFrameNumber(frame->getFrameNumber());

                    // std::cout << "entry set" << std::endl;
                    // std::cout << "frame number: " << frame->getFrameNumber() << std::endl;
                    // std::cout << "p frame number: " << pAddress->frameNumber << std::endl;
                    // read from the physical address
                    *value = physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->getValue(pAddress->valueOffset);

                    // std::cout << "value read: " << *value << std::endl;
                    delete pAddress;
                    numReads++;
                    numPageReplacements++;
                    return true;
                }
                else if (pageAlgorithm == PageAlgorithm::LRU)
                {
                    // it is lru
                    // std::cout << "lru" << std::endl;
                    // std::cout << "page frame needed: " << entry->getFrameNumber() << std::endl;
                    size_t diskPos = entry->getDiskLineNumber();
                    size_t entryNumber = entry->getPageNumber();

                    if(!lru1(processId, regularPageTable, frame, &entryNumber, &diskPos, ReplaceMode::OVERWRITE_AND_READ))
                        return false;

                    // std::cout << "lru done" << std::endl;

                    // set the page table entry
                    entry->setModified(false);
                    entry->setReferenced(true);
                    entry->setPresent(true);
                    entry->setFrameNumber(frame->getFrameNumber());
                    entry->lastReferenceTime = std::chrono::system_clock::now();

                    // std::cout << "entry set" << std::endl;

                    // read from the physical address
                    *value = physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->getValue(pAddress->valueOffset);

                    delete pAddress;
                    numReads++;
                    numPageReplacements++;
                    return true;
                }

                else if (pageAlgorithm == PageAlgorithm::WSCLOCK)
                {
                    // it is wsclock
                    // std::cout << "wsclock" << std::endl;
                    // std::cout << "page frame needed: " << entry->getFrameNumber() << std::endl;

                    size_t diskPos = entry->getDiskLineNumber();
                    size_t entryNumber = entry->getPageNumber();

                    // std::cout << "wsclock1" << std::endl;

                    if(!wsclock1(processId, regularPageTable, frame, &entryNumber, &diskPos, ReplaceMode::OVERWRITE_AND_READ))
                        return false;

                    // std::cout << "wsclock done" << std::endl;

                    // set the page table entry
                    entry->setModified(false);
                    entry->setReferenced(true);
                    entry->setPresent(true);
                    entry->setFrameNumber(frame->getFrameNumber());

                    // std::cout << "entry set" << std::endl;

                    // read from the physical address
                    *value = physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->getValue(pAddress->valueOffset);

                    delete pAddress;
                    numReads++;
                    numPageReplacements++;
                    return true;
                }
            }
            else
            {
                // invalid page table entry
                delete pAddress;
                return false;
            }
            delete pAddress;
            return false;
        }
    }
    else
    {
        // get inverted page table
        InvertedPageTable* invertedPageTable = (InvertedPageTable*)pageTable;

        // get page table entry
        PageTableEntry* entry = invertedPageTable->getEntry(processId, address->pageTableEntryNumber);
        if (entry == nullptr)
        {
            delete pAddress;
            return false;
        }

        // get frame number
        size_t frameNumber = entry->getFrameNumber();

        // translate virtual address to physical address
        if (mmu->addressTranslation(processId, address, pAddress, &frameNumber) == false)
        {
            delete pAddress;
            return false;
        }

        // std::cout << "\nvirtual address: " << std::endl;
        // address->print();
        // std::cout << "\nphysical address: " << std::endl;
        // pAddress->print();

        // check if page frame is in memory
        if (entry->isPresent())
        {
            // read from the physical address
            *value = physicalMemory->getFrameWithFrameNumber(pAddress->frameNumber)->getValue(pAddress->valueOffset);
            delete pAddress;
            numReads++;

            // set the page table entry
            entry->setReferenced(true);
            return true;
        }
        else
        {
            if (entry->isValid())
            {

            }
        }

    }

    return false;
}

bool MemoryManager::secondChance1 (int processId, RegularPageTable* pageTable, PageFrame* frame, size_t* entryNumber, size_t *diskPos, ReplaceMode mode)
{
    if (SCindex >= pageTable->getPageTableSize())
        SCindex = 0;
    // traverse all present entries in the page table
    for (size_t i = SCindex; i < pageTable->getPageTableSize(); ++i)
    {

        if (i == pageTable->getPageTableSize() - 1)
        {
            // if the last entry is reached, start from the beginning
            i = 0;
        }  

        PageTableEntry* entry = pageTable->getEntry(processId, i);
        if (entry->isPresent())
        {
            // check if it is referenced
            if (entry->isReferenced())
            {
                // set referenced bit to 0
                entry->setReferenced(false);

                // increment SCindex
                SCindex = (SCindex + 1) % pageTable->getPageTableSize();
            }
            else
            {
                // page to be replaced is found
                
                // find index of the frame to be replaced
                for (size_t j = 0; j < physicalMemory->getNumFrames(); ++j)
                {
                    if (physicalMemory->getFrame(j)->getFrameNumber() == entry->getFrameNumber())
                    {
                        
                        if (mode == ReplaceMode::APPEND)
                        {
                            // write the page frame to the disk
                            writePageFrameToDisk(physicalMemory->getFrame(j), diskPos);

                            // std::cerr << "written to disk" << std::endl;
                            // set the page table entry
                            entry->setPresent(false);
                            entry->setDiskLineNumber(*diskPos);

                            // set the frame
                            frame->setProcessId(processId);
                            frame->setNumValues(0);
                            frame->full = false;

                            // initialize the frame
                            physicalMemory->setFrame(j, frame);

                            // add entry to the page table
                            
                            // find invalid entry, and set it
                            for (size_t k = 0; k < pageTable->getPageTableSize(); ++k)
                            {
                                if (!pageTable->getEntry(processId, k)->isValid())
                                {
                                    size_t frameNumber = frame->getFrameNumber();
                                    pageTable->addEntry(processId, &frameNumber, &k);
                                    *entryNumber = k;
                                    break;
                                }
                            }


                            // increment SCindex
                            SCindex = (SCindex + 1) % pageTable->getPageTableSize();

                            return true;
                        }
                        else if (mode == ReplaceMode::OVERWRITE_AND_READ)
                        {

                            // std::cout << "\nFrame to be replaced: " << physicalMemory->getFrame(j)->getFrameNumber() << std::endl;
                            // read the new page frame from the disk, and write old page frame on top of it in the disk
                            overwriteAndReadFromDisk(frame, physicalMemory->getFrame(j), diskPos);

                            // set old frame's disk line number to same as new frame's disk line number
                            pageTable->getEntry(processId, physicalMemory->getFrame(j)->getFrameNumber())->setDiskLineNumber(pageTable->getEntry(processId, frame->getFrameNumber())->getDiskLineNumber());

                            // set the page table entry
                            entry->setPresent(false);

                            // std::cout << "2new frame number: " << frame->getFrameNumber() << std::endl;
                            // set frame on the physical memory
                            physicalMemory->setFrame(j, frame);

                            // increment SCindex
                            SCindex = (SCindex + 1) % pageTable->getPageTableSize();
                            return true;
                        }

                        else if (mode == ReplaceMode::READ_AND_DELETE)
                        {
                            // read the new page frame from the disk, and delete it from the disk

                            return false;
                        }
                        return true;
                    }
                }
            }
        }      
    }
    std::cout << "sc page replacement failed" << std::endl;
    return false;
}

// implements lru, using last reference time
bool MemoryManager::lru1 (int processId, RegularPageTable* pageTable, PageFrame* frame, size_t* entryNumber, size_t *diskPos, ReplaceMode mode)
{
    std::chrono::time_point<std::chrono::system_clock> minTime = std::chrono::system_clock::now();
    *entryNumber = 0;

    bool flag = false;
    // traverse all present entries in the page table
    for (size_t i = 0; i < pageTable->getPageTableSize(); ++i)
    {
        PageTableEntry* entry = pageTable->getEntry(processId, i);
        if (entry->isPresent())
        {
            // check if its time is less than minTime
            if (entry->lastReferenceTime < minTime)
            {
                // set minTime
                minTime = entry->lastReferenceTime;
                // set entry number
                *entryNumber = i;
                flag = true;
            }            
        }
    }

    if (!flag)
        return false;

    // find index of the frame to be replaced
    PageTableEntry *entry = pageTable->getEntry(processId, *entryNumber);

    // get frame to be replaced
    PageFrame *oldFrame = physicalMemory->getFrameWithFrameNumber(entry->getFrameNumber());

    // std::cout << "frame to be replaced: " << oldFrame->getFrameNumber() << std::endl;

    if (mode == ReplaceMode::APPEND)
    {
        // write the page frame to the disk
        writePageFrameToDisk(oldFrame, diskPos);

        // set the page table entry
        entry->setPresent(false);
        entry->setDiskLineNumber(*diskPos);

        // set the frame
        frame->setProcessId(processId);
        frame->setNumValues(0);
        frame->full = false;

        // add entry to the page table

        flag = false;
        // find invalid entry, and set it
        for (size_t k = 0; k < pageTable->getPageTableSize(); ++k)
        {
            if (!pageTable->getEntry(processId, k)->isValid())
            {
                size_t frameNumber = frame->getFrameNumber();
                pageTable->addEntry(processId, &frameNumber, &k);
                *entryNumber = k;
                flag = true;
                break;
            }
        }

        if (!flag)
            return false;

        // std::cout << "entry number = " << *entryNumber << std::endl;
        // std::cout << "page frame number = " << pageTable->getEntry(processId, *entryNumber)->getFrameNumber() << std::endl;

        // PageTableEntry *newEntry = pageTable->getEntry(processId, *entryNumber);
        // set the frame
        
        for (size_t i = 0; i < physicalMemory->getNumFrames(); ++i)
        {
            if (physicalMemory->getFrame(i)->getFrameNumber() == entry->getFrameNumber())
            {
                physicalMemory->setFrame(i, frame);
                break;
            }
        }

        // std::cout << "frame number = " << frame->getFrameNumber() << std::endl;

        return true;
    }

    else if (mode == ReplaceMode::OVERWRITE_AND_READ)
    {
        // read the new page frame from the disk, and write old page frame on top of it in the disk
        overwriteAndReadFromDisk(frame, oldFrame, diskPos);

        // set disk line number
        // pageTable->getEntry(processId, frame->getFrameNumber())->setDiskLineNumber(pageTable->getEntry(processId, oldFrame->getFrameNumber())->getDiskLineNumber());

        // set old frame's disk line number to same as new frame's disk line number
        pageTable->getEntry(processId, oldFrame->getFrameNumber())->setDiskLineNumber(pageTable->getEntry(processId, frame->getFrameNumber())->getDiskLineNumber());
        
        // set the page table entry
        entry->setPresent(false);

        // find the frame and set it
        for (size_t i = 0; i < physicalMemory->getNumFrames(); ++i)
        {
            if (physicalMemory->getFrame(i)->getFrameNumber() == entry->getFrameNumber())
            {
                physicalMemory->setFrame(i, frame);
                return true;
            }
        }
        return false;
    }

    else if (mode == ReplaceMode::READ_AND_DELETE)
    {
        // read the new page frame from the disk, and delete it from the disk

        return false;
    }

    return false;
}

bool MemoryManager::wsclock1 (int processId, RegularPageTable* pageTable, PageFrame* frame, size_t* entryNumber, size_t *diskPos, ReplaceMode mode)
{
    // get current time
    std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
    
    // smallest duration
    std::chrono::duration<double> minDuration = std::chrono::duration<double>::max();

    bool flag = false;

    if (WSCLOCKindex >= pageTable->getPageTableSize() - 1)
        WSCLOCKindex = 0;

    // traverse all present entries in the page table
    for (size_t i = this->WSCLOCKindex; i < pageTable->getPageTableSize(); ++i)
    {
        if (i == pageTable->getPageTableSize() - 1)
        {
            // if the last entry is reached, start from the beginning
            i = 0;
            WSCLOCKindex = 0;

            if (flag)
                break;
            }

        // std::cout << "i : " << i << std::endl;
        // std::cout << "WSCLOCKindex : " << WSCLOCKindex << std::endl;

        PageTableEntry* entry = pageTable->getEntry(processId, i);
        // std::cout << "page frame number: " << entry->getFrameNumber();
        // std::cout << " present: " << entry->isPresent() << std::endl;
        if (entry->isPresent())
        {            
            // check if it is referenced and threshold time has passed
            if (entry->isReferenced())
            {
                // set referenced bit to 0
                entry->setReferenced(false);
                // set the last reference time
                entry->lastReferenceTime = currentTime;
            }
            else
            {
                bool cond = (currentTime - entry->lastReferenceTime) > this->threshold;

                if (cond)
                {
                    // page to be replaced is found   
                    // set entry number
                    *entryNumber = entry->getPageNumber();
                    flag = true;

                    WSCLOCKindex = (i + 1) % pageTable->getPageTableSize();
                    break;
                }
                
                else 
                {
                    // check if it is the smallest duration
                    if ((currentTime - entry->lastReferenceTime) < minDuration)
                    {
                        // set minDuration
                        minDuration = currentTime - entry->lastReferenceTime;
                        // set entry number
                        *entryNumber = entry->getPageNumber();
                        flag = true;
                    }
                } 

                // std::cout << "duration: " << std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - entry->lastReferenceTime).count() << std::endl;
            }
        
            WSCLOCKindex = (i + 1) % pageTable->getPageTableSize();
        }
    }


    if (!flag)
        return false;

    // find index of the frame to be replaced
    PageTableEntry *entry = pageTable->getEntry(processId, *entryNumber);

    // get frame to be replaced
    PageFrame *oldFrame = physicalMemory->getFrameWithFrameNumber(entry->getFrameNumber());

    // std::cout << "frame to be replaced: " << oldFrame->getFrameNumber() << std::endl;
    // std::cout << "frame to be added: " << frame->getFrameNumber() << std::endl;
    if (mode == ReplaceMode::APPEND)
    {
        // write the page frame to the disk
        writePageFrameToDisk(oldFrame, diskPos);

        // set the page table entry
        entry->setPresent(false);
        entry->setDiskLineNumber(*diskPos);

        // set the frame
        frame->setProcessId(processId);
        frame->setNumValues(0);
        frame->full = false;

        // add entry to the page table

        flag = false;
        // find invalid entry, and set it
        for (size_t k = 0; k < pageTable->getPageTableSize(); ++k)
        {
            if (!pageTable->getEntry(processId, k)->isValid())
            {
                size_t frameNumber = frame->getFrameNumber();
                pageTable->addEntry(processId, &frameNumber, &k);
                *entryNumber = k;
                flag = true;
                break;
            }
        }

        if (!flag)
            return false;

        // std::cout << "entry number = " << *entryNumber << std::endl;

        // PageTableEntry *newEntry = pageTable->getEntry(processId, *entryNumber);
        // set the frame

        for (size_t i = 0; i < physicalMemory->getNumFrames(); ++i)
        {
            if (physicalMemory->getFrame(i)->getFrameNumber() == entry->getFrameNumber())
            {
                physicalMemory->setFrame(i, frame);
                break;
            }
        }

        // std::cout << "frame number = " << frame->getFrameNumber() << std::endl;
        return true;
    }
    else if (mode == ReplaceMode::OVERWRITE_AND_READ)
    {
        // read the new page frame from the disk, and write old page frame on top of it in the disk
        overwriteAndReadFromDisk(frame, oldFrame, diskPos);

        // set disk line number
        // pageTable->getEntry(processId, frame->getFrameNumber())->setDiskLineNumber(pageTable->getEntry(processId, oldFrame->getFrameNumber())->getDiskLineNumber());

        // set old frame's disk line number to same as new frame's disk line number
        pageTable->getEntry(processId, oldFrame->getFrameNumber())->setDiskLineNumber(pageTable->getEntry(processId, frame->getFrameNumber())->getDiskLineNumber());

        // set the page table entry
        entry->setPresent(false);


        // find the frame and set it
        for (size_t i = 0; i < physicalMemory->getNumFrames(); ++i)
        {
            if (physicalMemory->getFrame(i)->getFrameNumber() == entry->getFrameNumber())
            {
                physicalMemory->setFrame(i, frame);
                return true;
            }
        }

        return false;
    }

    return false;
}

void writeSpaces (std::ofstream& file, size_t numSpaces)
{
    for (size_t i = 0; i < numSpaces; ++i)
    {
        file << " ";
    }
}

void writeSpaces2 (std::fstream& file, size_t numSpaces)
{
    for (size_t i = 0; i < numSpaces; ++i)
    {
        file << " ";
    }
}

bool MemoryManager::writePageFrameToDisk (PageFrame* frame, size_t *diskLine)
{
    // open disk file
    std::ofstream diskFile;
    diskFile.open(diskFileName, std::ofstream::out | std::ofstream::app);

    // write to disk

    // get the current position of the cursor
    *diskLine = diskFile.tellp();

    diskFile << std::endl;
    // write process id
    diskFile << "Process Id: " << frame->getProcessId();
    writeSpaces(diskFile, diskEntrySegmentSpace - (std::string("Process Id: ").length() + std::to_string(frame->getProcessId()).length()));
    diskFile << std::endl;

    // write frame number
    diskFile << "Frame Number: " << frame->getFrameNumber();
    writeSpaces(diskFile, diskEntrySegmentSpace - (std::string("Frame Number: ").length() + std::to_string(frame->getFrameNumber()).length()));
    diskFile << std::endl;

    // write frame size
    diskFile << "Frame Size: " << frame->getFrameSize();
    writeSpaces(diskFile, diskEntrySegmentSpace - (std::string("Frame Size: ").length() + std::to_string(frame->getFrameSize()).length()));
    diskFile << std::endl;

    // write number of values
    diskFile << "Number of Values: " << frame->getNumValues();
    writeSpaces(diskFile, diskEntrySegmentSpace - (std::string("Number of Values: ").length() + std::to_string(frame->getNumValues()).length()));
    diskFile << std::endl;

    // write full
    diskFile << "Full: " << frame->isFull();
    writeSpaces(diskFile, diskEntrySegmentSpace - (std::string("Full: ").length() + std::to_string(frame->isFull()).length()));
    diskFile << std::endl;

    // write every value
    for (size_t i = 0; i < frame->getFrameSize(); ++i)
    {
        diskFile << frame->getValue(i);
        writeSpaces(diskFile, diskEntrySegmentSpace - std::to_string(frame->getValue(i)).length());
        diskFile << std::endl;
    }

    numDiskWrites++;

    // close disk file
    diskFile.close();

    // std::cout << "position: " << *diskLine << std::endl;

    return true;
}

bool MemoryManager::overwriteAndReadFromDisk (PageFrame *newFrame, PageFrame *oldFrame, size_t *diskLine)
{
    // open disk file
    std::fstream diskFile;
    
    // open in read/write mode
    diskFile.open(diskFileName, std::fstream::in | std::fstream::out);

    // set cursor to the given position
    diskFile.seekp(*diskLine);
    // read the page frame from the disk
    // read process id

    // it is in string format, "Process Id: 0"
    std::string line;

    // skip the first line
    std::getline(diskFile, line);    
    std::getline(diskFile, line);

    // check if line is empty
    if (line.empty())
        return false;


    // get the process id
    newFrame->processId = std::stoi(line.substr(12));

    // read frame number, "Frame Number: 0"
    std::getline(diskFile, line);

    // check if line is empty
    if (line.empty())
        return false;
    stringToSizeT(line.substr(14), &newFrame->frameNumber);
    // std::cout << "line: " << line << std::endl;
    // std::cout << "new frame number: " << newFrame->getFrameNumber() << std::endl;

    // read frame size, "Frame Size: 0"
    std::getline(diskFile, line);

    // check if line is empty
    if (line.empty())
        return false;
    stringToSizeT(line.substr(12), &newFrame->frameSize);
    // std::cout << "line frame size: " << line << std::endl;
    // std::cout << "new frame size: " << newFrame->getFrameSize() << std::endl;
    // read number of values, "Number of Values: 0"
    std::getline(diskFile, line);

    // check if line is empty
    if (line.empty())
        return false;
    stringToSizeT(line.substr(18), &newFrame->numValues);

    // read full, "Full: 0"
    std::getline(diskFile, line);

    // check if line is empty
    if (line.empty())
        return false;
    int fval = std::stoi(line.substr(6));
    newFrame->full = (fval == 1);

    // read every value
    for (size_t i = 0; i < newFrame->getFrameSize(); ++i)
    {
        std::getline(diskFile, line);
        if (line.empty())
            return false;
        newFrame->setValue(i, std::stoi(line));
    }

    // write the old page frame on top of the new one
    // set cursor to the given position
    diskFile.seekp(*diskLine);

    // write to disk
    diskFile << std::endl;
    // write process id
    diskFile << "Process Id: " << oldFrame->getProcessId();
    writeSpaces2(diskFile, diskEntrySegmentSpace - (std::string("Process Id: ").length() + std::to_string(oldFrame->getProcessId()).length()));
    diskFile << std::endl;

    // write frame number
    diskFile << "Frame Number: " << oldFrame->getFrameNumber();
    // std::cout << "old frame number: " << oldFrame->getFrameNumber() << std::endl;
    writeSpaces2(diskFile, diskEntrySegmentSpace - (std::string("Frame Number: ").length() + std::to_string(oldFrame->getFrameNumber()).length()));
    diskFile << std::endl;

    // write frame size
    diskFile << "Frame Size: " << oldFrame->getFrameSize();
    writeSpaces2(diskFile, diskEntrySegmentSpace - (std::string("Frame Size: ").length() + std::to_string(oldFrame->getFrameSize()).length()));
    diskFile << std::endl;

    // write number of values
    diskFile << "Number of Values: " << oldFrame->getNumValues();
    writeSpaces2(diskFile, diskEntrySegmentSpace - (std::string("Number of Values: ").length() + std::to_string(oldFrame->getNumValues()).length()));
    diskFile << std::endl;

    // write full
    diskFile << "Full: " << oldFrame->isFull();
    writeSpaces2(diskFile, diskEntrySegmentSpace - (std::string("Full: ").length() + std::to_string(oldFrame->isFull()).length()));
    diskFile << std::endl;

    // write every value
    for (size_t i = 0; i < oldFrame->getFrameSize(); ++i)
    {
        diskFile << oldFrame->getValue(i);
        writeSpaces2(diskFile, diskEntrySegmentSpace - std::to_string(oldFrame->getValue(i)).length());
        diskFile << std::endl;
    }

    // close disk file
    diskFile.close();

    numDiskWrites++;
    numDiskReads++;

    return true;
}

void MemoryManager::printStatistics ()
{
    std::cout << "\nNumber of Reads: " << numReads << std::endl;
    std::cout << "Number of Writes: " << numWrites << std::endl;
    std::cout << "Number of Page Misses: " << numPageMisses << std::endl;
    std::cout << "Number of Page Replacements: " << numPageReplacements << std::endl;
    std::cout << "Number of Disk Reads: " << numDiskReads << std::endl;
    std::cout << "Number of Disk Writes: " << numDiskWrites << std::endl << std::endl;
}

void MemoryManager::resetStatistics ()
{
    numReads = 0;
    numWrites = 0;
    numPageMisses = 0;
    numPageReplacements = 0;
    numDiskReads = 0;
    numDiskWrites = 0;
}

void MemoryManager::printPageTableInfo (int processId)
{
    // print page table info
    std::cout << "\nPage Table Info" << std::endl;

    if (!invertedFlag)
    {
        RegularPageTable* pageTable = (RegularPageTable*) pageTables[processId];
        size_t numPagesInDisk = 0;
        size_t numPagesInMemory = 0;
        
        for (size_t i = 0; i < pageTable->getPageTableSize(); ++i)
        {
            PageTableEntry* entry = pageTable->getEntry(0, i);
            
            if (!entry->isValid())
                continue;
            if (entry->isPresent())
            {
                numPagesInMemory++;
            }
            else
            {
                numPagesInDisk++;
            }
        }

        std::cout << "Number of Pages in Memory: " << numPagesInMemory << std::endl;
        std::cout << "Number of Pages in Disk: " << numPagesInDisk << std::endl;
        printStatistics();
    }
    else
    {

        return;
    }
}

// bool MemoryManager::secondChance2 (int processId, InvertedPageTable *pageTable, PageFrame *frame, size_t* entryNum, size_t *diskPos, ReplaceMode mode)
// {    
//     // traverse all present entries in the page table

//     std::vector<PageTableEntry*> entries = pageTable->entries->at(processId);
//     std::cout << "entries size: " << entries.size() << std::endl;

//     if (SCindex >= entries.size())
//         SCindex = 0;

//     for (size_t i = SCindex; i < entries.size(); ++i)
//     {
//         if (i == entries.size() - 1)
//         {
//             // if the last entry is reached, start from the beginning
//             i = 0;
//         }  

//         PageTableEntry* entry = entries.at(i);
//         if (entry->isPresent())
//         {
//             // std::cout << "\nsc entry number = " << i << std::endl;
//             // std::cout << "sc page number = " << entry->getPageNumber() << std::endl;
//             std::cout << "sc frame number = " << entry->getFrameNumber() << std::endl;
//             // std::cout << "sc present = " << entry->isPresent() << std::endl;
//             // std::cout << "sc referenced = " << entry->isReferenced() << std::endl;

//             // check if it is referenced
//             if (entry->isReferenced())
//             {
//                 // set referenced bit to 0
//                 entry->setReferenced(false);

//                 // increment SCindex
//                 SCindex = (SCindex + 1) % entries.size();
//             }
//             else
//             {
//                 // page to be replaced is found
                
//                 // find index of the frame to be replaced
//                 for (size_t j = 0; j < physicalMemory->getNumFrames(); ++j)
//                 {
//                     if (physicalMemory->getFrame(j)->getFrameNumber() == entry->getFrameNumber())
//                     {
                        
//                         if (mode == ReplaceMode::APPEND)
//                         {
//                             // write the page frame to the disk
//                             writePageFrameToDisk(physicalMemory->getFrame(j), diskPos);

//                             // std::cerr << "written to disk" << std::endl;
//                             // set the page table entry
//                             entry->setPresent(false);
//                             entry->setDiskLineNumber(*diskPos);

//                             // std::cout << "\nentry number = " << *entryNumber << std::endl;
//                             // std::cout << "disk line number = " << entry->getDiskLineNumber() << std::endl;

//                             // set the frame
//                             frame->setProcessId(processId);
//                             frame->setNumValues(0);
//                             frame->full = false;

//                             // initialize the frame
//                             physicalMemory->setFrame(j, frame);

//                             // add entry to the page table
                            
//                             // find invalid entry, and set it
//                             for (size_t k = 0; k < entries.size(); ++k)
//                             {
//                                 if (!entries.at(k)->isValid())
//                                 {
//                                     size_t frameNumber = frame->getFrameNumber();
//                                     PageTableEntry *invalidEntry = entries.at(k);
//                                     invalidEntry->setFrameNumber(frameNumber);
//                                     invalidEntry->setValid(true);
//                                     invalidEntry->setPresent(true);
//                                     invalidEntry->setDiskLineNumber(*diskPos);

//                                     *entryNum = k;
//                                     break;
//                                 }
//                             }

//                             // increment SCindex
//                             SCindex = (SCindex + 1) % entries.size();
//                             return true;
//                         }

//                         else if (mode == ReplaceMode::OVERWRITE_AND_READ)
//                         {
                                
//                                 // std::cout << "\nFrame to be replaced: " << physicalMemory->getFrame(j)->getFrameNumber() << std::endl;
//                                 // read the new page frame from the disk, and write old page frame on top of it in the disk
//                                 overwriteAndReadFromDisk(frame, physicalMemory->getFrame(j), diskPos);
    
//                                 // set disk line number
//                                 // pageTable->getEntry(processId, frame->getFrameNumber())->setDiskLineNumber(pageTable->getEntry(processId, physicalMemory->getFrame(j)->getFrameNumber())->getDiskLineNumber());
    
//                                 // set old frame's disk line number to same as new frame's disk line number
//                                 entries.at(i)->setDiskLineNumber(entries.at(j)->getDiskLineNumber());

//                                 // set the page table entry
//                                 entry->setPresent(false);
    
//                                 // std::cout << "2new frame number: " << frame->getFrameNumber() << std::endl;
//                                 // set frame on the physical memory
//                                 physicalMemory->setFrame(j, frame);
    
//                                 // increment SCindex
//                                 SCindex = (SCindex + 1) % entries.size();
//                                 return true;
//                         }
//                     }
//                 }
//             }
//         }
//     }

//     return false;
// }

// bool MemoryManager::lru2 (int processId, InvertedPageTable *pageTable, PageFrame *frame, size_t* entryNum, size_t *diskPos, ReplaceMode mode)
// {
//     return false;
// }

// bool MemoryManager::wsclock2 (int processId, InvertedPageTable *pageTable, PageFrame *frame, size_t* entryNum, size_t *diskPos, ReplaceMode mode)
// {
//     return false;
// }
