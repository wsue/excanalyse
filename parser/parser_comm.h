
#ifndef PARSER_COMM_H_
#define PARSER_COMM_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>


#ifndef DASM_ADDR_T
#define DASM_ADDR_T
typedef int64_t  dasm_addr_t;
#endif

#ifdef OS_WIN32
#define F64X        "0x%I64x"
#else
#define F64X        "0x%llx"
#endif

#define EXCREC_VERSION0       1       //  �쳣��¼�汾��
#define EXC_NAME_MAX                64
#define EXC_PATHNAME_MAX            256


/*-----------------------------------------------------------------------------
 *
 *  �쳣��¼��Ϣ
 *
 *-----------------------------------------------------------------------------*/
/** 
 * @brief ����ģʽ
 */
enum ETaskMode{
    TSKMODE_UNKNOWN   = 0,            //  δ֪�߳�����
    TSKMODE_L1,                       //  һ�������߳�
    TSKMODE_L2,                       //  ���������߳�
};

/** 
 * @brief ��������
 */
enum EAppErrType{
    APPETYP_UNKNOWN    = 0,            //  δ֪��������
    APPETYP_EXC,                       //  �쳣
    APPETYP_DEADLOCK,                  //  ����
    APPETYP_DEADLOOP,                  //  ��ѭ��
};



/** 
 * @brief �쳣�п��ļ���Ϣ
 */
typedef struct tag_ExcLibInfo{
    char            name[EXC_PATHNAME_MAX];         //  ����
    dasm_addr_t    baseaddr;                       //  ��ӳ��Ļ���ַ
    int             filesize;                       //  ���ļ���С
}T_ExcLibInfo;

/** 
 * @brief �쳣��������ÿ��������Ϣ
 */
typedef struct tag_ExcFuncInfo{
    dasm_addr_t    pc;                             //  pc��ַ
    char            funcname[EXC_NAME_MAX];          //  ������
    char            soname[EXC_PATHNAME_MAX];       //  ��������so�ļ���
}T_ExcFuncInfo;

typedef struct tag_ExcStackInfo{
    dasm_addr_t            base;                   //  ջ����ַ
    dasm_addr_t            bottom;                 //  ջ��
    dasm_addr_t            sp;                     //  spָ��

    char                    *stackstr;              //  ջ����
}T_ExcStackInfo;


/** 
 * @brief �쳣�ź���Ϣ
 */
typedef struct tag_ExcSigInfo{
    int             signum;                     //  �쳣�¼���
    dasm_addr_t    addr;                       //  �����ַ

    T_ExcFuncInfo   pc;
    T_ExcStackInfo  stack;                      //  ����ʱջ��Ϣ
    char            *regstr;                    //  �����Ĵ�����Ϣ
}T_ExcSigInfo;

typedef struct tag_ExcPidInfo{
    char            name[EXC_PATHNAME_MAX];     //  ������
    int             pid;                        //  ����pid
    unsigned char   md5[16];                    //  ���̶�Ӧ��elf�ļ�md5ֵ
    int             size;                       //  �ļ���С

    int             libnum;                     //  ʹ�õĶ�̬����Ŀ
    T_ExcLibInfo    *libs;                      //  ��̬����Ϣ,���峤����libnum����
}T_ExcPidInfo;

typedef struct tag_ExcTaskInfo{
    int                     tid;                        //  �߳��ں�̬id
    char                    name[EXC_NAME_MAX];         //  ������
    enum ETaskMode          mode;                       //  �߳�ģʽ
    dasm_addr_t            thrid;                      //  �߳��û�̬id

    T_ExcStackInfo          stack;
}T_ExcTaskInfo;

typedef struct tagMsgInfo{
    int             id;
    int             type;
    int             hver;
    int             appver;
    char            *str;
}T_MsgInfo;

typedef struct tag_ExcJobInfo{
    unsigned int            jno;                        //  Jno,0��ʾû��
    char                    name[EXC_NAME_MAX];         //  �������JOB����,���Ϊ�գ��Ǻ����Job��Ϣ����Ч
    enum ETaskMode          type;                       //  JOB��������(һ�������)
    int                     runstat;                    //  JOB����״̬
    int                     stat;                       //  JOB�Լ�״̬
    int                     rundur;                     //  ����ʱ��(��),0��ʾ��Ч

    T_MsgInfo               msg;

    T_ExcStackInfo          stack;
}T_ExcJobInfo;

typedef struct tag_AppExcRec{
    int                 version;                //  �汾��
    enum EAppErrType    errtype;             //  ��������    
    char                errtime[EXC_NAME_MAX];  //  ����ʱ��
    int                 pos[4];                 //  λ��

    T_ExcSigInfo        signal;             //  �ź���Ϣ
    T_ExcPidInfo        process;            //  ������Ϣ
    T_ExcTaskInfo       task;               //  ������Ϣ
    T_ExcJobInfo        job;                //  job��Ϣ

    int                 btfuncnum;          //  �ص���������
    T_ExcFuncInfo       *btfunc;            //  �ص�������Ϣ

    char                *lockstr;           //  ������Ϣ�ַ���
    char                *tipcstr;           //  ������Ϣ�ַ���

    char                *excstr;            //  �쳣��¼��Ӧ���ַ���
}T_AppExcRec;




void *Exc_Malloc(size_t size);
void Exc_Free(void *ptr);
char *Exc_StrDup(const char *str);

//  ɾ���ַ���β���Ŀհ�
static inline char* str_trim(char *in)
{
    if( in && *in ){
        char    *p  = in + strlen(in)-1;
        while( isspace(*p) && p != in ) p--;
        p[1]  = 0;
    }
    return in;
}

//  ɾ���ַ�����β�Ŀհ�
static inline char* str_chop(char *in)
{
    if( in && *in ){
        while( isspace( *in )) in++;
        return str_trim(in);
    }
    
    return in;
}

#endif


