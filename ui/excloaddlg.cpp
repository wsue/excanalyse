
#include <QFileDialog>
#include <QTextCodec>

#include "excloaddlg.h"

ExcLoadDlg::ExcLoadDlg(QWidget *parent,QString  &defpath):
    QDialog(parent) , m_curpath(defpath)
{

    m_ui.setupUi(this);

    //  关联事件
    //  2.  用户点打开"加载库路径"按钮
    connect(m_ui.OpenLib, SIGNAL(clicked()), this, SLOT(on_openlib_clicked()));
    //  3.  用户点"从文件中读"时充许从文件中读异常
    connect(m_ui.LoadExcButton, SIGNAL(clicked()), this, SLOT(on_openexc_clicked()));
}

ExcLoadDlg::~ExcLoadDlg()
{
}



void        ExcLoadDlg::on_openexc_clicked()
{
    if( m_excpath.isEmpty() ){
        m_excpath   = m_curpath;
    }
 
    QTextCodec *codec = QTextCodec::codecForLocale();
    
    QString name = QFileDialog::getOpenFileName(this,
            codec->toUnicode("打开异常记录文件"), m_excpath, tr("txt Files (Exc_Omp.txt Exc_pp.txt *.txt);;all files(*.*)"));
    if(!name.isEmpty()) {
        QFileInfo   file(name);
        m_curpath   = file.path();
        m_excpath   = m_curpath;

        QFile scriptFile(name);
        scriptFile.open(QIODevice::ReadOnly);    
        m_ui.ExcInfo->setPlainText(scriptFile.readAll());
        scriptFile.close();

        if( m_libpath.isEmpty() ){
            m_libpath   = m_curpath;
            m_ui.LibPath->setText(m_libpath);
        }
    }
}

void  ExcLoadDlg::on_openlib_clicked()
{
    if( m_libpath.isEmpty() ){
        m_libpath   = m_curpath;
    }

    QTextCodec *codec = QTextCodec::codecForLocale();
    
    QString name = QFileDialog::getExistingDirectory(
            this, codec->toUnicode("打开异常对应的elf文件和库路径"), m_libpath );
    if(!name.isEmpty()) {
        m_curpath   = name;
        m_libpath   = name;
        m_ui.LibPath->setText(name);
    }
}



