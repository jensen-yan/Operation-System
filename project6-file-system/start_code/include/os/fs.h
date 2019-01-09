#ifndef INCLUDE_FS_H_
#define INCLUDE_FS_H_
#include "sched.h"
#include "time.h"

#define MAX_NAME_LEN    24
#define MAX_DIR_BLK     8

//mode of file
#define O_RD            0
#define O_WR            1
#define O_RDWR          2

//initial addr
#define SUPERBLK_ADDR   0xa0f00200
#define BLKMAP_ADDR     0xa0f00400
#define INODEMAP_ADDR   0xa0f04400
#define INODE_ADDR      0xa0f04600
#define DATABLK_ADDR    0xa0f44600

#define VTP(X)  (X)//((uint32_t)(X) & 0x1fffffff)

#define VBLK_SD_ADDR       0xffff000
#define SUPERBLK_SD_ADDR   0x10000000
#define BLKMAP_SD_ADDR     0x10000200
#define INODEMAP_SD_ADDR   0x10004200
#define INODE_SD_ADDR      0x10004400
#define DATABLK_SD_ADDR    0x10044400

//size
#define SECTOR_SZ   0x200
#define BLK_SZ      0x1000

//Magic number
#define MAGICNUM    0x46494c45 //"FILE"

//superblock
typedef struct superblock{
    uint32_t magic_num; 
    uint32_t fs_sz; //file system size
    uint32_t start_sector;

    uint32_t blockmap_addr;
    uint32_t blockmap_off;
    uint32_t blockmap_num;

    uint32_t inodemap_addr;
    uint32_t inodemap_off;
    uint32_t inodemap_num;

    uint32_t inodes_addr;
    uint32_t inodes_off;
    uint32_t inodes_num;

    uint32_t datablock_addr;
    uint32_t datablock_off;
    uint32_t datablock_num;
} superblock_t;

//16*4=64B
#define INODES_PERBLK    64
typedef struct inode{
    uint32_t inode_id;
    uint16_t mode; //r, r/w
    uint16_t ref_num;
    uint32_t used_sz;
    uint32_t create_time;
    uint32_t modify_time;
    uint32_t num; //used num of data-block (for file) or dentry (for dentry)
    uint32_t direct[MAX_DIR_BLK]; //direct
    uint32_t first;     //1st 
    uint32_t second;    //2nd
} inode_t;

//23B + 1B + 4B*2 = 32B
//1 datablock : 4KB / 32B = 128
typedef struct dentry {
    char name[MAX_NAME_LEN];
    uint32_t  type; //0: NONE, 1: file, 2: dentry, 3:".", 4:".."
    //uint8_t ref;
    uint32_t inode_id;
} dentry_t;

#define MAX_NFD     64
typedef struct filedesc{
    uint32_t inode_id;
    uint32_t aviliable;
    uint32_t st_addr;
    uint32_t r_cur_pos;
    uint32_t w_cur_pos;
} filedesc_t;

superblock_t *superblk;
uint32_t *superblk_buf;
uint8_t *blkmap;
uint8_t *inodemap;
uint32_t *inode_buf;
inode_t *inode;
inode_t *cur_inode;
dentry_t *cur_dentry;
int cur_dep;
char path[4][MAX_NAME_LEN];
filedesc_t opfile[MAX_NFD];


#define WB_INODE(inode_id)                                                \
{                                                                         \
    sdwrite((char *)VTP(INODE_ADDR + BLK_SZ*((inode_id)/INODES_PERBLK)),  \
    INODE_SD_ADDR + BLK_SZ*((inode_id)/INODES_PERBLK), BLK_SZ);           \
}

#define WB_BLKSZ(datablk_addr, datablk_sd_addr)                        \
{                                                                      \
    sdwrite((char *)VTP(datablk_addr), datablk_sd_addr, BLK_SZ);      \
}

#define CLEAR_BLKMAP(sd_addr)                                           \
{                                                                       \
    blkmap[((sd_addr) - INODE_SD_ADDR)/BLK_SZ/8] &=                     \
    ~(1 << ((sd_addr) - INODE_SD_ADDR)/BLK_SZ%8);                       \
    sdwrite((char *)VTP((INODEMAP_ADDR + ((sd_addr) - INODE_SD_ADDR)/BLK_SZ/8)&0xfffffe00), \
    (sd_addr) & 0xfffffe00, 0x200);                                          \
}

#define cur_time (time_elapsed / 10000000)

int init_fs();
int do_mkfs();
int do_mkdir(char *sname);
int do_rmdir(char *sname);
int do_cd(char *dir);
void do_statfs();
int do_ls(char *dir);
int do_touch(char *sname);
int do_cat(char *sname);
int do_fopen(char *sname, int acess);
int do_fread(int fd, char *buff, int size);
int do_fwrite(int fd, char *buff, int size);
void do_close(int fd);
void do_cfs();
#endif