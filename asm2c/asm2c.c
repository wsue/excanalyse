


static int set_asmline(struct asm_line *op,const char *str)
{
    int     size        = 0;
    char    *p          = NULL;


    memset(op,0,sizeof(*op));
    INIT_LIST_HEAD(&op->list);

    op->str             = str;

    op->offset          = strtoull(str,&p,16);
    if( !op->offset )
        return 0;

    if( p != ':'){
        //  如果后面跟的不是':'，那这是一个函数名，函数名被<括起来>
        //  这时str指向<后,p指向>前
        op->p1          = op->cache;
        str             = strchr(p,'<');
        if( str ){
            str ++;
            p           = strchr(str,'>');
            if( p && p != str ){
                size    = p - str;
                if( size >= sizeof(op->cache) )
                    size    = sizeof(op->cache) -1;
                memcpy(op->cache,str,size);
            }
        }

        return 0;
    }

    //  到这里，表示是一个汇编语句，开始语句分析
    str             = p+1;
    while(isspace(*str))    str++;
    p               = str;

    while(isalnum(*p))      p++;    //  查找操作符
    size            = p - str;
    if( size > 16){      //  汇编操作符长度不会超过16个字符,如果超过，那把它当成普通字符串
        op->offset  = 0;
        return 0;
    }

    op->op          = op->cache;
    memcpy(op->op,str,size);

    str             = p;
    while(isspace(*str))    str++;
    if( (! *str ) || (*str == '#') )
        return 0;

    //  获取参数部分，参数部分都没有空格,多个参数用','隔开，#后面是注释
    p               = str;
    while( *p && (!isspace(*p)) && *p != '#' ) p++;

    if( p == str )
        return 0;

    op->p1      = op->cache + size +1;
    if( size + 1 + (p - str) +1 > sizeof(op->cache) )
        size    = sizeof(op->cache) - size -1 -1;
    else
        size    = (p - str);

    memcpy(op->p1,str,size);
    op->p2      = strchr(op->p1,',');
    if( op->p2 ){
        *op->p2++   = 0;
        op->p3      = strchr(op->p2,',');
        if( op->p3 )
            *op->p3++   = 0;
    }

    return 0;
}


static int asmlines_release(struct list_head *lists)
{
    struct asm_op   *entry;
    struct asm_op   *safe;
    list_for_each_entry_safe(entry, safe,lists, list){
        list_del(&entry->list);
        free(entry);
    }

    INIT_LIST_HEAD(lists);
}

static int asmlines_load(char *str)
{
    struct list_head lists;
    INIT_LIST_HEAD(&lists);

    while( str ){
        char    *p  = strchr(str,'\n');
        if( p )
            *p++    = 0;

        struct asm_op   *op = malloc(sizeof(struct asm_op));
        if( !op ){
            asmlines_release(&lists);
            return E_EXC_NOMEM;
        }

        list_add(lists,&op->list);


        int     ret = add_asmline(lists,str);
        if(  ret != 0 )
            return ret;
        str         = p;
    }

    return 0;
}



//  
//  处理流程:
//      1.  先把字符串转换成链表
//      2.  再把一个链表按每个函数分解，每个函数一个链表
//      3.  调用处理函数分析
