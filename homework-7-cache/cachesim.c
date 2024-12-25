#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// memory size 
const int mem_size = 1<<24;

// cache fifo counter 
int fifoCounter = 0;
//struct for cache 

typedef struct blockData{
    bool dirty;
    bool valid;
    int tag;
    int fifoInd;
    unsigned char* data; // do not statically allocate !!!!
}blockData;

// C doesn't separate "characters" from "bytes." To clarify what's a string
// (i.e. ASCII bytes that can be printed) and what's just an array of bytes,
// we define a new "byte" type, which internally is just an (unsigned) char.
typedef unsigned char byte;

// Struct describing an access from the trace file. Returned by `traceNextAccess`.
typedef struct {
    bool isStore;
    int address;
    int accessSize;
    byte data[8];
} cacheAccess;

// The trace file being read. Set by `traceInit`.
FILE * traceFile = NULL;

// The last line read from the trace file. Shouldn't need to access directly,
// this is used internally by `traceFinished` and `traceNextAccess`.
char traceLineBuffer[64] = {0};

/**
 * Opens a trace file, given its name. Must be called before `traceNextAccess`,
 * which will begin reading from this file.
 * @param filename: the name of the trace file to open
 */
void traceInit(char * filename) {
    assert(traceFile == NULL); // Must not initialize more than once
    assert(filename != NULL); // Filename must be set.
    traceFile = fopen(filename, "r");
    if (!traceFile) {
        fprintf(stderr, "Failed to open trace file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

/**
 * Checks if we've already read all accesses from the trace file.
 * @return true if the trace file is complete, false if there's more to read.
 */
bool traceFinished() {
    assert(traceFile != NULL); // Must have initialized the file with `traceInit`
    if (traceLineBuffer[0]) {
        // If there's a line buffered, it can be read.
        return false;
    }
    // Otherwise, read a new line into the buffer, and return `true` if the read fails.
    return fgets(traceLineBuffer, 64, traceFile) == NULL;
}

/**
 * Read the next access in the trace. Errors if `traceFinished() == true`.
 * @return The access as a `cacheAccess` struct.
 */
cacheAccess traceNextAccess() {
    // Error if the trace is finished. This will also populate the line buffer,
    // if it's not already full.
    assert(!traceFinished());
    char accessType[16];
    cacheAccess result;
    int charsRead;
    sscanf(traceLineBuffer, "%15s 0x%x %d %n", accessType, &result.address, &result.accessSize, &charsRead);

    // Check access type
    if (strcmp(accessType, "store") == 0) {
        result.isStore = true;

        // Load data bytes
        for (int i = 0; i < result.accessSize; i++) {
            sscanf(traceLineBuffer + charsRead + 2 * i, "%02hhx", &result.data[i]);
        }
    } else if (strcmp(accessType, "load") == 0) {
        result.isStore = false;
    } else {
        fprintf(stderr, "Invalid trace file access type: %s\n", accessType);
        exit(1);
    }

    // Zero line buffer to stop future accesses
    memset(traceLineBuffer, 0, sizeof(traceLineBuffer));
    return result;
}

int log2(int n) {
    int r=0;
    while (n>>=1) r++;
        return r;
}

// print out operation data 
void printOpData(byte opData[8], int accessSize, int block_offset){
    // printf("accessSize: %x", accessSize );
    // printf("block_offset: %x", block_offset );
    for(int i = block_offset; i < block_offset + accessSize; i++){
        printf("%02x", opData[i]);
        // printf("%x", i);
    }

}

int smallestFifo(struct blockData** cache, int ways, int index){
    int victimWay = 0;
    for(int i = 0; i < ways; i++){
        if (cache[index][i].fifoInd < cache[index][victimWay].fifoInd) {
            victimWay = i;
        }
    }
    //printf("victimWay %x",victimWay);
    return victimWay;
}

void replacementStore(struct blockData** cache, unsigned char* memory, int index, int ways, int address, int indexOff, int blockOff, int block_offset, byte opData[8],int blockSize, int accessSize, int block_start_address, int tag){
    // miss handling for a dirty or clean filled block
    // if it is valid && dirty && queue is 0 then we need to do a dirty replacement
    // we need to run a fifo operation that gives us the block with the smallest fifo index
    // then we take that new way and set it as the victim
    // smallest fifo should take inputs cache, ways, index 
    int victimWay = smallestFifo(cache, ways, index);
    int victim_address = (cache[index][victimWay].tag << (indexOff+blockOff)) | (index << blockOff);
    //int block_start_address = address & ~(blockSize - 1);

    if(cache[index][victimWay].dirty){
        memcpy(&memory[victim_address], cache[index][victimWay].data, blockSize);
        
        memcpy(cache[index][victimWay].data, &memory[block_start_address], blockSize);
        memcpy(&cache[index][victimWay].data[block_offset], opData, accessSize); // this line makes cache [0][1].valid == false error at this line as it turns valid blocks to invalid

        cache[index][victimWay].fifoInd = fifoCounter; 
        cache[index][victimWay].tag = tag;
        cache[index][victimWay].valid = true;
        cache[index][victimWay].dirty = true; 
        //printf("what index am I living in %x\n", index);
        //printf("what way am I living in %x\n", victimWay);
        printf("replacement 0x%x dirty\n", victim_address);
        printf("store 0x%x miss\n", address);
        //printf("memory val: %x\n",memory[victim_address] );
        return;
    }
    memcpy(cache[index][victimWay].data, &memory[block_start_address], blockSize);
    memcpy(&cache[index][victimWay].data[block_offset], opData, accessSize); // ACCESSSIZE LIMITED 

    // for load --> everything is the same except you do not use  memcpy(&cache[index][victimWay].data[block_offset], opData, accessSize);
    cache[index][victimWay].fifoInd = fifoCounter;
    cache[index][victimWay].dirty = true;
    cache[index][victimWay].valid = true;
    cache[index][victimWay].tag = tag;
        
    printf("replacement 0x%x clean\n", victim_address);
    printf("store 0x%x miss\n", address);
    return;
}

void replacementLoad(struct blockData** cache, unsigned char* memory, int index, int ways, int address, int indexOff, int blockOff, int block_offset, int blockSize, int accessSize, int block_start_address, int tag){
     // If we miss in the cache, and our value has the same row and column (set and way) as another value 
    // write inital value to memory 
    // overwrite old value with new value -->set valid bit to 1, dirty bit to 0, and tag and cache to corresponding values 
    
    // miss handling for a dirty or clean filled block
    // when replacing then we need ot check if it is dirty and replace it with first values in queue
    int victimWay = smallestFifo(cache, ways, index);
    int victim_address = (cache[index][victimWay].tag << (indexOff+blockOff)) | (index << blockOff);
    //int block_start_address = (tag << (indexOff+blockOff)) | (index << blockOff);
    //address & ~(blockSize - 1);
 

    // neeed to save new vicitim to memory
    if(cache[index][victimWay].dirty){
        memcpy(&memory[victim_address],cache[index][victimWay].data, blockSize);
        memcpy(cache[index][victimWay].data, &memory[block_start_address], blockSize);

        cache[index][victimWay].fifoInd = fifoCounter;
        cache[index][victimWay].tag = tag;
        cache[index][victimWay].valid = true;
        cache[index][victimWay].dirty = false; 

        printf("replacement 0x%x dirty\n", victim_address);
        printf("load 0x%x miss ", address);
        printOpData(&memory[block_start_address], accessSize,block_offset); // ffix this 
        printf("\n");
        return;
    }
    memcpy(cache[index][victimWay].data, &memory[block_start_address], blockSize);

    cache[index][victimWay].fifoInd = fifoCounter;
    cache[index][victimWay].tag = tag;
    cache[index][victimWay].valid = true;
    cache[index][victimWay].dirty = false;

    printf("replacement 0x%x clean\n", victim_address);
    printf("load 0x%x miss ", address);
    printOpData(&memory[block_start_address], accessSize,block_offset); // ffix this 
    printf("\n");
    return;
}

//store function
void store(unsigned int address, int blockOff, int indexOff, int tagOff, int blockSize, int sets, int ways, struct blockData** cache, unsigned char *memory, byte opData[8], int accessSize){
    //get actual tag, and index values 
    //printf("address: %x", address);
    int index = (address >> (blockOff)) & ((1 << indexOff) - 1);
    //printf("index: %x", index);
    //tag calculation
    int tag = address >> (indexOff + blockOff);
    //printf("tag: %x", tag);
    // block offset 
    int block_offset = address & ((1<<blockOff) - 1);
    //printf("block_offset: %x", block_offset);
    int block_start_address = (tag << (indexOff+blockOff)) | (index << blockOff);
    //address & ~(blockSize - 1);


    // Check if tag is in cache - so if there is nothing in the map then you can writeBack, else do not write anything to the map 
    for (int i = 0; i < ways; i++) {
        if (cache[index][i].valid && cache[index][i].tag == tag) {
            // Cache hit
            printf("store 0x%x hit\n", address);
            memcpy(&cache[index][i].data[block_offset], opData, accessSize);
            cache[index][i].dirty = true; 
            return;
        }
    }
    // miss handeling
    for(int i = 0; i < ways; i++){
        if(!cache[index][i].valid){
            // if your block is invalid, then just store.
            memcpy(cache[index][i].data, &memory[block_start_address], blockSize);
            memcpy(&cache[index][i].data[block_offset], opData, accessSize);

            cache[index][i].fifoInd = fifoCounter;
            //printf("fifoIndex: %x", cache[index][i].fifoInd);
            cache[index][i].dirty = true;
            cache[index][i].valid = true;
            cache[index][i].tag = tag;

            //printf("what index am I living in %x\n", index);
            //printf("what way am I living in %x\n", i);
            printf("store 0x%x miss\n", address); 
            return;
        }
    }
    replacementStore(cache,memory,index,ways,address,indexOff,blockOff,block_offset,opData, blockSize, accessSize, block_start_address, tag);
}

// load function 
void load(unsigned int address, int blockOff, int indexOff, int tagOff, int blockSize, int sets, int ways, struct blockData** cache, unsigned char* memory, int accessSize){
    //printf("address: %x\n", address);
    int index = (address >> (blockOff)) & ((1 << indexOff) - 1);
    //printf("index: %x\n", index);
    //tag calculation
    int tag = address >> (indexOff + blockOff);
    //printf("tag: %x\n", tag);
    // block offset 
    int block_offset = address & ((1<<blockOff) - 1);
    //printf("block_offset: %x\n", block_offset);
    int block_start_address =  (tag << (indexOff+blockOff)) | (index << blockOff);
    //address & ~(blockSize - 1);



    // for hit requested block is already in cache so no replacement is needed 
    for(int i = 0; i < ways; i++){
       // if hit is value then print load, address, and miss - do not add anything to the cache 
        if(cache[index][i].valid && cache[index][i].tag == tag){
            printf("load 0x%x hit ", address); 
            printOpData(cache[index][i].data, accessSize, block_offset); 
            printf("\n");
            return;
        }
    }
    // for miss block is not in cache 
    // The requested block is not in the cache.
    // Load block from memory into the appropriate set.
    // Evict the least recently used block if the set is full.
    // If evicted block is dirty, write it back to memory.

    // If we are Reading a value, and we miss then we must load it into the cache
        // set valid bit to 1, store data and tag in cache, and set dirty bit to 0
    
    for(int i =0; i < ways; i++){
        if(!cache[index][i].valid){
            // if your block is invalid, then just store.
            memcpy(cache[index][i].data, &memory[block_start_address], blockSize); // removed address_block 

            cache[index][i].fifoInd = fifoCounter;
            //printf("fifoIndex: %x", cache[index][i].fifoInd);
            cache[index][i].dirty = false;
            cache[index][i].valid = true;
            cache[index][i].tag = tag;
            //printf("what index am I living in %x\n", index);
            //printf("what way am I living in %x\n", i);
            printf("load 0x%x miss ", address);
            printOpData(&memory[block_start_address], accessSize,block_offset); // ffix this 
            printf("\n");
            return;
        }
    }
    replacementLoad(cache, memory, index, ways, address, indexOff, blockOff, block_offset, blockSize, accessSize, block_start_address, tag);
}

// Your code goes here, including an `int main` method.
int main(int argc, char* argv[]){
    if (argc != 5) {
        fprintf(stderr, "Usage: ./cachesim <trace-file> <cache-size-kB> <associativity> <block-size>\n");
        return EXIT_FAILURE;
    }
    // read from trace file 
    // Initialize the trace file
    traceInit(argv[1]);

    long ways, blockSize, cacheSize;
    int sets;
    // get cache size from the trace file (input number 2)
    sscanf(argv[2], "%ld", &cacheSize);

    // get number of ways from the trace file (input number 3)
    sscanf(argv[3], "%ld", &ways);

    // get block size from the trace file (input number 4)
    sscanf(argv[4], "%ld", &blockSize);

    // intialize frames, blocks, index, and tag
    int frames, blockOff, indexOff, tagOff;

    //calculate sets blocks, index offset, and tag offset
    frames = cacheSize*1024/blockSize;
    sets = frames/ways;
    blockOff = log2(blockSize);
    indexOff = log2(sets);
    tagOff = 24 - indexOff - blockOff;

    // read the instructions (load and stores)

     // for loop through all cache elements and create memory 
        // Allocate cache memory correctly
        // DO NOT MALLOC INSIDE OF WHILE LOOP
        struct blockData** cache = (struct blockData**)malloc(sets * sizeof(struct blockData*));
        for (int i = 0; i < sets; i++) {
            cache[i] = (struct blockData*)malloc(ways * sizeof(struct blockData));
            for (int j = 0; j < ways; j++) {
                // copying to much data so we have to allocate data to the sie of the block
                cache[i][j].data = (unsigned char*)malloc(blockSize); // ie dynamically allocating !!!

                cache[i][j].valid = false;
                cache[i][j].dirty = false;
                cache[i][j].tag = -1;
                cache[i][j].fifoInd = 0;
            }
        }

        //initalize memory 
        // does memory also have to be of type blockData?
        unsigned char *memory = (unsigned char *) malloc(mem_size); // allocated 16M bytes --> two big for stack so just did it in memory --> this was also causing the segmentation fault in main
        for(int i = 0; i <mem_size; i++){
            memory[i] = 0;
        }
    while(!traceFinished()){

        // valules that we are reading from the json file
        bool opType;
        unsigned int address, accessSize;
        byte opData[8];        
        
        // gets our instructions 
        cacheAccess access = traceNextAccess();
        fifoCounter++;
        // tells if is a store or not 
        opType = access.isStore;

        // gives address --> relevant for block off and index off
        address = access.address;

        // access size 
        accessSize = access.accessSize;
        // data that is being stored
        //memset(opData, 0, 8); 
        //memcpy(opData,access.data,8);

        if(access.isStore == true){
            store(address, blockOff, indexOff, tagOff, blockSize, sets, ways, cache, memory, access.data, accessSize);
        }
        else{
            load(address, blockOff, indexOff, tagOff, blockSize, sets, ways, cache, memory, accessSize);
        }
    }
    free(memory);
    for(int i=0; i<sets; i++){
        free(cache[i]);
    }
    free(cache);
    //fclose(traceFile);
    return EXIT_SUCCESS;
}