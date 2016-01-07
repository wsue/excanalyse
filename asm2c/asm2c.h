
#ifndef ASM_2_C_H_
#define ASM_2_C_H_


#define OP_CACHE_LEN        64  //  ����ָ����Ϣ�Ļ���
struct asm_line{
    struct list_head    list;       //  ����
    u64                 offset;     //  ָ��ƫ��
                                    //  ���offset == 0,�Ǳ�ʾ����һ��ע��
                                    //  ���offset !=0,����op!=NULL��ʾ��һ�����
                                    //                 op=NULL,��ʾ��һ��������ʼ,p1�б��溯����
                                    //                 ���p1ҲΪNULL��
    const char          *op;        //  ������
    const char          *p1;        //  ����1
    const char          *p2;        //  ����2
    const char          *p3;        //  ����3
    char                cache[OP_CACHE_LEN];    //  ���������op��p1��p2��p3ָ����ַ���
    const char          *str;       //  ָ��ԭʼ�ַ���

};


struct asmparse_ctl{
    struct list_head    list;           //  asm_line����

    char                *comment_buf;   //  ����comment��Ϣ
    int                 comment_sz;     //  comment_buf��С
    int                 comment_offset; //  comment_buf��ǰ���ó���
};


#endif

