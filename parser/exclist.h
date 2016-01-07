
#ifndef EXC_LIST_H_
#define EXC_LIST_H_

#define TOKEN_BEGIN_SECTION         "********** Begin of Record **************"
#define TOKEN_END_SECTION           "******** End   of Record ********"
#define TOKEN_APPEXCEND_SECTION     "********** End   of Exception ********"
#define TOKEN_DEADLOOP_STR          "Find dead loop in thread"
#define TOKEN_DEADLOCK_STR          "Find dead lock in thread"
#define TOKEN_APPEXC_STR            "Exception Process Result"

struct exclist_ctl *ctl;
int exclist_fmt_path(char *path,int pathsize,const struct exclist_ctl *ctl,const char *name);
int exclist_push_item(struct exclist_ctl *ctl,
        const char *rectime,const char *usetime,const char *title,struct ExcListItem  *item);

int apprec_push2list(struct exclist_ctl *ctl,
        const T_AppExcRec *rec, const char *libpath);
int excstr2apprec(T_AppExcRec *rec,char *pstr, char **rest);

#endif

