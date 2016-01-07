
#ifndef ASM_2_C_H_
#define ASM_2_C_H_


#define OP_CACHE_LEN        64  //  保存指令信息的缓存
struct asm_line{
    struct list_head    list;       //  链表
    u64                 offset;     //  指令偏移
                                    //  如果offset == 0,那表示这是一行注释
                                    //  如果offset !=0,并且op!=NULL表示是一个语句
                                    //                 op=NULL,表示是一个函数起始,p1中保存函数名
                                    //                 如果p1也为NULL则
    const char          *op;        //  操作码
    const char          *p1;        //  参数1
    const char          *p2;        //  参数2
    const char          *p3;        //  参数3
    char                cache[OP_CACHE_LEN];    //  缓存上面的op、p1、p2、p3指向的字符串
    const char          *str;       //  指向原始字符串

};


struct asmparse_ctl{
    struct list_head    list;           //  asm_line链表

    char                *comment_buf;   //  保存comment信息
    int                 comment_sz;     //  comment_buf大小
    int                 comment_offset; //  comment_buf当前已用长度
};


#endif

