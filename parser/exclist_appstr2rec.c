

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "exc_parse.h"
#include "exc_error.h"
#include "exc_str.h"
#include "parser_comm.h"
#include "exclist.h"

#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)

typedef int (*CONV_FUNC)(char *line,char *param,int param_size);
struct lineinfo{
    const char  *key;       /*  关键字      */
    CONV_FUNC   func;
    int         offset;
    int         size;
};


struct parse_op{
    const char  *keyword;
    int (*func)(T_AppExcRec *rec,char *pstr,const struct parse_op *op);
    void *param;
};


static inline int set_int_val(char *out,int size,unsigned int val)
{
    switch( size ){
        case    1: *(unsigned char *)out        = val; break;
        case    2: *(unsigned short *)out       = val; break;
        case    4: *(unsigned int *)out         = val; break;
        default:    return -1;
    }

    return 0;
}


/** 
 * @brief 输入"1, Shelf:2, Slot:21, Cpu:1"形式，输出具体位置
 * 
 * @param pos 
 * @param str 
 */
static void posstr2val(char *str,int *pos)
{
    pos[0]  = atoi(str);
    char    *p  = strstr(str,"Shelf:");
    if( p ){
        pos[1]  = atoi(p+strlen("Shelf:"));
    }
    p           = strstr(str,"Slot:");
    if( p ){
        pos[2]  = atoi(p+strlen("Slot:"));
    }
    p           = strstr(str,"Cpu:");
    if( p ){
        pos[3]  = atoi(p+strlen("Cpu:"));
    }
}

static void copystr(char *str,char *val,int size)
{
    if( str ){
        while( isspace(*str) )  str++;
        strncpy(val,str,size-1);
    }
}

static void str2int(char *str,char *val,int size)
{
    assert(set_int_val(val,size,atoi(str)) == 0 );
}

static void str2ll(char *str,char *val,int size)
{
    if( size == 8 ){
            *(unsigned long long *)val   = strtoull(str,NULL,0);
    }
    else{
        char    *p  = NULL;
        unsigned int    v = strtoul(str,&p,0);
        assert( set_int_val(val,size,v) == 0 );
    }
}

static void parse_pc(char *str,T_AppExcRec *rec,int size)
{
    rec->signal.pc.pc   = strtoull(str,NULL,0);
    /*
    char *p   = strstr(str,"ebp[");
    if( p ){
        rec->signal.stack.sp   = strtoull(strchr(p,'[')+1,NULL,0);
    }
    */
}

static void parse_task(char *str,T_ExcTaskInfo *task,int size)
{
    task->thrid = strtoull(str,NULL,0);
    char *p   = strchr(str,'(');
    if( p ){
        task->tid   = atoi(p+1);
        char    *pend   = strchr(p,')');
        if( pend ){
            *pend       = 0;
            p           = strchr(p,',');
            if(p)
                strncpy(task->name,p+1,sizeof(task->name)-1);
        }
    }
}

/** 
 * @brief 处理: Find dead lock in thread 0x488ec4c0(SCHE10_1)!
 * 
 * @param str 
 * @param stack 
 * @param size 
 */
static void parse_task2(char *str,T_ExcTaskInfo *task,int size)
{
    task->thrid = strtoull(str,NULL,0);
    char *p   = strchr(str,'(');
    if( p ){
        char    *pend   = strchr(p,')');
        if( pend )
            *pend       = 0;
        strncpy(task->name,p+1,sizeof(task->name)-1);
    }
}

static void parse_stack(char *str,T_ExcStackInfo *stack,int size)
{
    dasm_addr_t    base    = strtoull(str,NULL,0);
    char    *p  = strstr(str,"guardsize:");
    if( p  ){
        int guard   = atoi(strchr(p,':')+1);
        stack->base   = base + guard;

        if( p   = strstr(str,"task stacksize:") ){
            int size    = atoi(strchr(p,':')+1);
            stack->bottom =base + size;
        }
    }
}

static void sigstr2val(char *str,char *val,int size)
{
    int     i           = 0;
    const char    *signame[]  = {"",
        "SIGHUP",        "SIGINT",        "SIGQUIT",       "SIGILL",
        "SIGTRAP",       "SIGABRT",       "SIGBUS",        "SIGFPE",
        "SIGKILL",      "SIGUSR1",      "SIGSEGV",      "SIGUSR2",
        "SIGPIPE",      "SIGALRM",      "SIGTERM", "",     "SIGCHLD",
        "SIGCONT",      "SIGSTOP",      "SIGTSTP",      "SIGTTIN",
        "SIGTTOU",      "SIGURG",       "SIGXCPU",      "SIGXFSZ",
        "SIGVTALRM",    "SIGPROF",      "SIGWINCH",     "SIGIO",
        "SIGPWR",       "SIGSYS",   "","",    "SIGRTMIN",    NULL};

    if( str && *str ){
        for( i = 0; signame[i] ; i++ ){
            if( *signame[i] && strstr(str,signame[i]) ){
                assert(set_int_val(val,size,i) == 0 );
            }
        }
    }
    /*
       1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL
       5) SIGTRAP      6) SIGABRT      7) SIGBUS       8) SIGFPE
       9) SIGKILL     10) SIGUSR1     11) SIGSEGV     12) SIGUSR2
       13) SIGPIPE     14) SIGALRM     15) SIGTERM     17) SIGCHLD
       18) SIGCONT     19) SIGSTOP     20) SIGTSTP     21) SIGTTIN
       22) SIGTTOU     23) SIGURG      24) SIGXCPU     25) SIGXFSZ
       26) SIGVTALRM   27) SIGPROF     28) SIGWINCH    29) SIGIO
       30) SIGPWR      31) SIGSYS      34) SIGRTMIN    
       64) SIGRTMAX
       */
}

static void taskstr2mode(char *str,char *val,int size)
{
    int mode    = TSKMODE_UNKNOWN;
    if( strstr(str,"L1") ){
        mode    = TSKMODE_L1;
    }
    else if( strstr(str,"L2") ){
        mode    = TSKMODE_L2;
    }
    else{
        return ;
    }

    assert(set_int_val(val,size,mode) == 0 );
}


static void line2pcinfo(char *line,T_ExcFuncInfo *info)
{
    while(isspace(*line) )  line++;
    info->pc                 = strtoull(line,NULL,0);

    if( strstr(line,"sym find failed") )
        return ;

    while( isalnum(*line) )    line++;
    while( isspace(*line) )    line++;

    if( !line )
        return ;

    char    *p2         = strchr(line,'(');
    if( p2 ){
        *p2++           = 0;
    }
    strncpy(info->funcname,line,sizeof(info->funcname)-1);
    if( !strcmp(info->funcname,"PC") )
        info->funcname[0]  = 0;

    if( p2 ){
        line               = strchr(p2,')');
        if( line )         *line  = 0;
        strncpy(info->soname,p2,sizeof(info->soname)-1);
    }
}

/** 
 * @brief 得到所有以0x开始的行,行前充许有空格
 *      对于异常，充许行以failed to find name for 开始
 * 
 * @param strs  [out]   输出，每一行都以0x开始
 * @param count [in]
 * @param pstr  [in]
 * 
 * @return 
 */
static int getlines_0xhead(char *strs[],int count,char *pstr,int isexc)
{
    int i   = 0;
    for( ; i < count && pstr ; ){
        char    *p          = strchr(pstr,'\n');
        char    *line       = pstr;
        pstr                = p;
        if( pstr ) *pstr++  = 0;

        while(isspace(*line) )  line++;
        if( line[0] != '0' || line[1] != 'x' ){
            if( !isexc 
                    || strncmp(line,"failed to find name for",strlen("failed to find name for")) )
                continue;

            line    = strstr(line,"0x");
            if( !line )
                continue;
        }
        strs[i++]   = line;
    }

    return i;
}


static T_AppExcRec        s_tmprec;   //  仅用于初始化s_baseinf

#define VALINFO(x)          (int)offsetof(T_AppExcRec,x),sizeof(s_tmprec.x)

static struct lineinfo  s_baseinfo[]   = {
    {"User: Rack:",(CONV_FUNC)posstr2val,VALINFO(pos)},
    {"Used Time:",(CONV_FUNC)copystr,VALINFO(errtime)},
    {"This is an exception signal:",(CONV_FUNC)str2int,VALINFO(signal.signum)},
    {"registers: pc[",(CONV_FUNC)parse_pc,0,sizeof(s_tmprec)},
    {"Save context of task",(CONV_FUNC)parse_task,VALINFO(task)},
    {"stackbase: ",(CONV_FUNC)parse_stack,VALINFO(signal.stack)},
    {"SIGNAL:",(CONV_FUNC)sigstr2val,VALINFO(signal.signum)},
    {"ErrorAddr =",(CONV_FUNC)str2ll,VALINFO(signal.addr)},
    {"Error Address:",(CONV_FUNC)str2ll,VALINFO(signal.addr)},

    {"Find dead lock in thread ",(CONV_FUNC)parse_task2,VALINFO(task)},
    {"task id   :",(CONV_FUNC)str2ll,VALINFO(task.thrid)},
    {"task name :",(CONV_FUNC)copystr,VALINFO(task.name)},
    {"task tid :",(CONV_FUNC)str2ll,VALINFO(task.tid)},

    {"Job Name  :",(CONV_FUNC)copystr,VALINFO(job.name)},
    {"Job Type  :",(CONV_FUNC)str2int,VALINFO(job.type)},
    {"Job ID    :",(CONV_FUNC)str2ll,VALINFO(job.jno)},
    {"Job state :",(CONV_FUNC)str2int,VALINFO(job.stat)},
    {"Msg ID    :",(CONV_FUNC)str2ll,VALINFO(job.msg.id)},
    {"Headver   :",(CONV_FUNC)str2ll,VALINFO(job.msg.hver)},
    {"AppVer    :",(CONV_FUNC)str2ll,VALINFO(job.msg.appver)},
    {"Msg type  :",(CONV_FUNC)str2ll,VALINFO(job.msg.type)},
    {NULL,NULL,-1,-1}
};


static struct lineinfo  s_posinfo[] = {
    {"Rack:",(CONV_FUNC)posstr2val,VALINFO(pos)},
    {NULL,NULL,-1,-1}
};

static struct lineinfo  s_procinfo[] = {
    {"Proc Name:",(CONV_FUNC)copystr,VALINFO(process.name)},
    {"gpid     :",(CONV_FUNC)str2int,VALINFO(process.pid)},
    {NULL,NULL,-1,-1}
};

static struct lineinfo  s_taskinfo[] = {
    {"The task is a",(CONV_FUNC)taskstr2mode,VALINFO(task.mode)},
    {"Task Name   :",(CONV_FUNC)copystr,VALINFO(task.name)},
    {"Thread id   :",(CONV_FUNC)str2ll,VALINFO(task.thrid)},
    {"Taskid(LWP) :",(CONV_FUNC)str2int,VALINFO(task.tid)},
    {NULL,NULL,-1,-1}
};

static struct lineinfo  s_jobinfo[] = {
    {"Job name       :",(CONV_FUNC)copystr,VALINFO(job.name)},
    {"Job SvrId      :",(CONV_FUNC)str2ll,VALINFO(job.jno)},
    {"Job Run status :",(CONV_FUNC)str2int,VALINFO(job.runstat)},
    {"Job MsgId      :",(CONV_FUNC)str2ll,VALINFO(job.msg.id)},
    {"Job state      :",(CONV_FUNC)str2int,VALINFO(job.stat)},
    {NULL,NULL,-1,-1}
};

static struct lineinfo  s_curexcinfo[] = {
    {"pc   =",(CONV_FUNC)line2pcinfo,VALINFO(signal.pc)},
    {NULL,NULL,-1,-1}
};



/** 
 * @brief 使用输入的lineinfo分析一段内容
 * 
 * @param rec   [out]   输出内容
 * @param pstr  [in]    待分析的段
 * 
 * @return 
 */
static int phase2val_line(T_AppExcRec *rec,char *pstr,const struct parse_op *op)
{

    while( pstr ){
        char    *p      = strchr(pstr,'\n');
        char    *line   = pstr;
        struct lineinfo  *pinf   = (struct lineinfo  *)op->param;

        if( p ) *p++    = 0;
        pstr            = p;

        while( pinf->key && !(p = strstr(line,pinf->key)) ) pinf++;

        if( pinf->key ){
            p   += strlen(pinf->key);
            pinf->func(p,(char *)rec + pinf->offset,pinf->size);
        }
    }

    return 0;
}

/** 
 * @brief 分析寄存器信息段内容
 *      这段内容一般以----------------Current Exception Context Start--------------
 *      开始
 * 
 * @param rec 
 * @param pstr 
 * 
 * @return 
 */
static int phase2val_reg(T_AppExcRec *rec,char *pstr,const struct parse_op *op)
{
    char    *p  = strchr(pstr,'\n');
    if(p){
        rec->signal.regstr  = Exc_StrDup(p+1);
    }
    return 0;
}

#define TRY_GET_VAL(val,key)        do{ \
    p  =strstr(pstr,key);   if( p ) p       = strstr(p,"0x");   \
    if( !p )    return 0;   \
    val      = strtoull(p,NULL,0);}while(0)
static int phase2val_stack(T_AppExcRec *rec,char *pstr,const struct parse_op *op)
{
    T_ExcStackInfo  *pstack = NULL;
    char    *p              = pstr;

    if( op->param == (void *)2 ){
        p  = strstr(pstr,"Printing ");
        if( !p )
            return 0;

        char        *pend   = strchr(p,'\n');
        if( pend )  *pend++ = 0;

        p           += strlen("Printing ");
        if( !strncmp(p,"task",4) ){
            pstack  = &rec->task.stack;
        }
        else if ( !strncmp(p,"job",3) ){
            pstack  = &rec->job.stack;
        }
        else{
            return 0;
        }

        p   = strstr(p,"stack from 0x");
        if( !p )
            return 0;
        pstack->bottom  = strtoull(strchr(p,'0'),NULL,0);

        p   = strstr(p,"to 0x");
        if( !p )
            return 0;
        pstack->base    = strtoull(strchr(p,'0'),NULL,0);
        p               = pend;
    }
    else if( op->param == (void *)0 || op->param == (void *)1 ){
        pstack      = op->param == 0 ? &rec->task.stack : &rec->job.stack;
        TRY_GET_VAL(pstack->sp,"stack top");
        TRY_GET_VAL(pstack->base,"stack base");
        TRY_GET_VAL(pstack->bottom,"stack bottom");
        p   = strchr(p,'\n');
        if( p ) p++;
    }
    else
        return 0;

    if( p )
        pstack->stackstr    = Exc_StrDup(p+1);

    return 0;
}
#undef TRY_GET_VAL




/** 
 * @brief 分析进程使用的动态库信息,这些信息以
 *      ----------------ProcSelf SoInfo Start------------------------
 *      开始
 * 
 * @param rec 
 * @param pstr 
 * 
 * @return 
 */
static int phase2val_soinfo(T_AppExcRec *rec,char *pstr,const struct parse_op *op)
{
    pstr                = strchr(pstr,'\n');
    if( !pstr )
        return 0;
    
    char    *lines[64];
    int     count       = getlines_0xhead(lines,64,pstr,0);
    if( count <= 0 )
        return 0;

    rec->process.libs   = Exc_Malloc(count * sizeof(T_ExcLibInfo));
    if( rec->process.libs ){
        T_ExcLibInfo    *item   = rec->process.libs;
        int i           = 0;

        memset(item,0,(count * sizeof(T_ExcLibInfo)));

        for( ; i < count ; i++,item++ ){
            item->baseaddr  = strtoull(lines[i],NULL,0);

            char    *p  = lines[i];
            while( *p && (*p!= ' ' && *p != '\t' ))   p++;
            
            while( *p && ( *p == ' ' || *p == '\t') ) p++;
            if( p ){
                char    *pend   = p+1;
                while( *pend && ( *pend != ' ' && *pend != '\t') ) pend++;
                *pend++         = 0;

                strncpy(item->name,p,sizeof(item->name)-1);
                item->filesize  = atoi(pend);
            }
        }

        rec->process.libnum = count;
    }
    return 0;
}





/** 
 * @brief 分析调用链信息，这些信息以
 *      ----------------Function Calling Trace Start-----------------
 *      开始，对于死锁/死循环，会以
 *      Track function call list...
 *      开始
 * 
 * @param rec 
 * @param pstr 
 * 
 * @return 
 */
static int phase2val_bt(T_AppExcRec *rec,char *pstr,const struct parse_op *op)
{
    char    *p      = NULL;
    char    *funcs[64];

    if( rec->errtype == APPETYP_DEADLOCK 
            || rec->errtype == APPETYP_DEADLOOP ){
        pstr           = strstr(pstr,"Track function call list...");
    }
    else{
        pstr           = strstr(pstr,"Function Calling Trace Start");
    }

    if( !pstr )
        return 0;

    int count           = getlines_0xhead(funcs,64,pstr,1);
    if( count <= 0 )
        return 0;

    rec->btfunc         = Exc_Malloc(count * sizeof(T_ExcFuncInfo));
    if( rec->btfunc ){
        rec->btfuncnum  = count;
        int     i       = 0;
        T_ExcFuncInfo   *pitem  = rec->btfunc;
        memset(pitem,0,(count * sizeof(T_ExcFuncInfo)));

        for( ; i < count ; i ++,pitem++ ){
            line2pcinfo(funcs[i],pitem);
        }
    }

    return 0;
}






static const struct parse_op  s_parseop[]    = {
    {"BASE INFO PARSE",             phase2val_line,     s_baseinfo},
    {"Exception Registers Start",   phase2val_reg,      NULL},
    {"Board Position start",        phase2val_line,     s_posinfo},
    {"Information of Proc Start",   phase2val_line,     s_procinfo},
    {"Information of Task Start",   phase2val_line,     s_taskinfo},
    {"Information of job start",    phase2val_line,     s_jobinfo},
    {"Job Stack Info Start",        NULL,               NULL},  //  当前不处理
    {"Current Exception Context Start",phase2val_line,  s_curexcinfo},
    {"Task Stack start",            phase2val_stack,    0},
    {"Job Stack start",             phase2val_stack,    (void*)1},
    {"ProcSelf SoInfo Start",       phase2val_soinfo,   NULL},
    {"Function Calling Trace Start",phase2val_bt,       NULL},
    {"Current Message Start",       NULL,               NULL},
    {"Code Segment Start",          NULL,               NULL},
    //  死锁死循环时特殊段
    {" stack from ",                phase2val_stack,    (void *)2},
    {"Track function call list...", phase2val_bt,       NULL},
    {NULL,NULL}
};

static int phases2val(T_AppExcRec *rec,char    *phases[],int count)
{
    int i   = 0;
    if( !phases || count < 1 )
        return E_EXC_NOTFOUND;
    rec->version        = EXCREC_VERSION0;

    s_parseop[0].func(rec,phases[0],&s_parseop[0]);

    for( i  = 1; i < count ; i ++ ){
        const struct parse_op   *iter   = s_parseop +1;
        while( iter->keyword && !strstr(phases[i],iter->keyword) )   iter ++;
        if( iter->keyword && iter->func ){
            iter->func(rec,phases[i],iter);
        }
    }

    T_ExcStackInfo  *pstack = NULL;
    if( rec->task.stack.stackstr ){
        pstack              = &rec->task.stack;
    }
    else if( rec->job.stack.stackstr ){
        pstack              = &rec->job.stack;
    }

    if( rec->signal.regstr ){
        dasm_addr_t    sp  = 0;
        char    *p          = strstr(rec->signal.regstr,"SP");
        if( p ){
            p   += 2;
            while( *p && isspace(*p) )   p++;
            if( p[0] == '=')
                sp          = strtoull(p+1,NULL,0);
        }

        if( pstack && pstack->sp == 0 )
            pstack->sp      = sp;
        if( rec->signal.stack.sp == 0 )
            rec->signal.stack.sp = sp;
    }

    return 0;
}










static int get_exctype(const char *phase, const char **begin )
{
    int         type    = APPETYP_UNKNOWN;
    const char  *p      = strstr(phase,TOKEN_DEADLOOP_STR);

    if( begin )
        *begin  = phase;

    if(  p ){
        type    = APPETYP_DEADLOOP;
    }
    else if( (p = strstr(phase,TOKEN_DEADLOCK_STR)) ){
        type    = APPETYP_DEADLOCK;
    }
    else if( (p = strstr(phase,TOKEN_APPEXC_STR)) ){
        type    = APPETYP_EXC;
    }
    else
        return APPETYP_UNKNOWN;

    if( begin ){
        const char *start   = p;
        p                   = phase;
        while( p = strstr(p, TOKEN_APPEXCEND_SECTION) ){
            p   = strchr(p,'\n');
            if( !p || p > start )
                break;

            *begin      = p;
        }
    }

    return type;
}

/** 
 * @brief 把异常信息分成一段一段的(根据异常信息中的'-------'分隔符)
 * 
 * @param type      [out]       具体异常类型
 * @param phases    [out]       保存分段结果
 * @param count     [in]        phases数组个数
 * @param pstr      [in]        待分段的字符串
 * 
 * @return 0:   分段失败，pstr没有异常信息或里面的异常信息不认识
 *          >0  phases数组元素个数
 */
static int str2phases(enum EAppErrType *type,
        char *phases[],int count,char *pstr,char **rest)
{
    int         i       = 0;

    *rest   = NULL;
    *type == APPETYP_UNKNOWN;
    if( !phases || count <1 || !pstr || !*pstr){
        return 0;
    }

    //  记录格式应该如:
    //  段1
    //  ----    start-----
    //  段2
    //  ----    end-----
    //  ...
    //  ----    start---
    //  段n
    //  ----    end---
    //  除了第一个段外，其它每个段都应该由 --- start 与 --- end 包括
    //
    //  在死锁或死循环时,
    //  Exception Registers End段会包括栈与回调函数
    //  分解时应该把这两个段分出来

    //  1.  把 ----前的内容放到一个段
    phases[0]   = pstr;
    char *p     = strstr(pstr,"---------");
    if( !p ){
        if( (*type = get_exctype(phases[0],(const char **)&phases[0])) != APPETYP_UNKNOWN ){
            return 1;
        }
        else
            return 0;
    }

    if( p != pstr ){
        //  文件字符串一开始就以------开始，那在for循环中进行处理
        pstr    = p-1;
        *pstr++ = 0;

        if( (*type = get_exctype(phases[0],(const char **)&phases[0])) != APPETYP_UNKNOWN ){
            i       = 1;
        }
    }

    for( ; i < count && pstr ; ){
        char        c   = 0;
        //  每个循环开始，第一行都是以---------开始
        phases[i]   = pstr;
        pstr        = strchr(pstr,'\n');

        if( pstr ){
            pstr   = strstr(pstr,"---------");  //  让pstr指向下一个---------，以进行下一个循环判断
            if( pstr ){
                c           = pstr[-1];
                pstr[-1]    = 0;
            }
        }


        if( i == 0 ){
            //  如果i == 0 则一直查找起始段(表示这是什么异常)并保存在phases[0]中
            if( (*type = get_exctype(phases[0],(const char **)&phases[0])) != APPETYP_UNKNOWN ){
                i   ++;
            }

            continue;
        }

        if( get_exctype(phases[i],NULL) != APPETYP_UNKNOWN ){
            //  如果这属于下一个异常，则停止继续分解,同时忽略本段
            *rest   = phases[i];
            if( c )
                pstr[-1]    = c;
            break;
        }

        //  或这段包括异常结束符，那也停止继续分析
        p   = strstr(phases[i],"***************");
        if( p ){
            p[0]    = 0;
            *rest   = p+1;
            pstr    = NULL;
        }

        if( (*type == APPETYP_DEADLOCK
                    || *type == APPETYP_DEADLOOP)
                && (p  = strstr(phases[i],"-------Exception Registers End")) ){
            //  判断这段是否包括---------Exception Registers End关键字
            //  如果包括，那需要把栈与调用链分开
            if( p  = strstr(p,"Track function call list...") ){
                phases[++i] = p;
                p[-1]   = 0;
            }
        }

        i   ++;
    }
    
    return i;
}

/** 
 *
 * @brief 查找起始位置
 * 
 * @param rec   [out]   记录起始信息
 * @param pstr  [in]    待分析字符串
 * @param rest  [out]   未分析的字符串
 * 
 * @return 
 */
int excstr2apprec(T_AppExcRec *rec,char *pstr, char **rest)
{
    char    *p          = NULL;
    int     i           = 0;
    char    *phases[256];
    int     retstrlen   = 0;

    *rest               = NULL;
    memset(phases,0,sizeof(phases));


    int count   = str2phases(&rec->errtype,
            phases,256,pstr,rest);

    if( count < 1 || rec->errtype == APPETYP_UNKNOWN )
        return E_EXC_NOTFOUND;

    for( i = 0 ; i < count ; i ++ )
        retstrlen       += strlen(phases[i]) + 1;

    rec->excstr         = Exc_Malloc(retstrlen + 1);
    if( rec->excstr ){
        char    *p  = rec->excstr;
        for( i  = 0; i < count ; i ++ ){
            int len = strlen(phases[i]);
            memcpy(p,phases[i],len);
            p       += len;
            *p++    =  '\n';
        }

        *p          = 0;
    }

    int err = phases2val(rec,phases,count);
    if( err )
        return err;

    str_trim(rec->errtime);
    str_trim(rec->process.name);
    return 0;
}

#if 0
static void dump_stack(const T_ExcStackInfo *stack){
    if( stack ){
        printf("stack:[base:" F64X " bottom:" F64X " sp:" F64X " str:%s]\n",
                stack->base,stack->bottom,stack->sp,stack->stackstr);
    }
}
static void dump_rec(const T_ExcRec *rec)
{
    int     i   = 0;
    printf("exc rec ver:%d err:%d time:%s pos:%d,%d,%d,%d\n"
            "signal:%d addr:" F64X " pc:" F64X "(%s:%s) reg:%s\n",
            rec->version,rec->errtype,rec->errtime,
            rec->pos[0],rec->pos[1],rec->pos[2],rec->pos[3],
            rec->signal.signum,rec->signal.addr,
            rec->signal.pc.pc,rec->signal.pc.funcname,rec->signal.pc.soname,
            rec->signal.regstr);

    dump_stack(&rec->signal.stack);

    printf("\nproc:%s pid:%d size:%d libnum:%d libs:",
            rec->process.name,rec->process.pid,rec->process.size,rec->process.libnum);

    T_ExcLibInfo    *plib   = rec->process.libs;
    for( i = 0 ; i < rec->process.libnum ; i ++,plib++ ){
        printf("\tlib[%d]:%s baseaddr:" F64X " size:%d\n",
                i,plib->name,plib->baseaddr,plib->filesize);
    }

    printf("\ntask:%s tid:%d thrid:" F64X " mod:%d ",
            rec->task.name,rec->task.tid,rec->task.thrid,rec->task.mode);
    dump_stack(&rec->task.stack);

    printf("\njob:%s<0x%08x> typ:%d rstat:%d stat:%d msgid:%d rundur:%d ",
            rec->job.name,rec->job.jno,rec->job.type,rec->job.runstat,rec->job.stat,
            rec->job.msg.id,rec->job.rundur);
    dump_stack(&rec->job.stack);
    printf("\njob msg type:%d hver:%d appver:%d :%s\n",
            rec->job.msg.type,rec->job.msg.hver,rec->job.msg.appver,
            rec->job.msg.str
            );

    printf("\nbacktrace func num:%d\n",rec->btfuncnum);
    T_ExcFuncInfo   *btfunc = rec->btfunc;
    for( i = 0; i < rec->btfuncnum; i ++, btfunc++){
        printf("\t func[%d]:pc " F64X " <%s:%s>\n",
                i,btfunc->pc,btfunc->funcname,btfunc->soname);
    }
    

    printf("lockstr:%s\n",rec->lockstr);
    printf("tipcstr:%s\n",rec->tipcstr);
    printf("\n\norig exc str:%s\n",rec->excstr);
}
#endif
