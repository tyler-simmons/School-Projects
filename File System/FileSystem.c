#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define Drive2MB 2
#define Drive3MB 3
#define Drive5MB 5
#define Drive10MB 10
#define BLOCK_SIZE 512 //1 "block" - 512 byte
#define PAGE 4096 //page size in byte - 1 page = 8 blocks
#define FAT_PAGES 32 //Number of system pages needed for FAT
#define FAT_LIMIT 65535 //maximum number of entries in FAT
#define MAX_DIRECTORY_ENTRIES 1024 //max number of entries in root
#define ENTRY_SIZE 32
#define PAGE_2MB 488
#define PAGE_3MB 732 
#define PAGE_5MB 1220
#define PAGE_10MB 2441
#define DATA_START 172032


//index in fat array corresponds to block
//careful with index REMEMBER TO DIVIDE BY 2 TO GET THE BLOCK
//if next = EOF thats end of data (youre done getting data now return it)
/*16 bits (2 bytes) aka ranges to 65,535 -> this means it can keep track of 
up to 65.535 blocks of data -> so the FAT must be 131,070 bytes large
-> this means that 32 pages of 4096 bytes are needed to map it*/
struct FAT{
    uint16_t next; //2 bytes - this is the target block for next
};
//16 PAGES are needed for its map


//Directory entry
struct Entry{
    char *name; //8 bytes - file name
    char *extension; //3 bytes - extension
    uint8_t attr; //1 byte *** 0 = file, 1 = directory, 2 unused, 3 root ***
    uint16_t creation_time; //2 bytes - hours and minutes - when was it created
    uint16_t modify_time; //2 bytes - last time chagned
    uint16_t FATindexStart; //2 bytes - which block in FAT table it starts at
    uint32_t size; //number of bytes
};

void mapToMem();
void initializeRoot();
void newEmptyEntry(int indexInENTRY_TABLE);
int getEmptyBlockIndex();
int getEmptyEntry();
int search(char *fileName);
void openFile(char *directory);
void createFile(char *fileName, char *extension);
void writeFile(char *data);





int fd; //descriptor to open drive
struct FAT *FAT_TABLE; //FAT virtual representation
char *driveSelection; //file name of drive
int drive; //drive capacaity(MB)
int drivePage;
struct Entry *ENTRY_TABLE; // root directory
char *DATA; //simplest array - just bytes to store the data in
int CWD; //Current working dir

int main(int argc, char **argv){

    
    //default to 2MB drive
    if(argc == 1){
        driveSelection = "Drive2MB";
        drive = Drive2MB;
        drivePage = PAGE_2MB;
    } else if (argc == 2){
        //drive selections
        if (strcmp(*(argv + 1), "Drive2MB") == 0){
            drive = Drive2MB;
            drivePage = PAGE_2MB;
            driveSelection = "Drive2MB";
        } else if (strcmp(*(argv + 1), "Drive3MB") == 0){
            drive = Drive3MB;
            drivePage = PAGE_3MB;
            driveSelection = "Drive3MB";
        } else if (strcmp(*(argv + 1), "Drive5MB") == 0){
            drive = Drive5MB;
            drivePage = PAGE_5MB;
            driveSelection = "Drive5MB";
        } else if (strcmp(*(argv + 1), "Drive10MB") == 0){
            drive = Drive10MB;
            drivePage = PAGE_10MB;
            driveSelection = "Drive10MB";
        //invalid drive
        } else {
            printf("Error: invalid drive selection\n");
            exit(-1);
        }
    }

    printf("%zu\n", sizeof(struct Entry));
    
    
    printf("Final Project FileSystem\n\n\n");
    //printf("Input: Drive%dMB. 0 to mount, 1 to open");

    mapToMem();
    printf("init root\n");
    initializeRoot();
    //printf("index of next free: %d\n", getEmptyBlockIndex());
    
    return 0;

}

//Map the parts of the drive into process memory 
void mapToMem(){

    int i; //index tracker for all internal loops
    
    //file descriptor for mapping (filename should already be checked)
    fd = open(driveSelection, O_RDWR); 
    if (fd < 0){
        printf("Error: can't open descriptor\n");
    }
    
    //start with FAT mapped to the very beginning of drive all the way to 0x20000 on drive (pages 0-31)
    FAT_TABLE = mmap(0, FAT_PAGES * PAGE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (FAT_TABLE == MAP_FAILED){
        printf("Error: FAT can't be mapped\n");
        exit(-1);
    }
    //Now initialize all FAT entries to 0 (for now as drive is not ready yet)
    //0 will be the equivalent of null for our purpose
    for (i = 0; i < FAT_LIMIT; i++){
        (*(FAT_TABLE + i)).next = 0x0000;
    }

    /*Now map the entries directory - give it 8 pages to contain 1024 entries @ 32 bytes each
      with a buffer page between so pages 33 - 40 (0x22000-0x28000)*/
    ENTRY_TABLE = mmap(0, PAGE * 8, PROT_READ|PROT_WRITE, MAP_SHARED, fd, PAGE * 33);
    if (ENTRY_TABLE == MAP_FAILED){
        printf("Error: ENTRY_TABLE can't be mapped\n");
        exit(-1);
    } 
    //This only creates the memory mapping - it does not do anything with it
    for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++){
        //fill all parts of directory table with unused directories
        newEmptyEntry(i);
    }

    //Now map the data block array
    //before map we need to know limit of what we can map to (mapping from 42 - n)
    DATA = mmap(0, (drivePage - 42) * PAGE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, PAGE * 42);

    /*
    for (i = 0; i < (drivePage - 43) * PAGE; i++){
        *(DATA + i) = 'a';
    }
    */
    
}

//Precondition: entry is initialized
void initializeRoot(){
    
    
    ENTRY_TABLE[0].name = "/";
    ENTRY_TABLE[0].extension = "";
    ENTRY_TABLE[0].attr = 3;
    ENTRY_TABLE[0].creation_time = 0;
    ENTRY_TABLE[0].modify_time = 0;
    ENTRY_TABLE[0].FATindexStart = 1;
    ENTRY_TABLE[0].size = 512;

}


//utility for initialization
void newEmptyEntry(int indexInENTRY_TABLE){

    //new entry that exists in stack frame
    struct Entry temp;

    temp.name = "unused";
    temp.extension = "";
    temp.attr = 2;
    temp.creation_time = 0;
    temp.modify_time = 0;
    temp.FATindexStart = 0;
    temp.size = 0;

    FAT_TABLE[1].next = 0xFFFF;

    memcpy(&ENTRY_TABLE[indexInENTRY_TABLE], &temp, ENTRY_SIZE);

}


//returns index in FAT where the next empty block is
int getEmptyBlockIndex(){
    int i;
    for(i = 2; i < FAT_LIMIT; i++){
        if(FAT_TABLE[i].next == 0x0000){
            return i;
        }
    }
    return -1;
}

//returns index in ENTRY_TABLE of next empty entry
int getEmptyEntry(){
    int i;
    for (i = 1; i < MAX_DIRECTORY_ENTRIES; i++){
        if(ENTRY_TABLE[i].attr == 2){   //unused entry
            return i;    
        }
    }
    return -1;
}

//return index in entry table of the file if exists (ignores extension)
int search(char *fileName){
    int i;
    
    for(i = 0; i < MAX_DIRECTORY_ENTRIES; i++){
        if (strcmp(ENTRY_TABLE[i].name, fileName) == 0){
            return i;
        }
    }
    return -1;
}

void openFile(char *directory){
    if(search(directory) == -1){
        printf("Error opening directory: does not exist\n");
    } else {
        CWD = search(directory);
    }
}

void createFile(char *fileName, char *extension){
    
    //does file exist
    if (search(fileName) >= 0){
        printf("Error: file already exists\n");
        return;
    }

    int FAT_index, ENTRY_index;
    
    //is there space
    if((FAT_index = (getEmptyBlockIndex())) == -1 ){
        printf("Error: no space remaining\n");
        return;
    }
    //is there another dir entry
    if((ENTRY_index = getEmptyEntry()) == -1){
        printf("Error: No directory remaining\n");
        return;
    }

    //is filname valid
    if (strlen(fileName) > 8 || strlen(extension) > 3){
        printf("Error: invalid filename\n");
        return;
    }
    
    ENTRY_TABLE[ENTRY_index].name = fileName;
    ENTRY_TABLE[ENTRY_index].extension = extension;
    ENTRY_TABLE[ENTRY_index].attr = 0;
    ENTRY_TABLE[ENTRY_index].creation_time = time(0);
    ENTRY_TABLE[ENTRY_index].modify_time = time(0);
    ENTRY_TABLE[ENTRY_index].FATindexStart = FAT_index;
    ENTRY_TABLE[ENTRY_index].size = 0;

    FAT_TABLE[FAT_index].next = 0xFFFF;

}


void writeFile(char *data){
    int size;
    int blocksNeeded = 1;
    size = strlen(data);
    
    int i;
    i = size;

    while (i > 0){
        blocksNeeded++;
        i = i - 512;
    }
    
    ENTRY_TABLE[CWD].size = size;
    ENTRY_TABLE[CWD].modify_time = time(0);

    int j,k;
    int blocks[blocksNeeded];
    blocks[0] = ENTRY_TABLE[CWD].FATindexStart;
    for (j = 1; j < blocksNeeded; j++){
        blocks[j] = getEmptyBlockIndex();
    }
    
    for (k = 0; k<(blocksNeeded-1); k++){
        FAT_TABLE[k].next = blocks[k+1];    
    }

    if(size <+ 512){
        int track;
        for (track = 0; track < size; track++){
            DATA[(DATA_START + (ENTRY_TABLE[CWD].FATindexStart * 512)) + track] = *(data + i);
        }
    }
        
    




}









