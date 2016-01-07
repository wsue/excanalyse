

#ifndef DASM_DLG_H_
#define DASM_DLG_H_

#include <QDialog>

#include "ui_dasmdlg.h"

class DasmDlg : public QDialog
{
    Q_OBJECT

    private:
        Ui::DasmDialog          m_ui;
        QString                 &m_curpath;

    public:
        DasmDlg(QWidget *parent,QString  &defpath);
        ~DasmDlg();

        private slots:
        void        on_openelf_clicked();
        void        on_execaction_clicked();
};

#endif


