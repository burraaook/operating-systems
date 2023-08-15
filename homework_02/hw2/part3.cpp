#include "include/util.h"
#include "include/memoryManagement.h"

using namespace memorymanagement;

MemoryManager *memoryManager = nullptr;
size_t merge_sort ();
void merge (virtualAddress* vector, int start, int mid, int end);
void merge_sort2 (virtualAddress *array, size_t left, size_t right);
size_t quick_sort ();
void quickSort(virtualAddress* vector, int start, int end);

int main ()
{
    size_t physicalMemorySize = 3; // 2^14 = 16KB
    size_t virtualMemorySize = 7; // 2^17 = 128KB
    size_t frameSize = 0;
    bool inverted = false;
    size_t printCount = 10000000;
    std::string diskFileName = "diskFileName.dat";

    // assign maximum value of size_t
    size_t minPageMisses = std::numeric_limits<size_t>::max();

    std::cout << "Starting tests..." << std::endl;

    std::cout << "Testing with merge sort" << std::endl;
    // try frame sizes 3 up to 20, for merge sort
    for (size_t i = 3; i <= 7; i++)
    {
        std::cout << "Testing with frame size: " << i << std::endl;
        memoryManager = new MemoryManager (1, i, physicalMemorySize, virtualMemorySize, PageAlgorithm::SC, inverted, printCount, diskFileName);

        size_t result = merge_sort();
        if (result < minPageMisses)
        {
            minPageMisses = result;
            frameSize = i;
        }
    }
    delete memoryManager;

    std::cout << "Best frame size for merge sort: " << frameSize << std::endl;

    // assign maximum value of size_t
    minPageMisses = std::numeric_limits<size_t>::max();

    std::cout << "Testing with quick sort" << std::endl;
    // try frame sizes 3 up to 20, for quick sort
    for (size_t i = 3; i <= 7; i++)
    {
        std::cout << "Testing with frame size: " << i << std::endl;
        memoryManager = new MemoryManager (1, i, physicalMemorySize, virtualMemorySize, PageAlgorithm::SC, inverted, printCount, diskFileName);

        size_t result = quick_sort();
        if (result < minPageMisses)
        {
            minPageMisses = result;
            frameSize = i;
        }
    }
    delete memoryManager;

    std::cout << "Best frame size for quick sort: " << frameSize << std::endl;
}

size_t merge_sort ()
{
    size_t size = 1000; // 1000 integers
    // define virtual address pointer array
    virtualAddress *array = new virtualAddress[size];
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(1, 2000);

    for (size_t i = 0; i < size; i++)
    {
        if (!memoryManager->allocateMemory(0, &array[i]))
        {
            std::cout << "Error allocating memory" << std::endl;
            return 0;
        }
        // write random value
        int value = distribution(generator);

        if (!memoryManager->writeMemory(0, &array[i], value) != 0)
        {
            std::cout << "Error writing to memory" << std::endl;
            return 0;
        }
    }

    // fill memory with random values
    for (size_t i = 0; i < size; i++)
    {
        virtualAddress temp;

        if (!memoryManager->allocateMemory(0, &temp))
        {
            break;
        }

        int value = distribution(generator);

        if (!memoryManager->writeMemory(0, &temp, value))
        {
            std::cout << "Error writing to memory" << std::endl;
            return 0;
        }

    }

    // sort the array
    merge_sort2(array, 0, size - 1);

    size_t ret = memoryManager->getPageMissCount();

    std::cout << "Merge sort page misses: " << ret << std::endl;


    // deallocate memory
    delete [] array;
    // allocate memory for each and write random values
    return ret;
}
void merge_sort2 (virtualAddress *array, size_t left, size_t right)
{
    if (left < right)
    {
        size_t middle = (left + right) / 2;

        merge_sort2(array, left, middle);
        merge_sort2(array, middle + 1, right);

        merge(array, left, middle, right);
    }
}

void merge (virtualAddress* vector, int start, int mid, int end)
{
    int leftSize = mid - start + 1;
    int rightSize = end - mid;

    virtualAddress* left = new virtualAddress[leftSize];
    virtualAddress* right = new virtualAddress[rightSize];

    for (int i = 0; i < leftSize; i++)
    {
        left[i].copy(&vector[start + i]);
    }

    for (int i = 0; i < rightSize; i++)
    {
        right[i].copy(&vector[mid + 1 + i]);
    }

    int i = 0;
    int j = 0;
    int k = start;

    while (i < leftSize && j < rightSize)
    {
        int val1 = -1;
        int val2 = -1;

        if (!memoryManager->readMemory(0, &left[i], &val1))
            return;
        if (!memoryManager->readMemory(0, &right[j], &val2))
            return;

        if (val1 <= val2)
        {
            vector[k].copy(&left[i]);
            i++;
        }
        else
        {
            vector[k].copy(&right[j]);
            j++;
        }
        k++;
    }

    while (i < leftSize)
    {
        vector[k].copy(&left[i]);
        i++;
        k++;
    }

    while (j < rightSize)
    {
        vector[k].copy(&right[j]);
        j++;
        k++;
    }
}
size_t quick_sort ()
{
    size_t size = 1000; // 1000 integers
    // define virtual address pointer array
    
    virtualAddress *array = new virtualAddress[size];
    std::random_device rd;
    std::mt19937 generator(rd());

    std::uniform_int_distribution<int> distribution(1, 2000);

    for (size_t i = 0; i < size; i++)
    {
        if (!memoryManager->allocateMemory(0, &array[i]))
        {
            std::cout << "Error allocating memory" << std::endl;
            return 0;
        }
        // write random value
        int value = distribution(generator);

        if (!memoryManager->writeMemory(0, &array[i], value) != 0)
        {
            std::cout << "Error writing to memory" << std::endl;
            return 0;
        }
    }

    // fill memory with random values
    for (size_t i = 0; i < size; i++)
    {
        virtualAddress temp;

        if (!memoryManager->allocateMemory(0, &temp))
        {
            break;
        }

        int value = distribution(generator);

        if (!memoryManager->writeMemory(0, &temp, value))
        {
            std::cout << "Error writing to memory" << std::endl;
            return 0;
        }
    }

    // sort the array
    quickSort(array, 0, size - 1);

    std::cout << "Quick sort page misses: " << memoryManager->getPageMissCount() << std::endl;

    size_t ret = memoryManager->getPageMissCount();
    
    return ret;
}

void quickSort (virtualAddress* vector, int start, int end)
{
    if (start < end)
    {
        int i = start - 1;

        for (int j = start; j <= end - 1; j++)
        {
            int val1 = -1;
            int val2 = -1;

            if (!memoryManager->readMemory(0, &vector[j], &val1))
                return;
            if (!memoryManager->readMemory(0, &vector[end], &val2))
                return;

            if (val1 <= val2)
            {
                i++;
                virtualAddress temp;
                temp.copy(&vector[i]);
                vector[i].copy(&vector[j]);
                vector[j].copy(&temp);
            }
        }

        virtualAddress temp;
        temp.copy(&vector[i + 1]);
        vector[i + 1].copy(&vector[end]);
        vector[end].copy(&temp);

        int partition = i + 1;

        quickSort(vector, start, partition - 1);
        quickSort(vector, partition + 1, end);
    }
}