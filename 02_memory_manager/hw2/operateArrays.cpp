#include "include/memoryManagement.h"
#include "include/util.h"

using namespace memorymanagement;

struct ThreadMatrixVectorMultiplyArgs
{
    virtualAddress** matrix;
    virtualAddress* vector;
    virtualAddress* result;
    size_t matrixRow;
    size_t matrixCol;
    size_t vectorSize;
    size_t start;
    size_t end;
};

struct ThreadTransposeVectorMultiplyArgs
{
    virtualAddress* vector;
    virtualAddress* transposedVector;
    virtualAddress** result;
    size_t vectorSize;
    size_t start;
    size_t end;
};

struct ThreadSummationMatrixVectorArgs
{
    size_t start;
    size_t end;
    virtualAddress* result1;
    virtualAddress** result2;
    virtualAddress* summation;
    size_t vectorSize;
    size_t matrixRow;
};

struct ThreadBinarySearchArgs
{
    virtualAddress* vector;
    size_t vectorSize;
    int searchVal;
    int* index;
};

struct ThreadLinearSearchArgs
{
    virtualAddress* vector;
    size_t vectorSize;
    int searchVal;
    int* index;
};

// define mutex
pthread_mutex_t mutex;

void* threadMatrixVectorMultiply (void* args);
void* threadTransposeVectorMultiply (void* args);
void* threadSummationMatrixVector (void* args);
void* threadBinarySearch (void* args);
void* threadLinearSearch (void* args);

void test1 ();
bool matrixVectorMultiply (virtualAddress** matrix, virtualAddress* vector, virtualAddress* result, size_t matrixRow, size_t matrixCol, size_t vectorSize);
bool transposeVectorMultiply (virtualAddress* vector, virtualAddress* transposedVector, virtualAddress** result, size_t vectorSize);
bool summationMatrixVector (size_t matrixRow, virtualAddress* result1, virtualAddress** result2, size_t vectorSize, virtualAddress* summation);
bool binaryAndLinearSearch (virtualAddress* vector, size_t vectorSize, int searchVal, int *index1, int *index2);

MemoryManager *memoryManager = nullptr;

int main (int argc, char** argv) 
{   
    // get the arguments

    // if inverted there are 8 args
    // if not inverted there are 7 args

    size_t frameSize = 0;
    size_t physicalMemorySize = 0;
    size_t virtualMemorySize = 0;
    PageAlgorithm pageAlgorithm = PageAlgorithm::LRU;
    bool inverted = false;
    size_t numPrints = 0;
    std::string diskFileName = "";

    // if inverted
    if (argc < 7 || argc > 8)
    {
        std::cerr << "Error: invalid number of arguments" << std::endl;
        std::cerr << "for inverted: <frame size> <# physical frames> <# virtual frames> <page algorithm> \"inverted\" <# print> <disk file name>" << std::endl;
        std::cerr << "for not inverted: <frame size> <# physical frames> <# virtual frames> <page algorithm> <# print> <disk file name>" << std::endl;
        return 0;
    }
    else if (argc == 8)
    {
        if (strcmp(argv[5], "inverted") == 0)
        {
            inverted = true;
            // inverted = false;
        }
        else
        {
            std::cerr << "Error: inverted must be set to \"inverted\"" << std::endl;
            return 0;
        }

        // get disk file name
        diskFileName = argv[7];

        // convert the arguments to size_t
        if (!stringToSizeT(argv[1], &frameSize))
        {
            std::cerr << "Error: frame size must be a number" << std::endl;
            return 0;
        }

        if (!stringToSizeT(argv[2], &physicalMemorySize))
        {
            std::cerr << "Error: # of physical frame must be a number" << std::endl;
            return 0;
        }

        if (!stringToSizeT(argv[3], &virtualMemorySize))
        {
            std::cerr << "Error: # of virtual frame must be a number" << std::endl;
            return 0;
        }

        if (!stringToSizeT(argv[6], &numPrints))
        {
            std::cerr << "Error: # of prints must be a number" << std::endl;
            return 0;
        }

        // get the page algorithm
        if (strcmp(argv[4], "SC") == 0)
        {
            pageAlgorithm = PageAlgorithm::SC;
        }
        else if (strcmp(argv[4], "LRU") == 0)
        {
            pageAlgorithm = PageAlgorithm::LRU;
        }
        else if (strcmp(argv[4], "WSCLOCK") == 0)
        {
            pageAlgorithm = PageAlgorithm::WSCLOCK;
        }
        else
        {
            std::cerr << "Error: page algorithm must be \"SC\", \"LRU\", or \"WSCLOCK\"" << std::endl;
            return 0;
        }
    }
    else if (argc == 7)
    {
        // get disk file name
        diskFileName = argv[6];

        // convert the arguments to size_t
        if (!stringToSizeT(argv[1], &frameSize))
        {
            std::cerr << "Error: frame size must be a number" << std::endl;
            return 0;
        }

        if (!stringToSizeT(argv[2], &physicalMemorySize))
        {
            std::cerr << "Error: # of physical frame must be a number" << std::endl;
            return 0;
        }

        if (!stringToSizeT(argv[3], &virtualMemorySize))
        {
            std::cerr << "Error: # of virtual frame must be a number" << std::endl;
            return 0;
        }

        if (!stringToSizeT(argv[5], &numPrints))
        {
            std::cerr << "Error: # of prints must be a number" << std::endl;
            return 0;
        }

        // get the page algorithm
        if (strcmp(argv[4], "SC") == 0)
        {
            pageAlgorithm = PageAlgorithm::SC;
        }
        else if (strcmp(argv[4], "LRU") == 0)
        {
            pageAlgorithm = PageAlgorithm::LRU;
        }
        else if (strcmp(argv[4], "WSCLOCK") == 0)
        {
            pageAlgorithm = PageAlgorithm::WSCLOCK;
        }
        else
        {
            std::cerr << "Error: page algorithm must be \"SC\", \"LRU\", or \"WSCLOCK\"" << std::endl;
            return 0;
        }
    }

    // std::cout << "frame size: " << frameSize << std::endl;
    // std::cout << "# physical frames: " << physicalMemorySize << std::endl;
    // std::cout << "# virtual frames: " << virtualMemorySize << std::endl;
    // std::cout << "page algorithm: " << (pageAlgorithm == PageAlgorithm::SC ? "SC" : pageAlgorithm == PageAlgorithm::LRU ? "LRU" : "WSCLOCK") << std::endl;
    // std::cout << "# prints: " << numPrints << std::endl;
    // std::cout << "disk file name: " << diskFileName << std::endl;
    // std::cout << "inverted: " << (inverted ? "true" : "false") << std::endl;

    memoryManager = new MemoryManager(1, frameSize, physicalMemorySize, virtualMemorySize, pageAlgorithm, inverted, numPrints, diskFileName);


    test1 ();

    // virtualAddress* vAddress = new virtualAddress();
    // int val = -1;
    // virtualAddress *address2 = new virtualAddress();
    // for (int i = 0; i < 64*128; i++)
    // {
    //     if (memoryManager->allocateMemory(0, vAddress))
    //     {
    //         if (i == 2)
    //         {
    //             address2->copy(vAddress);
    //         }
    //         if (memoryManager->writeMemory(0, vAddress, i))
    //         {
    //             if (memoryManager->readMemory(0, vAddress, &val))
    //             {
    //                 std::cout << "Value read: " << val << std::endl;
    //             }
    //             else
    //             {
    //                 std::cout << "Error reading memory" << std::endl;
    //             }
    //         }
    //         else
    //         {
    //             std::cout << "Error writing memory" << std::endl;
    //         }
    //     }
    //     else
    //     {
    //         std::cout << "Error allocating memory" << std::endl;
    //     }
    // }

    // std::cout << "writing to first address again" << std::endl;
    // // memoryManager->writeMemory(0, address2, 100);
    // memoryManager->readMemory(0, address2, &val);
    // std::cout << "Value read: " << val << std::endl;

    memoryManager->printStatistics();
    return 0;
}

void test1 ()
{
    std::cout << "Test 1" << std::endl;
    std::cout << "frame size: " << memoryManager->getFrameSize() << std::endl;
    std::cout << "# physical frames: " << memoryManager->getPhysicalMemorySize() << std::endl;
    std::cout << "# virtual frames: " << memoryManager->getPageTableSize() << std::endl;
    std::cout << "page algorithm: " << (memoryManager->getPageAlgorithm() == PageAlgorithm::SC ? "SC" : memoryManager->getPageAlgorithm() == PageAlgorithm::LRU ? "LRU" : "WSCLOCK") << std::endl;
    std::cout << "disk file name: " << memoryManager->getDiskFileName() << std::endl << std::endl;

    // initialize mutex with default attributes
    pthread_mutex_init(&mutex, NULL);

    size_t matrixRow = 1000;
    size_t matrixCol = 3;

    size_t vectorSize = 3;

    std::random_device rd;
    std::mt19937 generator(rd());
    int min = 1;
    int max = 500;

    std::uniform_int_distribution<int> distribution(min, max);

    // virtual address pointers for matrix
    virtualAddress** matrix = new virtualAddress*[matrixRow];
    for (size_t i = 0; i < matrixRow; i++)
    {
        matrix[i] = new virtualAddress[matrixCol];
        if (!matrix[i])
        {
            std::cerr << "Error: could not allocate memory" << std::endl;
            return;
        }
    }

    // virtual address pointers for vector
    virtualAddress* vector = new virtualAddress[vectorSize];
    
    // virtual address pointers for result1(m*v)
    virtualAddress* result1 = new virtualAddress[matrixRow];

    // virtual address pointers for transposed vector
    virtualAddress* transposedVector = new virtualAddress[vectorSize];

    // virtual address pointers for result2(v*v^t)
    virtualAddress** result2 = new virtualAddress*[vectorSize];
    for (size_t i = 0; i < vectorSize; i++)
    {
        result2[i] = new virtualAddress[1];
        if (!result2[i])
        {
            std::cerr << "Error: could not allocate memory" << std::endl;
            return;
        }
    }

    // virtual address pointers for summation
    size_t summationSize = matrixRow > vectorSize ? matrixRow : vectorSize;
    virtualAddress* summation = new virtualAddress[summationSize];

    // allocate each virtual address in matrix
    for (size_t i = 0; i < matrixRow; i++)
    {
        for (size_t j = 0; j < matrixCol; j++)
        {
            if (!memoryManager->allocateMemory(0, &matrix[i][j]))
            {
                std::cerr << "Error: could not allocate memory" << std::endl;
                return;
            }
        }
    }

    // allocate each virtual address in vector
    for (size_t i = 0; i < vectorSize; i++)
    {
        if (!memoryManager->allocateMemory(0, &vector[i]))
        {
            std::cerr << "Error: could not allocate memory" << std::endl;
            return;
        }
    }

    // allocate each virtual address in result1
    for (size_t i = 0; i < matrixRow; i++)
    {
        if (!memoryManager->allocateMemory(0, &result1[i]))
        {
            std::cerr << "Error: could not allocate memory" << std::endl;
            return;
        }
    }

    // allocate each virtual address in transposed vector
    for (size_t i = 0; i < vectorSize; i++)
    {
        if (!memoryManager->allocateMemory(0, &transposedVector[i]))
        {
            std::cerr << "Error: could not allocate memory" << std::endl;
            return;
        }
    }

    // allocate each virtual address in result2
    for (size_t i = 0; i < vectorSize; i++)
    {
        if (!memoryManager->allocateMemory(0, &result2[i][0]))
        {
            std::cerr << "Error: could not allocate memory" << std::endl;
            return;
        }
    }

    // allocate each virtual address in summation
    for (size_t i = 0; i < summationSize; i++)
    {
        if (!memoryManager->allocateMemory(0, &summation[i]))
        {
            std::cerr << "Error: could not allocate memory" << std::endl;
            return;
        }
    }

    // fill rest of the virtual memory with random values
    // std::cout << "Filling virtual memory with random values" << std::endl;
    // while (true)
    // {
    //     virtualAddress* address = new virtualAddress();
    //     if (!memoryManager->allocateMemory(0, address))
    //     {
    //         break;
    //     }
    //     int val = distribution(generator);
    //     if (!memoryManager->writeMemory(0, address, val))
    //     {
    //         std::cerr << "Error: could not write to memory" << std::endl;
    //         return;
    //     }
    // }

    std::cout << "\nGenerating vector" << std::endl;
    // fill vector with random generated integers
    for (size_t i = 0; i < vectorSize; i++)
    {
        int val = distribution(generator);
        // if (!memoryManager->allocateMemory(0, &vector[i]))
        // {
        //     std::cerr << "Error: could not allocate memory" << std::endl;
        //     return;
        // }
        if (!memoryManager->writeMemory(0, &vector[i], val))
        {
            std::cerr << "Error: could not write to memory" << std::endl;
            return;
        }
    }

    // print vector
    // std::cout << "Vector:" << std::endl;
    // for (size_t i = 0; i < vectorSize; i++)
    // {
    //     int val = -1;
    //     if (!memoryManager->readMemory(0, &vector[i], &val))
    //     {
    //         std::cerr << "Error: could not read memory" << std::endl;
    //         return;
    //     }
    //     std::cout << val << std::endl;
    // }

    std::cout << "Vector size: " << vectorSize << " generated" << std::endl;

    // print matrix
    // std::cout << "\nMatrix:" << std::endl;
    // for (size_t i = 0; i < matrixRow; i++)
    // {
    //     for (size_t j = 0; j < matrixCol; j++)
    //     {
    //         int val = -1;
    //         if (!memoryManager->readMemory(0, &matrix[i][j], &val))
    //         {
    //             std::cerr << "Error: could not read memory" << std::endl;
    //             return;
    //         }
    //         std::cout << val << " ";
    //     }
    //     std::cout << std::endl;
    // }


    std::cout << "\nGenerating matrix" << std::endl;
    // fill matrix with random generated integers
    for (size_t i = 0; i < matrixRow; i++)
    {
        for (size_t j = 0; j < matrixCol; j++)
        {
            int val = distribution(generator);
            // if (!memoryManager->allocateMemory(0, &matrix[i][j]))
            // {
            //     std::cerr << "Error: could not allocate memory" << std::endl;
            //     return;
            // }
            if (!memoryManager->writeMemory(0, &matrix[i][j], val))
            {
                std::cerr << "Error: could not write to memory" << std::endl;
                return;
            }
        }
    }
    std::cout << "Matrix size: " << matrixRow << "x" << matrixCol << " generated" << std::endl;

    std::cout << "\nCalculating M * V" << std::endl;
    if (!matrixVectorMultiply(matrix, vector, result1, matrixRow, matrixCol, vectorSize))
    {
        std::cerr << "Error: could not multiply matrix and vector" << std::endl;
        return;
    }

    // print result1 vector
    // std::cout << "Result vector(m * v):" << std::endl;
    // for (size_t i = 0; i < matrixRow; i++)
    // {
    //     int val = -1;
    //     if (!memoryManager->readMemory(0, &result1[i], &val))
    //     {
    //         std::cerr << "Error: could not read memory" << std::endl;
    //         return;
    //     }
    //     std::cout << val << std::endl;
    // }

    std::cout << "\nV * M is calculated" << std::endl;

    // multiply transpose with vector
    std::cout << "\nCalculating V * V^t" << std::endl;
    if (!transposeVectorMultiply(vector, transposedVector, result2, vectorSize))
    {
        std::cerr << "Error: could not multiply vector and its transpose" << std::endl;
        return;
    }

    // print result2 matrix
    // std::cout << "Result v*v^t:" << std::endl;
    // for (size_t i = 0; i < vectorSize; i++)
    // {
    //     int val = -1;
    //     if (!memoryManager->readMemory(0, &result2[i][0], &val))
    //     {
    //         std::cerr << "Error: could not read memory" << std::endl;
    //         return;
    //     }
    //     std::cout << val << std::endl;
    // }

    std::cout << "\nV * V^t is calculated" << std::endl;



    std::cout << "\nCalculating V * M + V * V^t" << std::endl;
    if (!summationMatrixVector(matrixRow, result1, result2, vectorSize, summation))
    {
        std::cerr << "Error: could not sum the two results" << std::endl;
        return;
    }
    // // print summation vector
    // std::cout << "Summation vector:" << std::endl;
    // for (size_t i = 0; i < summationSize; i++)
    // {
    //     int val = -1;
    //     if (!memoryManager->readMemory(0, &summation[i], &val))
    //     {
    //         std::cerr << "Error2: could not read memory" << std::endl;
    //         return;
    //     }
    //     std::cout << val << std::endl;
    // }

    std::cout << "\nV * M + V * V^t is calculated" << std::endl;

    int searchVal = 0;
    
    // get value from summation vector
    if (!memoryManager->readMemory(0, &summation[summationSize - 1], &searchVal))
    {
        std::cerr << "Error: could not read memory" << std::endl;
        return;
    }

    std::cout << "Search value: " << searchVal << std::endl;

    int index1 = -1;
    int index2 = -1;

    std::cout << "\nMaking binary and linear search in parallel" << std::endl;
    // make binary search and linear search in parallel
    if (!binaryAndLinearSearch(summation, summationSize, searchVal, &index1, &index2))  
    {
        std::cerr << "Error: could not make binary and linear search" << std::endl;
        return;
    }

    
    // binary search result
    if (index1 == -1)
    {
        std::cout << "Binary search result: not found" << std::endl;
    }
    else
    {
        std::cout << "Binary search result: " << index1 << std::endl;
    }

    // linear search result
    if (index2 == -1)
    {
        std::cout << "Linear search result: not found" << std::endl;
    }
    else
    {
        std::cout << "Linear search result: " << index2 << std::endl;
    }

    // // fill all virtual memory with random values
    // std::cout << "\nFilling all virtual memory with random values" << std::endl;
    // while (true)
    // {
    //     virtualAddress* address = new virtualAddress();
    //     if (!memoryManager->allocateMemory(0, address))
    //     {
    //         break;
    //         return;
    //     }
    //     int val = distribution(generator);
    //     if (!memoryManager->writeMemory(0, address, val))
    //     {
    //         std::cerr << "Error: could not write to memory" << std::endl;
    //         return;
    //     }
    // }
    
    // std::cout << "\nRest of the virtual memory is filled with random values" << std::endl;

    // destroy mutex
    pthread_mutex_destroy(&mutex);
}

bool matrixVectorMultiply (virtualAddress** matrix, virtualAddress* vector, virtualAddress* result, size_t matrixRow, size_t matrixCol, size_t vectorSize)
{
    // check if the matrix and vector are compatible
    if (matrixCol != vectorSize)
    {
        std::cerr << "Error: matrix column and vector size are not compatible" << std::endl;
        return false;
    }
    // make multiplication in parallel

    // determine the number of threads according to the matrix row sizeWW

    size_t numThreads = 20;

    if (numThreads >= matrixRow)
    {
        numThreads = 1;
    }

    pthread_t threads[numThreads];

    ThreadMatrixVectorMultiplyArgs args[numThreads];

    size_t start = 0;
    size_t end = 0;

    size_t step = matrixRow / numThreads;

    for (size_t i = 0; i < numThreads; i++)
    {
        start = i * step;
        end = (i + 1) * step;

        if (i == numThreads - 1)
        {
            // if it's the last thread, assign the remaining work
            end = matrixRow;
        }

        // std::cout << "start: " << start << " end: " << end << std::endl;
        args[i].matrix = matrix;
        args[i].vector = vector;
        args[i].result = result;
        args[i].matrixRow = matrixRow;
        args[i].matrixCol = matrixCol;
        args[i].vectorSize = vectorSize;
        args[i].start = start;
        args[i].end = end;

        pthread_create(&threads[i], NULL, threadMatrixVectorMultiply, (void*)&args[i]);
    }


    // wait for threads to finish
    for (size_t i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return true;
}

void* threadMatrixVectorMultiply (void* args)
{
    // std::cerr << "Thread started: " << pthread_self() << std::endl;
    ThreadMatrixVectorMultiplyArgs* arguments = (ThreadMatrixVectorMultiplyArgs*)args;

    // multiply the matrix with the vector
    for (size_t i = arguments->start; i < arguments->end; i++)
    {
        int sum = 0;
        for (size_t j = 0; j < arguments->matrixCol; j++)
        {
            int val1 = -1;
            int val2 = -1;

            // mutex lock
            pthread_mutex_lock(&mutex);
            if (!memoryManager->readMemory(0, &arguments->matrix[i][j], &val1))
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            if (!memoryManager->readMemory(0, &arguments->vector[j], &val2))
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            pthread_mutex_unlock(&mutex);

            sum += val1 * val2;
        }

        // mutex lock
        pthread_mutex_lock(&mutex);
        if (!memoryManager->writeMemory(0, &arguments->result[i], sum))
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

bool transposeVectorMultiply(virtualAddress* vector, virtualAddress* transposedVector, virtualAddress** result, size_t vectorSize)
{

    // copy vector to transposed vector
    for (size_t i = 0; i < vectorSize; i++)
    {
        int val = -1;
        if (!memoryManager->readMemory(0, &vector[i], &val))
        {
            std::cerr << "Error: could not read memory" << std::endl;
            return false;
        }
        if (!memoryManager->writeMemory(0, &transposedVector[i], val))
        {
            std::cerr << "Error: could not write to memory" << std::endl;
            return false;
        }
    }

    // make multiplication in parallel

    // create threads

    size_t numThreads = 4;
    if (vectorSize <= numThreads)
    {
        numThreads = 1;
    }
    size_t start = 0;
    size_t end = 0;
    size_t step = vectorSize / numThreads;

    pthread_t threads[numThreads];

    ThreadTransposeVectorMultiplyArgs args[numThreads];

    for (size_t i = 0; i < numThreads; i++)
    {
        start = i * step;
        end = (i + 1) * step;

        if (i == numThreads - 1)
        {
            // if it's the last thread, assign the remaining work
            end = vectorSize;
        }

        args[i].vector = vector;
        args[i].transposedVector = transposedVector;
        args[i].result = result;
        args[i].vectorSize = vectorSize;
        args[i].start = start;
        args[i].end = end;

        pthread_create(&threads[i], NULL, threadTransposeVectorMultiply, (void*)&args[i]);
    }
    // wait for threads to finish
    for (size_t i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return true;
}

void* threadTransposeVectorMultiply (void* args)
{
    // std::cerr << "Thread started: " << pthread_self() << std::endl;
    ThreadTransposeVectorMultiplyArgs* arguments = (ThreadTransposeVectorMultiplyArgs*)args;

    // std::cerr << "Thread: " << pthread_self() << " start: " << arguments->start << " end: " << arguments->end << std::endl;
    // multiply the matrix with the vector
    for (size_t i = arguments->start; i < arguments->end; i++)
    {
        int sum = 0;
        for (size_t j = 0; j < arguments->vectorSize; j++)
        {
            int val1 = -1;
            int val2 = -1;

            // mutex lock
            pthread_mutex_lock(&mutex);
            if (!memoryManager->readMemory(0, &arguments->vector[j], &val1))
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            if (!memoryManager->readMemory(0, &arguments->transposedVector[j], &val2))
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            pthread_mutex_unlock(&mutex);

            sum += val1 * val2;
            // std::cerr << "Thread: " << pthread_self() << " sum: " << sum << std::endl;
        }

        // mutex lock
        pthread_mutex_lock(&mutex);
        if (!memoryManager->writeMemory(0, &arguments->result[i][0], sum))
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

bool summationMatrixVector (size_t matrixRow, virtualAddress* result1, virtualAddress** result2, size_t vectorSize, virtualAddress* summation)
{

    size_t summationSize = matrixRow > vectorSize ? matrixRow : vectorSize;

    // make summation in parallel

    // create threads
    size_t numThreads = 4;

    pthread_t threads[numThreads];

    ThreadSummationMatrixVectorArgs args[numThreads];

    size_t start = 0;
    size_t end = 0;

    size_t step = summationSize / numThreads;

    for (size_t i = 0; i < numThreads; i++)
    {
        start = i * step;
        end = (i + 1) * step;

        if (i == numThreads - 1)
        {
            // if it's the last thread, assign the remaining work
            end = summationSize;
        }

        args[i].start = start;
        args[i].end = end;
        args[i].result1 = result1;
        args[i].result2 = result2;
        args[i].summation = summation;
        args[i].vectorSize = vectorSize;
        args[i].matrixRow = matrixRow;

        pthread_create(&threads[i], NULL, threadSummationMatrixVector, (void*)&args[i]);
    }

    // wait for threads to finish
    for (size_t i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return true;
}

void* threadSummationMatrixVector (void* args)
{
    ThreadSummationMatrixVectorArgs* arguments = (ThreadSummationMatrixVectorArgs*)args;

    // sum the two results
    for (size_t i = arguments->start; i < arguments->end; i++)
    {
        int val1 = -1;
        int val2 = -1;

        // mutex lock
        pthread_mutex_lock(&mutex);
        if (i >= arguments->matrixRow)
        {
            val1 = 0;
        }
        else
        {
            if (!memoryManager->readMemory(0, &arguments->result1[i], &val1))
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }
        if (i >= arguments->vectorSize)
        {
            val2 = 0;
        }
        else
        {
            if (!memoryManager->readMemory(0, &arguments->result2[i][0], &val2))
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }
        pthread_mutex_unlock(&mutex);

        int sum = val1 + val2;

        // mutex lock
        pthread_mutex_lock(&mutex);
        if (!memoryManager->writeMemory(0, &arguments->summation[i], sum))
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
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

        // mutex lock
        pthread_mutex_lock(&mutex);
        if (!memoryManager->readMemory(0, &left[i], &val1))
        {
            pthread_mutex_unlock(&mutex);
            return;
        }
        if (!memoryManager->readMemory(0, &right[j], &val2))
        {
            pthread_mutex_unlock(&mutex);
            return;
        }
        pthread_mutex_unlock(&mutex);

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

void merge_sort (virtualAddress* vector, int start, int end)
{
    if (start < end)
    {
        int mid = (start + end) / 2;

        merge_sort(vector, start, mid);
        merge_sort(vector, mid + 1, end);

        merge(vector, start, mid, end);
    }
}



bool binaryAndLinearSearch (virtualAddress* vector, size_t vectorSize, int searchVal, int *index1, int *index2)
{

    // make binary search and linear search in parallel

    // sort the vector

    merge_sort(vector, 0, vectorSize - 1);

    // create threads
    
    pthread_t threads[2];

    // binary search
    ThreadBinarySearchArgs args1;
    args1.vector = vector;
    args1.vectorSize = vectorSize;
    args1.searchVal = searchVal;
    args1.index = index1;

    // liner search
    ThreadLinearSearchArgs args2;
    args2.vector = vector;
    args2.vectorSize = vectorSize;
    args2.searchVal = searchVal;
    args2.index = index2;

    pthread_create(&threads[0], NULL, threadBinarySearch, (void*)&args1);
    pthread_create(&threads[1], NULL, threadLinearSearch, (void*)&args2);

    // wait for threads to finish
    for (size_t i = 0; i < 2; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return true;
}

void* threadBinarySearch (void* args)
{
    ThreadBinarySearchArgs* arguments = (ThreadBinarySearchArgs*)args;

    // binary search
    int start = 0;
    int end = arguments->vectorSize - 1;
    int mid = 0;

    while (start <= end)
    {
        mid = (start + end) / 2;

        int val = -1;

        // mutex lock
        pthread_mutex_lock(&mutex);
        if (!memoryManager->readMemory(0, &arguments->vector[mid], &val))
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        pthread_mutex_unlock(&mutex);

        if (val == arguments->searchVal)
        {
            arguments->index[0] = mid;
            return NULL;
        }
        else if (val < arguments->searchVal)
        {
            start = mid + 1;
        }
        else
        {
            end = mid - 1;
        }
    }

    arguments->index[0] = -1;
    return NULL;
}

void* threadLinearSearch (void* args)
{
    ThreadLinearSearchArgs* arguments = (ThreadLinearSearchArgs*)args;

    // linear search
    for (size_t i = 0; i < arguments->vectorSize; i++)
    {
        int val = -1;

        // mutex lock
        pthread_mutex_lock(&mutex);
        if (!memoryManager->readMemory(0, &arguments->vector[i], &val))
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        pthread_mutex_unlock(&mutex);

        if (val == arguments->searchVal)
        {
            arguments->index[0] = i;
            return NULL;
        }
    }

    arguments->index[0] = -1;
    return NULL;
}