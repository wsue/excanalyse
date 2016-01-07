
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


static int analyse_func(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param);

T_AppExcAnalyser     g_StackOverflowAnalyser   = {
    EXCSTR_STACKOVERFLOW_MODNAME,
    EXCSTR_STACKOVERFLOW_MODDESC,
    analyse_func
};

static int analyse_func(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param)
{
    const T_ExcStackInfo  *pstack = NULL;
    if( !rec || !callback )
        return E_EXC_INVALID;

    //  判断版本是否支持
    if( rec->version != EXCREC_VERSION0 )
        return 0;

    //  判断是否是异常，栈越界会引起异常
    if( rec->errtype != APPETYP_EXC )
        return 0;

    if( rec->task.stack.sp != 0 
            && rec->task.stack.base != 0 
            && rec->task.stack.sp <= rec->task.stack.base ){
        const T_ExcTaskInfo   *ptask  = &rec->task;
        pstack                  = &ptask->stack;
        callback(param,&g_StackOverflowAnalyser,0,
                exc_tr(EXCSTR_TSKINFO),
                ptask->name,ptask->tid,ptask->thrid);
    } 
    else if( rec->job.jno    != 0
            && rec->job.stack.sp != 0 
            && rec->job.stack.base != 0 
            && rec->job.stack.sp <= rec->job.stack.base ){
        const T_ExcJobInfo   *pjob    = &rec->job;
        pstack                  = &pjob->stack;
        callback(param,&g_StackOverflowAnalyser,0,
                exc_tr(EXCSTR_JOBINFO),
                pjob->name,pjob->jno);
    } 
    else
        return 0;

    callback(param,&g_StackOverflowAnalyser,0,
            exc_tr(EXCSTR_STACKOVERFLOW_1),
            pstack->sp,pstack->base);
    return 1;
}
