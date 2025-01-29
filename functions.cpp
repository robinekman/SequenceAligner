#include "easeq.h"

void InitializeHashMap(HashMap *map)
{
    map -> entries[0].key = 'A';
    strcpy(map->entries[0].value, "00");

    map->entries[1].key = 'T';
    strcpy(map->entries[1].value, "01");
    
    map->entries[2].key = 'G';
    strcpy(map->entries[2].value, "10");
    
    map->entries[3].key = 'C';
    strcpy(map->entries[3].value, "11");

}

char *Encode(HashMap *map, char key, char *translation)
{
    for (int i = 0; i < 4; ++i)
    {
        if (map->entries[i].key == key)
        {
            translation[0] = map->entries[i].value[0];
            translation[1] = map->entries[i].value[1];
            return translation;
        }
    }
    return NULL;
}

void PrintBinary(u64 num) {
    for (int i = 63; i >= 0; i--) {
        printf("%llu", (num >> i) & 1);
    }
    printf("\n");
}

u64 FindTextFileLength(const char* filePath)
{
    FILE *file;
    u64 NumberOfNucleotides;
    u64 length = 0;
    char buffer[1024];

    // Open the file in binary mode
    if (filePath != NULL)
    {
        file = fopen(filePath, "rb");
    }
    else
    {
        printf("Error reading file");
        return 1;
    }

    u64 BytesRead = 0;
    while ((BytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        length += BytesRead;
    }

    NumberOfNucleotides = length;
    fclose(file);

    return NumberOfNucleotides;
}


char *EncodeChars(char *ReadChars, u64 NumTemplateNucleotides, HashMap &map, char *EncodeBuffer)
{
    if (ReadChars != NULL) 
    {

        int j = 0;
        for (u64 i = 0; i < NumTemplateNucleotides; ++i) 
        {
            char Translation[2]; 
            char *TranslateChars = Encode(&map, ReadChars[i], Translation);

            if ((Translation[0] != '\0' && Translation[1] != '\0'))
            {
                EncodeBuffer[j] = Translation[0];
                EncodeBuffer [j+1] = Translation[1];
                j += 2;
            }
            else
            {
                return NULL;
            }
        }
    }
    else
    {
        return NULL;
    }
        return EncodeBuffer;

}

u64 BinaryStringToU64(const char *BinaryString) {
    u64 Result = 0;
    for (int i = 0; i < 64; i++) {
        Result <<= 1; // Shift result left by 1 bit
        if (BinaryString[i] == '1') {
            Result |= 1; // Set the least significant bit to 1
        }
    }

    return Result;
}

void CharsToU64(u64 *U64Buffer, char *StringBuffer, u32 BatchSize)
{
    HashMap map;
    InitializeHashMap(&map);

    char buffer[32];
    char BinaryBuffer[64];
    u32 U64BufferIndex = 0;

    for(u32 i = 0; i < BatchSize; i+=32)
    {
        EncodeChars(&StringBuffer[i], sizeof(buffer), map, &BinaryBuffer[0]);
        U64Buffer[U64BufferIndex] = BinaryStringToU64(&BinaryBuffer[0]);
        U64BufferIndex++;
    }
}




void SIMDBitshiftWrapper(uint64_t TestNumber, AlignedArray *BitWrapper) {
    __m256i InputVector = _mm256_set1_epi64x(TestNumber);

    // Arrays for shift values
    u64 rightShiftValues[8][4] = 
    {
        {0, 2, 4, 6}, {8, 10, 12, 14}, {16, 18, 20, 22}, {24, 26, 28, 30},
        {32, 34, 36, 38}, {40, 42, 44, 46}, {48, 50, 52, 54}, {56, 58, 60, 62}
    };
    u64 leftShiftValues[8][4] = 
    {
        {0, 62, 60, 58}, {56, 54, 52, 50}, {48, 46, 44, 42}, {40, 38, 36, 34},
        {32, 30, 28, 26}, {24, 22, 20, 18}, {16, 14, 12, 10}, {8, 6, 4, 2}
    };

    // Temporary storage for results
    __m256i resultOR[8];

    for (int i = 0; i < 8; ++i) {
        // Create shift vectors
        __m256i rightShiftVector = _mm256_loadu_si256((__m256i*)rightShiftValues[i]);
        __m256i leftShiftVector = _mm256_loadu_si256((__m256i*)leftShiftValues[i]);

        // Perform shifts
        __m256i shiftedRight = _mm256_srlv_epi64(InputVector, rightShiftVector);
        __m256i shiftedLeft = _mm256_sllv_epi64(InputVector, leftShiftVector);

        // Perform bitwise OR
        resultOR[i] = _mm256_or_si256(shiftedLeft, shiftedRight);
    }

    // Store results in the output array
    for (int i = 0; i < 8; ++i) {
        _mm256_store_si256((__m256i*)&BitWrapper->data[i * 4], resultOR[i]);
    }
}

void ConvertToWideChar(const char* source, WCHAR* dest, int destSize) 
{
    MultiByteToWideChar(CP_ACP, 0, source, -1, dest, destSize);
}

void PopulateFileList(const char* directory) {
    // Create search pattern
    char SearchPattern[MAX_PATH_LENGTH];
    sprintf(SearchPattern, "%s*.txt", directory);

    // Convert to wide-character string
    WCHAR WideSearchPattern[MAX_PATH_LENGTH];
    ConvertToWideChar(SearchPattern, WideSearchPattern, MAX_PATH_LENGTH);

    WIN32_FIND_DATAW FindData;
    HANDLE handle_Find = FindFirstFileW(WideSearchPattern, &FindData);

    if (handle_Find == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "No files found in directory: %s\n", directory);
        return;
    }

    do {
        // Skip directories
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        // Generate full file path
        WCHAR WideFilePath[MAX_PATH_LENGTH];
        swprintf(WideFilePath, MAX_PATH_LENGTH, L"%s%s", L"..\\sequenced\\", FindData.cFileName);

        // Convert back to narrow string
        char FilePath[MAX_PATH_LENGTH];
        WideCharToMultiByte(CP_ACP, 0, WideFilePath, -1, FilePath, MAX_PATH_LENGTH, NULL, NULL);

        // Add file to the array
        if (global_FileCount < MAX_FILES) {
            strcpy(global_SequencedFiles[global_FileCount], FilePath);
            global_FileCount++;
        } else {
            fprintf(stderr, "File list is full. Some files will not be processed.\n");
            break;
        }

    } while (FindNextFileW(handle_Find, &FindData) != 0);

    FindClose(handle_Find);
}

u32 CountMatchingNucleotides(u64 Template, u64 Seqeuenced) {
    u64 XORResult = Template ^ Seqeuenced; // XOR the two sequences
    int matches = 0;

    // Iterate over all 32 nucleotides (64 bits total, 2 bits per nucleotide)
    for (int i = 0; i < 64; i += 2) {
        // Extract the two bits at the current position
        u64 pair = (XORResult >> i) & 0b11;
        if (pair == 0) {
            matches++; // If pair is 00, it is a match
        }
    }
    return matches;
}



#define BatchSize 2048
void AlignSequence(const char* FilePath) {
    //I currently assume the template index will only consist of nucleotides
    HANDLE handle_SequencedFile;
    HANDLE handle_TemplateFile;
    char TemplateStringBuffer[BatchSize];
    u32 TemplateInvalidStringBuffer[BatchSize];
    u64* TemplateU64Buffer = (u64*)malloc((BatchSize / 32) * sizeof(u64));
    char SequencedStringBuffer[BatchSize];
    u32 SequencedInvalidStringBuffer[BatchSize];
    u64* SequencedU64Buffer = (u64*)malloc((BatchSize / 32) * sizeof(u64));
    DWORD BatchCharsRead = 0;
    u32 TemplateU64BufferLength = 0;
    u32 SequencedU64BufferLength = 0;
    u32 TotalCharsRead = 0;
    u32 TemplateBestMatch;
    u32 SingleBestMatch;
    u32 MultipleBestMatch;

    handle_SequencedFile = CreateFileA(FilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle_SequencedFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open file %s. Error: %lu\n", FilePath, GetLastError());
        return;
    }
    printf("FilePath: %s\n", FilePath);
    handle_TemplateFile = CreateFileA("..\\template\\template.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle_TemplateFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open file %s. Error: %lu\n", "..\\template\\template.txt", GetLastError());
        return;
    }
    BOOL readSuccess = TRUE;
    while (readSuccess) 
    {
        memset(TemplateStringBuffer, 0, BatchSize);
        ReadFile(handle_TemplateFile, TemplateStringBuffer, BatchSize, &BatchCharsRead, NULL);
        if (!readSuccess || BatchCharsRead == 0) break;

        u32 OutputIndex = 0;
        u32 InvalidIndex = 0;
        for (u32 i = 0; i < BatchCharsRead; ++i) 
        {
            char current = toupper(TemplateStringBuffer[i]);
            if (current == 'A' || current == 'T' || current == 'G' || current == 'C') 
            {
                if (OutputIndex < BatchSize) 
                {
                    TemplateStringBuffer[OutputIndex++] = current;
                }
            } 
            else 
            {
                if (InvalidIndex < BatchSize) 
                {
                    TemplateInvalidStringBuffer[InvalidIndex++] = TotalCharsRead + i;
                }
            }
        }
        CharsToU64(&TemplateU64Buffer[0], &TemplateStringBuffer[0], BatchCharsRead);
        /*
        for (int i = 0; i < 32; i++)
        {
            printf("%llu\n", TemplateU64Buffer[i]);
        }
        */
        SetFilePointer(handle_SequencedFile, 0, NULL, FILE_BEGIN);
        BOOL readSuccess = TRUE;
        while (readSuccess) 
        {
            memset(SequencedStringBuffer, 0, BatchSize);
            readSuccess = ReadFile(handle_SequencedFile, SequencedStringBuffer, BatchSize, &BatchCharsRead, NULL);

            if (!readSuccess || BatchCharsRead == 0) break;

            //BatchCharsRead ranges from 0-BatchSize
            u32 OutputIndex = 0;
            u32 InvalidIndex = 0;
            for (u32 i = 0; i < BatchCharsRead; ++i) 
            {
                char current = toupper(SequencedStringBuffer[i]);
                if (current == 'A' || current == 'T' || current == 'G' || current == 'C') 
                {
                    if (OutputIndex < BatchSize) 
                    {
                        SequencedStringBuffer[OutputIndex++] = current;
                    }
                } 
                else 
                {
                    if (InvalidIndex < BatchSize) 
                    {
                        SequencedInvalidStringBuffer[InvalidIndex++] = TotalCharsRead + i;
                    }
                }
            }

            TotalCharsRead += BatchCharsRead;
            CharsToU64(&SequencedU64Buffer[0], &SequencedStringBuffer[0], BatchCharsRead);
            //This will get messed up if SequenceSize % 32 != 0 !!!
            SequencedU64BufferLength = BatchSize / 64;

            if (BatchCharsRead != BatchSize)
            {
                SequencedU64BufferLength = BatchCharsRead/32;
            }
            for (int i = 0; i < SequencedU64BufferLength; ++i)
            {
                //printf("%llu\n", SequencedU64Buffer[i]);
                //printf("%llu\n", TemplateU64Buffer[i]);
                //PrintBinary(SequencedU64Buffer[i]);
                //PrintBinary(TemplateU64Buffer[i]);
                
            }

            AlignedArray BitWrapper;
            for (int i = 0; i < BatchSize/64; ++i)
            {
                SIMDBitshiftWrapper(SequencedU64Buffer[i], &BitWrapper);
            }

            for (int i = 0; i < BatchSize/64; ++i)
            {
                //PrintBinary(BigResults);
                u32 NucleotideMatches = CountMatchingNucleotides(SequencedU64Buffer[i],TemplateU64Buffer[i]);
                //printf("%lu\n", NucleotideMatches);
            }
        }
    }
    
    
    CloseHandle(handle_SequencedFile);
    CloseHandle(handle_TemplateFile);
    free(TemplateU64Buffer);
    free(SequencedU64Buffer);
}

