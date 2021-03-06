
#include "ExcAnalyseplugin.h" 
#include <QtPlugin> 
#include <QStringList> 
#include <QMenu> 
#include <QMessageBox>
#include <QKeySequence> 
#include <QTextCodec>

#include <coreplugin/coreconstants.h> 
#include <coreplugin/actionmanager/actionmanager.h> 
#include <coreplugin/actionmanager/actioncontainer.h> 
#include <coreplugin/actionmanager/command.h> 
#include <coreplugin/modemanager.h> 
#include <coreplugin/icore.h> 
 
#include "navwidget.h"

 

const char *exc_tr(const char *str)
{
    return str;
}

ExcAnalysePlugin::ExcAnalysePlugin() 
{ 
	// Do notning 
} 


ExcAnalysePlugin::~ExcAnalysePlugin() 
{ 
	// Do notning 
} 



bool ExcAnalysePlugin::initialize(const QStringList& args, QString *errMsg) 
{ 
    Q_UNUSED(args); 
    Q_UNUSED(errMsg); 
#if 0
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());  
    QTextCodec::setCodecForTr(QTextCodec::codecForName("gbk")); 
#endif
#if 0
    /*
     *      增加菜单显示,但不推荐，一般使用下面方法
     */
    Core::ActionManager* am = Core::ICore::instance()->actionManager(); 
    Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_TOOLS);

    QAction* DemoDLMenu = ac->menu()->addAction("DemoDLMenu"); 

    //
    //  另一种实现方法

    /*
     *      增加菜单显示的第二种方法，
     *      在显示菜单同时规定了它的行为
     */

    //  得到动作管理器
    Core::ActionManager* am = Core::ICore::instance()->actionManager(); 

    /*
     *  通过动作管理器创建新命令,并设置命令显示的文本
     */
    Core::Command* cmd = am->registerAction(new QAction(this), 
            "DoNothingPlugin.DemoDLMenu", 
            QList<int>() <<  
            Core::Constants::C_GLOBAL_ID); 
    cmd->action()->setText("DemoDL Menu"); 

    /*
     *  把命令添加到菜单上
     */
    am->actionContainer(Core::Constants::M_TOOLS)->addAction(cmd); 

    /*
     *  关联动作与处理函数
     */
    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(about())); 
#endif    
#if 0


#endif

    /*      增加一个NavWidgetFactory对象    */
    addAutoReleasedObject(new NavWidgetFactory); 

    Core::ActionManager * am    = Core::ICore::instance()->actionManager(); 
    QTextCodec          *codec  = QTextCodec::codecForLocale();
    
    /*
     *  通过动作管理器创建新命令,并设置命令显示的文本
     */
    Core::Command* uicmd = am->registerAction(new QAction(this), 
            "ExcPlugin.ShowExcUI", 
            QList<int>() <<  
            Core::Constants::C_GLOBAL_ID); 

    uicmd->action()->setText(codec->toUnicode("异常记录分析")); 
    /*
     *  把命令添加到菜单上
     */
    am->actionContainer(Core::Constants::M_TOOLS)->addAction(uicmd); 

    /*
     *  关联动作与处理函数
     */
    connect(uicmd->action(), SIGNAL(triggered(bool)), this, SLOT(showui())); 
    

    return true; 
} 



void ExcAnalysePlugin::extensionsInitialized() 
{ 

    // Do nothing 
} 
 

/*	鎻掍欢琚嵏杞芥椂璋冪敤
*/
void ExcAnalysePlugin::shutdown() 
{ 
    // Do nothing 
} 
 


void ExcAnalysePlugin::showui()
{
    Core::ICore::instance()->modeManager()->activateMode(Core::Constants::MODE_EDIT);

    QTextCodec *codec = QTextCodec::codecForLocale();
    QMessageBox::information(NULL,
            codec->toUnicode("异常分析工具"),
            codec->toUnicode("如果界面没显示，请点菜单\"Windows\"->\"Show SideBar\",然后在导航条中选择\"异常分析\""));

}











//	娉ㄥ唽鑷繁鐨勮涓�
Q_EXPORT_PLUGIN(ExcAnalysePlugin) 

