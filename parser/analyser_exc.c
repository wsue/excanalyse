
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


static int analyse_exc(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param);

T_AppExcAnalyser     g_ExcAnalyser   = {
    EXCSTR_EXC_MODNAME,
    EXCSTR_EXC_MODDESC,
    analyse_exc
};

static int analyse_exc(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param)
{
    if( !rec || !callback )
        return E_EXC_INVALID;

    //  判断版本是否支持
    if( rec->version != EXCREC_VERSION0 )
        return 0;

    //  判断是否是异常，只分析异常
    if( rec->errtype != APPETYP_EXC )
        return 0;

    callback(param,&g_ExcAnalyser,0,exc_tr(EXCSTR_EXC_1));
    if( rec->job.jno ){
        const T_ExcJobInfo   *pjob    = &rec->job;
        callback(param,&g_ExcAnalyser,0,
                exc_tr(EXCSTR_JOBINFO),
                pjob->name,pjob->jno);
    }
    else{
        const T_ExcTaskInfo   *ptask  = &rec->task;
        callback(param,&g_ExcAnalyser,0,
                exc_tr(EXCSTR_TSKINFO),
                ptask->name,ptask->tid,ptask->thrid);
    }
    
    callback(param,&g_ExcAnalyser,0,exc_tr(EXCSTR_EXC_2));
    return 0;
}

