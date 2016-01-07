

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "exc_parse.h"
#include "exc_error.h"
#include "exc_str.h"
#include "parser_comm.h"
#include "analyser.h"

static int analyse_baseinfo(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param);

T_AppExcAnalyser     g_NormalAnalyser   = {
    EXCSTR_BASEINFO_MODNAME,
    EXCSTR_BASEINFO_MODDESC,
    analyse_baseinfo,
};


static const char * taskmod2str(enum ETaskMode mod)
{
    switch( mod ){
        case TSKMODE_L1:
            return exc_tr(EXCSTR_TSKMOD_L1);
            break;

        case TSKMODE_L2:
            return exc_tr(EXCSTR_TSKMOD_L2);

        default:
            return exc_tr(EXCSTR_TSKMOD_OTHER);
    }
}


#define OUTPUT(fmt,arg...)  \
        callback(param,&g_NormalAnalyser,0,exc_tr(fmt),##arg)

static int analyse_baseinfo(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param)
{
    int             i;
    int             btnum;
    T_ExcFuncInfo   *btitem;
    char            buf[256];

    if( !rec || !callback )
        return E_EXC_INVALID;

    //  判断版本是否支持
    if( rec->version != EXCREC_VERSION0 )
        return 0;

    //  输出异常时间
    OUTPUT(EXCSTR_BASEINFO_ERRTIME,rec->errtime);
    

    //  输出异常原因
    OUTPUT(EXCSTR_BASEINFO_ERRTYPE,rec->signal.pc.pc,rec->signal.pc.funcname);
    switch( rec->errtype ){
        case APPETYP_DEADLOCK:
            if( rec->job.rundur )
                sprintf(buf,"%d",rec->job.rundur);
            OUTPUT(EXCSTR_BASEINFO_ERRTYPE_DEADLOCK,
                    rec->job.rundur ? buf : exc_tr(EXCSTR_UNKNOWN));
            break;

        case APPETYP_DEADLOOP:
            if( rec->job.rundur )
                sprintf(buf,"%d",rec->job.rundur);
            OUTPUT(EXCSTR_BASEINFO_ERRTYPE_DEADLOOP,
                    rec->job.rundur ? buf : exc_tr(EXCSTR_UNKNOWN));
            break;

        case APPETYP_EXC:
            switch( rec->signal.signum ){
                case SIGILL:
                    OUTPUT(EXCSTR_BASEINFO_ERRTYPE_SIGILL);
                    break;

                case SIGFPE:
                    OUTPUT(EXCSTR_BASEINFO_ERRTYPE_SIGFPE);
                    break;

                case SIGPIPE:
                    OUTPUT(EXCSTR_BASEINFO_ERRTYPE_SIGPIPE);
                    break;

                case SIGBUS:
                    OUTPUT(EXCSTR_BASEINFO_ERRTYPE_SIGBUS,rec->signal.addr);
                    break;

                case SIGSEGV:
                    OUTPUT(EXCSTR_BASEINFO_ERRTYPE_SIGSEGV,rec->signal.addr);
                    break;

                default:
                    OUTPUT(EXCSTR_BASEINFO_ERRTYPE_EXC,rec->signal.addr);
                    break;
            }
            break;

        default:
            OUTPUT(EXCSTR_BASEINFO_ERRTYPE_UNKNOWN);
            break;
    }

    //  输出进程/任务/JOB信息
    OUTPUT(EXCSTR_BASEINFO_PIDINFO,rec->process.name,rec->process.pid,rec->process.size);

    if( rec->task.name || rec->task.tid || rec->task.thrid ){
        OUTPUT(EXCSTR_BASEINFO_TASKINFO,rec->task.name,
                taskmod2str(rec->task.mode),
                rec->task.tid,rec->task.thrid);
    }

    if( rec->job.jno ){
        OUTPUT(EXCSTR_BASEINFO_JOB,rec->job.name,rec->job.jno,taskmod2str(rec->job.type),
               rec->job.stat,rec->job.msg.id); 
    }

    return 0;
}
#undef OUTPUT

