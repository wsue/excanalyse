
#ifndef EXC_PARSE_H_
#define EXC_PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif


#define EXC_TITLE_LEN  80
#define EXC_PATH_LEN   256

    struct ExcItemInfo{
        char    title[EXC_TITLE_LEN];  //  �쳣����
        char    path[EXC_PATH_LEN];    //  �쳣��Ϣ·��
    };

    struct ExcListItem{
        char                        desc[EXC_TITLE_LEN];  //  �쳣��������
        int                         num;                //  value����
        struct ExcItemInfo     info[0];            //  ��ͬ���쳣������
    };

    struct ExcList{
        int                     count;          //  infos����
        struct ExcListItem **list;       //  ÿ���쳣�ķ�����Ϣ
    };

    /** 
     * @brief ����һЩ�쳣��Ϣ������tmppathĿ¼�����ɷ������
     *      UI�ɸ���excinfo��ʾ�������ļ�����
     * 
     * @param exclist   [out]   ���������ÿ���쳣һ����¼���Ա���/�ļ�����ʽ����
     * @param tmppath   [in]    ���ĸ�Ŀ¼��������ʱ�ļ�
     * @param excstr    [in]    �쳣��Ϣ
     * @param elfpath   [in]    �����ʱ����·��
     * 
     * @return 0:   �ɹ�
     *          <0  ������
     */
    int Exc_Load(struct ExcList *exclist,const char *tmppath,const char *excstr, const char *elfpath);

    void Exc_Release(struct ExcList *exclist);

    enum EExcDasmMode{
        EExcDasmAll,           //  ȫ�������
        EExcDasmFuncList,      //  �õ������б���Ϣ
        EExcDasmFuncMatchList, //  �õ����к���������ָ���ַ��������б���Ϣ
        EExcDasmFuncMatch,     //  ��������к���������ָ���ַ����ĺ���
        EExcDasmFunc,          //  �����һ������
        EExcDasmAddr,          //  �����һ����ַ���ڵĺ���
    };

    /** 
     * @brief �ṩ�ļ�����๦��
     * 
     * @param fout      [in]    ������
     * @param elffile   [in]    ��������ļ�
     * @param mode      [in]    ����෽ʽ
     * @param param     [in]    ģʽ����
     * 
     * @return 0��ʾ�ɹ�,��0��ʾ������
     */
    int Exc_Dasm(FILE *fout,
            const char *elffile,enum EExcDasmMode mode,const char *param);

    /*-----------------------------------------------------------------------------
     *
     *  ���ʻ��ӿ�
     *
     *-----------------------------------------------------------------------------*/
    /** 
     * @brief �ṩ�ַ���ת�����ܣ���������ַ���ת���ɱ�������
     * 
     * @param str   [in]    ��ת�����ַ���
     * 
     * @return  ת�����������û��ת���ɹ����򷵻�str��
     */
    const char *exc_tr(const char *str);

    int Exc_Init(void);

#ifdef __cplusplus
}
#endif

#endif

