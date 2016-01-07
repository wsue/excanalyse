
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <stdarg.h>

#include "exc_parse.h"
#include "exc_error.h"
#include "exc_str.h"
#include "parser_comm.h"
#include "exclist.h"
#include "dasm.h"
#include "analyser.h"


enum    EFileId{
    EBaseFile   = 0,
    EParseFile,
    EStackFile,
    EPCFile,
    EBt0File,
};


static const char *errtype2str(const T_AppExcRec *rec)
{
    switch(rec->errtype ){
        case APPETYP_DEADLOCK:
            return exc_tr(EXCSTR_DEADLOCK);

        case APPETYP_DEADLOOP:
            return exc_tr(EXCSTR_DEADLOOP);

        case APPETYP_EXC:
            switch( rec->signal.signum ){
                case SIGILL:
                    return exc_tr(EXCSTR_EXC_SIGILL);
                    break;

                case SIGFPE:
                    return exc_tr(EXCSTR_EXC_SIGFPE);
                    break;

                case SIGPIPE:
                    return exc_tr(EXCSTR_EXC_SIGPIPE);
                    break;

                case SIGBUS:
                    return exc_tr(EXCSTR_EXC_SIGBUS);
                    break;

                case SIGSEGV:
                    return exc_tr(EXCSTR_EXC_SIGSEV);
                    break;

                default:
                    break;
            }

        default:
            break;
    }
    return exc_tr(EXCSTR_EXC_OTHER);
} 

static int item_setbase(struct exclist_ctl *ctl,struct ExcItemInfo *info,
        const T_AppExcRec *rec,const char *libpath)
{
    int     i   = 0;
    exclist_fmt_path(info->path,sizeof(info->path),ctl,"base");
    snprintf(info->title,sizeof(info->title)-1,exc_tr(EXCSTR_PARSER_BASE_TITLE));

    FILE    *fp = fopen(info->path,"w");
    if( !fp )
        return E_EXC_FOPEN;

    fprintf(fp,exc_tr(EXCSTR_PARSER_BASE_INFO_START),
            rec->errtime,
            rec->pos[0],rec->pos[1],rec->pos[2],rec->pos[3],
            errtype2str(rec),
            rec->signal.pc.pc,
            rec->signal.pc.funcname,
            rec->signal.addr,
            rec->process.name,rec->process.pid
           );

    if( rec->task.name[0] || rec->task.tid || rec->task.thrid ){
        fprintf(fp,exc_tr(EXCSTR_TSKINFO "\n"),
                rec->task.name,rec->task.tid,rec->task.thrid);
    }

    if( rec->job.jno ){
        fprintf(fp,exc_tr(EXCSTR_JOBINFO " " EXCSTR_MSGID "\n"),
                rec->job.name,rec->job.jno,rec->job.msg.id);
    }

    fprintf(fp,exc_tr("\n"EXCSTR_PARSER_BASE_INFO_BT));
    for( ; i < rec->btfuncnum ; i ++ ){
        fprintf(fp, "\t" F64X "(%s:%s)\n",
                rec->btfunc[i].pc,
                rec->btfunc[i].funcname,
                rec->btfunc[i].soname);
    }

    fprintf(fp,"\n");

    fprintf(fp,exc_tr(EXCSTR_PARSER_BASE_INFO_END),
            libpath? libpath:"",rec->excstr);
    fclose(fp);
    return 0;
}


struct analyser_ctl{
    T_AppExcAnalyser    *last;      //  最后一个分析模块
    int                 num;        //  分析模块个数
    FILE                *fp;        //  文件指针
};

static void analyser_callback(void *param,
        T_AppExcAnalyser *analyser,
        int err,const char *fmt,...)
{
    struct analyser_ctl    *pinf    = (struct analyser_ctl *)param;
    if( analyser != pinf->last){
        fprintf(pinf->fp,exc_tr(EXCSTR_PARSER_PARSE_STR4),analyser->name);
        pinf->last   = analyser;
        pinf->num ++;
    }

    fprintf(pinf->fp,"\t");
    va_list ap;
    va_start (ap, fmt);
    vfprintf(pinf->fp,fmt,ap);
    va_end(ap);
}

static int item_setanalyse(struct exclist_ctl *ctl,struct ExcItemInfo *info,
        const T_AppExcRec *rec)
{
    exclist_fmt_path(info->path,sizeof(info->path),ctl,"parse");
    snprintf(info->title,sizeof(info->title)-1,exc_tr(EXCSTR_PARSER_PARSE_TITLE));

    struct analyser_ctl    analyser;

    FILE    *fp = fopen(info->path,"w");
    if( !fp )
        return E_EXC_FOPEN;

    fprintf(fp,exc_tr(EXCSTR_PARSER_PARSE_STR1));

    memset(&analyser,0,sizeof(analyser));
    analyser.fp     = fp;

    int err = Analyser_Analyse(rec,analyser_callback,&analyser);
    if( err >= 0 ){
        fprintf(fp,exc_tr(EXCSTR_PARSER_PARSE_STR2),analyser.num);
        err = 0;
    }
    else{
        fprintf(fp,exc_tr(EXCSTR_PARSER_PARSE_STR3),err);
    }

    fclose(fp);
    return err;
}

static int item_setstack(struct exclist_ctl *ctl,struct ExcItemInfo *info,
        const T_AppExcRec *rec)
{
    const T_ExcStackInfo  *pstack = NULL;

    exclist_fmt_path(info->path,sizeof(info->path),ctl,"stack");
    snprintf(info->title,sizeof(info->title)-1,exc_tr(EXCSTR_PARSER_STACK_TITLE));

    FILE    *fp = fopen(info->path,"w");
    if( !fp )
        return E_EXC_FOPEN;

    if( rec->job.stack.stackstr ){
        fprintf(fp,exc_tr(EXCSTR_JOBINFO),
                rec->job.name,rec->job.jno);
        pstack  = &rec->job.stack;
    }
    else if( rec->task.stack.stackstr ){
        fprintf(fp,exc_tr(EXCSTR_TSKINFO),
                rec->task.name,rec->task.tid,rec->task.thrid);

        pstack  = &rec->task.stack;
    }
    else{
        fprintf(fp,exc_tr(EXCSTR_PARSER_STACK_STR1));
    }

    if( pstack ){
        fprintf(fp,exc_tr(EXCSTR_PARSER_STACK_INFO),
                pstack->base,pstack->bottom,pstack->sp,pstack->stackstr);
    }

    fclose(fp);
    return 0;
}


static int set_a_dasm(struct exclist_ctl *ctl,struct ExcItemInfo *info,int index,
        const char *regstr,
        const T_DasmItem *dasmitem,const T_ExcFuncInfo *func)
{
    char    name[32];

    if( dasmitem ){
        assert(dasmitem->pc == func->pc);
    }

    if( index == -1 ){
        strcpy(name,"PC");
    }
    else{
        sprintf(name,"%d",index );
    }

    exclist_fmt_path(info->path,sizeof(info->path),ctl,name);

    if( !func->funcname[0] ){
        snprintf(info->title,sizeof(info->title)-1,
                "%s " F64X ,name,func->pc);
    }
    else if( !func->soname[0] ){
        snprintf(info->title,sizeof(info->title)-1,
                "%s %s(" F64X ")",name,func->funcname,func->pc);
    }
    else{
        snprintf(info->title,sizeof(info->title)-1,
                "%s %s(" F64X ":%s)",name,func->funcname,func->pc,func->soname);
    }


    FILE    *fp = fopen(info->path,"w");
    if( !fp )
        return E_EXC_FOPEN;

    fprintf(fp,exc_tr(EXCSTR_PARSER_DASM_STR1),func->pc,dasmitem ? dasmitem->pc -dasmitem->elf_offset :-1 );

    fprintf(fp,exc_tr(EXCSTR_PARSER_DASM_STR2),func->soname,func->funcname);

    if( dasmitem ){
        fprintf(fp,exc_tr(EXCSTR_PARSER_DASM_STR3),dasmitem->funcaddr,dasmitem->funcname,dasmitem->elfname,dasmitem->elf_offset);
    }

    if( !dasmitem || (func->funcname[0] && strcmp(func->funcname,dasmitem->funcname))
            || (func->soname[0] && strcmp(func->soname,dasmitem->elfname)) ){
        fprintf(fp,exc_tr(EXCSTR_PARSER_DASM_STR6));
    }

    if( regstr && *regstr)
        fprintf(fp,exc_tr(EXCSTR_PARSER_DASM_STR7),regstr);

    if( dasmitem ){
        fprintf(fp,exc_tr(EXCSTR_PARSER_DASM_STR4),dasmitem->dasmstr ? dasmitem->dasmstr : exc_tr(EXCSTR_PARSER_DASM_STR5));
    }

    fclose(fp);
    return 0;
}


static int item_setdasms(struct exclist_ctl *ctl,struct ExcItemInfo *info,
        const T_AppExcRec *rec, const char *libpath)
{
    int             i           = 0;
    T_DasmInfo *dasminfo   = NULL;
    T_DasmItem *dasmitem   = NULL;
    T_ExcFuncInfo   *btitem     = NULL;

    int err = Dasm_GetFromRec(&dasminfo,rec,libpath);
    if( err )
        goto err_end;

    dasmitem            = dasminfo ? dasminfo->items : NULL;
    btitem              = rec->btfunc;
    err = set_a_dasm(ctl,&info[0],-1,rec->signal.regstr,dasmitem,&rec->signal.pc);
    if( err )
        goto err_end;

    if( rec->btfuncnum == 0 )
        goto err_end;


    if( !dasmitem 
            || (dasmitem->pc == dasmitem[1].pc 
                && !dasmitem[1].funcaddr ) 
      ){
        err = set_a_dasm(ctl,&info[1],0,NULL,dasmitem,btitem);
    }
    else{
        err = set_a_dasm(ctl,&info[1],0,NULL,dasmitem+1,btitem);
    }
    if( err )
        goto err_end;

    btitem  ++;
    if( dasmitem )
        dasmitem    += 2;

    for( i = 1; i < rec->btfuncnum; i ++,btitem++ ){
        err = set_a_dasm(ctl,&info[1+i],i,NULL,dasmitem,btitem);
        if( err )
            goto err_end;

        if( dasmitem ) 
            dasmitem++;
    }

err_end:
    Dasm_Release( dasminfo);

    return err;
}


int apprec_push2list(struct exclist_ctl *ctl,
        const T_AppExcRec *rec, const char *libpath)
{
    char    desc[128];
    int     size    = sizeof(struct ExcListItem) 
        + (EPCFile +1 + rec->btfuncnum) * sizeof(struct ExcItemInfo);
    struct ExcListItem *item   = Exc_Malloc(size);
    if( !item ){
        return E_EXC_NOMEM;
    }

    memset(item,0,size);
    item->num           = EPCFile + 1 + rec->btfuncnum;
    snprintf(desc,sizeof(desc)-1,"%d-%d-%d-%d %s %s",
            rec->pos[0],rec->pos[1],rec->pos[2],rec->pos[3],
            errtype2str(rec),rec->process.name);

    int    err = item_setbase(ctl,&item->info[EBaseFile],rec,libpath);
    if( err ){
        Exc_Free(item);
        return err;
    }
    err = item_setanalyse(ctl,&item->info[EParseFile],rec);
    if( err ){
        Exc_Free(item);
        return err;
    }

    err = item_setstack(ctl,&item->info[EStackFile],rec);
    if( err ){
        Exc_Free(item);
        return err;
    }

    err = item_setdasms(ctl,&item->info[EPCFile],rec,libpath);
    if( err ){
        Exc_Free(item);
        return err;
    }

    err = exclist_push_item(ctl,NULL,rec->errtime,desc,item);
    if( err )
        Exc_Free(item);
    return err;
}


