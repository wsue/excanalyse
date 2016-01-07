


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>

#include "dumpwrap.h"
#include "exc_parse.h"
#include "exc_error.h"
#include "exc_str.h"
#include "parser_comm.h"
#include "dasm.h"





#define DISASM_STR_MAX    (320*1024)
struct funcs_ctl{
    char            cache[DISASM_STR_MAX];
    int             len;
    T_DasmItem *cur;                           //  当前输出记录指针
    T_DasmInfo *info;                        //  所有等输出的记录
};

static unsigned int funcs_dump(void *param,const char *buf,int len)
{
    struct funcs_ctl   *funcctl   = ( struct funcs_ctl   *)param;
    if( funcctl->cur ){
        int rest    = sizeof(funcctl->cache) - funcctl->len -1;
        if( rest > len )
            rest    = len;

        if( rest > 0 ){
            memcpy(funcctl->cache + funcctl->len,buf,rest);
            funcctl->len += rest;
        }
    }

    return len;
}

static unsigned int funcs_errdump(void *param,const char *fmt,...)
{
    va_list ap;    
    va_start (ap, fmt);
    int len = vfprintf(stderr,fmt,ap);
    va_end(ap);
    return len;
}



static void funcs_endfunc(struct funcs_ctl *funcctl)
{
    if( funcctl->cur ){
        Exc_Free(funcctl->cur->dasmstr);
        if(funcctl->len <= 0 ) 
            funcctl->cur->dasmstr    = NULL;
        else{
            funcctl->cur->dasmstr    = Exc_Malloc(funcctl->len+1);
            if( funcctl->cur->dasmstr ){
                memcpy(funcctl->cur->dasmstr,funcctl->cache,funcctl->len );
                funcctl->cur->dasmstr[funcctl->len] = 0;
            }
        }

        funcctl->cur       = NULL;
        funcctl->len       = 0;
    }
}

static int func_startfunc(struct funcs_ctl  *funcctl,
        const char *fname,dasm_addr_t vma_offset,
        const char *funcname,
        dasm_addr_t    code_start,
        dasm_addr_t    code_end)
{
    int i   = 0;
    T_DasmItem         *item   = funcctl->info->items;
    funcs_endfunc(funcctl);

    for( ; i < funcctl->info->num ; i ++,item++){
        int ok  = 0;
        if( item->funcaddr ){
            continue;
        }

        if( item->pc  
                && (item->pc < code_start + vma_offset
                    || item->pc >= code_end + vma_offset )
          ){        //  如果pc不在范围
            continue;
        }

        if( funcname 
                && item->funcname[0] 
                && strncmp(item->funcname,funcname,sizeof(item->funcname)-1) ){
            //  如果有函数名，但是函数名对应不上
            continue;
        }


        if( funcname ){
            strncpy(item->funcname,funcname,sizeof(item->funcname)-1);
        }
        char    *p          = strrchr(fname,'/');
        if( p )
            strncpy(item->elfname,p+1,sizeof(item->elfname)-1);
        item->funcaddr      = code_start;
        item->elf_offset    = vma_offset;

        funcctl->cur        = item;
        funcctl->len        = 0;

        return item->pc ? item->pc - vma_offset :code_start;
    }

    return 0;
} 


static dasm_addr_t    funcs_actcheck(void *param,
        enum EDumpWrapAct newact,
        const char *fname,dasm_addr_t elfoffset,
        const char *funcname,
        dasm_addr_t    code_start,
        dasm_addr_t    code_end)
{
    struct funcs_ctl   *funcctl   = ( struct funcs_ctl   *)param;
    switch( newact ){
        case EDumpWrapFuncStart:
            return func_startfunc(funcctl,
                    fname,elfoffset,
                    funcname,
                    code_start,
                    code_end);

        case EDumpWrapFuncEnd:
        case EDumpWrapEnd:
            funcs_endfunc(funcctl);
            break;

        default:
            break;
    }

    return 0;
}



static int Exc_Dasm(
        T_DasmInfo * info,const char *elffile,dasm_addr_t elfoffset,int fsize)
{

}

static int DasmAElf(
        T_DasmInfo * info,const char *path,
        const char *libname,
        int libsize,
        dasm_addr_t elfoffset)
{
    struct  funcs_ctl       fctl;
    char    fullpath[256];
    if( !path || !*path || !libname || !*libname )
        return E_EXC_INVALID;

    snprintf(fullpath,sizeof(fullpath),"%s/%s",path,libname);
    fullpath[255]   = 0;


    memset(&fctl,0,sizeof(fctl));
    fctl.info           = info;

    return dumpwrap_load(fullpath,libsize,elfoffset,
            funcs_dump,funcs_errdump,funcs_actcheck,
            &fctl);
}

/** 
 * @brief 根据rec信息创建异常调用链的反汇编信息
 * 
 * @param rec       [in]    异常记录
 * @param elffile   [in]    异常记录对应的进程的elf文件所在路径
 * @param libpath   [in]    异常记录中使用的共享库所在路径
 * 
 * @return 反汇编调用链
 */
int Dasm_GetFromRec(T_DasmInfo **pinf,
        const T_AppExcRec *rec,const char *libpath)
{
    int i   = 0;
    int size;
    T_DasmInfo     *dasms  = NULL;
    T_DasmItem     *item;
    T_ExcFuncInfo       *btitem;

    if( pinf )
        *pinf   = NULL;

    if( !pinf || !rec )
        return E_EXC_INVALID;

    if( rec->btfuncnum <= 0 
            || !libpath || !*libpath )
        return 0;

    if( !rec->btfunc )
        return E_EXC_INVALID;

    size    = sizeof(T_DasmInfo) 
        + sizeof(T_DasmItem) * (1+rec->btfuncnum);
    dasms   = Exc_Malloc(size);
    if( !dasms )
        return E_EXC_NOMEM;

    memset(dasms,0,size);
    dasms->num  = rec->btfuncnum +1;
    btitem      = rec->btfunc;
    item        = dasms->items;
    item->pc    = rec->signal.pc.pc;        
    item++;
    for( i = 0; i < rec->btfuncnum ; i ++,item++,btitem++ ){
        item->pc    = btitem->pc;        
    }

    DasmAElf(dasms,libpath,
            rec->process.name,rec->process.size,0);

    if( libpath ){
        T_ExcLibInfo    *lib    = rec->process.libs;
        for( i = 0; i < rec->process.libnum ; i ++ , lib++ ){
            DasmAElf(dasms,libpath,
                    lib->name,
                    lib->filesize,
                    lib->baseaddr);
        }
    }

    *pinf   = dasms;
    return 0;
}


void Dasm_Release(T_DasmInfo * info)
{
    if( info ){
        int i   = 0;
        T_DasmItem *item = info->items;
        for( ; i < info->num ; i ++, item++){
            Exc_Free(item->dasmstr);
        }

        Exc_Free(info);
    }
}






struct dasm_op_ctl{
    FILE    *fp;
    union {
        const char      *name;
        dasm_addr_t    addr;
    };
};

static unsigned int dasm_op_dump(void *param,const char *buf,int len)
{
    struct dasm_op_ctl  *pctl   = (struct dasm_op_ctl *)param;
    fwrite(buf,len,1,pctl->fp);
    return len;
}

static unsigned int dasm_op_errdump(void *param,const char *fmt,...)
{
    struct dasm_op_ctl  *pctl   = (struct dasm_op_ctl *)param;    
    va_list ap;    
    va_start (ap, fmt);
    int len = vfprintf(pctl->fp,fmt,ap);
    va_end(ap);
    return len;
}

static dasm_addr_t dasm_all_act(void *param,
        enum EDumpWrapAct newact,
        const char *fname,dasm_addr_t elfoffset,
        const char *funcname,
        dasm_addr_t    code_start,
        dasm_addr_t    code_end)
{
    return 1;
}


static dasm_addr_t dasm_funclist_act(void *param,
        enum EDumpWrapAct newact,
        const char *fname,dasm_addr_t elfoffset,
        const char *funcname,
        dasm_addr_t    code_start,
        dasm_addr_t    code_end)
{
    if( newact == EDumpWrapFuncStart ){
        struct dasm_op_ctl  *pctl   = (struct dasm_op_ctl *)param;    
        fprintf(pctl->fp,F64X " - " F64X "    %s\n",code_start,code_end,funcname);
    }

    return 0;
}

static dasm_addr_t dasm_funcmatchlist_act(void *param,
        enum EDumpWrapAct newact,
        const char *fname,dasm_addr_t elfoffset,
        const char *funcname,
        dasm_addr_t    code_start,
        dasm_addr_t    code_end)
{
    if( newact == EDumpWrapFuncStart ){
        struct dasm_op_ctl  *pctl   = (struct dasm_op_ctl *)param;
        if( funcname && strstr(funcname,pctl->name) )
            fprintf(pctl->fp,F64X " - " F64X "    %s\n",code_start,code_end,funcname);
    }

    return 0;
}

static dasm_addr_t dasm_funcmatch_act(void *param,
        enum EDumpWrapAct newact,
        const char *fname,dasm_addr_t elfoffset,
        const char *funcname,
        dasm_addr_t    code_start,
        dasm_addr_t    code_end)
{
    if( newact == EDumpWrapFuncStart ){
        struct dasm_op_ctl  *pctl   = (struct dasm_op_ctl *)param;
        if( funcname && strstr(funcname,pctl->name) )
            return 1;
    }

    return 0;
}

static dasm_addr_t dasm_func_act(void *param,
        enum EDumpWrapAct newact,
        const char *fname,dasm_addr_t elfoffset,
        const char *funcname,
        dasm_addr_t    code_start,
        dasm_addr_t    code_end)
{
    if( newact == EDumpWrapFuncStart ){
        struct dasm_op_ctl  *pctl   = (struct dasm_op_ctl *)param;
        if( funcname && !strcmp(funcname,pctl->name) )
            return 1;
    }

    return 0;
}

static dasm_addr_t dasm_addr_act(void *param,
        enum EDumpWrapAct newact,
        const char *fname,dasm_addr_t elfoffset,
        const char *funcname,
        dasm_addr_t    code_start,
        dasm_addr_t    code_end)
{
    if( newact == EDumpWrapFuncStart ){
        struct dasm_op_ctl  *pctl   = (struct dasm_op_ctl *)param;
        if( pctl->addr >= code_start  && pctl->addr < code_end )
            return 1;
    }

    return 0;
}


int Exc_Dasm(FILE *fout,
        const char *elffile,enum EExcDasmMode mode,const char *param)
{
    int         ret = E_EXC_INVALID;
    struct dasm_op_ctl  ctl;
    if( !fout || !elffile || !*elffile )
        return E_EXC_INVALID;

    memset(&ctl,0,sizeof(ctl));
    ctl.fp      = fout;

    switch(mode){
        case EExcDasmAll:
            dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_DASM_ALL));

            ret = dumpwrap_load(elffile,0,0,
                    dasm_op_dump,dasm_op_errdump,dasm_all_act,
                    &ctl);
            
            break;

        case EExcDasmFuncList:
            dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_FUNCLIST));
            ret = dumpwrap_load(elffile,0,0,
                    NULL,dasm_op_errdump,dasm_funclist_act,
                    &ctl);
            break;

        case EExcDasmFuncMatchList:
            if( !param || !*param ){
                dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_NEED_STRING_PARAM));
                return E_EXC_INVALID;
            }

            ctl.name        = param;
            dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_LIST_ALL_FUNCMATCH),param);

            ret = dumpwrap_load(elffile,0,0,
                    NULL,dasm_op_errdump,dasm_funcmatchlist_act,
                    &ctl);
            break;

        case EExcDasmFuncMatch:
            if( !param || !*param ){
                dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_NEED_STRING_PARAM));
                return E_EXC_INVALID;
            }


            ctl.name        = param;
            dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_DASM_ALL_FUNCMATCH),param);


            ret = dumpwrap_load(elffile,0,0,
                    dasm_op_dump,dasm_op_errdump,dasm_funcmatch_act,
                    &ctl);
            break;

        case EExcDasmFunc:
            if( !param || !*param ){
                dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_NEED_STRING_PARAM));
                return E_EXC_INVALID;
            }

            ctl.name        = param;
            dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_DASM_FUNC),param);

            ret = dumpwrap_load(elffile,0,0,
                    dasm_op_dump,dasm_op_errdump,dasm_func_act,
                    &ctl);
            break;

        case EExcDasmAddr:
            if( !param || !*param ){
                dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_NEED_ADDR_PARAM));
                return E_EXC_INVALID;
            }

            ctl.addr    = strtoull(param,NULL,0);
            dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_DASM_ADDR),ctl.addr);
            ret = dumpwrap_load(elffile,0,0,
                    dasm_op_dump,dasm_op_errdump,dasm_addr_act,
                    &ctl);
            break;

        default:
            return E_EXC_INVALID;
            break;
    }

    dasm_op_errdump((void *)&ctl,exc_tr(EXCSTR_DASM_END),ret);
    
    return 0;
}
