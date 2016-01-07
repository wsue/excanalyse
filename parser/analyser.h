
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
 * @brief �쳣�����ص�����,���ڸ�ʽ������ַ���
 * 
 * @param module    [in]    ���з�����ģ��
 * @param err       [in]    ģ��������,0��ʾ�ɹ�������Ϊ������
 * @param result    [in]    ģ��������ַ������
 * @param param     [in]    �ص������Լ��Ĳ���
 */
typedef void (*EXC_ANALYSE_CALLBACK)(void *param,
        struct tag_AppExcAnalyser *module,
        int err,const char *fmt,...);

typedef struct tag_AppExcAnalyser{
    const char  *name;                              //  ģ����,����Ψһ
    const char  *desc;                              //  ģ������
    int  (*analyse)(const T_AppExcRec *rec,
            EXC_ANALYSE_CALLBACK callback,void *param);     //  ģ���������

    int         priority;
    struct tag_AppExcAnalyser    *next;             //  ָ����һ��ģ��
}T_AppExcAnalyser;


//int Analyser_Init(void);

/** 
 * @brief �������е��쳣����ģ����з���
 * 
 * @param rec       [in]    ���������쳣��¼
 * @param callback  [in]    �����ɹ�ʱ�����õ���ʾ�ӿ�
 * @param param     [in]    callback�Լ��Ĳ���
 * 
 * @return 0��ʾ�ɹ���������ʾ������
 */
int Analyser_Analyse(const T_AppExcRec *rec,EXC_ANALYSE_CALLBACK callback,void *param);


extern T_AppExcAnalyser     g_NormalAnalyser;
extern T_AppExcAnalyser     g_DeadLockAnalyser;
extern T_AppExcAnalyser     g_DeadLoopAnalyser;
extern T_AppExcAnalyser     g_ExcAnalyser;
extern T_AppExcAnalyser     g_StackOverflowAnalyser;

#endif

