#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLEN 1000

void write_bootblock(FILE *image, FILE *bbfile, Elf32_Phdr *Phdr);
Elf32_Phdr *read_exec_file(FILE *opfile);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr);
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz);

Elf32_Phdr *read_exec_file(FILE *opfile)
{ 
    Elf32_Ehdr *eheader;
    Elf32_Phdr *pheader;
    eheader = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    fread(eheader,sizeof(Elf32_Ehdr),1,opfile);
    pheader = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
    fseek(opfile,eheader->e_phoff,SEEK_SET);
    fread(pheader,sizeof(Elf32_Phdr),1,opfile);
    return pheader;
}

uint8_t count_kernel_sectors(Elf32_Phdr *Phdr)
{
    return ((uint8_t)((Phdr->p_filesz/512)+1));
}

void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{
    uint8_t buffer[512];
	memset(buffer, 0, sizeof(buffer));
    fseek(file,phdr->p_offset,SEEK_SET);
    fwrite (buffer,phdr->p_filesz,1,image);
    fseek(file,phdr->p_offset,SEEK_SET);
    fread  (buffer,phdr->p_filesz,1,file);
    fwrite (buffer,phdr->p_filesz,1,image);
}

void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *Phdr, int kernelsz)
{
    uint8_t buffer[512];
	memset(buffer, 0, sizeof(buffer));    
	int i;
    fseek(knfile,Phdr->p_offset,SEEK_SET);
    for (i=0; i<kernelsz-1; i++)
    {
        fread  (buffer,512,1,knfile);
        fwrite (buffer,512,1,image);
    }
	memset(buffer, 0, sizeof(buffer));    
    fread  (buffer,(Phdr->p_filesz%512),1,knfile);
    fwrite (buffer,512,1,image);
	memset(buffer, 0, sizeof(buffer)); 
	for(i++; i <= (Phdr->p_memsz+511)/512; i++)
    {
        fwrite (buffer,512,1,image);
    }
}

void record_kernel_sectors(FILE *image, uint8_t kernelsz)
{
    uint8_t* p_kernelsz=&kernelsz;
    fseek   (image,510,SEEK_SET);
    fwrite(p_kernelsz,1,1,image);
}

void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz)
{
    if (Phdr_bb!=NULL){
        printf("bootblock file size is %d bytes\n",Phdr_bb->p_filesz);
        printf("bootblock messager:\n");
        printf("bootblock image memory size is 0x%x\n",Phdr_bb->p_memsz);
        printf("bootblock image memory offset is 0x%x\n",Phdr_bb->p_offset); 
    }
    if(Phdr_k!=NULL){
        printf("kernel messager:\n"); 
        printf("kernel image memory size is 0x%x\n", Phdr_k->p_memsz);
        printf("kernel image memory offset is 0x%x\n", Phdr_k->p_offset);
        printf("number of kernel image sectors is %d\n", kernelsz);
    }
    return;
}

int main(int argc, char* argv[])
{
    FILE * p2bootblock;
    FILE * p2kernel;
    FILE * p2image;
    int kernelsz;
    Elf32_Phdr * bbpheader;
    Elf32_Phdr * kernelpheader;
    p2bootblock     = (FILE *) fopen ("bootblock","rb");
    p2kernel        = (FILE *) fopen ("main","rb");
    p2image         = (FILE *) fopen ("image","wb" );
    bbpheader       = read_exec_file(p2bootblock); 
    write_bootblock (p2image, p2bootblock,bbpheader);
    kernelpheader   = read_exec_file(p2kernel);
    kernelsz        = count_kernel_sectors(kernelpheader);
    
  
    record_kernel_sectors (p2image, kernelsz);
    fseek                 (p2image,512,SEEK_SET);
    write_kernel          (p2image,p2kernel,kernelpheader, kernelsz);
    char ex[]="--extended";
    char bb[]="bootblock";
    char k[]="main";
    int  cmpres_a1_ex;//result: argv[1] compare with "--extended"
    int  cmpres_a2_bb;//result: argv[2] compare with "bootblock"
    int  cmpres_a2_k;//result: argv[2] compare with "kernel"
    int  cmpres_a3_k;//result: argv[3] compare with "kernel"
    int  cmpres_a3_bb;
    if (argc==1)
        printf("no print options\n");
    else if (argc>=2){
        cmpres_a1_ex = strcmp(argv[1],ex);
        if (cmpres_a1_ex!=0){
            printf("wrong command\n");
            return 0;
        }
        else{  
            if (argc==2){
                extent_opt(bbpheader, kernelpheader, kernelsz);
                return 0; 
            }

            cmpres_a2_bb=strcmp(argv[2],bb);
            cmpres_a2_k=strcmp(argv[2],k);
            if (argc==3){ 
                if (cmpres_a2_bb==0)
                    extent_opt(bbpheader, NULL, 0); 
                else if (cmpres_a2_k==0)
                    extent_opt(NULL, kernelpheader, kernelsz); 
                else {
                    printf("wrong option format\n");
                    return 0;
                }
                return 0;
            }
            cmpres_a3_k=strcmp(argv[3],k); 
            cmpres_a3_bb=strcmp(argv[3],bb); 
            if (argc==4)
            {
                if (cmpres_a2_bb==0&&cmpres_a3_k==0||cmpres_a2_k==0&&cmpres_a3_bb==0)
                    extent_opt(bbpheader, kernelpheader, kernelsz);
                else{
                   printf("wrong option format\n");
                   return 0;
                } 
            }
            else{
                printf("too many parameters\n");
                return 0;
            }
        }
        
    }
    fclose (p2bootblock);
    fclose (p2kernel);
    fclose (p2image);
    return 0;
}
