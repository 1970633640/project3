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

void mem_dump() {
	void *temp = head;
    printf("head=%lld region=%lld\n",head,region);
    while (temp < head+region) {
		if (((block*)temp)->blank==1) {
			printf("free: %lld ~ %lld (%lld)\n", temp, temp + ((block*)temp)->size,((block*)temp)->size);
		}
		temp += ((block*)temp)->size;
	}
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
    void * temp;
    void * ans;
    temp=head;
    ans=NULL;
    if(size%8!=0)size=(1+size/8)*8;
    size+=sizeof(block);
    if(style==M_BESTFIT)
    {
        while(temp<=head+region)
        {
            if(((block*)temp)->blank==1 &&((block*)temp)->size>=size)
            {
                if(ans==NULL || ((block*)temp)->size>((block*)ans)->size)
                    ans=temp;
            }
            temp+=((block*)temp)->size;
        }
    }
    else if(style==M_WORSTFIT)
    {
        while(temp<=head+region)
        {
            if(((block*)temp)->blank==1 &&((block*)temp)->size>=size)
            {
                if(ans==NULL || ((block*)temp)->size>((block*)ans)->size)
                    ans=temp;
            }
            temp+=((block*)temp)->size;
        }
    }
    else if(style==M_FIRSTFIT)
    {
        while(temp<=head+region)
        {
            if(((block*)temp)->blank==1 &&((block*)temp)->size>=size)
            {
                ans=temp;
                break;
            }
            temp+=((block*)temp)->size;
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
int main()
{
    mem_init(1000);
    mem_dump();
    return 0;
}
