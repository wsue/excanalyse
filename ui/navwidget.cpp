
#include <QTableWidget> 
#include <QMessageBox>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>
#include <QStringListModel>
#include <QDesktopServices>
#include <QTextCodec>
#include <QFile>

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h> 
#include <coreplugin/actionmanager/actionmanager.h> 
#include <coreplugin/actionmanager/actioncontainer.h> 
#include <coreplugin/actionmanager/command.h> 
#include <coreplugin/modemanager.h> 
#include <coreplugin/icore.h>
#include <texteditor/basetexteditor.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "navwidget.h"
#include "excloaddlg.h"
#include "dasmdlg.h"

using namespace Core;
using namespace TextEditor;

ExcInfoManager::ExcInfoManager():
    m_plist(new ExcList),
    m_selectionModel(new QItemSelectionModel(this, this))
{
    memset(m_plist,0,sizeof(*m_plist));
}

ExcInfoManager::~ExcInfoManager()
{
    ResetList();
    delete  m_plist;
}

// Model stuff
QModelIndex ExcInfoManager::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
        return QModelIndex();
    else
        return createIndex(row, column, 0);
}

QModelIndex ExcInfoManager::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int ExcInfoManager::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return m_ItemList.count();
}

int ExcInfoManager::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant ExcInfoManager::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_ItemList.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return m_ItemList.at(index.row())->Title;


    return QVariant();
}

Qt::ItemFlags ExcInfoManager::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}


//
//  其它接口
void ExcInfoManager::AddItem(Item    *it)
{
    beginInsertRows(QModelIndex(), m_ItemList.size(), m_ItemList.size());

    m_ItemList.append(it);

    endInsertRows();
}


bool ExcInfoManager::SetList(QString excinfo,QString sopath)
{
    ResetList();

    QString tmp = QDesktopServices::storageLocation(QDesktopServices::TempLocation);

    int ret = Exc_Load(m_plist,tmp.toLatin1().data(),
            excinfo.toLatin1().data(),
            sopath.toLatin1().data());
    if( ret < 0){
        char    buf[256];
        sprintf(buf,exc_tr("Analyse Exc fail, errno:%d"),ret);
        QMessageBox::information( 0 /* Core::ICore::instance()->mainWindow()  */,
                exc_tr("Analyse Exc fail"), buf);
        return false;
    }

    QTextCodec *codec = QTextCodec::codecForLocale();
    
    for( int i = 0 ; i < m_plist->count ; i ++ ){
        ExcListItem    *pinfo  = m_plist->list[i];
        if( pinfo->num > 0 ){
            AddItem(codec->toUnicode(pinfo->desc),pinfo->info[0].path);
            for( int j = 0 ; j < pinfo->num ; j ++ ){
                AddItem(codec->toUnicode(pinfo->info[j].title),pinfo->info[j].path);
            }
        }
    }

    return true;
}

void ExcInfoManager::ResetList()
{
    Core::EditorManager *editorManager = Core::EditorManager::instance();    
    foreach (Item *item, m_ItemList) {
        const QList<Core::IEditor *> editors = editorManager->editorsForFileName(item->Info);
        editorManager->closeEditors(editors,false);
        delete item;
    }

    m_ItemList.clear();
    Exc_Release(m_plist);
}

QVariant ExcInfoManager::PathforIndex(QModelIndex index)
{
    if (index.row() < 0 || index.row() >= m_ItemList.size())
        return QVariant();

    return m_ItemList.at(index.row())->Info;
}









 
ExcInfoModel::TreeItem::TreeItem(const QString &title,
        const QString &file, ExcInfoModel::TreeItem *parent )
{
    parentItem  = parent;
    Title       = title;
    File        = file;
}

ExcInfoModel::TreeItem::~TreeItem()
{
    //  先关闭窗口
    if( !File.isEmpty() ){
        Core::EditorManager *editorManager  = Core::EditorManager::instance();    
        const QList<Core::IEditor *> editors= editorManager->editorsForFileName(File);
        editorManager->closeEditors(editors,false);
    }

    qDeleteAll(childItems);
}
//! [1]

//! [2]
void ExcInfoModel::TreeItem::appendChild(ExcInfoModel::TreeItem *item)
{
    childItems.append(item);
}
//! [2]

//! [3]
ExcInfoModel::TreeItem *ExcInfoModel::TreeItem::child(int row)
{
    return childItems.value(row);
}
//! [3]

//! [4]
int ExcInfoModel::TreeItem::childCount() const
{
    return childItems.count();
}
//! [4]


//! [6]
QVariant ExcInfoModel::TreeItem::data(int column) const
{
    return Title;
}
//! [6]

//! [7]
ExcInfoModel::TreeItem *ExcInfoModel::TreeItem::parent()
{
    return parentItem;
}
//! [7]

//! [8]
int ExcInfoModel::TreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}
//! [8]
           

//! [0]
ExcInfoModel::ExcInfoModel(QObject *parent):
     QAbstractItemModel(parent),
     m_plist(new ExcList),
     m_selectionModel(new QItemSelectionModel(this, this))
{
    QTextCodec *codec = QTextCodec::codecForLocale();
    rootItem    = new   TreeItem(codec->toUnicode("异常记录"),QString());

    memset(m_plist,0,sizeof(*m_plist));
}
//! [0]

//! [1]
ExcInfoModel::~ExcInfoModel()
{
    ResetList();
    delete rootItem;
}
//! [1]

//! [2]
int ExcInfoModel::columnCount(const QModelIndex &parent) const
{
    return 1;
    /*
       if (parent.isValid())
       return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
       else
       return rootItem->columnCount();
       */
}
//! [2]

//! [3]
QVariant ExcInfoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column());
}

QVariant ExcInfoModel::file(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item->file();
}
//! [3]

//! [4]
Qt::ItemFlags ExcInfoModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//! [4]

//! [5]
QVariant ExcInfoModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}
//! [5]

//! [6]
QModelIndex ExcInfoModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
//! [6]

//! [7]
QModelIndex ExcInfoModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}
//! [7]

//! [8]
int ExcInfoModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}
//! [8]


#define DBGTRACE(fmt,arg...)    //printf("[%s:%d]\t"fmt "\n",__func__,__LINE__,##arg)

bool ExcInfoModel::SetList(QString excinfo,QString sopath)
{
    beginResetModel (); 
    ResetList();

    QString tmp = QDesktopServices::storageLocation(QDesktopServices::TempLocation);

    int ret = Exc_Load(m_plist,tmp.toLatin1().data(),
            excinfo.toLatin1().data(),
            sopath.toLatin1().data());
    DBGTRACE("load rec ret:%d count:%d %x",ret,m_plist->count,m_plist->list);
    if( ret < 0){
        endResetModel ();

        char    buf[256];
        sprintf(buf,exc_tr("Analyse Exc fail, errno:%d"),ret);
        QMessageBox::information( 0 /* Core::ICore::instance()->mainWindow()  */,
                exc_tr("Analyse Exc fail"), buf);
        return false;
    }

    DBGTRACE("write orig");

    tmp             += "/exc_orig.txt";
    FILE            *fp = fopen(tmp.toLatin1().data(),"w");
    if( fp ){
        fwrite(excinfo.toLatin1().data(),excinfo.toLatin1().size(),1,fp);
        fclose(fp);
    }

    QTextCodec *codec = QTextCodec::codecForLocale();

    DBGTRACE("add root");
    TreeItem    *item   = new TreeItem(
            codec->toUnicode("原始异常"),tmp,rootItem);
    rootItem->appendChild(item);

    DBGTRACE("add items");
    for( int i = 0 ; i < m_plist->count ; i ++ ){
        ExcListItem    *pinfo  = m_plist->list[i];
        DBGTRACE("add item %d %x",i,pinfo);
        if( pinfo && pinfo->num > 0 ){
            DBGTRACE("add item %d %d %lx",i,pinfo->num,pinfo);

            TreeItem    *item   = new TreeItem(
                    codec->toUnicode(pinfo->desc),pinfo->info[0].path,rootItem);
            rootItem->appendChild(item);

            DBGTRACE("add item %d child",i);
            for( int j = 1 ; j < pinfo->num ; j ++ ){
                TreeItem    *child   = new TreeItem(
                        codec->toUnicode(pinfo->info[j].title),
                        pinfo->info[j].path,item);
                item->appendChild(child);
            }
        }
    }

    endResetModel ();
    return true;
}

void ExcInfoModel::ResetList()
{
    delete  rootItem;
    Exc_Release(m_plist);

    QTextCodec *codec = QTextCodec::codecForLocale();
    rootItem    = new TreeItem(codec->toUnicode("异常记录分析"),QString());
}

QVariant ExcInfoModel::PathforIndex(QModelIndex index)
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item ? item->file() : QVariant();
}






void ExcInfoTreeView::ShowItem(const QModelIndex &index)
{
    QString path    = m_infoman->PathforIndex(index).toString();
    if( !path.isEmpty() ){
        BaseTextEditor::openEditorAt(path, 1);
    }
}

void ExcInfoTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    //
    //  注册弹出菜单
    //
    QMenu menu;
    QTextCodec *codec = QTextCodec::codecForLocale();

    QAction *PopMenu = menu.addAction(codec->toUnicode("读取新异常记录"));
    connect(PopMenu, SIGNAL(triggered()),this, SLOT(PopDialog()));

    PopMenu = menu.addAction(codec->toUnicode("反汇编功能"));
    connect(PopMenu, SIGNAL(triggered()),this, SLOT(PopDasmDialog()));

    menu.exec(mapToGlobal(event->pos()));
}

void ExcInfoTreeView::PopDialog()
{
    ExcLoadDlg  dlg(this,m_defpath);
    if( dlg.exec() ){
        m_infoman->SetList(dlg.GetExcInfo(),
                dlg.GetExcLibPath());
    }
}

void ExcInfoTreeView::PopDasmDialog()
{
    DasmDlg dlg(this,m_defpath);
    dlg.exec();
}

ExcInfoTreeView::ExcInfoTreeView():m_infoman(NULL),m_defpath(".")
{
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);

    connect(this, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(ShowItem(const QModelIndex &)));

    Core::ActionManager * am    = Core::ICore::instance()->actionManager(); 
    QTextCodec          *codec  = QTextCodec::codecForLocale();
    Core::Command* dasmcmd = am->registerAction(new QAction(this), 
            "ExcPlugin.ShowDasmUI", 
            QList<int>() << Core::Constants::C_GLOBAL_ID); 
    dasmcmd->action()->setText(codec->toUnicode("反汇编功能")); 
    am->actionContainer(Core::Constants::M_TOOLS)->addAction(dasmcmd); 
    connect(dasmcmd->action(), SIGNAL(triggered(bool)), this, SLOT(PopDasmDialog())); 
}

ExcInfoTreeView::~ExcInfoTreeView()
{
}

void ExcInfoTreeView::setModel(QAbstractItemModel *model)
{
    ExcInfoAbstractModel *manager = qobject_cast<ExcInfoAbstractModel *>(model);
    m_infoman = manager;
    QTreeView::setModel(model);
    QTreeView::setHeaderHidden(true);
    setSelectionModel(manager->selectionModel());
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}









NavWidgetFactory::NavWidgetFactory() : m_infoman(new ExcInfoAbstractModel()) { } 
NavWidgetFactory::~NavWidgetFactory() 
{
    delete  m_infoman;
} 

Core::NavigationView NavWidgetFactory::createWidget() 
{ 

    ExcInfoTreeView *excview = new ExcInfoTreeView();
    excview->setModel((QAbstractItemModel *)(m_infoman));
    Core::NavigationView view;
    view.widget = excview;

    return view; 
} 

QString NavWidgetFactory::id() const
{ 
    return "ExcNavId"; 
} 

QString NavWidgetFactory::displayName() const
{ 
    QTextCodec *codec = QTextCodec::codecForLocale();
    return codec->toUnicode("异常分析");
} 

