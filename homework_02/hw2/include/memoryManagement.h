
#ifndef MEMORYMANAGEMENT_H
#define MEMORYMANAGEMENT_H

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include "util.h"

namespace memorymanagement
{

    class physicalAddress
    {
    friend class MemoryManager;
    friend class MMU;

    private:
        size_t frameNumber;
        size_t valueOffset;
        size_t pageFrameSize;  // as integer power of 2
        size_t memorySize; // as integer power of 2

    public:
        physicalAddress (const size_t *memorySize);
        physicalAddress (size_t frameNumber, size_t valueOffset, size_t pageFrameSize, size_t memorySize);
        ~physicalAddress ();

        // increment operator
        physicalAddress& operator++ ();
        physicalAddress operator++ (int);
    
        void print ();
    };

    class virtualAddress
    {
    friend class MemoryManager;
    friend class MMU;
    
    private:
        size_t pageTableEntryNumber;
        size_t valueOffset;
        size_t pageFrameSize;  // as integer power of 2
        size_t pageTableSize;  // as integer power of 2
    public:
        virtualAddress ();
        virtualAddress (size_t pageTableEntryNumber, size_t valueOffset, size_t pageFrameSize, size_t pageTableSize);
        ~virtualAddress ();

        // increment operator
        virtualAddress& operator++ ();
        virtualAddress operator++ (int);

        // method
        void copy (virtualAddress *address);

        void print () const;
            
    };

    class ArrayType
    {
    private:
        size_t size;
        // addresses of each element in the array
        virtualAddress *addresses;

    public:
        ArrayType (size_t size);
        ~ArrayType ();

        // getters
        size_t getSize ();
        virtualAddress getAddress (size_t index);

        // setters
        void setSize (size_t size);
        void setAddress (size_t index, virtualAddress address);        
    };

    class PageFrame
    {
    friend class MemoryManager;
    private:
        size_t frameNumber;
        int processId;
        size_t frameSize;  // as integer power of 2
        size_t numValues;
        int *values;
        bool full;

    public:

        PageFrame (size_t frameNumber, int processId, size_t frameSize);
        ~PageFrame ();

        // getters
        size_t getFrameNumber ();
        int getProcessId ();
        size_t getFrameSize ();
        size_t getNumValues ();

        // setters
        void setFrameNumber (size_t frameNumber);
        void setProcessId (int processId);
        void setFrameSize (size_t frameSize);
        void setNumValues (size_t numValues);

        // methods
        bool addValue (int value);
        int getValue (size_t index);
        bool setValue (size_t index, int value);

        bool isFull ();
        void print ();
    };

    class PhysicalMemory
    {
    private:
        size_t numFrames;
        size_t frameSize;  // as integer power of 2
        PageFrame **frames; // array of pointers to PageFrame objects
        size_t memorySize;
    public:
        PhysicalMemory (size_t numFrames, size_t frameSize, size_t memorySize);
        ~PhysicalMemory ();

        // getters
        size_t getNumFrames ();
        size_t getFrameSize ();
        size_t getMemorySize ();

        // setters
        void setNumFrames (size_t numFrames);
        void setFrameSize (size_t frameSize);
        bool setFrame (PageFrame *frame);

        // methods
        bool addFrame (PageFrame *frame);
        PageFrame *getFrame (size_t index);
        void setFrame (int index, PageFrame *frame);
        bool setFrame (size_t index, PageFrame *frame);
        PageFrame* getFrameWithFrameNumber (size_t frameNumber);
        void print ();
    };

    class PageTableEntry
    {
    friend class MemoryManager;
    private:
        int pageNumber;
        size_t frameNumber;
        int processId;

        // valid means that the page is in the physical memory or disk
        bool valid;
        bool dirty;
        bool referenced;


        // present means that the page is in the physical memory
        bool present;
        bool modified;
        int protection;
        size_t pageFrameSize;  // as integer power of 2

        size_t diskLineNumber;

        // last time the page was referenced
        std::chrono::time_point<std::chrono::system_clock> lastReferenceTime;

    public:
        PageTableEntry ();
        PageTableEntry (int pageNumber, size_t frameNumber, int processId, size_t pageFrameSize);
        ~PageTableEntry ();

        // getters
        int getPageNumber ();
        size_t getFrameNumber ();
        int getProcessId ();
        bool isValid ();
        bool isDirty ();
        bool isReferenced ();
        bool isPresent ();
        bool isModified ();
        int getProtection ();
        size_t getPageFrameSize ();
        size_t getDiskLineNumber ();

        // setters
        void setPageNumber (int pageNumber);
        void setFrameNumber (size_t frameNumber);
        void setProcessId (int processId);
        void setValid (bool valid);
        void setDirty (bool dirty);
        void setReferenced (bool referenced);
        void setPresent (bool present);
        void setModified (bool modified);
        void setProtection (int protection);
        void setPageFrameSize (size_t pageFrameSize);
        void setDiskLineNumber (size_t diskLineNumber);

        // methods
        void print ();

    };

    // virtual page table class
    class PageTable
    {
    public:
        PageTable ();
        virtual ~PageTable ();

        // getters
        virtual int getNumProcesses () = 0;
        virtual size_t getNumEntries (int processId) = 0;
        virtual size_t getPageFrameSize () = 0;
        virtual size_t getPageTableSize () = 0;
        virtual PageTableType getType () = 0;
        virtual PageTableEntry *getEntry (int processId, int pageNumber) = 0;

        // setters
        virtual bool setEntry (int processId, int pageNumber, PageTableEntry *entry) = 0;

        // methods
        virtual void print () = 0;
        virtual bool addEntry (int processId, PageTableEntry *entry) = 0;
        // virtual bool addValue (int processId, int pageNumber, int value) = 0;
    };

    // regular page table class, it only has page table entries for one process
    class RegularPageTable : public PageTable
    {
    friend class MemoryManager;

    private:
        size_t numEntries;
        int processId;
        size_t pageFrameSize;  // as integer power of 2
        size_t pageTableSize;  // as integer power of 2
        PageTableType type;

        // page table entries
        PageTableEntry **entries;

    public:
        RegularPageTable (int processId, size_t pageFrameSize, size_t pageTableSize);
        ~RegularPageTable ();

        // getters
        int getNumProcesses ();
        size_t getNumEntries (int processId);
        size_t getPageFrameSize ();
        size_t getPageTableSize ();
        PageTableType getType ();
        PageTableEntry *getEntry (int processId, int pageNumber);

        // setters
        bool setEntry (int processId, int pageNumber, PageTableEntry *entry);

        // methods
        void print ();
        bool addEntry (int processId, PageTableEntry *entry);
        bool addEntry (int processId, const size_t *pageFrameNumber, size_t *entryNumber);

        // bool addValue (int processId, int pageNumber, int value);
    };


    class InvertedPageTable : public PageTable
    {
    friend class MemoryManager;

    private:
        int numProcesses;
        size_t pageFrameSize;  // as integer power of 2
        PageTableType type;
        size_t pageTableSize;  // as integer power of 2

        // page table entries for each process, <processId, list of entries>
        std::unordered_map<int, std::vector<PageTableEntry *>> *entries;

    public:
        InvertedPageTable (size_t pageFrameSize, size_t pageTableSize);
        ~InvertedPageTable ();

        // getters
        int getNumProcesses ();
        size_t getNumEntries (int processId);
        size_t getPageFrameSize ();
        size_t getPageTableSize ();
        PageTableType getType ();
        PageTableEntry *getEntry (int processId, int pageNumber);

        // setters
        bool setEntry (int processId, int pageNumber, PageTableEntry *entry);

        // methods
        void print ();

        PageTableEntry *firstAvailableEntry (int processId);
        bool addEntry (int processId, PageTableEntry *entry);
        // bool addValue (int processId, int pageNumber, int value);
    };

    class MMU
    {
    private:
        
        // different tlbs for each process
        std::unordered_map<int, std::vector<PageTableEntry *>> *tlbs;
        
        size_t pageFrameSize;  // as integer power of 2
        size_t tlbSize;

    public:
        MMU (size_t pageFrameSize, size_t tlbSize);
        ~MMU ();

        // getters
        size_t getPageFrameSize ();
        size_t getTlbSize ();

        // setters
        void setPageFrameSize (size_t pageFrameSize);
        void setTlbSize (size_t tlbSize);

        // methods
        bool addEntry (int processId, PageTableEntry *entry);
        PageTableEntry *getEntry (int processId, int pageNumber);
        bool setEntry (int processId, int pageNumber, PageTableEntry *entry);
    
        bool addressTranslation (int processId, const virtualAddress *virtualAddress, physicalAddress *physicalAddress, const size_t *frameNumber);
    };

    class MemoryManager
    {
    private:
        PhysicalMemory *physicalMemory;
        
        // page tables for each process, if it is inverted, then there is only one page table
        PageTable **pageTables;

        // memory management unit
        MMU *mmu;

        int numProcesses;
        int maxProcesses;
        size_t maxPageFrameNumber;

        // input parameters
        std::string diskFileName;
        bool invertedFlag;
        size_t printCount;
        PageAlgorithm pageAlgorithm;
        size_t frameSize;  // as integer power of 2
        size_t physicalMemorySize;
        size_t pageTableSize;

        // statistics
        size_t numReads;
        size_t numWrites;
        size_t numPageMisses;
        size_t numPageReplacements;
        size_t numDiskWrites;
        size_t numDiskReads;

        // page algorithm variables
        size_t SCindex; // second chance index
        size_t WSCLOCKindex; // wsclock index
        // threshold for wsclock, it is compared with (current time - last reference time)
        std::chrono::duration<double> threshold;
        
        // estimated working set w()
        
    public:
        MemoryManager (int maxProcesses, size_t frameSize, size_t physicalMemorySize, size_t pageTableSize, PageAlgorithm pageAlgorithm,
                        bool invertedFlag, size_t printCount, std::string diskFileName);
        ~MemoryManager ();

        // getters
        std::string getDiskFileName ();
        bool getInvertedFlag ();
        size_t getPrintCount ();
        PageAlgorithm getPageAlgorithm ();
        int getNumProcesses ();
        size_t getFrameSize ();
        size_t getPhysicalMemorySize ();
        size_t getPageTableSize ();
        size_t getPageMissCount ();
        // methods

        // // initalize the memory manager for a process
        // void initializeProcess (int processId);

        // allocate memory for a process, return true if successful
        bool allocateMemory (int processId, virtualAddress *virtualAddress);
        // bool allocateMemory (int processId, const size_t *size, virtualAddress *virtualAddress);

        // deallocate memory for a process, return true if successful
        // bool freeMemory (int processId, const size_t* free_size, const size_t *start_address);

        // // read a value from memory, return true if successful
        bool readMemory (int processId, const virtualAddress *address, int *value);

        // // write a value to memory, return true if successful
        bool writeMemory (int processId, const virtualAddress *address, int value);

        // page replacement algorithms
        bool secondChance1 (int processId, RegularPageTable *pageTable, PageFrame *frame, size_t* entryNum, size_t *diskPos, ReplaceMode mode);
        bool lru1 (int processId, RegularPageTable *pageTable, PageFrame *frame, size_t* entryNum, size_t *diskPos, ReplaceMode mode);
        bool wsclock1 (int processId, RegularPageTable *pageTable, PageFrame *frame, size_t* entryNum, size_t *diskPos, ReplaceMode mode);

        bool writePageFrameToDisk (PageFrame *frame, size_t *diskLine);
        bool overwriteAndReadFromDisk (PageFrame *newFrame, PageFrame *oldFrame, size_t *diskLine);

        // print the memory manager
        void printStatistics ();
        void resetStatistics ();
        void printPageTableInfo (int processId);
    };
} // namespace virtualMemory

#endif // MEMORY_MANAGEMENT_H