#include "easeq.h"
#include "functions.cpp"
#include "profiler.cpp"

DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    int CoreNumber = (int)(size_t)lpParam;

    DWORD_PTR AffinityMask = 1ULL << CoreNumber;
    HANDLE ThreadHandle = GetCurrentThread();

    if (SetThreadAffinityMask(ThreadHandle, AffinityMask) == 0) {
        fprintf(stderr, "Failed to set thread affinity for core %d\n", CoreNumber);
        return 1;
    }

    while (true) 
    { // Keep checking for files to process
        char FilePath[MAX_PATH_LENGTH] = { 0 };
        int FileFound = 0;

        // Lock the mutex to check for files
        WaitForSingleObject(global_handle_Mutex, INFINITE);

        for (int i = 0; i < global_FileCount; ++i) {
            if (!global_FileAssigned[i]) { // Find the first unassigned file
                global_FileAssigned[i] = 1;
                strcpy(FilePath, global_SequencedFiles[i]);
                FileFound = 1;
                break;
            }
        }

        ReleaseMutex(global_handle_Mutex); // Unlock the mutex

        if (!FileFound) {
            // Exit if no files are left
            printf("Core #%d has no more files to process.\n", CoreNumber);
            return 0;
        }

        // Process the file outside the critical section
        AlignSequence(FilePath);
    }
}

int main()
{
    //Initialize the profiler
    LARGE_INTEGER Frequency;
    LARGE_INTEGER StartQPC;
    LARGE_INTEGER EndQPC;
    
    QueryPerformanceFrequency(&Frequency);
    i64 PerfCountFrequency = Frequency.QuadPart;
    QueryPerformanceCounter(&StartQPC);
    i64 StartingCycleCount = __rdtsc();
    //____________________

    BeginProfile();
    EndAndPrintProfile();


    HANDLE Threads[NumberOfCPUCores];
    const char* directory = "..\\sequenced\\";
    global_handle_Mutex = CreateMutex(NULL, FALSE, NULL); // Create mutex for synchronization
    if (global_handle_Mutex == NULL) 
    {
        fprintf(stderr, "Failed to create mutex.\n");
        return 1;
    }

    // Populate the list of files
    PopulateFileList(directory);

    // Create Threads
    for (int i = 0; i < NumberOfCPUCores; i++) {
        Threads[i] = CreateThread(
            NULL,                 // Default security attributes
            0,                    // Default stack size
            ThreadFunction,       // Thread function
            (LPVOID)(size_t)i,    // Thread parameter: core number
            0,                    // Default creation flags
            NULL                  // Ignoring thread ID
        );

        if (Threads[i] == NULL) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }

    // Wait for all Threads to complete
    WaitForMultipleObjects(NumberOfCPUCores, Threads, TRUE, INFINITE);

    // Close thread handles
    for (int i = 0; i < NumberOfCPUCores; i++) {
        CloseHandle(Threads[i]);
    }

    // Close the mutex handle
    CloseHandle(global_handle_Mutex);

    //____________________




    QueryPerformanceCounter(&EndQPC);
    i64 EndingCycleCount = __rdtsc();
    i64 TimeElapsed = ((EndQPC.QuadPart - StartQPC.QuadPart)*1000'000) / Frequency.QuadPart;

    printf("Time elapsed (microsec): %lld\n", TimeElapsed);
    printf("Time elapsed (millisec): %lld\n", TimeElapsed/1000);
    printf("Time elapsed (seconds): %f\n", (f32)TimeElapsed/1000'000);


    return 0;
}   