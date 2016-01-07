
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

#define EXCREC_VERSION0       1       //  异常记录版本号
#define EXC_NAME_MAX                64
#define EXC_PATHNAME_MAX            256


/*-----------------------------------------------------------------------------
 *
 *  异常记录信息
 *
 *-----------------------------------------------------------------------------*/
/** 
 * @brief 任务模式
 */
enum ETaskMode{
    TSKMODE_UNKNOWN   = 0,            //  未知线程类型
    TSKMODE_L1,                       //  一级调度线程
    TSKMODE_L2,                       //  二级调度线程
};

/** 
 * @brief 错误类型
 */
enum EAppErrType{
    APPETYP_UNKNOWN    = 0,            //  未知错误类型
    APPETYP_EXC,                       //  异常
    APPETYP_DEADLOCK,                  //  死锁
    APPETYP_DEADLOOP,                  //  死循环
};



/** 
 * @brief 异常中库文件信息
 */
typedef struct tag_ExcLibInfo{
    char            name[EXC_PATHNAME_MAX];         //  库名
    dasm_addr_t    baseaddr;                       //  库映射的基地址
    int             filesize;                       //  库文件大小
}T_ExcLibInfo;

/** 
 * @brief 异常调用链中每个函数信息
 */
typedef struct tag_ExcFuncInfo{
    dasm_addr_t    pc;                             //  pc地址
    char            funcname[EXC_NAME_MAX];          //  函数名
    char            soname[EXC_PATHNAME_MAX];       //  函数所在so文件名
}T_ExcFuncInfo;

typedef struct tag_ExcStackInfo{
    dasm_addr_t            base;                   //  栈基地址
    dasm_addr_t            bottom;                 //  栈底
    dasm_addr_t            sp;                     //  sp指针

    char                    *stackstr;              //  栈内容
}T_ExcStackInfo;


/** 
 * @brief 异常信号信息
 */
typedef struct tag_ExcSigInfo{
    int             signum;                     //  异常事件号
    dasm_addr_t    addr;                       //  错误地址

    T_ExcFuncInfo   pc;
    T_ExcStackInfo  stack;                      //  错误时栈信息
    char            *regstr;                    //  其它寄存器信息
}T_ExcSigInfo;

typedef struct tag_ExcPidInfo{
    char            name[EXC_PATHNAME_MAX];     //  进程名
    int             pid;                        //  进程pid
    unsigned char   md5[16];                    //  进程对应的elf文件md5值
    int             size;                       //  文件大小

    int             libnum;                     //  使用的动态库数目
    T_ExcLibInfo    *libs;                      //  动态库信息,具体长度由libnum决定
}T_ExcPidInfo;

typedef struct tag_ExcTaskInfo{
    int                     tid;                        //  线程内核态id
    char                    name[EXC_NAME_MAX];         //  任务名
    enum ETaskMode          mode;                       //  线程模式
    dasm_addr_t            thrid;                      //  线程用户态id

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
    unsigned int            jno;                        //  Jno,0表示没有
    char                    name[EXC_NAME_MAX];         //  出错误的JOB名字,如果为空，那后面的Job信息都无效
    enum ETaskMode          type;                       //  JOB调度类型(一级或二级)
    int                     runstat;                    //  JOB运行状态
    int                     stat;                       //  JOB自己状态
    int                     rundur;                     //  运行时间(秒),0表示无效

    T_MsgInfo               msg;

    T_ExcStackInfo          stack;
}T_ExcJobInfo;

typedef struct tag_AppExcRec{
    int                 version;                //  版本号
    enum EAppErrType    errtype;             //  错误类型    
    char                errtime[EXC_NAME_MAX];  //  错误时间
    int                 pos[4];                 //  位置

    T_ExcSigInfo        signal;             //  信号信息
    T_ExcPidInfo        process;            //  进程信息
    T_ExcTaskInfo       task;               //  任务信息
    T_ExcJobInfo        job;                //  job信息

    int                 btfuncnum;          //  回调函数个数
    T_ExcFuncInfo       *btfunc;            //  回调函数信息

    char                *lockstr;           //  死锁信息字符串
    char                *tipcstr;           //  死锁信息字符串

    char                *excstr;            //  异常记录对应的字符串
}T_AppExcRec;




void *Exc_Malloc(size_t size);
void Exc_Free(void *ptr);
char *Exc_StrDup(const char *str);

//  删除字符串尾部的空白
static inline char* str_trim(char *in)
{
    if( in && *in ){
        char    *p  = in + strlen(in)-1;
        while( isspace(*p) && p != in ) p--;
        p[1]  = 0;
    }
    return in;
}

//  删除字符串首尾的空白
static inline char* str_chop(char *in)
{
    if( in && *in ){
        while( isspace( *in )) in++;
        return str_trim(in);
    }
    
    return in;
}

#endif


