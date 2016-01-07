
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



static int analyse_deadloop(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param);

T_AppExcAnalyser     g_DeadLoopAnalyser   = {
    EXCSTR_DEADLOOP_MODNAME,
    EXCSTR_DEADLOOP_MODDESC,
    analyse_deadloop
};

static int analyse_deadloop(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param)
{

    if( !rec || !callback )
        return E_EXC_INVALID;

    //  判断版本是否支持
    if( rec->version != EXCREC_VERSION0 )
        return 0;

    //  判断是否是死锁，本模块只支持分析死锁异常
    if( rec->errtype != APPETYP_DEADLOOP )
        return 0;


    callback(param,&g_DeadLoopAnalyser,0,exc_tr(EXCSTR_DEADLOOP_1));

    return 1;
}


