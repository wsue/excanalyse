
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "exc_parse.h"
#include "exc_error.h"
#include "exc_str.h"
#include "parser_comm.h"
#include "exclist.h"

struct exclist_ctl{
    char                path[256];      //  ·��ģ��

    FILE                *fp_sum;        //  �����ļ����,
    //  ����Ϣֻ�ڻ����ļ��У�������Ϣ���˻����ļ�
    //  ������Ϣ�ڻ����ļ��У�����list����Ϣ��¼
    int                 sum_index;      //  ������Ϣ������

    int                 list_num;
    int                 list_index;          //  list
    struct ExcListItem **list;
};


#ifdef OS_WIN32
#define MKDIR(dir)          mkdir(dir)
#define LSTAT(file,path_stat)    stat((path), (path_stat))
#else
#define MKDIR(dir)          mkdir(dir,0755)
#define LSTAT(file,path_stat)    lstat((path), (path_stat))
#endif
static int excblock_parse(struct exclist_ctl *ctl,char *str,const char *elfpath);
static int excblock_parse_otherexc(struct exclist_ctl *ctl,char *str);
static int excblock_parse_appexc(struct exclist_ctl *ctl,char *str,const char *elfpath);


static int clear_dir(const char *path)
{
    struct stat path_stat;
    remove(path);

    if (LSTAT(path, &path_stat) < 0) {
        return errno == ENOENT ? 0:-1;
    }else if(!S_ISDIR(path_stat.st_mode)){
        return -1;
    }

    //  ������沽��ûɾ���������ж��Ǹ�Ŀ¼�Ƕ�Ŀ¼�������ļ�����ɾ��
    DIR     *dir    = opendir(path);
    if( !dir )
        return errno == ENOENT ? 0:-1;

    int     err         = 0;
    int     size        = strlen(path) + 256;
    char    *buf        = Exc_Malloc(size);
    if( !buf ){
        err             = -1;
    }
    else{
        struct dirent *ent  = readdir(dir);
        buf[size-1]     = 0;
        while(ent){
            snprintf(buf,size-1,"%s/%s",path,ent->d_name);
            if( remove(buf) != 0 && errno != ENOENT)
                err     = -1;
            ent     = readdir(dir);
        }
    }
    Exc_Free(buf);
    closedir(dir);

    return -1;
}

static int reinit_tmpdir(struct exclist_ctl *ctl,const char *tmppath)
{
    snprintf(ctl->path,sizeof(ctl->path)-3,"%s/exc",tmppath);
    clear_dir(ctl->path);

    MKDIR(ctl->path);
    strcat(ctl->path,"/r");

    return 0;
}

/** 
 * @brief ��ʼ��exclist_ctl����ֵ��������sum��
 * 
 * @param ctl 
 * @param tmppath   [in]    �����ļ�����ʱ·��
 * 
 * @return 0:   �ɹ�
 *          E_EXC_NOMEM:   ���ڴ�
 *          E_EXC_FOPEN:   ���ļ�ʧ��
 */
static int exclist_init(struct exclist_ctl *ctl,const char *tmppath)
{
    struct ExcListItem *item = NULL;

    assert( ctl != NULL && tmppath != NULL );

    memset(ctl,0,sizeof(*ctl));
    reinit_tmpdir(ctl,tmppath);

    ctl->list  = Exc_Malloc(sizeof(struct ExcList *)* 10 );
    if( !ctl->list )
        return E_EXC_NOMEM;

    memset(ctl->list,0,sizeof(struct ExcList *)* 10 );
    ctl->list_num       = 10;

    item    = Exc_Malloc(sizeof(struct ExcListItem) + sizeof(struct ExcItemInfo));
    if( !item ){
        Exc_Free(ctl->list);
        ctl->list_num   = 0;
        ctl->list       = NULL;
        return E_EXC_NOMEM;
    }

    ctl->list[0]        = item;
    ctl->list_index     = 1;

    memset(item,0, sizeof(struct ExcListItem) + sizeof(struct ExcItemInfo));
    item->num           = 1;
    snprintf(item->desc,sizeof(item->desc)-1,exc_tr(EXCSTR_PARSER_SUM_TITLE));
    exclist_fmt_path(item->info[0].path,sizeof(item->info[0].path),ctl,"info");

    ctl->fp_sum         = fopen(item->info[0].path,"w");
    ctl->sum_index      = 1;
    if( !ctl->fp_sum ){
        Exc_Free(ctl->list[0]);
        Exc_Free(ctl->list);
        ctl->list_num   = 0;
        ctl->list       = NULL;
        return E_EXC_FOPEN;
    }

    return 0;
}

static int exclist_release(struct exclist_ctl *ctl)
{
    if( ctl ){
        if( ctl->fp_sum ){
            fclose( ctl->fp_sum );
            ctl->fp_sum = NULL;
        }
        if(ctl->list){
            int i = 0;
            for( ; i < ctl->list_num ; i ++ ){
                Exc_Free(ctl->list[i]);
            }

            Exc_Free(ctl->list);
            ctl->list       = NULL;
            ctl->list_num   = 0;
        }
    }
}


int exclist_fmt_path(char *path,int pathsize,const struct exclist_ctl *ctl,const char *name)
{
    assert(path && pathsize > 0 && ctl && name && *name);
    snprintf(path,pathsize-1,
            "%s_%03d_%s.txt",ctl->path,ctl->sum_index,name);
}

/** 
 * @brief ��exclist_ctl������һ�ע��
 *          item����Ϊ��̬������ڴ棬���ҵ��ô˺�����item���ڴ���exclist_ctl����
 * @param ctl 
 * @param item 
 * 
 * @return  0:   �ɹ�
 *          E_EXC_NOMEM:   �ڴ�����ʧ��
 */
static int exclist_push(struct exclist_ctl *ctl, struct ExcListItem *item)
{
    struct ExcListItem *new    = NULL;
    assert( ctl != NULL && ctl->list != NULL && ctl->list_num > 0 && item != NULL );

    if( ctl->list_index >= ctl->list_num ){
        int     size    = ctl->list_num + 32;
        struct ExcListItem **list  = Exc_Malloc(sizeof(struct ExcListItem *) * size);
        if( !list )
            return E_EXC_NOMEM;

        memset(list,0,sizeof(struct ExcListItem *) * size);
        memcpy(list,ctl->list,sizeof(struct ExcListItem *) * ctl->list_num);
        Exc_Free(ctl->list);
        ctl->list       = list;
        ctl->list_num   = size;
    }

    ctl->list[ctl->list_index++] = item ;
    return 0;
}


int exclist_push_item(struct exclist_ctl *ctl,
        const char *rectime,const char *usetime,const char *title,struct ExcListItem  *item)
{
    int ret =   0;
    if( item ){
        if( usetime )
            snprintf(item->desc,sizeof(item->desc)-1,"#%03d %s %s",ctl->sum_index,usetime,title);
        else if ( rectime )
            snprintf(item->desc,sizeof(item->desc)-1,"#%03d %s %s",ctl->sum_index,rectime,title);
        else 
            snprintf(item->desc,sizeof(item->desc)-1,"#%03d %s",ctl->sum_index,title);

        ret = exclist_push(ctl,item);
    }

    if( usetime ){
        fprintf(ctl->fp_sum,"#%d\t%s\t%s\n",
                ctl->sum_index,
                usetime ,
                title ? title:exc_tr(EXCSTR_UNKNOWN)
               );
    }
    else if( rectime){
        fprintf(ctl->fp_sum,"#%d\tR[%s]\t%s\n",
                ctl->sum_index,
                rectime ,
                title ? title:exc_tr(EXCSTR_UNKNOWN)
               );
    }
    else{
        fprintf(ctl->fp_sum,"#%d\t%s\t%s\n",
                ctl->sum_index,
                exc_tr(EXCSTR_UNKNOWN),
                title ? title:exc_tr(EXCSTR_UNKNOWN)
               );
    }

    ctl->sum_index++;
    return 0;
}

static int exclist_push_simple_record(struct exclist_ctl *ctl,const char *rectime,const char *usetime,const char *title,const char *info)
{
    FILE    *fp;
    struct ExcListItem *item = NULL;

    if( info && *info ){
        item    = Exc_Malloc(
                sizeof(struct ExcListItem) + sizeof(struct ExcItemInfo));
        if( !item )
            return E_EXC_NOMEM;

        memset(item,0,sizeof(struct ExcListItem) + sizeof(struct ExcItemInfo));
        item->num   = 1;

        snprintf(item->info[0].title,sizeof(item->info[0].title)-1,exc_tr(EXCSTR_PARSER_BASE_TITLE));
        exclist_fmt_path(item->info[0].path,sizeof(item->info[0].path),ctl,"info");

        fp      = fopen(item->info[0].path,"w");
        if( !fp ){
            Exc_Free(item);
            return E_EXC_NOMEM;
        }
        fwrite(info,strlen(info),1,fp);
        fclose(fp);
    }

    int ret = exclist_push_item(ctl,rectime,usetime,title,item);
    if( ret != 0 )
        Exc_Free(item);

    return ret;
}



static int exclist_parse(struct exclist_ctl *ctl,char *str,const char *elfpath)
{
    int     ret         = 0;
    char    *excblock    = str; //  ��¼һ���쳣��

    while(str && (ret == 0 ) ){
        //  1.  �� ********** Begin of Record **************,
        //      �ֽ�
        //      �����ֽ�Ľ���ǣ�
        //          ÿ�ο�����*** Begin of Record **��ʼ,���������־Ҳֻ���ڿ�ʼ
        //          �м���ܰ�����End   of Record **
        //  2.  ÿ�ֽ⵽��һ��ʱ���ж��϶ε������Ƿ�Ϊ��
        //      
        char    *pend   = NULL;
        char    *p      = strstr(str,"********** Begin of Record **************");
        if( p ){
            //  ��־�׻���\n��ʼ,�����\n��Ϊ\0,���û�ҵ���ֱ�Ӱ���ʼλ������Ϊ0
            char *pstart    = p;  
            while( pstart != str && *pstart == '*' )   pstart --;
            *pstart     = 0;

            //  ����βҲ��Ϊ\0
            pend            = strchr(p,'\n');
            if( pend )
                *pend++     = 0;
        }

        ret = excblock_parse(ctl,excblock,elfpath);

        excblock    = pend;
        str         = pend;
    }

    return ret;
}

static int excblock_parse(struct exclist_ctl *ctl,char *str,const char *elfpath)
{
    //  ɾ��end of record���������
    //  end of record �� begin of record ��������ǿհף������д���
    char    *p      = strstr(str,TOKEN_END_SECTION);
    if( p ){
        *p          = 0;
        p           = strrchr(str,'\n');
        if( p )
            *p      = 0;
    }

    //  ����쳣��¼������*** Begin of Record ******������������
    p      = strstr(str,TOKEN_BEGIN_SECTION);
    if( p ){
        p           = strchr(p,'\n');
        if( !p )
            return 0;   //  ���û��\n���Ǳ�ʾ����û���ݣ�ֱ�ӷ���

        str         = p +1;
    }



    str = str_chop(str);
    if( !*str )
        return 0;   //  ���û��������ֱ�ӷ���

    if( strstr(str,TOKEN_DEADLOCK_STR)
            || strstr(str,TOKEN_DEADLOOP_STR)
            || strstr(str,TOKEN_APPEXC_STR)
      ){
        int ret = excblock_parse_appexc(ctl,str,elfpath);
        if( ret == 0 )
            return 0;
    }
    return excblock_parse_otherexc(ctl,str);
}

static int excblock_parse_otherexc(struct exclist_ctl *ctl,char *str)
{
    char    *p          = NULL;
    char    *rectime    = NULL;
    char    *usetime    = NULL;
    char    *title      = NULL;
    assert(ctl != NULL && str != NULL );

    //  ����Record Time: 2010-12-30  21:50:15
    //  ÿ����¼������������,û����Ϊ�ǷǷ��ļ�¼������
    //  �������rectimeָ���ַ����Ŀ�ʼ
    rectime     = strstr(str,"Record Time:");
    if( !rectime ){
        return exclist_push_simple_record(ctl,rectime,usetime,str,NULL);
    }

    rectime     = strchr(rectime,':');
    rectime     ++;
    str           = strchr(rectime,'\n');
    if( !str ){
        return exclist_push_simple_record(ctl,rectime,usetime,exc_tr(EXCSTR_EMPTY),NULL);
    }
    *str++      = 0;


    //  ����Used Time: 2010-12-30  21:50:13 
    //  ÿ����¼���������У���������������У�����usetimeָ��ʱ��
    usetime    = strstr(str,"Used Time:");
    if( !usetime ){
        usetime    = strstr(str,"Child Exit At:");
    }

    if( usetime ){
        usetime     = strchr(usetime,':');
        usetime     ++;
        while(isspace(*rectime) )   rectime++;
        str         = strchr(usetime,'\n');
        if( !str ){
            return exclist_push_simple_record(ctl,rectime,usetime,exc_tr(EXCSTR_EMPTY),NULL);
        }
        *str++      = 0;
    }


    rectime = str_chop(rectime);
    usetime = str_chop(usetime);

    while(isspace(*str) )   str++;

    if( (strchr(str,'\n') == NULL) || strstr(str,"MCM Need Board Reset,") || strlen(str) < 120 ){
        //  ������ݲ��ֱ࣬��д��summary��,дʱ�����е�\n�����ɿո�
        p  = str;
        for ( ;*p; p++ ){
            if( *p == '\r' || *p == '\n' )  *p  = ' ';
        }

        return exclist_push_simple_record(ctl,rectime,usetime,str,NULL);
    }

    //  ������ݶ࣬�򱣴����ļ���,���Ǳ���Ӧ�þ�����ʾ������Ϣ
    if( (p = strstr(str,", CmdStr:"))
            || (p = strstr(str,",CmdStr:"))){
        p               = strchr(p,'C');
        char    *end    = strchr(p,'\n');
        char    title[80]   = "";
        int     len     = end ? end - p : strlen(p);
        if( len > 78 )
            len         = 78;
        memcpy(title,p,len);
        return exclist_push_simple_record(ctl,rectime,usetime,title,str);
    }
    else if( strstr(str,"Board Mem used over  limited")){
        return exclist_push_simple_record(ctl,rectime,usetime,"Board Mem used over limited",str);
    }
    else if( strstr(str,"Oops:")){
        return exclist_push_simple_record(ctl,rectime,usetime,"Kernel Oops",str);
    }
    else if( strstr(str,"BoardReset() is called by Application: ")){
        return exclist_push_simple_record(ctl,rectime,usetime,"BSP BoardReset",str);
    }
    else if( strstr(str,"do_IRQ")
            || strstr(str,"cpu_idle") ){
        return exclist_push_simple_record(ctl,rectime,usetime,"Kernel Message",str);
    }
    else{
        return exclist_push_simple_record(ctl,rectime,usetime,exc_tr(EXCSTR_UNKNOWN),str);
    }
}



static void apprec_init(T_AppExcRec *rec)
{
    if( rec ){
        memset(rec,0,sizeof(*rec));
        rec->pos[0]     = -1;
        rec->pos[1]     = -1;
        rec->pos[2]     = -1;
        rec->pos[3]     = -1;
        rec->signal.addr = -1;
    }
}

/** 
 * @brief �ͷ�һ���쳣��¼
 * 
 * @param rec   [in]    ���ͷŵļ�¼
 * 
 */
static void apprec_release(T_AppExcRec *rec)
{
    if( rec ){
        Exc_Free(rec->signal.stack.stackstr);
        Exc_Free(rec->signal.regstr);

        Exc_Free(rec->process.libs);

        Exc_Free(rec->task.stack.stackstr);

        Exc_Free(rec->job.msg.str);
        Exc_Free(rec->job.stack.stackstr);

        Exc_Free(rec->btfunc);
        Exc_Free(rec->lockstr);
        Exc_Free(rec->tipcstr);
        Exc_Free(rec->excstr);

        memset(rec,0,sizeof(*rec));
    }
    return ;
}


static int excblock_parse_appexc(struct exclist_ctl *ctl,char *str,const char *elfpath)
{
    while( str && *str){
        char    *rest   = NULL;
        T_AppExcRec   rec;
        apprec_init(&rec);

        int err = excstr2apprec(&rec,str,&rest);
        if( !err )
            err     = apprec_push2list(ctl,&rec,elfpath);
        apprec_release(&rec);
        if( err )
            return err;
        str         = rest;
    }

    return 0;
}


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
int Exc_Load(struct ExcList *exclist,const char *tmppath,const char *excstr, const char *elfpath)
{
    if( !exclist )
        return E_EXC_INVALID;

    memset(exclist,0,sizeof(*exclist));

    if( !tmppath || ! *tmppath || strlen(tmppath) > 200 
            || !excstr || !*excstr )
        return E_EXC_INVALID;

    struct exclist_ctl ctl;


    int ret = exclist_init(&ctl,tmppath);
    if( ret != 0 )
        return ret;

    char    *str        = Exc_StrDup(excstr);
    if( !str )
        ret = E_EXC_NOMEM;
    else{
        ret     = exclist_parse(&ctl,str,elfpath);
        Exc_Free(str);

        if( ret == 0 ){
            exclist->count  = ctl.list_index;
            exclist->list   = ctl.list;
            ctl.list_index  = 0;
            ctl.list        = NULL;
        }
    }

    exclist_release(&ctl);
    return ret;
}

void Exc_Release(struct ExcList *exclist)
{
    if( exclist && exclist->list){
        int i   = 0;
        for( i = 0; i < exclist->count; i ++ ){
            if( exclist->list[i] ){
                int j   = 0;
                struct ExcListItem    *item   = exclist->list[i];
                for( ; j < item->num ; j ++ )
                    unlink(item->info[j].path);
                Exc_Free(exclist->list[i]);
            }
        }
        Exc_Free(exclist->list);
        exclist->list   = NULL;
        exclist->count  = 0;
    }
}

