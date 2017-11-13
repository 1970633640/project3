#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mem.h"
int init=0;
int m_error;

typedef struct block
{
    int blank;
    int size;
} block;
void * head;
int region;

void mem_dump()
{
    void *current=head;
    block *current_piece;
    current_piece=(block *)current;
    while(current!=(head+region))
    {
        current_piece=(block *)current;
        int shift=current_piece->size;
        if(current_piece->blank)
        {
            printf("□ %d   Bytes a free piece begins at %p and ends at %p\n",shift,current,current+shift);
        }
        else
        {
            printf("■ %d   Bytes a non-free piece begins at %p and ends at %p\n",shift,current,current+shift);
        }
        current+=shift;
    }
    printf("-------------------------------------\n");
}


int mem_init(int size_of_region)
{
    //printf("%d %d",getpagesize(),sizeof(block));
    if (size_of_region <= 0 || init == 1)
    {
        m_error = E_BAD_ARGS;
        return -1;
    }
    int gps=getpagesize();
    if(size_of_region%gps!=0)
        size_of_region=(1+size_of_region/gps)*gps;
    int fd = open("/dev/zero", O_RDWR);
    // size_of_region (in bytes) needs to be evenly divisible by the page size
    void *ptr = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    // close the device (don't worry, mapping should be unaffected)
    close(fd);
    head=ptr;
    ((block*)head)->blank=1;
    ((block*)head)->size=size_of_region;
    region=size_of_region;
    //printf("%p %d",head,size_of_region);
    init=1;
    return 0;
}
void *mem_alloc(int size, int style)
{
    void * next;
    void * ans;
    next=head;
    ans=NULL;
    if(size%8!=0)size=(1+size/8)*8;
    size+=sizeof(block);
    if(style==M_BESTFIT)
    {
        while(next<=head+region)
        {
            if(((block*)next)->blank==1 &&((block*)next)->size>=size)
            {
                if(ans==NULL || ((block*)next)->size>((block*)ans)->size)
                    ans=next;
            }
            next+=((block*)next)->size;
        }
    }
    else if(style==M_WORSTFIT)
    {
        while(next<=head+region)
        {
            if(((block*)next)->blank==1 &&((block*)next)->size>=size)
            {
                if(ans==NULL || ((block*)next)->size>((block*)ans)->size)
                    ans=next;
            }
            next+=((block*)next)->size;
        }
    }
    else if(style==M_FIRSTFIT)
    {
        while(next<=head+region)
        {
            if(((block*)next)->blank==1 &&((block*)next)->size>=size)
            {
                ans=next;
                break;
            }
            next+=((block*)next)->size;
        }
    }
    if (ans == NULL)
    {
        m_error = E_NO_SPACE;
        return NULL;
    }
    if(((block*)ans)->size>size) //set next
    {
        ((block*)(ans+size))->blank=1;
        ((block*)(ans+size))->size=((block*)ans)->size-size;
    }
    ((block*)ans)->blank=0;
    ((block*)ans)->size=size;
    ans+=sizeof(block);
    return ans;
}
int mem_free(void* ptr)
{
    if (ptr == NULL)
    {
        return -1;
    }
    ptr-=sizeof(block);
    ((block*)ptr)->blank=1;
    void * current=head;
    while(current<head+region && current+((block*)current)->size<head+region)
    {
        void * next=current+((block*)current)->size;
        while(((block*)current)->blank==1 && ((block*)next)->blank==1 ) //if current.blank==1 AND next.blank==true
        {
            ((block*)current)->size+=((block*)next)->size; //current.size+=next.size
             next=current+((block*)current)->size;
        }
        current+=((block*)current)->size;
    }
}
int main()
{
    mem_init(1000);
    void* a=mem_alloc(20,M_FIRSTFIT);
    void* b=mem_alloc(20,M_FIRSTFIT);
    void* c=mem_alloc(20,M_FIRSTFIT);
    void* d=mem_alloc(20,M_FIRSTFIT);
    void* e=mem_alloc(20,M_FIRSTFIT);
    void* f=mem_alloc(20,M_FIRSTFIT);
    mem_dump();
    mem_free(c);
    mem_free(e);
    mem_dump();
    mem_free(d);
    mem_dump();
    mem_free(a);
    mem_free(b);
    mem_free(c);
    mem_free(f);
    mem_dump();
    return 0;
}
