#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_bootblock(FILE *image, FILE *bbfile, Elf32_Phdr *Phdr);
Elf32_Phdr *read_exec_file(FILE *opfile);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr);
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz);

Elf32_Phdr *read_exec_file(FILE *opfile)
{
    Elf32_Ehdr *elf;
    Elf32_Phdr *Phdr = (Elf32_Phdr *)calloc(1, sizeof(Elf32_Phdr));
    uint8_t buf[512];

    fread(buf, 1, 512, opfile);
    elf = (Elf32_Ehdr *)buf;
    memcpy(Phdr, (void *)elf + elf->e_phoff, 32);
    return Phdr;
}

uint8_t count_kernel_sectors(Elf32_Phdr *Phdr)
{
    uint8_t sector_num = (Phdr->p_memsz + 511) / 512;
    return sector_num;
}

void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{
    int i;
    uint8_t buf[512];
    memset(buf, 0, sizeof(buf));
    fseek(file, phdr->p_offset, SEEK_SET);
    for(i = 0; i < phdr->p_filesz / 512; i++)
    {
        fread(buf, 1, 512, file);
        fwrite(buf, 1, 512, image);    
    }
    if((phdr->p_filesz) % 512)
    {
        memset(buf, 0, sizeof(buf));
        fread(buf, 1, (phdr->p_filesz) % 512, file);
        fwrite(buf, 1, 512, image);
    }
    memset(buf, 0, sizeof(buf));
    for(; i < phdr->p_memsz / 512; i++)
        fwrite(buf, 1, 512, image);
    fwrite(buf, 1, (phdr->p_memsz) % 512, image);
        
}

void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *phdr, int kernelsz)
{
    int i;
    uint8_t buf[512];
    fseek(knfile, phdr->p_offset, SEEK_SET);
    for(i = 0; i < phdr->p_filesz / 512; i++)
    {
        fread(buf, 1, 512, knfile);
        fwrite(buf, 1, 512, image);    
    }
    if((phdr->p_filesz) % 512)
    {
        memset(buf, 0, sizeof(buf));
        fread(buf, 1, (phdr->p_filesz) % 512, knfile);
        fwrite(buf, 1, 512, image);
    }
    memset(buf, 0, sizeof(buf));
    for(; i < phdr->p_memsz / 512; i++)
        fwrite(buf, 1, 512, image);
    fwrite(buf, 1, (phdr->p_memsz) % 512, image);
}

void record_kernel_sectors(FILE *image, uint8_t kernelsz)
{
    uint8_t *buf = &kernelsz;
    fseek(image, 510, SEEK_SET);
    fwrite(buf, 1, 1, image);
}

void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz)
{
    if(Phdr_bb != NULL)
    {
        printf("bootblock size is %d byte!\n", Phdr_bb->p_filesz);
        printf("Bootblock message :\n");
        printf("Bootblock image memory size is 0x%x\n", Phdr_bb->p_memsz);
        printf("Bootblock image memory offset is 0x%x\n", Phdr_bb->p_offset);
    }
    if(Phdr_k != NULL)
    {
        printf("kernel message :\n");
        printf("kernel image memeory size is 0x%x\n", Phdr_k->p_memsz);
        printf("kernel image memory offset is 0x%x\n", Phdr_k->p_offset);
        printf("kernel sector size is 0x%x\n", kernelsz);
    }
}

int main(int argc, char *argv[])
{
    FILE *file = fopen("bootblock", "rb");
    assert(file);
    FILE *knfile = fopen("main", "rb");
    assert(knfile);
    FILE *image = fopen("image", "wb");

    Elf32_Phdr *Phdr_bb = read_exec_file(file);
    Elf32_Phdr *Phdr_k = read_exec_file(knfile);
    
    int kernelsz = (int)count_kernel_sectors(Phdr_k);

    write_bootblock(image, file, Phdr_bb);

    FILE *fp = image;
    fseek(fp, 512, SEEK_SET);
    write_kernel(fp, knfile, Phdr_k, kernelsz);

    fp = image;
    record_kernel_sectors(fp, kernelsz);
    if(argc < 2)
        Phdr_bb = Phdr_k = NULL;
    else if(argc == 2)
    {
        printf("Wrong arguments or too few arguments: \"%s\"\n", argv[1]);
        Phdr_bb = Phdr_k = NULL;
    }
    else if(argc == 3)
    {
        char *s = "--extended";
        while(*s == *(argv[1]) && *argv[1] && *s)
            argv[1]++, s++;
        if(*s || *(argv[1]))
        {
            printf("Please input correct arguments: --extended\n");
            Phdr_bb = Phdr_k = NULL;
        }
        else if(argv[2][0] == 'b')
        {
            s = "bootblock";
            while(*s == *(argv[2]) && *argv[2] && *s)
                argv[2]++, s++;
            if(*s || *(argv[2]))
            {
                printf("Please input correct arguments: \"bootblock\" or \"main\"\n");
                Phdr_bb = Phdr_k = NULL;
            }
            else
                Phdr_k = NULL;
        }
        else
        {
            s = "main";
            while(*s == *(argv[2]) && *argv[2] && *s)
                argv[2]++, s++;
            if(*s || *(argv[2]))
            {
                printf("Please input correct arguments: \"bootblock\" or \"main\"\n");
                Phdr_bb = Phdr_k = NULL;
            }
            else
                Phdr_bb = NULL;
        }
    }
    else if(argc == 4)
    {
        char *s = "--extended";
        while(*s == *(argv[1]) && *argv[1] && *s)
            argv[1]++, s++;
        if(*s || *(argv[1]))
        {
            printf("Please input correct arguments: --extended\n");
            Phdr_bb = Phdr_k = NULL;
        }
        else
        {
            int i, j;
            if(argv[2][0] == 'b')
                i = 2, j = 3;
            else
                i = 3, j = 2;
            s = "bootblock";
            while(*s == *(argv[i]) && *argv[i] && *s)
                argv[i]++, s++;
            if(*s || *(argv[i]))
            {
                printf("Please input correct arguments: \"bootblock\"\n");
                Phdr_bb = NULL;
            }

            s = "main";
            while(*s == *(argv[j]) && *argv[j] && *s)
                argv[j]++, s++;
            if(*s || *(argv[j]))
            {
                printf("Please input correct arguments: \"main\"\n");
                Phdr_k = NULL;
            }
        }
    }
    else
    {
        printf("Too many arguments\n");
        Phdr_bb = Phdr_k = NULL;   
    }
    extent_opt(Phdr_bb, Phdr_k, kernelsz);
    fclose(file);
    fclose(knfile);
    fclose(image);

    return 0;
}

