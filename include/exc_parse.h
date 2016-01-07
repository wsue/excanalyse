
#ifndef EXC_PARSE_H_
#define EXC_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif


#define EXC_TITLE_LEN  80
#define EXC_PATH_LEN   256

    struct ExcItemInfo{
        char    title[EXC_TITLE_LEN];  //  异常标题
        char    path[EXC_PATH_LEN];    //  异常信息路径
    };

    struct ExcListItem{
        char                        desc[EXC_TITLE_LEN];  //  异常基本描述
        int                         num;                //  value个数
        struct ExcItemInfo     info[0];            //  不同的异常分析项
    };

    struct ExcList{
        int                     count;          //  infos个数
        struct ExcListItem **list;       //  每个异常的分析信息
    };

    /** 
     * @brief 加载一些异常信息，并在tmppath目录下生成分析结果
     *      UI可根据excinfo显示标题与文件内容
     * 
     * @param exclist   [out]   分析结果，每个异常一个记录，以标题/文件的形式保存
     * @param tmppath   [in]    在哪个目录下生成临时文件
     * @param excstr    [in]    异常信息
     * @param elfpath   [in]    反汇编时查找路径
     * 
     * @return 0:   成功
     *          <0  错误码
     */
    int Exc_Load(struct ExcList *exclist,const char *tmppath,const char *excstr, const char *elfpath);

    void Exc_Release(struct ExcList *exclist);

    enum EExcDasmMode{
        EExcDasmAll,           //  全部反汇编
        EExcDasmFuncList,      //  得到函数列表信息
        EExcDasmFuncMatchList, //  得到所有函数名包括指定字符串函数列表信息
        EExcDasmFuncMatch,     //  反汇编所有函数名包含指定字符串的函数
        EExcDasmFunc,          //  反汇编一个函数
        EExcDasmAddr,          //  反汇编一个地址所在的函数
    };

    /** 
     * @brief 提供文件反汇编功能
     * 
     * @param fout      [in]    输出结果
     * @param elffile   [in]    待反汇编文件
     * @param mode      [in]    反汇编方式
     * @param param     [in]    模式参数
     * 
     * @return 0表示成功,非0表示错误码
     */
    int Exc_Dasm(FILE *fout,
            const char *elffile,enum EExcDasmMode mode,const char *param);

    /*-----------------------------------------------------------------------------
     *
     *  国际化接口
     *
     *-----------------------------------------------------------------------------*/
    /** 
     * @brief 提供字符串转换功能，把输入的字符串转换成本地语言
     * 
     * @param str   [in]    待转换的字符串
     * 
     * @return  转换后结果。如果没有转换成功，则返回str。
     */
    const char *exc_tr(const char *str);

    int Exc_Init(void);

#ifdef __cplusplus
}
#endif

#endif

