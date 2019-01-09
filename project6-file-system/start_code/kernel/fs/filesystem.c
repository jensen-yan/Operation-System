#include "fs.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "string.h"
#include "stdio.h"

uint64_t inode_blk_ld = 0;
uint32_t used_inode_id = 0;
uint32_t alloc_inode()
{
    uint32_t i = used_inode_id;
    inodemap[i/8] = inodemap[i/8] & 1 << i % 8;
    sdwrite((char *)VTP(inodemap), INODEMAP_SD_ADDR, 0x200);
    if(!(inode_blk_ld & 1 << i / INODES_PERBLK))
    {
        uint32_t addr = INODE_ADDR + BLK_SZ * (i / INODES_PERBLK);
        uint32_t sd_addr = INODE_SD_ADDR + BLK_SZ * (i / INODES_PERBLK);
        sdread((char *)VTP(addr), sd_addr, BLK_SZ);
        inode_blk_ld &= 1 << i / INODES_PERBLK;
    }
    used_inode_id++;
    return i;
}

void free_inode(int inode_id)
{
    int i;
    for(i = 0; i < MAX_DIR_BLK; i++)
    {
        if(inode[inode_id].direct[i] != 0)
            CLEAR_BLKMAP(inode[inode_id].direct[i]);
    }
    if(inode[inode_id].first != 0)
        CLEAR_BLKMAP(inode[inode_id].first);
    if(inode[inode_id].second != 0)
        CLEAR_BLKMAP(inode[inode_id].second);
    inodemap[inode_id/8] &= ~(1 << inode_id%8);
    sdwrite((char *)VTP(INODEMAP_ADDR), INODEMAP_SD_ADDR, 0x200);
}

uint32_t used_datablk_id = 65;
uint32_t alloc_datablk()
{
    uint32_t i = used_datablk_id++;
    blkmap[i/8] = blkmap[i/8] & 1 << i % 8;
    sdwrite((char *)VTP((uint32_t)&blkmap[i/8] & 0xfffffe00), BLKMAP_SD_ADDR + i/4096 * 0x200, 0x200);
    return DATABLK_SD_ADDR + (i-65) * 0x1000;
}

uint32_t buf[1024];
void init_entry(uint32_t pt, uint32_t cur_inode_id, uint32_t par_inode_id)
{
    bzero(buf, sizeof(buf));
    dentry_t *entry = (dentry_t *)buf;
    strcpy((char *)entry[0].name, (char *)".");
    entry[0].type = 3;
    entry[0].inode_id = cur_inode_id;
    strcpy((char *)entry[1].name, (char *)"..");
    entry[1].type = 4;
    entry[1].inode_id = par_inode_id;
    sdwrite((char *)VTP(buf), pt, BLK_SZ);   
}

int alloc_fd()
{
    int i;
    for(i = 0; i < MAX_NFD; i++)
        if(opfile[i].inode_id == 0)
            return i;
    return -1;
}

int init_fs()
{
    //read super block
    superblk_buf = (uint32_t *)SUPERBLK_ADDR;
    superblk = (superblock_t *)SUPERBLK_ADDR;
    sdread((char *)VTP(SUPERBLK_ADDR), (uint32_t)SUPERBLK_SD_ADDR, 0x200);
    cur_dep = -1;
    int i;
    for(i = 0; i < 4; i++)
        path[i][0] = '\0';
    //debug
    if(superblk->magic_num != MAGICNUM)
        return 0;
    //read block map
    blkmap = (uint8_t *)BLKMAP_ADDR;
    sdread((char *)VTP(BLKMAP_ADDR), (uint32_t)BLKMAP_SD_ADDR, 0x4000);
    //read inode map
    inodemap = (uint8_t *)INODEMAP_ADDR;
    sdread((char *)VTP(INODEMAP_ADDR), (uint32_t)INODEMAP_SD_ADDR, 0x200);
    //read inodes
    inode_buf = (uint32_t *)INODE_ADDR;
    inode = (inode_t *)inode_buf;
    sdread((char *)VTP(INODE_ADDR), (uint32_t)INODE_SD_ADDR, 0x2000);
    //current dir
    cur_inode = inode;
    cur_dentry = NULL;
    bzero((char *)opfile, sizeof(opfile));
    return 1;
}

int do_mkfs()
{
    int i;
    if(superblk->magic_num == MAGICNUM)
        return 0;
    my_printk("[FS] Start initialize filesystem!          \n");
    my_printk("[FS] Setting superblock...                 \n");
    screen_reflush();

    superblk_buf = (uint32_t *)SUPERBLK_ADDR;
    bzero((void *)superblk_buf, 0x200);
    //head
    superblk_buf[0]  = MAGICNUM;
    superblk_buf[1]  = 0x20000000; //512MB
    superblk_buf[2]  = 524288; //256MB
    //block map
    superblk_buf[3]  = BLKMAP_ADDR;
    superblk_buf[4]  = 1;
    superblk_buf[5]  = 131072; //512MB <--> blocks
    //inode map
    superblk_buf[6]  = INODEMAP_ADDR;
    superblk_buf[7]  = 33;
    superblk_buf[8]  = 4096;
    //inodes
    superblk_buf[9]  = INODE_ADDR;
    superblk_buf[10] = 34;
    superblk_buf[11] = 4096;
    //data block
    superblk_buf[12] = DATABLK_ADDR;
    superblk_buf[13] = 546;
    superblk_buf[14] = 131008;
    //write
    superblk = (superblock_t *)superblk_buf;
    sdwrite((char *)VTP(superblk_buf), SUPERBLK_SD_ADDR, 512);

    //inode map initializa
    my_printk("[FS] Setting inode-map...                  \n");
    screen_reflush();
    inodemap = (uint8_t *)INODEMAP_ADDR;
    bzero((void *)inodemap, superblk->inodemap_num/8);
    inodemap[0] = 1;
    sdwrite((char *)VTP(inodemap), INODEMAP_SD_ADDR, superblk->inodemap_num/8);
    //block map initializa
    my_printk("[FS] Setting block-map...                  \n");
    screen_reflush();
    blkmap = (uint8_t *)BLKMAP_ADDR;
    bzero((void *)blkmap, superblk->blockmap_num/8);
    blkmap[0]  = 1;
    blkmap[16] = 1;
    sdwrite((char *)VTP(blkmap), BLKMAP_SD_ADDR, superblk->blockmap_num/8);
    //inode initializa
    my_printk("[FS] Setting inode...                      \n");
    screen_reflush();
    inode_buf = (uint32_t *)INODE_ADDR;
    bzero((void *)inode_buf, 0x1000);
    inode = (inode_t *)inode_buf;
    used_inode_id = 0;
    inode[0].inode_id = used_inode_id++;
    inode[0].mode = O_RDWR;
    inode[0].ref_num = 0;
    inode[0].used_sz = 8;
    inode[0].create_time = cur_time;
    inode[0].modify_time = cur_time;
    inode[0].num = 2;
    //alloc datablock
    inode[0].direct[0] = alloc_datablk();
    for(i = 1; i < MAX_DIR_BLK; i++)
        inode[0].direct[i] = 0;
    inode[0].first = inode[0].second = 0;
    WB_INODE(0);
    //write dentry
    init_entry(inode[0].direct[0], 0, -1);
    //finish
    my_printk("[FS] Initialize filesystem finished!       \n");
    screen_reflush();
    cur_inode = inode;
    cur_dentry = NULL;
    return 1;
}

int do_mkdir(char *sname)
{
    if(superblk->magic_num != MAGICNUM)
    {
        my_printk("[ERROR] No File System!\n");
        return -1;
    }
    int i, j, k;
    dentry_t *dentry_tmp;
    for(i = 0, j = 0; i < cur_inode->num; i++)
    {
        j = i / INODES_PERBLK;
        k = i % INODES_PERBLK;
        if(k == 0)
        {
            sdread((char *)VTP(buf), cur_inode->direct[j], BLK_SZ);
            dentry_tmp = (dentry_t *)buf;
        }
        if(dentry_tmp[k].type == 2 && !strcmp((char *)dentry_tmp[k].name, sname))
        {
            return 0; //the dentry has already existed
        }
    }

    //add dentry in current dentry
    j = i / INODES_PERBLK;
    k = i % INODES_PERBLK;
    if(k == 0)
    {
        sdread((char *)VTP(buf), cur_inode->direct[j], BLK_SZ);
        dentry_tmp = (dentry_t *)buf;
    }
    strcpy((char *)dentry_tmp[k].name, sname);
    dentry_tmp[k].type = 2;
    i = dentry_tmp[k].inode_id = alloc_inode();
    sdwrite((char *)VTP(buf), cur_inode->direct[j], BLK_SZ);
    cur_inode->ref_num++; //.. --> parent inode
    cur_inode->num++;
    WB_INODE(cur_inode->inode_id);

    //initialize the new dentry
    inode[i].mode = O_RDWR;
    inode[i].inode_id = i;
    inode[i].used_sz = 4;
    inode[i].create_time = cur_time;
    inode[i].modify_time = cur_time;
    inode[i].num = 2;
    inode[i].direct[0] = alloc_datablk();
    for(j = 1; j < MAX_DIR_BLK; j++)
        inode[i].direct[j] = 0;
    inode[i].first = inode[i].second = 0;
    WB_INODE(i);
    init_entry(inode[i].direct[0], inode[i].inode_id, cur_inode->inode_id);
    return 1;
}

int do_rmdir(char *sname)
{
    if(superblk->magic_num != MAGICNUM)
    {
        my_printk("[ERROR] No File System!\n");
        return -1;
    }
    int i, j, k;
    dentry_t *dentry_tmp;
    for(i = 0, j = 0; i < cur_inode->num; i++)
    {
        j = i / INODES_PERBLK;
        k = i % INODES_PERBLK;
        if(k == 0)
        {
            sdread((char *)VTP(buf), cur_inode->direct[j], BLK_SZ);
            dentry_tmp = (dentry_t *)buf;
        }
        if(dentry_tmp[k].type == 2 && !strcmp((char *)dentry_tmp[k].name, sname))
        {
            free_inode(dentry_tmp[k].inode_id);
            dentry_tmp[k].name[0] = '\0';
            dentry_tmp[k].type = 0;
            dentry_tmp[k].inode_id = -1;
            WB_BLKSZ(buf, cur_inode->direct[j]);
            cur_inode->ref_num--;
            cur_inode->modify_time = cur_time;
            cur_inode->used_sz -= 4;
            cur_inode->num--;
            WB_INODE(cur_inode->inode_id);
            return 1;
        }
    }
    return 0;
}

void split(int *argc, char argv[][MAX_NAME_LEN], char *cmd)
{
    int j = 0, k = 0, i = strlen(cmd);
    if(i <= 0)
    {
        *argc = 0;
        return ;
    }
    if(cmd[i-1] != '/')
    {
        cmd[i] = '/';
        cmd[++i] = '\0';
    }
    if(cmd[j] == '/')
        j++;
    for(*argc = k = 0; j < i; j++)
    {
        if(cmd[j] == '/')
        {
            argv[*argc][k] = '\0';
            ++(*argc), k = 0;
            continue;
        }
        argv[*argc][k++] = cmd[j];
    }
}

int do_cd(char *dir)
{
    if(superblk->magic_num != MAGICNUM)
    {
        my_printk("[ERROR] No File System!\n");
        return ;
    }
    int ndir, i, j, k, n, m;
    char bdir[4][MAX_NAME_LEN];
    //tmp path
    int dep_tmp = dir[0] == '/'? -1 : cur_dep;
    char path_tmp[4][MAX_NAME_LEN];
    for(n = 0; n < 4; n++)
        strcpy((char *)path_tmp[n], (char *)path[n]);
    //directory structure
    inode_t *cur_dir = dir[0] == '/'? &inode[0] : cur_inode;
    dentry_t *dentry_tmp;
    //split the directory
    split(&ndir, bdir, dir);

    for(n = 0; n < ndir; n++)
    {
        int find = 0;
        if(!strcmp(bdir[n], "."))
            continue;
        if(!strcmp(bdir[n], ".."))
        {
            if(dep_tmp <= -1)
                return 0;
            sdread((char *)VTP(buf), cur_dir->direct[0], BLK_SZ);
            dentry_tmp = (dentry_t *)buf;
            m = dentry_tmp[1].inode_id;
            if(!(inode_blk_ld & 1 << m/INODES_PERBLK))
                sdread((char *)VTP(INODE_ADDR+BLK_SZ*(m/INODES_PERBLK)), INODE_SD_ADDR+BLK_SZ*(m/INODES_PERBLK), BLK_SZ);
            cur_dir = &inode[m];
            path_tmp[dep_tmp][0] = '\0';
            dep_tmp--;
            continue;
        }    
        for(i = 0; i < cur_dir->num; i++)
        {
            j = i / INODES_PERBLK;
            k = i % INODES_PERBLK;
            if(k == 0)
            {
                sdread((char *)VTP(buf), cur_dir->direct[j], BLK_SZ);
                dentry_tmp = (dentry_t *)buf;
            }
            if(dentry_tmp[k].type == 2 && !strcmp((char *)dentry_tmp[k].name, bdir[n]))
            {
                m = dentry_tmp[k].inode_id;
                if(!(inode_blk_ld & 1 << m/INODES_PERBLK))
                    sdread((char *)VTP(INODE_ADDR+BLK_SZ*(m/INODES_PERBLK)), INODE_SD_ADDR+BLK_SZ*(m/INODES_PERBLK), BLK_SZ);
                cur_dir = &inode[m];
                dep_tmp++;
                strcpy((char *)path_tmp[dep_tmp], bdir[n]);
                find = 1;
                break;
            }
        }
        if(find == 0)
            return 0;
    }
    cur_inode = cur_dir;
    cur_dentry = &dentry_tmp[k];
    cur_dep = dep_tmp;
    for(n = 0; n < 4; n++)
        strcpy((char *)path[n], (char *)path_tmp[n]);
    return 1;
}

void do_statfs()
{
    if(superblk->magic_num != MAGICNUM)
    {
        my_printk("[ERROR] No File System!\n");
        return ;
    }
    int i, j;
    uint32_t used_block = 0, used_inodes = 0;
    for(i = 0; i < superblk->inodes_num/(INODES_PERBLK*8); i++)
        for(j = 0; j < 8; j++)
            used_block += (blkmap[i] >> j) & 1;
    for(; i < 0x4000; i++)
    {
        if(blkmap[i] == 0)
            break;
        for(j = 0; j < 8; j++)
            used_block += (blkmap[i] >> j) & 1;
    }
    for(i = 0; i < superblk->inodes_num/8; i++)
        for(j = 0; j < 8; j++)
            used_inodes += (inodemap[i] >> j) & 1;
    my_printk("Magic Number: 0x%x(KFS)\n", superblk->magic_num);
    my_printk("start sector: %d (0x%x), used block: %d/%d\n", superblk->start_sector, superblk->start_sector, used_block, superblk->datablock_num);
    my_printk("block map offset: %d, occupied block: %d\n", superblk->blockmap_off, superblk->datablock_num/(8*512*8));
    my_printk("inode map offset: %d, occupied sector: %d, used: %d\n", superblk->inodemap_off, superblk->inodemap_num/(8*512), used_inodes);
    my_printk("inode offset: %d, occupied block: %d\n", superblk->inodes_off, superblk->inodes_num/INODES_PERBLK);
    my_printk("data offset: %d, occupied block: %d\n", superblk->datablock_off, superblk->datablock_num);
    my_printk("entry size: %dB\n", sizeof(dentry_t));
    screen_reflush();
}

int do_ls(char *dir)
{
    if(superblk->magic_num != MAGICNUM)
    {
        my_printk("[ERROR] No File System!\n");
        return -1;
    }
    int ndir, i, j, k, n, m;
    char bdir[4][MAX_NAME_LEN];
    //directory structure
    int dep_tmp = dir[0] == '/'? -1 : cur_dep;
    inode_t *cur_dir = dir[0] == '/'? &inode[0] : cur_inode;
    dentry_t *dentry_tmp;
    //split the directory
    split(&ndir, bdir, dir);

    for(n = 0; n < ndir; n++)
    {
        int find = 0;
        if(!strcmp(bdir[n], "."))
            continue;
        if(!strcmp(bdir[n], ".."))
        {
            if(dep_tmp <= -1)
                return 0;
            sdread((char *)VTP(buf), cur_dir->direct[0], BLK_SZ);
            dentry_tmp = (dentry_t *)buf;
            m = dentry_tmp[1].inode_id;
            if(!(inode_blk_ld & 1 << m/INODES_PERBLK))
                sdread((char *)VTP(INODE_ADDR+BLK_SZ*(m/INODES_PERBLK)), INODE_SD_ADDR+BLK_SZ*(m/INODES_PERBLK), BLK_SZ);
            cur_dir = &inode[m];
            dep_tmp--;
            continue;
        }    
        for(i = 0; i < cur_dir->num; i++)
        {
            j = i / INODES_PERBLK;
            k = i % INODES_PERBLK;
            if(k == 0)
            {
                sdread((char *)VTP(buf), cur_dir->direct[j], BLK_SZ);
                dentry_tmp = (dentry_t *)buf;
            }
            if(dentry_tmp[k].type == 2 && !strcmp((char *)dentry_tmp[k].name, bdir[n]))
            {
                m = dentry_tmp[k].inode_id;
                if(!(inode_blk_ld & 1 << m/INODES_PERBLK))
                    sdread((char *)VTP(INODE_ADDR+BLK_SZ*(m/INODES_PERBLK)), INODE_SD_ADDR+BLK_SZ*(m/INODES_PERBLK), BLK_SZ);
                cur_dir = &inode[m];
                dep_tmp++;
                find = 1;
                break;
            }
        }
        if(find == 0)
            return 0;
    }

    for(i = 0; i < cur_dir->num; i++)
    {
        j = i / INODES_PERBLK;
        k = i % INODES_PERBLK;
        if(k == 0)
        {
            sdread((char *)VTP(buf), cur_dir->direct[j], BLK_SZ);
            dentry_tmp = (dentry_t *)buf;
        }
        my_printk("%s ", dentry_tmp[k].name);
    }
    my_printk("\n");
    screen_reflush();
    return 1;
}

int do_touch(char *sname)
{
    if(superblk->magic_num != MAGICNUM)
    {
        my_printk("[ERROR] No File System!\n");
        return -1;
    }
    int i, j, k;
    dentry_t *dentry_tmp;
    //check exists
    for(i = 0, j = 0; i < cur_inode->num; i++)
    {
        j = i / INODES_PERBLK;
        k = i % INODES_PERBLK;
        if(k == 0)
        {
            sdread((char *)VTP(buf), cur_inode->direct[j], BLK_SZ);
            dentry_tmp = (dentry_t *)buf;
        }
        if(dentry_tmp[k].type == 1 && !strcmp((char *)dentry_tmp[k].name, sname))
        {
            return 0; //the dentry has already existed
        }
    }

    //add dentry in current dentry
    j = i / INODES_PERBLK;
    k = i % INODES_PERBLK;
    if(k == 0)
    {
        sdread((char *)VTP(buf), cur_inode->direct[j], BLK_SZ);
        dentry_tmp = (dentry_t *)buf;
    }
    strcpy((char *)dentry_tmp[k].name, sname);
    dentry_tmp[k].type = 1;
    i = dentry_tmp[k].inode_id = alloc_inode();
    sdwrite((char *)VTP(buf), cur_inode->direct[j], BLK_SZ);
    cur_inode->num++;
    WB_INODE(cur_inode->inode_id);

    //initialize the new dentry
    inode[i].mode = O_RDWR;
    inode[i].inode_id = i;
    inode[i].used_sz = 0;
    inode[i].create_time = cur_time;
    inode[i].modify_time = cur_time;
    inode[i].num = 1;
    inode[i].direct[0] = alloc_datablk();
    for(j = 1; j < MAX_DIR_BLK; j++)
        inode[i].direct[j] = 0;
    inode[i].first = inode[i].second = 0;
    WB_INODE(i);
    bzero(buf, sizeof(buf));
    sdwrite((char *)VTP(buf), inode[i].direct[0], BLK_SZ);
    return 1;
}

int do_cat(char *sname)
{
    if(superblk->magic_num != MAGICNUM)
    {
        my_printk("[ERROR] No File System!\n");
        return -1;
    }
    int i, j, k;
    dentry_t *dentry_tmp;
    //check exists
    for(i = 0, j = 0; i < cur_inode->num; i++)
    {
        j = i / INODES_PERBLK;
        k = i % INODES_PERBLK;
        if(k == 0)
        {
            sdread((char *)VTP(buf), cur_inode->direct[j], BLK_SZ);
            dentry_tmp = (dentry_t *)buf;
        }
        if(dentry_tmp[k].type == 1 && !strcmp((char *)dentry_tmp[k].name, sname))
        {
            int m = dentry_tmp[k].inode_id;
            if(!(inode_blk_ld & 1 << m/INODES_PERBLK))
                sdread((char *)VTP(INODE_ADDR+BLK_SZ*(m/INODES_PERBLK)), INODE_SD_ADDR+BLK_SZ*(m/INODES_PERBLK), BLK_SZ);
            char *buff = (char *)DATABLK_ADDR;
            for(j = 0; j < inode[m].num; j++)
            {
                bzero((char *)buff, BLK_SZ);
                sdread((char *)VTP(buff), inode[m].direct[j], BLK_SZ);
                for(k = 0; k < inode[m].used_sz - j*BLK_SZ; k++)
                {
                    my_printk("%c", buff[k]);
                }
                    
            }
            return 1;
        }
    }
    //No file
    return 0;
}

int do_fopen(char *sname, int acess)
{
    if(superblk->magic_num != MAGICNUM)
    {
        my_printk("[ERROR] No File System!\n");
        return -1;
    }
    int i, j, k;
    dentry_t *dentry_tmp;
    //check exists
    for(i = 0, j = 0; i < cur_inode->num; i++)
    {
        j = i / INODES_PERBLK;
        k = i % INODES_PERBLK;
        if(k == 0)
        {
            sdread((char *)VTP(buf), cur_inode->direct[j], BLK_SZ);
            dentry_tmp = (dentry_t *)buf;
        }
        if(dentry_tmp[k].type == 1 && !strcmp((char *)dentry_tmp[k].name, sname))
        {
            int m = dentry_tmp[k].inode_id;
            if(!(inode_blk_ld & 1 << m/INODES_PERBLK))
                sdread((char *)VTP(INODE_ADDR+BLK_SZ*(m/INODES_PERBLK)), INODE_SD_ADDR+BLK_SZ*(m/INODES_PERBLK), BLK_SZ);
            if(inode[m].mode != acess && inode[m].mode != O_RDWR)
                return -1; //Cannot Access
            m = alloc_fd();
            opfile[m].inode_id = dentry_tmp[k].inode_id;
            opfile[m].r_cur_pos = 0;
            opfile[m].w_cur_pos = 0;
            opfile[m].st_addr = 0;
            opfile[m].aviliable = acess;
            return m;
        }
    }
    if(acess == O_RD)
    {
        //No file
        return -1;
    }
    return -1;
}

int do_fread(int fd, char *buff, int size)
{
    int k = opfile[fd].inode_id;
    if(inode[k].used_sz - opfile[fd].r_cur_pos < size)
        return 0;
    int i = opfile[fd].r_cur_pos/BLK_SZ;
    int r = 0;
    char *buff_t = (char *)DATABLK_ADDR;
    if(opfile[fd].r_cur_pos%BLK_SZ != 0)
    {
        bzero(buff_t, BLK_SZ);
        sdread((char *)VTP(buff_t), inode[k].direct[i], BLK_SZ);
        if(BLK_SZ-opfile[fd].r_cur_pos%BLK_SZ <= size)
        {
            memcpy(buff, buff_t+opfile[fd].r_cur_pos%BLK_SZ, BLK_SZ-opfile[fd].r_cur_pos%BLK_SZ);
            opfile[fd].r_cur_pos += BLK_SZ-opfile[fd].r_cur_pos%BLK_SZ;
        }
        else
        {
            memcpy(buff, buff_t+opfile[fd].r_cur_pos%BLK_SZ, size);
            opfile[fd].r_cur_pos += size;
        }
        r += BLK_SZ-opfile[fd].r_cur_pos%BLK_SZ;
        i++;
    }
    while(r < size)
    {
        bzero(buff_t, BLK_SZ);
        sdread((char *)VTP(buff_t), inode[k].direct[i], BLK_SZ);
        if(r + BLK_SZ <= size)
        {
            memcpy(buff+r, buff_t, BLK_SZ);
            opfile[fd].r_cur_pos += BLK_SZ;
        }
        else
        {
            memcpy(buff+r, buff_t, size-r);
            opfile[fd].r_cur_pos += size-r;
        }
        i++;
        r += BLK_SZ;
    }
    return 1;
}

int do_fwrite(int fd, char *buff, int size)
{
    int k = opfile[fd].inode_id;
    int i = opfile[fd].w_cur_pos/BLK_SZ;
    int r = 0;
    char *buff_t = (char *)DATABLK_ADDR;
    if(opfile[fd].w_cur_pos%BLK_SZ != 0)
    {
        bzero(buff_t, BLK_SZ);
        sdread((char *)VTP(buff_t), inode[k].direct[i], BLK_SZ);
        if(BLK_SZ-opfile[fd].w_cur_pos%BLK_SZ <= size)
        {
            memcpy(buff_t+opfile[fd].w_cur_pos%BLK_SZ, buff, BLK_SZ-opfile[fd].w_cur_pos%BLK_SZ);
            opfile[fd].w_cur_pos += BLK_SZ-opfile[fd].w_cur_pos%BLK_SZ;
        }
        else
        {
            memcpy(buff_t+opfile[fd].w_cur_pos%BLK_SZ, buff, size);
            opfile[fd].w_cur_pos += size;
        }
        sdwrite((char *)VTP(buff_t), inode[k].direct[i], BLK_SZ);
        r += BLK_SZ-opfile[fd].w_cur_pos%BLK_SZ;
        i++;
    }
    while(r < size)
    {
        bzero(buff_t, BLK_SZ);
        if(inode[k].direct[i] != 0)
            sdread((char *)VTP(buff_t), inode[k].direct[i], BLK_SZ);
        else
            inode[k].direct[i] = alloc_datablk();
        if(r + BLK_SZ <= size)
        {
            memcpy(buff_t, buff+r, BLK_SZ);
            opfile[fd].w_cur_pos += BLK_SZ;
        }
        else
        {
            memcpy(buff_t, buff+r, size-r);
            opfile[fd].w_cur_pos += size-r;
        }
        sdwrite((char *)VTP(buff_t), inode[k].direct[i], BLK_SZ);
        i++;
        r += BLK_SZ;
    }
    inode[k].modify_time = cur_time;
    inode[k].num = i > inode[k].num ? i : inode[k].num;
    inode[k].used_sz += size;
    WB_INODE(inode[k].inode_id);
    return 1;
}

void do_close(int fd)
{
    int k = opfile[fd].inode_id;
    inode[k].modify_time = cur_time;
    WB_INODE(inode[k].inode_id);
    bzero((char *)&opfile[fd], sizeof(opfile[fd]));
}

void do_cfs()
{
    bzero((char *)superblk, 512);
    sdwrite((char *)VTP(superblk), SUPERBLK_SD_ADDR, 512);
    my_printk("The file system was removed!\n");
}
