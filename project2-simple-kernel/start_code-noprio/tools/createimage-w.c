#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define buffercount 2048

int extend = 0;
const int sector_size = 512;
uint8_t buffer_bb[buffercount], buffer_k[buffercount];

void write_bootblock(FILE* image, FILE* bbfile, Elf32_Phdr* Phdr);
Elf32_Phdr* read_exec_file(FILE* opfile, uint8_t* buffer);
uint8_t count_kernel_sectors(Elf32_Phdr* Phdr);
void write_kernel(FILE* image, FILE* knfile, Elf32_Phdr* Phdr, int kernelsz);
void extent_opt(Elf32_Phdr* Phdr_bb, Elf32_Phdr* Phdr_k, int kernelsz);

Elf32_Phdr* read_exec_file(FILE* opfile, uint8_t* buffer)
{
    fread(buffer, 1, buffercount, opfile);
    Elf32_Ehdr* ehdr = (void*)buffer;
    return (Elf32_Phdr*)((void*)buffer + ehdr->e_phoff);
}

uint8_t count_kernel_sectors(Elf32_Phdr* Phdr)
{
    int memsz = Phdr->p_memsz;
    uint8_t counter = 0;
    while (memsz > 0)
    {
        counter++;
        memsz -= sector_size;
    }
    return counter;
}

void write_bootblock(FILE* image, FILE* bbfile, Elf32_Phdr* Phdr)
{
    uint8_t data[sector_size];
    memset(data, 0, sizeof(data));
    fseek(bbfile, Phdr->p_offset, SEEK_SET);
    fread(data, 1, Phdr->p_memsz, bbfile); 
    data[510] = 0x55;
    data[511] = 0xaa;
    fwrite(data, 1, sector_size, image);
    fflush(image);
    return;
}

void write_kernel(FILE* image, FILE* knfile, Elf32_Phdr* Phdr, int kernelsz)
{
    uint8_t data[sector_size*kernelsz];
    memset(data, 0, sizeof(data));
    fseek(knfile, Phdr->p_offset, SEEK_SET);
    fread(data, 1, Phdr->p_memsz, knfile);
    fseek(image, 0, SEEK_END);
    fwrite(data, 1, sector_size*kernelsz, image);
    fflush(image);
    return;
}

void record_kernel_sectors(FILE* image, uint8_t kernelsz)
{
    uint8_t data[1];
    data[0] = kernelsz;
    fseek(image, -515, SEEK_END);
    fwrite(data, 1, 1, image);
    fflush(image);
    return;
}

void extent_opt(Elf32_Phdr* Phdr_bb, Elf32_Phdr* Phdr_k, int kernelsz)
{
    printf("bootblock size is %d byte!\n", Phdr_bb->p_memsz);
    printf("Bootblock messager :\n");
    printf("Bootblock image memory size is 0x%x\n", Phdr_bb->p_memsz);
    printf("Bootblock image memory offset is 0x%x\n", Phdr_bb->p_offset);
    printf("kernel messager :\n");
    printf("kernel image memory size is 0x%x\n", Phdr_k->p_memsz);
    printf("kernel image memory offset is 0x%x\n", Phdr_k->p_offset);
    printf("kernel sector size is 0x%x\n", kernelsz);
}

int main(int argc, char* argv[]) 
{
    if ((argc > 1) && (strcmp(argv[1], "--extended") == 0)) 
        extend = 1;

    FILE* image_file = fopen("image", "wb");
    if (image_file == NULL) 
    {
        printf("Cannot create image file!\n");
        return 1;
    }

    const int bootoffset = (extend == 0) ? 1 : 2;
    const int kerneloffset = (extend == 0) ? 2 : 3;

    // boot file 
    FILE* boot_file = fopen(argv[bootoffset], "rb");
    if (boot_file == NULL)
    {
        printf("There is no boot file!\n");
        return 1;
    }

    Elf32_Phdr* boot_Phdr;
    boot_Phdr = read_exec_file(boot_file, buffer_bb);
    write_bootblock(image_file, boot_file, boot_Phdr);
    fclose(boot_file);


    // kernel file
    FILE* kernel_file = fopen(argv[kerneloffset], "rb");
    if (kernel_file == NULL)
    {
        printf("There is no kernel file!\n");
        return 1;
    }

    Elf32_Phdr* kernel_Phdr;
    kernel_Phdr = read_exec_file(kernel_file, buffer_k);
    uint8_t kernel_sectors;
    kernel_sectors = count_kernel_sectors(kernel_Phdr);
    write_kernel(image_file, kernel_file, kernel_Phdr, kernel_sectors);
    fclose(kernel_file);

    record_kernel_sectors(image_file, kernel_sectors);
    if (extend == 1)
        extent_opt(boot_Phdr, kernel_Phdr, kernel_sectors);
    fflush(image_file);
    fclose(image_file);
    return 0;
}
