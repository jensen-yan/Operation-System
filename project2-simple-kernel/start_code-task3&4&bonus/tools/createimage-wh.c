#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 2048
#define SECTOR_SIZE 512

char buffer[BUF_SIZE];

void write_bootblock(FILE *image, FILE *bbfile, Elf32_Phdr *Phdr);
Elf32_Phdr *read_exec_file(FILE *opfile);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr);
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz);

//使用的文件读写函数原型
// size_t fread(void *ptr, size_t size_of_elements,
//  size_t number_of_elements, FILE *a_file);

// size_t fwrite(const void *ptr, size_t size_of_elements,
//  size_t number_of_elements, FILE *a_file);
// int fseek(FILE *stream, long offset, int whence);

Elf32_Phdr *read_exec_file(FILE *opfile)
//读取elf文件, 返回progrem segment head头指针.
{
    char file_head_p[BUF_SIZE];
    fread(file_head_p, 1, BUF_SIZE, opfile);
    Elf32_Ehdr *ELF_head_p = (Elf32_Ehdr *)file_head_p;
    Elf32_Phdr *result = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
    memcpy(result, (void *)(file_head_p + ELF_head_p->e_phoff), sizeof(Elf32_Phdr));
    return result;
    //警告: 会产生未释放的内存空间, 需要在外面自行释放
}

uint8_t count_kernel_sectors(Elf32_Phdr *Phdr)
{
    printf("count_kernel_sectors() get Phdr->p_memsz %d\nmake", Phdr->p_memsz);
    return (uint8_t)((Phdr->p_memsz + SECTOR_SIZE - 1) / SECTOR_SIZE);
}

void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{
    fseek(file, phdr->p_offset, SEEK_SET); //从头开始搜索到程序段的位置
    fread(buffer, 1, phdr->p_memsz, file);
    fwrite(buffer, 1, phdr->p_memsz, image);
    puts("Bootblock write finished.");
}

void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *Phdr, int kernelsz)
{
    fseek(knfile, Phdr->p_offset, SEEK_SET); //从头开始搜索到程序段的位置
    fseek(image, SECTOR_SIZE, SEEK_SET);
    //按kernelsz进行写入操作
    uint32_t kernel_filesz=Phdr->p_filesz;
    uint32_t kernel_memsz=Phdr->p_memsz;
    uint32_t kernel_0sz=kernel_memsz-kernel_filesz;
    while (kernel_filesz-- > 0)
    {
        fread(buffer, 1, 1, knfile);
        fwrite(buffer, 1, 1, image);
        // fseek(knfile, SECTOR_SIZE, SEEK_CUR);
        // fseek(image, SECTOR_SIZE, SEEK_CUR);
    }
    buffer[0]=0;
    while (kernel_0sz-- > 0)
    {
        fwrite(buffer, 1, 1, image);
    }

    puts("Kernel write finished.");
}

void record_kernel_sectors(FILE *image, uint8_t kernelsz)
{
    fseek(image, SECTOR_SIZE - 2, SEEK_SET);
    fwrite(&kernelsz, 1, 1, image);
    puts("Kernel size recorded.");
}

void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz)
{
    puts("----------------------------------");
    
    puts("--------KERNEL--------");
    printf("number of kernel sectors: 0x%x(%d)\n", kernelsz, (int)kernelsz);
    printf("kernel image file size: 0x%x(%d)\n", Phdr_k->p_filesz,Phdr_k->p_filesz);
    printf("kernel image memory size: 0x%x(%d)\n", Phdr_k->p_memsz,Phdr_k->p_memsz);
    printf("kernel image memory offset: 0x%x\n", Phdr_k->p_offset);
    puts("--------BOOTBLOCK--------");
    printf("bootblock image file size: 0x%x(%d)\n", Phdr_bb->p_filesz,Phdr_bb->p_filesz);
    printf("bootblock image memory size: 0x%x\n", Phdr_bb->p_memsz);
    printf("bootblock image memory offset: 0x%x\n", Phdr_bb->p_offset);
}

int main(int argc, char* argv[], char* env[])
{
    printf("\n\nCreateimage by aw.\nCompiled in %s, %s.\nFilename:%s\n", __DATE__, __TIME__, __FILE__);
    FILE *bootblock_file = fopen("bootblock", "rb"); //binary read;
    FILE *kernel_file = fopen("main", "rb");       //binary read;
    FILE *image_file = fopen("image", "wb");         //bin write;

    //找到程序头
    Elf32_Phdr *segment_head_bootblock = read_exec_file(bootblock_file);
    Elf32_Phdr *segment_head_kernel = read_exec_file(kernel_file);

    //计算kernel占用的sector数
    uint8_t kernel_sectors = count_kernel_sectors(segment_head_kernel);

    //写镜像
    write_bootblock(image_file, bootblock_file, segment_head_bootblock);
    write_kernel(image_file, kernel_file, segment_head_kernel,kernel_sectors);
    record_kernel_sectors(image_file, kernel_sectors);

    //显示相关信息
    extent_opt(segment_head_bootblock, segment_head_kernel, kernel_sectors);

    //清理内存, 关闭文件缓冲;
    free(segment_head_bootblock);
    free(segment_head_kernel);

    fclose(image_file);
    fclose(kernel_file);
    fclose(bootblock_file);

    puts("Create image finished.");
    return 0;
}