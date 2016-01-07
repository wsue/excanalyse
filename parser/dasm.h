
#ifndef EXC_DASM_H_
#define EXC_DASM_H_

typedef struct tag_DasmItem{

    dasm_addr_t        pc;                     //  �����ҵ�pc
    dasm_addr_t        funcaddr;               //  pc���ں�����ʼ����,0��ʾ��δ�����ɹ�
    char                funcname[EXC_NAME_MAX]; //  pc���ں�����

    char                elfname[EXC_NAME_MAX];  //  ����
    dasm_addr_t        elf_offset;             //  ��ƫ��

    char                *dasmstr;
}T_DasmItem;

typedef struct tag_DasmInfo{
    int                 num;
    T_DasmItem          items[0];
}T_DasmInfo;



/** 
 * @brief �õ�һ���쳣��¼�ĵ������������Ϣ
 * 
 * @param *pinf     [out]   �쳣�������������Ϣ
 * @param rec       [in]    �쳣��¼
 * @param elffile   [in]    �����쳣��elf�ļ�·��
 * @param libpath   [in]    elf�ļ��쳣ʱʹ�õĹ����·��
 * 
 * @return  0:  �ɹ�
 */
int Dasm_GetFromRec(T_DasmInfo **pinf,
        const T_AppExcRec *rec,const char *libpath);

/** 
 * @brief �ͷ�һ�������ṹ
 * 
 * @param info [in] ���ͷŵĽṹ
 */
void Dasm_Release(T_DasmInfo * info);


#endif


