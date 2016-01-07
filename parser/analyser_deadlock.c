
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


static int analyse_deadlock(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param);

T_AppExcAnalyser     g_DeadLockAnalyser   = {
    EXCSTR_DEADLOCK_MODNAME,
    EXCSTR_DEADLOCK_MODDESC,
    analyse_deadlock
};

static int analyse_deadlock(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param)
{
    int             i;
    int             btnum;
    T_ExcFuncInfo   *btitem;

    if( !rec || !callback )
        return E_EXC_INVALID;

    //  判断版本是否支持
    if( rec->version != EXCREC_VERSION0 )
        return 0;

    //  判断是否是死锁，本模块只支持分析死锁异常
    if( rec->errtype != APPETYP_DEADLOCK )
        return 0;

    callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_0));
    //  判断是否有调用链
    //  如果没有只能推测
    if( rec->btfuncnum <1 ){
        if( rec->lockstr ){
            //  如果有锁记录，
            //  那死锁原因可能是其它线程长时间占用锁引起
            callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_1),rec->lockstr,exc_tr(EXCSTR_UNKNOWN));
        }
        else{
            //  未知的死锁原因，请联系支持
            callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_2));
        }

        //  死锁不需要其它模块再分析,都在本模块进行
        return 1;
    }

    //  如果调用链中有linux异常处理函数，先把它们排除
    btnum       = rec->btfuncnum;
    btitem      = rec->btfunc;
    for( ; btnum > 0 ; btnum --, btitem++ ){
        if( strstr( btitem->funcname,"SignalHandler") ){
            btnum --;
            btitem ++;
            if( btnum == 0 ){
                callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_2));
                return 1;
            }
            break;
        }
    }

    if( btnum == 0 ){
        //  如果没有找到调用链，那把回调函数重置到调用链起始
        btnum       = rec->btfuncnum;
        btitem      = rec->btfunc;
    }

    for( ; btnum > 0 ; btnum --, btitem++ ){
        if( !btitem->funcname[0] )      //  没有找到函数名
            continue;

        //  判断死锁类型
        //  1.  是否调用了waitpid,这可能是创建子进程后，子进程没及时退出导致的死锁
        if( strstr( btitem->funcname,"waitpid") ){
            callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_3),btitem->funcname);
            return 1;
        }

        //  2.  是否调用了锁操作
        if( strstr( btitem->funcname,"SemP") 
                || strstr( btitem->funcname,"lock") 
                || strstr( btitem->funcname,"Lock") 
          ){
            if( rec->lockstr ){
                callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_1),rec->lockstr,btitem->funcname);
                return 1;
            }
        }

        //  3.  是否调用了read/recv/sleep操作
        if(  strstr( btitem->funcname,"recv") 
                || strstr( btitem->funcname,"sleep") 
                || strstr( btitem->funcname,"accept") 
                || (strstr( btitem->funcname,"read") && !strstr(btitem->funcname,"Thread"))
          ){
            callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_4),btitem->funcname);
            return 1;
        }

        //  4.  是否调用了write/send操作
        if( strstr( btitem->funcname,"write") 
                || strstr( btitem->funcname,"send") 
          ){
            if( btnum > 1 && strstr(btitem[1].funcname,"XOS_MulticastMsg" )){
                callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_6),btitem[1].funcname);
            }
            else{
                callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_5),btitem->funcname);
            }
            return 1;
        }

    }

    callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_2));

    return 1;
}
