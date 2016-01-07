
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
 * @brief 异常信息管理器，管理listview显示的内容
 * 在MVC架构中对应model
 */
struct ExcListItem;

//#define       ExcInfoAbstractModel  ExcInfoManager
#define       ExcInfoAbstractModel  ExcInfoModel

class ExcInfoManager :public QAbstractListModel
{
    Q_OBJECT

    private:
        struct Item{
            QString Title;  /*  显示标题    */
            QString Info;   /*  对应的内容  */
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


        //  自用参数
        bool SetList(QString excinfo,QString sopath);
        QVariant PathforIndex(QModelIndex index);

    private:
        void ResetList();

        
        void AddItem(Item *item);

        //
        //  接口
        void AddItem(const char *name,const char *info){
            AddItem(new Item(name,info));
        }
        void AddItem(const QString &name,const QString &info){
            AddItem(new Item(name,info));
        }



        QList<Item *> m_ItemList;
        ExcList       *m_plist;     //  存储当前显示的项信息
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
                //  项内容
                QString         Title;
                QString         File;

                //  表示树
                QList<TreeItem*> childItems;
                TreeItem        *parentItem;
        };

        ExcList       *m_plist;     //  存储当前显示的项信息
        TreeItem *rootItem;
        QItemSelectionModel *m_selectionModel;
};

/** 
 * @brief 显示异常信息列表，
 * MVC模型中对应V
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
        QString                 m_defpath;      //  默认路径
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



