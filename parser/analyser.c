
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <assert.h>

#include "exc_parse.h"
#include "exc_error.h"
#include "exc_str.h"
#include "parser_comm.h"
#include "analyser.h"



#define EXC_MOD_PRIO_MAX       99
#define EXC_MOD_PRIO_MIN       0

static T_AppExcAnalyser    *s_ModuleList;       //  ����Ҫ���б�

/** 
 * @brief ע��һ������ģ��
 * 
 * @param mod   [in]    ��ע���ģ��
 * @param prio  [in]    ģ�����ȼ���ȡֵΪ0-99,Խ
 *                      ���ȼ�Խ������ȵ��ô�ģ����з���
 *                      ���һ��ģ����ж�׼ȷ�ʸ߲��Ҳ�������
 *                      ��Ӧ��ҹ����ߵ����ȼ�
 * 
 * @return  0��ʾ�ɹ�������ֵ��ʾ������
 */
static int AddModule(T_AppExcAnalyser *mod,int priority)
{
    int     i   = 0;
    if( !mod || !mod->name[0] 
      )
        return E_EXC_INVALID;

    assert( priority >= EXC_MOD_PRIO_MIN 
            && priority <= EXC_MOD_PRIO_MAX );

    mod->next           = NULL;
    mod->priority       = priority;

    if( !s_ModuleList ){
        s_ModuleList    = mod;
    }
    else{
        T_AppExcAnalyser    *tmp    = NULL;
        T_AppExcAnalyser    *pnext  = s_ModuleList;
        while(pnext){
            if( !strcmp(pnext->name,mod->name) )
                return E_EXC_EXIST;

            pnext   = pnext->next;
        }

        /*
         *      �����ȼ�������ȷ��λ��
         */
        if( priority > s_ModuleList->priority ){
            tmp             = s_ModuleList;
            s_ModuleList    = mod;
            mod->next       = tmp;
        }
        else{
            pnext  = s_ModuleList;
            while( 1 ){
                if( !pnext->next ){
                    pnext->next = mod;
                    mod->next   = NULL;
                    break;
                }

                if( priority < pnext->next->priority ){
                    tmp         = pnext->next;
                    pnext->next = mod;
                    mod->next   = tmp;
                    break;
                }

                pnext   = pnext->next;
            }
        }
    }

    return 0;
}


static int Analyser_Init(void)
{
    //      ע�������Ϣģ�飬��ģ����������쳣�Ļ�����Ϣ��
    //      ���˴�ģ�飬�����ģ��ֻ��Ҫ����Լ��ķ�����������Լ���ص���Ϣ��
    //      ����Ҫ���ǻ�����Ϣ
    int ret = AddModule(&g_NormalAnalyser,99);
    if( ret != 0 )
        return ret;

    //      ��������ѭ����Ӧ���Ƚ��з����������쳣ģ��
    ret = AddModule(&g_DeadLoopAnalyser,80);
    if( ret != 0 )
        return ret;

    ret     = AddModule( &g_DeadLockAnalyser,80);
    if( ret != 0 )
        return ret;


    ret     = AddModule( &g_StackOverflowAnalyser,79);
    if( ret != 0 )
        return ret;

    //  g_ExcAnalyser:    ֻ�����϶����Ͳ��ˣ��Ż����������
    ret     = AddModule( &g_ExcAnalyser,1);
    if( ret != 0 )
        return ret;

    return 0;
}

/** 
 * @brief �������е��쳣����ģ����з���
 * 
 * @param rec       [in]    ���������쳣��¼
 * @param callback  [in]    �����ɹ�ʱ�����õ���ʾ�ӿ�
 * @param param     [in]    callback�Լ��Ĳ���
 * 
 * @return  1:  ������ȷ�е�ԭ��
 *          0:  ����ģ����ɷ���
 *          <0: ������
 */
int Analyser_Analyse(const T_AppExcRec *rec,EXC_ANALYSE_CALLBACK callback,void *param)
{
    static int isinit       = 0;
    int         ret         = 0;

    if( !callback || !rec )
        return E_EXC_INVALID;

    if( !isinit ){
        ret = Analyser_Init();
        if( ret == 0 )
            isinit      = 1;
        else
            return ret;
    }

    T_AppExcAnalyser *item  = s_ModuleList;

    if( !item )
        return E_EXC_NOINIT;

    while( item ){
        ret = item->analyse(rec,callback,param);
        if( ret == 1 )
            return 1;
        else if ( ret < 0 )
            return ret;

        item    = item->next;
    }

    return 0;
}



