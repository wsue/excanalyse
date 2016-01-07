
#ifndef EXC_DASM_H_
#define EXC_DASM_H_

typedef struct tag_DasmItem{

    dasm_addr_t        pc;                     //  待查找的pc
    dasm_addr_t        funcaddr;               //  pc所在函数起始代码,0表示还未反汇编成功
    char                funcname[EXC_NAME_MAX]; //  pc所在函数名

    char                elfname[EXC_NAME_MAX];  //  库名
    dasm_addr_t        elf_offset;             //  库偏移

    char                *dasmstr;
}T_DasmItem;

typedef struct tag_DasmInfo{
    int                 num;
    T_DasmItem          items[0];
}T_DasmInfo;



/** 
 * @brief 得到一个异常记录的调用链反汇编信息
 * 
 * @param *pinf     [out]   异常调用链反汇编信息
 * @param rec       [in]    异常记录
 * @param elffile   [in]    产生异常的elf文件路径
 * @param libpath   [in]    elf文件异常时使用的共享库路径
 * 
 * @return  0:  成功
 */
int Dasm_GetFromRec(T_DasmInfo **pinf,
        const T_AppExcRec *rec,const char *libpath);

/** 
 * @brief 释放一个反汇编结构
 * 
 * @param info [in] 待释放的结构
 */
void Dasm_Release(T_DasmInfo * info);


#endif


