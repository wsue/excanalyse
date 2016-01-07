
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTextCodec>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dasmdlg.h"
#include "exc_parse.h"

DasmDlg::DasmDlg(QWidget *parent,QString  &defpath):
    QDialog(parent) , m_curpath(defpath)
{

    m_ui.setupUi(this);

    //  关联事件
    //  1.  用户点“打开”按钮
    connect(m_ui.OpenElf, SIGNAL(clicked()), this, SLOT(on_openelf_clicked()));
    //  2.  用户点"执行"按钮
    connect(m_ui.ExecAct, SIGNAL(clicked()), this, SLOT(on_execaction_clicked()));
    connect(m_ui.CloseButton, SIGNAL(clicked()), this, SLOT(close()));
}

DasmDlg::~DasmDlg()
{
}

void        DasmDlg::on_openelf_clicked()
{
    QTextCodec *codec = QTextCodec::codecForLocale();
    QString name = QFileDialog::getOpenFileName(this,
            codec->toUnicode("选择要反汇编的文件"), m_curpath, tr("Elf Files (*.exe *.so *.dll);;all files(*.*)"));
    if(!name.isEmpty()) {
        QFileInfo   file(name);
        m_curpath   = file.path();
        m_ui.ElfName->setText(name);
    }
}

void        DasmDlg::on_execaction_clicked()
{
    QTextCodec *codec = QTextCodec::codecForLocale();

    QString elffile     = m_ui.ElfName->text().trimmed() ;
    QString param       = m_ui.ActParam->text().trimmed ();
    enum EExcDasmMode actmod = EExcDasmAll;

    switch(m_ui.ActMode->currentIndex() ){
        case 0: actmod  = EExcDasmAll; break;
        case 1: actmod  = param.isEmpty() ? EExcDasmFuncList : EExcDasmFuncMatchList;
                break;
        case 2: actmod  = EExcDasmFuncMatch; break;
        case 3: actmod  = EExcDasmFunc;    break;
        case 4: actmod  = EExcDasmAddr;    break;
        default:    break;
    }

    if( elffile.isEmpty() ){
        QMessageBox::warning( 0 /* Core::ICore::instance()->mainWindow()  */,
                codec->toUnicode("无法执行操作"),
                codec->toUnicode("未指定elf文件名"));
        return ;
    }

    if( param.isEmpty() 
            && (actmod == EExcDasmFuncMatch 
                || actmod == EExcDasmFunc 
                || actmod == EExcDasmAddr ) ){
        QMessageBox::warning( 0 /* Core::ICore::instance()->mainWindow()  */,
                codec->toUnicode("无法执行操作"),
                actmod == EExcDasmAddr ? 
                codec->toUnicode("操作需要传入一个数字类型的参数")
                :codec->toUnicode("操作需要传入字符串参数"));
        return ;
    }

    QString tmp ;
    tmp = QDesktopServices::storageLocation(QDesktopServices::TempLocation)+ "/exc_dasm.txt";

    if( actmod  == EExcDasmAll ){
        QString name = QFileDialog::getSaveFileName(this,
                codec->toUnicode("全部反汇编需要的时间较长，你想把反汇编结果另存到文件吗?"), m_curpath, tr("txt Files (*.txt)"));
        if(!name.isEmpty()) {
            tmp = name;
        }
    }

    FILE    *fp = fopen(tmp.toLatin1().data(),"w+");
    if( !fp ){
        QMessageBox::warning  ( 0 /* Core::ICore::instance()->mainWindow()  */,
                codec->toUnicode("无法执行操作"),
                codec->toUnicode("创建输出文件失败"));
        return ;
    }

    int ret     = Exc_Dasm(fp,elffile.toLatin1().data(),
            actmod,param.toLatin1().data());

    QFile scriptFile;
    fseek(fp,0,SEEK_SET);
    scriptFile.open(fp,QIODevice::ReadOnly);   

    m_ui.textEdit->setPlainText(scriptFile.readAll());
    scriptFile.close();

    if( ret != 0 ){
        QString  err;
        err.sprintf("analyse error code:%d",ret);
        QMessageBox::warning( 0 /* Core::ICore::instance()->mainWindow()  */,
                exc_tr("dasm fail"),
                err);
    }
}

