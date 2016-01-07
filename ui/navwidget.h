
#ifndef NAV_WIDGET_H_
#define NAV_WIDGET_H_

#include <coreplugin/inavigationwidgetfactory.h> 

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>
#include <QtGui/QListView>
#include <QtGui/QTreeView>
#include <QAbstractItemModel>
#include <QVector>

#include "exc_error.h"
#include "exc_parse.h"

namespace ProjectExplorer {
class SessionManager;
}

namespace Core {
class IEditor;
}

namespace TextEditor {
    class ITextEditor;
}

/** 
 * @brief �쳣��Ϣ������������listview��ʾ������
 * ��MVC�ܹ��ж�Ӧmodel
 */
struct ExcListItem;

//#define       ExcInfoAbstractModel  ExcInfoManager
#define       ExcInfoAbstractModel  ExcInfoModel

class ExcInfoManager :public QAbstractListModel
{
    Q_OBJECT

    private:
        struct Item{
            QString Title;  /*  ��ʾ����    */
            QString Info;   /*  ��Ӧ������  */
            public:
            Item(){}
            Item(const char *title):Title(title?title:""){}
            Item(QString &title):Title(title){}
            Item(const char *title,const char *info): 
                Title(title?title:""),
                Info(info?info:""){
                }
            Item(const QString &title,const QString &info): 
                Title(title),
                Info(info){
                }
        };

    public:
        ExcInfoManager();
        ~ExcInfoManager();

        // Model stuff
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex &child) const;
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;

        QItemSelectionModel *selectionModel() const
        {
            return m_selectionModel;
        }


        //  ���ò���
        bool SetList(QString excinfo,QString sopath);
        QVariant PathforIndex(QModelIndex index);

    private:
        void ResetList();

        
        void AddItem(Item *item);

        //
        //  �ӿ�
        void AddItem(const char *name,const char *info){
            AddItem(new Item(name,info));
        }
        void AddItem(const QString &name,const QString &info){
            AddItem(new Item(name,info));
        }



        QList<Item *> m_ItemList;
        ExcList       *m_plist;     //  �洢��ǰ��ʾ������Ϣ
        QItemSelectionModel *m_selectionModel;
};


class ExcInfoModel : public QAbstractItemModel
{
    Q_OBJECT

    public:
        ExcInfoModel(QObject *parent = 0);
        ~ExcInfoModel();

        QVariant data(const QModelIndex &index, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        QVariant headerData(int section, Qt::Orientation orientation,
                int role = Qt::DisplayRole) const;
        QModelIndex index(int row, int column,
                const QModelIndex &parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex &index) const;
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;

        QItemSelectionModel *selectionModel() const
        {
            return m_selectionModel;
        }
        bool SetList(QString excinfo,QString sopath);
        QVariant PathforIndex(QModelIndex index);

    private:
        void ResetList();
        QVariant file(const QModelIndex &index, int role) const;

        class TreeItem
        {
            public:
                TreeItem(const QString &title,const QString &file, TreeItem *parent = 0);
                ~TreeItem();

                void appendChild(TreeItem *child);

                TreeItem *child(int row);
                int childCount() const;
                int columnCount() const{    return 1;}
                QVariant data(int column) const;
                QVariant file() const{return File; }

                int row() const;
                TreeItem *parent();

            private:
                //  ������
                QString         Title;
                QString         File;

                //  ��ʾ��
                QList<TreeItem*> childItems;
                TreeItem        *parentItem;
        };

        ExcList       *m_plist;     //  �洢��ǰ��ʾ������Ϣ
        TreeItem *rootItem;
        QItemSelectionModel *m_selectionModel;
};

/** 
 * @brief ��ʾ�쳣��Ϣ�б�
 * MVCģ���ж�ӦV
 */
class ExcInfoTreeView : public QTreeView
{
    Q_OBJECT

    public:
        ExcInfoTreeView();
        ~ExcInfoTreeView();

        void setModel(QAbstractItemModel *model);

        void contextMenuEvent(QContextMenuEvent *event);

        protected slots:
            void PopDialog();
        void ShowItem(const QModelIndex &index);

        void PopDasmDialog();

    private:

        ExcInfoAbstractModel    *m_infoman;
        QString                 m_defpath;      //  Ĭ��·��
};


class NavWidgetFactory : public Core::INavigationWidgetFactory 
{ 
public: 
    NavWidgetFactory(); 
    ~NavWidgetFactory(); 
 
    Core::NavigationView createWidget(); 
    QString displayName() const; 
    QString id() const; 

private:
    ExcInfoAbstractModel  *m_infoman;
}; 


#endif



