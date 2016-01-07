
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
     *      ���Ӳ˵���ʾ,�����Ƽ���һ��ʹ�����淽��
     */
    Core::ActionManager* am = Core::ICore::instance()->actionManager(); 
    Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_TOOLS);

    QAction* DemoDLMenu = ac->menu()->addAction("DemoDLMenu"); 

    //
    //  ��һ��ʵ�ַ���

    /*
     *      ���Ӳ˵���ʾ�ĵڶ��ַ�����
     *      ����ʾ�˵�ͬʱ�涨��������Ϊ
     */

    //  �õ�����������
    Core::ActionManager* am = Core::ICore::instance()->actionManager(); 

    /*
     *  ͨ����������������������,������������ʾ���ı�
     */
    Core::Command* cmd = am->registerAction(new QAction(this), 
            "DoNothingPlugin.DemoDLMenu", 
            QList<int>() <<  
            Core::Constants::C_GLOBAL_ID); 
    cmd->action()->setText("DemoDL Menu"); 

    /*
     *  ��������ӵ��˵���
     */
    am->actionContainer(Core::Constants::M_TOOLS)->addAction(cmd); 

    /*
     *  ���������봦����
     */
    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(about())); 
#endif    
#if 0


#endif

    /*      ����һ��NavWidgetFactory����    */
    addAutoReleasedObject(new NavWidgetFactory); 

    Core::ActionManager * am    = Core::ICore::instance()->actionManager(); 
    QTextCodec          *codec  = QTextCodec::codecForLocale();
    
    /*
     *  ͨ����������������������,������������ʾ���ı�
     */
    Core::Command* uicmd = am->registerAction(new QAction(this), 
            "ExcPlugin.ShowExcUI", 
            QList<int>() <<  
            Core::Constants::C_GLOBAL_ID); 

    uicmd->action()->setText(codec->toUnicode("�쳣��¼����")); 
    /*
     *  ��������ӵ��˵���
     */
    am->actionContainer(Core::Constants::M_TOOLS)->addAction(uicmd); 

    /*
     *  ���������봦����
     */
    connect(uicmd->action(), SIGNAL(triggered(bool)), this, SLOT(showui())); 
    

    return true; 
} 



void ExcAnalysePlugin::extensionsInitialized() 
{ 

    // Do nothing 
} 
 

/*	插件被卸载时调用
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
            codec->toUnicode("�쳣��������"),
            codec->toUnicode("�������û��ʾ�����˵�\"Windows\"->\"Show SideBar\",Ȼ���ڵ�������ѡ��\"�쳣����\""));

}











//	注册自己的行为
Q_EXPORT_PLUGIN(ExcAnalysePlugin) 

