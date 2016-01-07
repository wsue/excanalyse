
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "exc_parse.h"
#include "exc_error.h"
#include "parser_comm.h"














#define MAGIC_HEAD          0xfafbfcfd
#define MAGIC_HEAD_FREE     0xabcdfcfd
#define MAGIC_END           0xCDEFfcfd
struct exc_malloc_h{
    unsigned int    magic;  
    unsigned long   bt[3];
    unsigned int    size;
};

struct exc_malloc_e{
    unsigned int    magic;
};

void *Exc_Malloc(size_t size)
{
    if( size <= 0 )
        return NULL;

    char    *p  = malloc(size 
            + sizeof(struct exc_malloc_h)
            + sizeof(struct exc_malloc_e));
    if( !p )
        return NULL;

    struct exc_malloc_h *ph = (struct exc_malloc_h *)p;
    p           += sizeof(struct exc_malloc_h);

    ph->magic   = MAGIC_HEAD;
    ph->size    = size;

    unsigned long   v   =(unsigned long)__builtin_return_address(1);
    ph->bt[0]   = v;
    ph->bt[1]   = 0;
    ph->bt[2]   = 0;

    //printf("malloc %d at %p %p\n",size,v,ph);
#if 0
    v   =__builtin_return_address(2);
    ph->bt[1]   = v;
    v   =__builtin_return_address(3);
    ph->bt[2]   = v;
#endif
    struct exc_malloc_e *pe = (struct exc_malloc_e *)
        ((char *)p +  + size );
    pe->magic   = MAGIC_END;
    return p;
}

void Exc_Free(void *ptr)
{
    if( !ptr )
        return ;

    char    *p              = (char *)ptr;
    struct exc_malloc_h *ph = (struct exc_malloc_h *)(p - sizeof(struct exc_malloc_h));
    if( ph->magic !=MAGIC_HEAD ){
        fprintf(stderr,"%lx(%d) wrong magic head %x bt:%lx %lx %lx\n",
                ph,ph->size,ph->magic,ph->bt[0],ph->bt[1],ph->bt[2]);
        return ;
    }

    struct exc_malloc_e *pe = (struct exc_malloc_e *)(p + ph->size);
    if( pe->magic != MAGIC_END ){
        fprintf(stderr,"%lx(%d) wrong magic end bt:%lx %lx %lx\n",ph,ph->size,ph->bt[0],ph->bt[1],ph->bt[2]);
        return ;
    }

    ph->magic   = MAGIC_HEAD_FREE;
    free(ph);
}

char *Exc_StrDup(const char *str)
{
    if( !str )
        return NULL;

    int len = strlen(str) + 1;
    char    *p  = Exc_Malloc(len);
    if( !p )
        return NULL;
    strcpy(p,str);
    return p;
}
