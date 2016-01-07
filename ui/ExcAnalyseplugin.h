#ifndef DONOTHINGPLUGIN_H 
#define DONOTHINGPLUGIN_H 
 
#include <extensionsystem/iplugin.h> 
 
class ExcAnalysePlugin : public ExtensionSystem::IPlugin 
{ 
    Q_OBJECT

    public: 
        ExcAnalysePlugin(); 
        virtual ~ExcAnalysePlugin(); 

        bool initialize(const QStringList & arguments, QString * errorString); 
        void extensionsInitialized(); 
        void shutdown(); 

    private slots:
        void showui();
}; 
 
#endif // DONOTHINGPLUGIN_H 

