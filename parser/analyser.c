
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

static T_AppExcAnalyser    *s_ModuleList;       //  分析要块列表

/** 
 * @brief 注册一个分析模块
 * 
 * @param mod   [in]    待注册的模块
 * @param prio  [in]    模块优先级，取值为0-99,越
 *                      优先级越高则会先调用此模块进行分析
 *                      如果一个模块的判断准确率高并且不会误判
 *                      那应该夜里更高的优先级
 * 
 * @return  0表示成功，其它值表示错误码
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
         *      按优先级插入正确的位置
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
    //      注册基本信息模块，此模块用于输出异常的基本信息，
    //      有了此模块，后面的模块只需要输出自己的分析结果及与自己相关的信息，
    //      不再要考虑基本信息
    int ret = AddModule(&g_NormalAnalyser,99);
    if( ret != 0 )
        return ret;

    //      死锁和死循环是应该先进行分析的两个异常模块
    ret = AddModule(&g_DeadLoopAnalyser,80);
    if( ret != 0 )
        return ret;

    ret     = AddModule( &g_DeadLockAnalyser,80);
    if( ret != 0 )
        return ret;


    ret     = AddModule( &g_StackOverflowAnalyser,79);
    if( ret != 0 )
        return ret;

    //  g_ExcAnalyser:    只有以上都解释不了，才会调用它解释
    ret     = AddModule( &g_ExcAnalyser,1);
    if( ret != 0 )
        return ret;

    return 0;
}

/** 
 * @brief 调用所有的异常分析模块进行分析
 * 
 * @param rec       [in]    待分析的异常记录
 * @param callback  [in]    分析成功时，调用的显示接口
 * @param param     [in]    callback自己的参数
 * 
 * @return  1:  分析到确切的原因
 *          0:  所有模块完成分析
 *          <0: 错误码
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



