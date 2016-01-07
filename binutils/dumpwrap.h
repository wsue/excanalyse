#ifndef DUMP_WRAP_H_
#define DUMP_WRAP_H_


#include <stdint.h>

/*  
 *  错误码
 */
#define BFDWRAP_ERR_FILE                50
#define BFDWRAP_ERR_DISASM              70

#define BFDWRAP_ERR_FILE_SZ             (BFDWRAP_ERR_FILE+1)    /*  文件大小错误            */
#define BFDWRAP_ERR_FILE_NOFOUND        (BFDWRAP_ERR_FILE+2)    /*  文件不存在              */
#define BFDWRAP_ERR_FILE_OPEN           (BFDWRAP_ERR_FILE+3)    /*  文件打开失败            */
#define BFDWRAP_ERR_FILE_MODE           (BFDWRAP_ERR_FILE+4)    /*  文件格式错误            */
#define BFDWRAP_ERR_FILE_UNSUPPORT      (BFDWRAP_ERR_FILE+5)    /*  不支持这种类型的文件    */
#define BFDWRAP_ERR_FILE_MULTIRECOG     (BFDWRAP_ERR_FILE+6)    /*   需要指定格式           */
#define BFDWRAP_ERR_FILE_BADFMT         (BFDWRAP_ERR_FILE+7)    /*   文件格式错误           */
#define BFDWRAP_ERR_FILE_LOADERR        (BFDWRAP_ERR_FILE+8)    /*   加载信息失败           */



#define BFDWRAP_ERR_DISASM_NOFUNC       (BFDWRAP_ERR_DISASM+1)    /*  没有反汇编函数        */
#define BFDWRAP_ERR_DISASM_NOARCH       (BFDWRAP_ERR_DISASM+2)    /*  没有找到架构          */
#define BFDWRAP_ERR_DISASM_NOSIZE       (BFDWRAP_ERR_DISASM+3)    /*  读取size信息失败      */
#define BFDWRAP_ERR_DISASM_NODYMRELSZ   (BFDWRAP_ERR_DISASM+4)    /*  没有找到对应的动态符号区 */

#define BFDWRAP_ERR_NO_MEM              20  /*  没有内存        */
#define BFDWRAP_ERR_ARG                 21  /*  参数错误        */
#define BFDWRAP_ERR_NOFOUND             22  /*  没有找到        */



/*
 *  其它定义
 */
#define BFDWRAP_MAX_PATH                256 /*  文件路径长度    */
#define BFDWRAP_FUNC_LEN                32  /*  函数名长度      */

#ifndef DASM_ADDR_T
#define DASM_ADDR_T
typedef int64_t  dasm_addr_t;
#endif


enum EDumpWrapAct{
    EDumpWrapBegin  = 0,        //  开始分析
    EDumpWrapEnd,               //  分析文件完成
    EDumpWrapFuncStart,         //  开始输出函数
    EDumpWrapFuncEnd            //  结束输出函数
};
/** 
 * @brief 向记录中打印信息
 * 
 * @param param     [in]    函数私有参数
 * @param buf       [in]    待输出内容
 * @param size      [in]    输出内容大小
 * 
 * @return 写入的字符数
 */

typedef unsigned int (*DASM_DUMP_CALLBACK)(void *param,const char *buf,int len);
typedef unsigned int (*DASM_DUMPFMT_CALLBACK)(void *param,const char *fmt,...);

/** 
 * @brief 设置下一步的行为
 * 
 * @param param     [in]    函数私有参数
 * @param newact    [in]    后面的行为类型
 * @param fname     [in]    当前处理的elf文件名
 * @param elfoffset [in]    文件偏移
 * @param funcname  [in]    函数名
 * @param code_start [in]   函数汇编代码起始
 * @param code_end   [in]   函数汇编代码结束
 * 
 * @return  0       不需要记录
 *          其它值  希望知道函数内某个pc针对对应的代码行
 */
typedef dasm_addr_t    (*DASM_ACT_CHECK)(void *param,
            enum EDumpWrapAct newact,
            const char *fname,dasm_addr_t elfoffset,
            const char *funcname,
            dasm_addr_t    code_start,
            dasm_addr_t    code_end);




/** 
 * @brief objdump封装接口
 * 
 * @param hctl          [out]   输出回调,同时在filename字段中传入待分析的文件名
 * 
 * @return 0:   操作成功
 *          其它值: 错误码
 */
int dumpwrap_load(
        const char      *fname,     //  文件名
        int             fsize,      //  文件大小,0表示不校验
        dasm_addr_t    vma_offset, //  文件中每个符号的偏移
        DASM_DUMP_CALLBACK dump,   //  反汇编输出回调
        DASM_DUMPFMT_CALLBACK err, //  错误输出回调
        DASM_ACT_CHECK     act,    //  动作转换回调
        void *param );              //  三个回调的参数
//typedef void *bfdwrap_t;
//bfdwrap_t dumpwrap_open_elf(const char *file_name,int *err,dasm_addr_t vma_offset);
//int dumpwrap_dump_elf(bfdwrap_t handle,int showline,struct dumpwrap_ctl *hout);
//void dumpwrap_close_elf(bfdwrap_t handle);



#endif

