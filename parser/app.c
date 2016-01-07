
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

//#include <mcheck.h>

#include "exc_parse.h"
#include "exc_error.h"
#include "parser_comm.h"

static void show_help(const char *appname);
static void parse_exc(const char *tmppath,const char *excfile, const char *libpath);
int main(int argc, char ** argv)
{
    //mtrace();

    if( argc < 3 ){
        show_help(argv[0]);
        return 1;
    }


    if( !strcmp(argv[1],"exc") ){
        if( argc < 3 ){
            show_help(argv[0]);
            return 1;
        }
        parse_exc(".", argv[2],argc > 3 ?argv[3] :NULL);
        return 0;
    }

    if( !strcmp(argv[1],"dasm") ){
        return Exc_Dasm(stdout,
                argv[2],EExcDasmAll,NULL);
    }

    if( !strcmp(argv[1],"flist") ){
        return Exc_Dasm(stdout,
                argv[2],EExcDasmFuncList,NULL);
    }

    if( !strcmp(argv[1],"fmatchlist") ){
        if( argc < 4 ){
            show_help(argv[0]);
            return 1;
        }

        return Exc_Dasm(stdout,
                argv[2],EExcDasmFuncMatchList,argv[3]);
    }

    if( !strcmp(argv[1],"fmatch") ){
        if( argc < 4 ){
            show_help(argv[0]);
            return 1;
        }

        return Exc_Dasm(stdout,
                argv[2],EExcDasmFuncMatch,argv[3]);
    }

    if( !strcmp(argv[1],"func") ){
        if( argc < 4 ){
            show_help(argv[0]);
            return 1;
        }

        return Exc_Dasm(stdout,
                argv[2],EExcDasmFunc,argv[3]);
    }
    if( !strcmp(argv[1],"addr") ){
        if( argc < 4 ){
            show_help(argv[0]);
            return 1;
        }

        return Exc_Dasm(stdout,
                argv[2],EExcDasmAddr,argv[3]);
    }

    //muntrace();
    return 0;
}

const char *exc_tr(const char *str){
    return str; 
}

static void show_help(const char *appname)
{
    printf("help : %s op args\n"
            "op include:\n"
            "   exc excrecord_file [[libpath]] : analyse exc record\n"
            "   dasm elffile : disassemble an elf file\n"
            "   flist elffile : show an elf file's function list\n"
            "   fmatchlist elffile funcname: show an elf file's function list that match speccial name\n"
            "   fmatch elffile funcname : disassemble all function include special name in the elf file\n"
            "   func elffile funcname : disassemble a function in the elf file\n"
            "   addr elffile addr : disassemble a function include special address in the elf file\n"
            ,
            appname);
}

static void parse_exc(const char *tmppath,const char *excfile, const char *libpath)
{
    int i   = 0;
    int             err;
    int             fd;
    int             size;
    char            *buf    = NULL;
    T_AppExcRec   rec;

    //
    //  读取异常记录
    //
    fd  = open(excfile,O_RDONLY);
    if( !fd ){
        printf("open excrecored file %s fail,errno:%d\n",excfile,errno);
        return ;
    }

    size    = lseek(fd,0,SEEK_END); //  得到文件大小
    if( size > 0 ){
        if( lseek(fd,SEEK_SET,0) != 0 ){
            //  把文件指针重指向文件头失败
            printf("reseek file to begin fail,errno:%d\n",errno);
        }
        else{
            buf = Exc_Malloc(size+1);
            if( !buf ){
                printf("malloc buf %d fail,errno:%d\n",size+1,errno);
            }
            else{
                //  如果分配内存成功,那读文件内容到buf中
                int len = read(fd,buf,size);
                if( len < 1 ){
                    //  读文件失败
                    printf("read file %d size fail(=%d), errno:%d\n",size,len,errno);
                    Exc_Free(buf);
                    buf         = NULL;
                }
                else{
                    buf[len]    = 0;    //  保证字符串0结束
                }
            }
        }
    }

    close(fd);

    if( !buf ){
        //  没有读异常记录成功，返回
        return ;
    }


    struct ExcList exclist;
    err = Exc_Load(&exclist,tmppath,buf,libpath);
    Exc_Free(buf);

    if( err >= 0 ){
        printf("load exc succ, save ret to %s ,ret:%d cnt:%d\n",tmppath,err,exclist.count);
        for( i = 0; i < exclist.count ; i ++ ){
            printf("rec %d %d:%s>\n",i,exclist.list[i]->num,exclist.list[i]->desc);
        }
    }
    else
        printf("load exc err %d\n",err);

    printf("press any key to show detail list\n");
    getchar();

    for( i = 0; i < exclist.count ; i++ ){
        int j   = 0;
        struct ExcListItem    *item   = exclist.list[i];
        printf("###########\tt[%d]\t%s\n",i,item->desc);
        for( j = 0; j < item->num ; j++){
            char    buf[1028]   = "";
            fd  = open(item->info[j].path,O_RDONLY);
            if( fd == -1 ){
                printf("open file %d %d %s fail,exit\n",i,j,item->info[j].path);
                exit(1);
            }
            printf(">>>>>\t[%d:%d]\t%s\n",i,j,item->info[j].title);
            while(read(fd,buf,1024) >0 ){
                printf(buf);
            }
            close(fd);
            printf("\n");
        }
    }
    Exc_Release(&exclist);
}
