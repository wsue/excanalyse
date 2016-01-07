
#ifndef LOAD_EXC_DLG_H_
#define LOAD_EXC_DLG_H_

#include <QDialog>

#include "ui_excloaddlg.h"

class ExcLoadDlg : public QDialog
{
    Q_OBJECT

    public:
        ExcLoadDlg(QWidget *parent,QString  &defpath);
        ~ExcLoadDlg();

    public:
        
        QString         GetExcInfo(){   return m_ui.ExcInfo->toPlainText(); } 
        QString         GetExcLibPath(){return m_ui.LibPath->text();}
        private slots:
            void        on_openlib_clicked();
            void        on_openexc_clicked();
    private:
            Ui::ExcLoadDialog  m_ui;
            QString                 &m_curpath;

            QString                 m_excpath;
            QString                 m_libpath;
};

#endif


