
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

    //  �жϰ汾�Ƿ�֧��
    if( rec->version != EXCREC_VERSION0 )
        return 0;

    //  �ж��Ƿ�����������ģ��ֻ֧�ַ��������쳣
    if( rec->errtype != APPETYP_DEADLOCK )
        return 0;

    callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_0));
    //  �ж��Ƿ��е�����
    //  ���û��ֻ���Ʋ�
    if( rec->btfuncnum <1 ){
        if( rec->lockstr ){
            //  ���������¼��
            //  ������ԭ������������̳߳�ʱ��ռ��������
            callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_1),rec->lockstr,exc_tr(EXCSTR_UNKNOWN));
        }
        else{
            //  δ֪������ԭ������ϵ֧��
            callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_2));
        }

        //  ��������Ҫ����ģ���ٷ���,���ڱ�ģ�����
        return 1;
    }

    //  �������������linux�쳣���������Ȱ������ų�
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
        //  ���û���ҵ����������ǰѻص��������õ���������ʼ
        btnum       = rec->btfuncnum;
        btitem      = rec->btfunc;
    }

    for( ; btnum > 0 ; btnum --, btitem++ ){
        if( !btitem->funcname[0] )      //  û���ҵ�������
            continue;

        //  �ж���������
        //  1.  �Ƿ������waitpid,������Ǵ����ӽ��̺��ӽ���û��ʱ�˳����µ�����
        if( strstr( btitem->funcname,"waitpid") ){
            callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_3),btitem->funcname);
            return 1;
        }

        //  2.  �Ƿ������������
        if( strstr( btitem->funcname,"SemP") 
                || strstr( btitem->funcname,"lock") 
                || strstr( btitem->funcname,"Lock") 
          ){
            if( rec->lockstr ){
                callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_1),rec->lockstr,btitem->funcname);
                return 1;
            }
        }

        //  3.  �Ƿ������read/recv/sleep����
        if(  strstr( btitem->funcname,"recv") 
                || strstr( btitem->funcname,"sleep") 
                || strstr( btitem->funcname,"accept") 
                || (strstr( btitem->funcname,"read") && !strstr(btitem->funcname,"Thread"))
          ){
            callback(param,&g_DeadLockAnalyser,0,exc_tr(EXCSTR_DEADLOCK_4),btitem->funcname);
            return 1;
        }

        //  4.  �Ƿ������write/send����
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
