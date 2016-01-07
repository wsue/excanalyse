
#ifndef EXC_ANALYSER_H_
#define EXC_ANALYSER_H_

#include <signal.h>

#ifndef SIGPIPE 
#define SIGPIPE 13
#endif


#ifndef SIGBUS 
#define SIGBUS  7
#endif


struct tag_AppExcAnalyser;

/** 
 * @brief 异常分析回调函数,用于格式化输出字符串
 * 
 * @param module    [in]    进行分析的模块
 * @param err       [in]    模块分析结果,0表示成功，其它为错误码
 * @param result    [in]    模块分析的字符串结果
 * @param param     [in]    回调函数自己的参数
 */
typedef void (*EXC_ANALYSE_CALLBACK)(void *param,
        struct tag_AppExcAnalyser *module,
        int err,const char *fmt,...);

typedef struct tag_AppExcAnalyser{
    const char  *name;                              //  模块名,必须唯一
    const char  *desc;                              //  模块描述
    int  (*analyse)(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param);     //  模块分析函数

    int         priority;
    struct tag_AppExcAnalyser    *next;             //  指责下一个模块
}T_AppExcAnalyser;


//int Analyser_Init(void);

/** 
 * @brief 调用所有的异常分析模块进行分析
 * 
 * @param rec       [in]    待分析的异常记录
 * @param callback  [in]    分析成功时，调用的显示接口
 * @param param     [in]    callback自己的参数
 * 
 * @return 0表示成功，其它表示错误码
 */
int Analyser_Analyse(const T_AppExcRec *rec,EXC_ANALYSE_CALLBACK callback,void *param);


extern T_AppExcAnalyser     g_NormalAnalyser;
extern T_AppExcAnalyser     g_DeadLockAnalyser;
extern T_AppExcAnalyser     g_DeadLoopAnalyser;
extern T_AppExcAnalyser     g_ExcAnalyser;
extern T_AppExcAnalyser     g_StackOverflowAnalyser;

#endif

