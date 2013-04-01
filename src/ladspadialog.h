#ifndef LADSPADIALOG_H
#define LADSPADIALOG_H

#include <ladspa.h>
#include <qwidget.h>
#include <QVBoxLayout>
#include <QAbstractItemModel>

class LadspaModel: public QAbstractItemModel
{
Q_OBJECT
  friend class LadspaDialog;

  int rowCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column,
		    const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
};

class LadspaDialog: public QWidget
{
Q_OBJECT
  LadspaModel ladspaModel;
  QVBoxLayout vbox;
  class QLabel *pluginLabel, *pluginMaker, *pluginCopyright;
  class QLineEdit *searchLine;
  class QCheckBox *extCtrlPortsCheck;
  class QPushButton *insertButton;
  class QPushButton *insertPolyButton;
  int selectedLib;
  int selectedDesc;

  void search(bool select);
           
public: 
  class QTreeView *ladspaView;
    
public:
  LadspaDialog();
  ~LadspaDialog();
    
signals:
  void createLadspaModule(int index, int n, bool poly, bool extCtrlPorts);  

private slots:
  void insertClicked();
  void insertPolyClicked();
  void searchClicked();
  void searchLineEdited(const QString &);
  void pluginHighlighted(const class QItemSelection &, const QItemSelection &);
};
  
#endif
