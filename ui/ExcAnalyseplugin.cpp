
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
     *      Ôö¼Ó²Ëµ¥ÏÔÊ¾,µ«²»ÍÆ¼ö£¬Ò»°ãÊ¹ÓÃÏÂÃæ·½·¨
     */
    Core::ActionManager* am = Core::ICore::instance()->actionManager(); 
    Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_TOOLS);

    QAction* DemoDLMenu = ac->menu()->addAction("DemoDLMenu"); 

    //
    //  ÁíÒ»ÖÖÊµÏÖ·½·¨

    /*
     *      Ôö¼Ó²Ëµ¥ÏÔÊ¾µÄµÚ¶þÖÖ·½·¨£¬
     *      ÔÚÏÔÊ¾²Ëµ¥Í¬Ê±¹æ¶¨ÁËËüµÄÐÐÎª
     */

    //  µÃµ½¶¯×÷¹ÜÀíÆ÷
    Core::ActionManager* am = Core::ICore::instance()->actionManager(); 

    /*
     *  Í¨¹ý¶¯×÷¹ÜÀíÆ÷´´½¨ÐÂÃüÁî,²¢ÉèÖÃÃüÁîÏÔÊ¾µÄÎÄ±¾
     */
    Core::Command* cmd = am->registerAction(new QAction(this), 
            "DoNothingPlugin.DemoDLMenu", 
            QList<int>() <<  
            Core::Constants::C_GLOBAL_ID); 
    cmd->action()->setText("DemoDL Menu"); 

    /*
     *  °ÑÃüÁîÌí¼Óµ½²Ëµ¥ÉÏ
     */
    am->actionContainer(Core::Constants::M_TOOLS)->addAction(cmd); 

    /*
     *  ¹ØÁª¶¯×÷Óë´¦Àíº¯Êý
     */
    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(about())); 
#endif    
#if 0


#endif

    /*      Ôö¼ÓÒ»¸öNavWidgetFactory¶ÔÏó    */
    addAutoReleasedObject(new NavWidgetFactory); 

    Core::ActionManager * am    = Core::ICore::instance()->actionManager(); 
    QTextCodec          *codec  = QTextCodec::codecForLocale();
    
    /*
     *  Í¨¹ý¶¯×÷¹ÜÀíÆ÷´´½¨ÐÂÃüÁî,²¢ÉèÖÃÃüÁîÏÔÊ¾µÄÎÄ±¾
     */
    Core::Command* uicmd = am->registerAction(new QAction(this), 
            "ExcPlugin.ShowExcUI", 
            QList<int>() <<  
            Core::Constants::C_GLOBAL_ID); 

    uicmd->action()->setText(codec->toUnicode("Òì³£¼ÇÂ¼·ÖÎö")); 
    /*
     *  °ÑÃüÁîÌí¼Óµ½²Ëµ¥ÉÏ
     */
    am->actionContainer(Core::Constants::M_TOOLS)->addAction(uicmd); 

    /*
     *  ¹ØÁª¶¯×÷Óë´¦Àíº¯Êý
     */
    connect(uicmd->action(), SIGNAL(triggered(bool)), this, SLOT(showui())); 
    

    return true; 
} 



void ExcAnalysePlugin::extensionsInitialized() 
{ 

    // Do nothing 
} 
 

/*	æ’ä»¶è¢«å¸è½½æ—¶è°ƒç”¨
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
            codec->toUnicode("Òì³£·ÖÎö¹¤¾ß"),
            codec->toUnicode("Èç¹û½çÃæÃ»ÏÔÊ¾£¬Çëµã²Ëµ¥\"Windows\"->\"Show SideBar\",È»ºóÔÚµ¼º½ÌõÖÐÑ¡Ôñ\"Òì³£·ÖÎö\""));

}











//	æ³¨å†Œè‡ªå·±çš„è¡Œä¸º
Q_EXPORT_PLUGIN(ExcAnalysePlugin) 

