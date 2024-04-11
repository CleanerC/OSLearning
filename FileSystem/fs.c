#include "fs.h"

/* stuctures */
/***********************************************/
typedef struct {
    int inode_metadata_offset;
    int dir_offset;
    int data_offset;
    int bitmap_offset;

    int bitmap_used;
} SuperBlock;

typedef struct {
//   char type;
  int Two_offset;

  int size;
} Inode;

typedef struct {
    bool used;
    int inode_num;
    char name[MAX_FILE_LENGTH + 1];

    int ref_cnt;
} D_Entry;

typedef struct {
    bool used;
    int inode_num;
    
    int offset;
} File_Descriptor;

typedef struct {
    int data_block[1024];
} First_L;

typedef struct {
    int first_level_blocks[1024];
} Second_L;

/* helper function */
/*************************************************/
void fill_buffer(char* buffer, int offset);

int find_empty();

/* global var */
/*************************************************/

SuperBlock super;

//array
Inode Table[MAX_NUM_OF_FILES];

//array;
D_Entry Directory[MAX_NUM_OF_FILES];

//array
char bitmap[DISK_BLOCKS];

File_Descriptor fd_arr[MAX_FILE_DESCRIPTOR];


static bool mounted = false;

char empty_buffer[BLOCK_SIZE] = {'\0'};

/* function */
/*************************************************/

int make_fs(const char *disk_name)
{

    // create and open a disk;
    if(make_disk(disk_name) == -1) { return -1; }
    if(open_disk(disk_name) == -1) { return -1; }

    SuperBlock SB;
    
    SB.dir_offset = 1;
    //starting block for inode
    SB.inode_metadata_offset = 2;
    //starting block for bitmap
    SB.bitmap_offset = 3;
    SB.bitmap_used = (DISK_BLOCKS / BLOCK_SIZE) + ((DISK_BLOCKS % BLOCK_SIZE) != 0);

    //starting block for data blocks
    SB.data_offset = 3 + SB.bitmap_used;

    char* buffer = malloc(BLOCK_SIZE);
    fill_buffer(buffer, 0);

    memcpy(buffer, (void*)&SB, sizeof(SuperBlock));
    fill_buffer(buffer, sizeof(SuperBlock));
    if(block_write(0,buffer) == -1) { return -1; }

    D_Entry empty_dir;
    empty_dir.used = false;
    empty_dir.name[0] = '\0';
    empty_dir.inode_num = -1;
    empty_dir.ref_cnt = 0;

    //writing Dentry to disk
    for(int ii = 0; ii < MAX_NUM_OF_FILES; ii++) {
        memcpy(buffer + ii * sizeof(D_Entry), (void*)&empty_dir, sizeof(D_Entry));
    }
    fill_buffer(buffer, sizeof(D_Entry) * MAX_NUM_OF_FILES);
    if(block_write(SB.dir_offset, buffer) == -1) { return -1; }

    Inode empty_inode;
    empty_inode.Two_offset = -1;    //to be changed later
    // empty_inode.type = '\0';
    empty_inode.size = 0;

    for(int ii = 0; ii < MAX_NUM_OF_FILES; ii++) {
        memcpy(buffer + ii * sizeof(Inode), &empty_inode, sizeof(Inode));
    }
    fill_buffer(buffer, sizeof(Inode) * MAX_NUM_OF_FILES);
    if(block_write(SB.inode_metadata_offset, buffer) == -1) { return -1; }

    char empty_bit = '0';
    for(int ii = 0; ii < BLOCK_SIZE; ii++) {
        memcpy(buffer + ii * sizeof(char), &empty_bit, sizeof(char));
    }
    for(int ii = 0; ii < SB.bitmap_used; ii++) {
            if(block_write(SB.bitmap_offset + ii, buffer) == -1) { return -1; }
    }

    if(close_disk(disk_name) == -1) { return -1; }

    free(buffer);
    return 0;
}

int mount_fs(const char *disk_name)
{
    if(!disk_name || mounted) { return -1; }
    if(open_disk(disk_name) == -1) { return -1;}

    //buffer to read
    char* buffer = malloc(BLOCK_SIZE);

    //mount SuperBlock
    if(block_read(0, buffer) == -1) { return -1; }
    memcpy(&super, buffer, sizeof(SuperBlock));

    //mount Directory Entry Block
    if(block_read(super.dir_offset, buffer) == -1) { return -1; }
    memcpy(Directory, buffer, MAX_NUM_OF_FILES * sizeof(D_Entry));

    //mount Inode array
    if(block_read(super.inode_metadata_offset, buffer) == -1) { return -1; }
    memcpy(Table, buffer, MAX_NUM_OF_FILES * sizeof(Inode));

    //mount the Bitmap
    for(int ii = 0; ii < super.bitmap_used; ii++) {
        if(block_read(super.bitmap_offset + ii, buffer) == -1) { return -1; }
        // assert(memcpy(bitmap + ii * BLOCK_SIZE, buffer, BLOCK_SIZE) == bitmap + ii * BLOCK_SIZE);
        memcpy(bitmap + ii * BLOCK_SIZE, buffer, BLOCK_SIZE);
    }

    for(int ii = 0; ii < MAX_NUM_OF_FILES; ii++) {
        fd_arr[ii].used = false;
    }

    mounted = true;
    free(buffer);
    return 0;
}

int umount_fs(const char *disk_name){

    if(!disk_name) { return -1; }
    if(!mounted) { return -1; }

    //buffer to write
    char *buffer = malloc(BLOCK_SIZE);
    fill_buffer(buffer, 0);
    
    //superBlock write to disk
    memcpy(buffer, &super, sizeof(SuperBlock));
    if(block_write(0,buffer) == -1) { return -1; }

    //Directory entry 
    memcpy(buffer, Directory, sizeof(D_Entry) * MAX_NUM_OF_FILES);
    fill_buffer(buffer, sizeof(D_Entry) * MAX_NUM_OF_FILES);
    if(block_write(super.dir_offset, buffer) == -1) { return -1; }

    //inode block to disk
    memcpy(buffer, Table, sizeof(Inode) * MAX_NUM_OF_FILES);
    fill_buffer(buffer, sizeof(Inode) * MAX_NUM_OF_FILES);
    if(block_write(super.inode_metadata_offset, buffer) == -1) { return -1; }

    //bitmap to disk
    for(int ii = 0; ii < super.bitmap_used; ii++) {
        memcpy(buffer, bitmap + ii * BLOCK_SIZE, BLOCK_SIZE);
        if(block_write(super.bitmap_offset + ii, buffer) == -1) { return -1; }
    }

    //Clear the file descriptor array
    for(int ii = 0; ii < MAX_FILE_DESCRIPTOR; ii++) {
        fd_arr[ii].used = false;
        fd_arr[ii].inode_num = -1;      //to be changed
    }

    if(close_disk(disk_name) == -1) { return -1; }

    mounted = false;

    free(buffer);

    return 0;
}

int fs_open(const char *name){
    int fd; 
    //finds next availible fd
    for(fd = 0; fd < MAX_FILE_DESCRIPTOR; fd++) {
        if(!fd_arr[fd].used) { break; }
        else if(fd == MAX_FILE_DESCRIPTOR -1) { return -1; }
    }

    for(int ii = 0; ii < MAX_NUM_OF_FILES; ii++) {
        if(strcmp(name, Directory[ii].name) == 0) {
            fd_arr[fd].used = true;
            fd_arr[fd].inode_num = ii;
            fd_arr[fd].offset = 0;
            (Directory[fd_arr[fd].inode_num].ref_cnt)++;

            return fd;
        }
    }

    return -1;
}

int fs_close(int fildes){
    if(!fd_arr[fildes].used) {
        return -1;
    }
    (Directory[fd_arr[fildes].inode_num].ref_cnt)--;
    fd_arr[fildes].used = false;
    fd_arr[fildes].inode_num = -1;
    fd_arr[fildes].offset = 0;
    
    return 0;
}

int fs_create(const char *name){
    if(strlen(name) > MAX_FILE_LENGTH || !name) { return -1; }

    //check how many files we have and also checks if file w same name exits
    int cnt = 0;
    for(int ii = 0; ii < MAX_NUM_OF_FILES; ii++) {
        if(Directory[ii].used) {
            if(strcmp(name, Directory[ii].name) == 0) {
                return -1;
            }
            cnt++;
        }
    }
    if(cnt == MAX_NUM_OF_FILES) { return -1; }

    //find emoty block to fit the only second-layer-indirect-block
    int offset = find_empty();

    //inode and Dentry are the same for my implementation
    for(int ii = 0; ii < MAX_NUM_OF_FILES; ii++) {
        if(!Directory[ii].used) {
            //set up the Inode
            // Table[ii].type = '1';
            Table[ii].Two_offset = offset;

            //set up the Dentry
            Directory[ii].used = true;
            strcpy(Directory[ii].name, name);
            Directory[ii].inode_num = ii;
            break;
        }
    }

    //create a Second and first level indirect Block
    Second_L first_arr;
    First_L dataBlocks;
    for(int ii = 0; ii < (BLOCK_SIZE / 4); ii++) {
        //initializing
        dataBlocks.data_block[ii] = -1;
        first_arr.first_level_blocks[ii] = -1;
    }

    //find a block to store first level indirect block
    int first = find_empty();  
    //copy offset of data block into the second-layer-indirect block
    first_arr.first_level_blocks[0] = first;
    if(block_write(offset, &first_arr) == -1) { return -1; }

    //find a block to store data
    int data = find_empty();

    //copy offset of data block into the first-layer-indirect block
    dataBlocks.data_block[0] = data;
    if(block_write(first, &dataBlocks) == -1) { return -1; }

    return 0;
}

int fs_delete(const char *name){
    int inode;
    for(inode = 0; inode < MAX_NUM_OF_FILES; inode++) {
        if(strcmp(Directory[inode].name, name) == 0) {
            if(Directory[inode].ref_cnt > 0) {
                return -1;
            }
            break;
        } else if (inode == MAX_NUM_OF_FILES - 1) {
            return -1;
        }
    }

    Second_L indirect;
    First_L dataBlocks;

    if(block_read(Table[inode].Two_offset, &indirect) == -1) { return -1; }
    int index = 0;
    while(indirect.first_level_blocks[index] != -1) {
        if(block_read(indirect.first_level_blocks[index], &dataBlocks) == -1) { return -1; }

        //clear data blocks set corresponding bitmap to 0;
        int ii = 0;
        while (dataBlocks.data_block[ii] != -1) {
            if (block_write(dataBlocks.data_block[ii], &empty_buffer) == -1) { return -1; }
            bitmap[dataBlocks.data_block[ii]] = '0';
            if(ii == 1023) { break; }
            ii++;
        }
        
        //clear the first level entries
        if (block_write(indirect.first_level_blocks[index], &empty_buffer) == -1) { return -1; }
        bitmap[indirect.first_level_blocks[index]] = '0';
        index++;
    }
    
    //clear the only second level entry
    // Table[inode].type = '0';
    if (block_write(Table[inode].Two_offset, &empty_buffer) == -1 ) { return -1; }
    Directory[inode].inode_num = -1;
    Directory[inode].name[0] = '\0';
    Directory[inode].used = false;
    bitmap[Table[inode].Two_offset] = '0';

    return 0;
}

int fs_read(int fildes, void *buf, size_t nbyte){
    if(fildes < 0 || fildes > MAX_FILE_DESCRIPTOR || !fd_arr[fildes].used) { return -1; }

    int bytesRead = 0;

    //starting address
    int fd_off_blk = fd_arr[fildes].offset / BLOCK_SIZE;
    int off_inblk = fd_arr[fildes].offset % BLOCK_SIZE;

    int inodenum = fd_arr[fildes].inode_num;
    Inode* inode = &Table[inodenum];

    int second_index = fd_arr[fildes].offset / (2 << 21);
    //create buffer
    Second_L* second = malloc(BLOCK_SIZE);
    First_L* first = malloc(BLOCK_SIZE);
    if(block_read(inode->Two_offset, second) == -1) { return -1; }
    if(block_read(second->first_level_blocks[second_index], first) == -1) { return -1; }

    char* diskdata = malloc(BLOCK_SIZE);
    if(block_read(first->data_block[fd_off_blk], diskdata) == -1) { return -1; }

    int toRead;
    if((inode->size - fd_arr[fildes].offset) >= nbyte) {
        toRead = nbyte;
    } else {
        toRead = (inode->size - fd_arr[fildes].offset);
    }

    for(int ii = 0; ii < toRead; ii++) {
        ((char*)buf)[ii] = diskdata[off_inblk];
        off_inblk++;
        if(off_inblk == BLOCK_SIZE && fd_off_blk == 1023) {
            second_index++;
            //load the next first
            if(block_read(second->first_level_blocks[second_index], first) == -1) { return -1; }
            //load the next data block
            fd_off_blk = 0;
            block_read(first->data_block[fd_off_blk], diskdata);
            off_inblk = 0;
        }
        if(off_inblk == BLOCK_SIZE) {
            //move on to the next data block
            fd_off_blk++;
            if(block_read(first->data_block[fd_off_blk], diskdata) == -1) { return -1; }
            off_inblk = 0;
        }
        bytesRead++;
    }

    free(diskdata); free(second); free(first);
    return bytesRead;
}

int fs_write(int fildes, void *buf, size_t nbyte){
    if(fildes < 0 || fildes > MAX_FILE_DESCRIPTOR || !fd_arr[fildes].used) { return -1; }

    int bytesCpied = 0;
    
    //starting address
    int fd_off_blk = fd_arr[fildes].offset / BLOCK_SIZE;
    int off_inblk = fd_arr[fildes].offset % BLOCK_SIZE;

    int inodenum = fd_arr[fildes].inode_num;
    // D_Entry* dentry = &Directory[inodenum];
    Inode* inode = &Table[inodenum];
    if((fd_arr[fildes].offset + nbyte) > inode->size) { inode->size = (fd_arr[fildes].offset + nbyte); }

    int second_index = (fd_arr[fildes].offset - 1) / (2 << 21); //offset / 4mb
    //create buffer
    Second_L* second = malloc(BLOCK_SIZE);
    First_L* first = malloc(BLOCK_SIZE);
    if(block_read(inode->Two_offset, second) == -1) { return -1; }
    if(block_read(second->first_level_blocks[second_index], first) == -1) { return -1; }

    char* diskdata = malloc(BLOCK_SIZE);
    if(block_read(first->data_block[fd_off_blk], diskdata) == -1) { return -1; }

    //write to disk
    for(int ii = 0; ii < nbyte; ii++) {
        diskdata[off_inblk] = ((char*)buf)[ii];
        off_inblk++;

        if(fd_off_blk == 1023 && off_inblk == BLOCK_SIZE) {
            //when first is full
            //update the diskdata
            block_write(first->data_block[fd_off_blk], diskdata);
            if(block_write(second->first_level_blocks[second_index], first) == -1) { return -1; }
            second_index++;
            //create a new first level block
            second->first_level_blocks[second_index] = find_empty();
            if(block_write(inode->Two_offset, second) == -1) { return -1; }     //upate second
            //load new first
            if(block_read(second->first_level_blocks[second_index], first) == -1) { return -1; }
            fd_off_blk = 0; //inblock set to 0
            //create a new data block
            first->data_block[fd_off_blk] = find_empty();
            //update the new first
            block_write(second->first_level_blocks[second_index], first);
            //open new diskdata block
            off_inblk = 0;  //first block offset to 0
            block_read(first->data_block[off_inblk], diskdata);
        }
        if(off_inblk == BLOCK_SIZE) {
            if(block_write(first->data_block[fd_off_blk], diskdata) == -1) { return -1; }
            //move on to the next data block
            fd_off_blk++;
            if(first->data_block[fd_off_blk] == -1) {
                //need to malloc new block;
                int new = find_empty();
                first->data_block[fd_off_blk] = new;
                //update
                if(block_write(second->first_level_blocks[second_index], first) == -1) { return -1; }

                if(block_read(first->data_block[fd_off_blk], diskdata) == -1) { return -1; }
            } else {
                if(block_read(first->data_block[fd_off_blk], diskdata) == -1) { return -1; }
            }
            off_inblk = 0;
        }
        bytesCpied++;
    }
    if(block_write(first->data_block[fd_off_blk], diskdata) == -1) { return -1; }

    free(diskdata); free(second); free(first);
    fd_arr[fildes].offset = fd_arr[fildes].offset + bytesCpied;
    return bytesCpied;
}

int fs_get_filesize(int fildes){
    if(!fd_arr[fildes].used || fildes < 0 || fildes > MAX_FILE_DESCRIPTOR) { return -1; }
    return Table[fd_arr[fildes].inode_num].size;
}

int fs_listfiles(char ***files){
    int num_of_files = 0;
    // Count the number of files present
    for(int i = 0; i < MAX_NUM_OF_FILES; i++){
        if(Directory[i].used){
            num_of_files++;
        }
    }

    // Allocate enough space for the file names
    *files = (char**) calloc(num_of_files + 1, sizeof(char*));
    for(int i = 0; i < num_of_files; i++){
        files[0][i] = (char*) calloc(MAX_FILE_LENGTH + 1, sizeof(char));
    }
    files[0][num_of_files] = (char*) malloc(sizeof(char));

    int index = 0;
    // Fill the names of files
    for(int i = 0; i < MAX_NUM_OF_FILES; i++){
        if(Directory[i].used){
            strncpy(files[0][index], Directory[i].name, sizeof(char[MAX_FILE_LENGTH + 1]));
            index++;
        }
    }
    files[0][num_of_files] = NULL;

    return 0;
}

int fs_lseek(int fildes, off_t offset){
    if(offset < 0 || offset > fs_get_filesize(fildes) || !fd_arr[fildes].used || fildes < 0 || fildes > MAX_FILE_DESCRIPTOR) { return -1; }
    fd_arr[fildes].offset = offset;
    return 0;
}

int fs_truncate(int fildes, off_t length){
    if(length < 0 || length > fs_get_filesize(fildes) || !fd_arr[fildes].used || fildes < 0 || fildes > MAX_FILE_DESCRIPTOR) { return -1; }

    Inode* inode = &Table[fd_arr[fildes].inode_num];
    inode->size = length;
    //starting    
    int fd_off_blk = fd_arr[fildes].offset / BLOCK_SIZE + 1;
    // int off_inblk = fd_arr[fildes].offset % BLOCK_SIZE;

    // int bytesToFree = BLOCK_SIZE - off_inblk;
    //create buffer
    Second_L* second = malloc(BLOCK_SIZE);
    First_L* first = malloc(BLOCK_SIZE);
    if(block_read(inode->Two_offset, second) == -1) { return -1; }
    if(block_read(second->first_level_blocks[0], first) == -1) { return -1; } 
    for(int ii = fd_off_blk; ii < BLOCK_SIZE / 4; ii++) {
        if (first->data_block[ii] != -1) {
            block_write(first->data_block[ii], empty_buffer);
            bitmap[first->data_block[ii]] = '0';
            first->data_block[ii] = -1;
            block_write(second->first_level_blocks[0], first);
        }
    }

    free(second); free(first);
    return 0;
}

void fill_buffer(char* buffer,int offset) {
    for(int ii = offset; ii < BLOCK_SIZE; ii++) {
        buffer[ii] = '\0';
    }
}

int find_empty() {
    int offset;
    for(offset = super.data_offset; offset < DISK_BLOCKS; offset++) {
        if(bitmap[offset] == '0') {
            //mark as used
            bitmap[offset] = '1';
            break;
        }
    }
    return offset;
}