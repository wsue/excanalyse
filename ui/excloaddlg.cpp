
#include <QFileDialog>
#include <QTextCodec>

#include "excloaddlg.h"

ExcLoadDlg::ExcLoadDlg(QWidget *parent,QString  &defpath):
    QDialog(parent) , m_curpath(defpath)
{

    m_ui.setupUi(this);

    //  �����¼�
    //  2.  �û����"���ؿ�·��"��ť
    connect(m_ui.OpenLib, SIGNAL(clicked()), this, SLOT(on_openlib_clicked()));
    //  3.  �û���"���ļ��ж�"ʱ������ļ��ж��쳣
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
            codec->toUnicode("���쳣��¼�ļ�"), m_excpath, tr("txt Files (Exc_Omp.txt Exc_pp.txt *.txt);;all files(*.*)"));
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
            this, codec->toUnicode("���쳣��Ӧ��elf�ļ��Ϳ�·��"), m_libpath );
    if(!name.isEmpty()) {
        m_curpath   = name;
        m_libpath   = name;
        m_ui.LibPath->setText(name);
    }
}



