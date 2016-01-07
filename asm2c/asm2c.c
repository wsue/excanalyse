


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
        //  ���������Ĳ���':'��������һ������������������<������>
        //  ��ʱstrָ��<��,pָ��>ǰ
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

    //  �������ʾ��һ�������䣬��ʼ������
    str             = p+1;
    while(isspace(*str))    str++;
    p               = str;

    while(isalnum(*p))      p++;    //  ���Ҳ�����
    size            = p - str;
    if( size > 16){      //  �����������Ȳ��ᳬ��16���ַ�,����������ǰ���������ͨ�ַ���
        op->offset  = 0;
        return 0;
    }

    op->op          = op->cache;
    memcpy(op->op,str,size);

    str             = p;
    while(isspace(*str))    str++;
    if( (! *str ) || (*str == '#') )
        return 0;

    //  ��ȡ�������֣��������ֶ�û�пո�,���������','������#������ע��
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
//  ��������:
//      1.  �Ȱ��ַ���ת��������
//      2.  �ٰ�һ������ÿ�������ֽ⣬ÿ������һ������
//      3.  ���ô���������
