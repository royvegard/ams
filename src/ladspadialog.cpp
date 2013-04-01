#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <dlfcn.h>
#include <qregexp.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <QTreeView>
#include "ladspadialog.h"
#include "synthdata.h"
#include <iostream>

int LadspaModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
    return ::synthdata->ladspaLib.count();

  if (parent.internalId() == -1) {
    const LadspaLib &l = synthdata->ladspaLib.at(parent.row());
    return l.desc.count();
  }

  return 0;
}

QVariant LadspaModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || role != Qt::DisplayRole)
    return QVariant();

  if (index.internalId() == -1) {
    const LadspaLib &l = synthdata->ladspaLib.at(index.row());
    return l.name;
  }
  return synthdata->ladspaLib.at(index.internalId()).desc.at(index.row())->Name;
}

QVariant LadspaModel::headerData(int /*section*/, Qt::Orientation orientation,
				  int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return tr("Available Ladspa Plugins");

  return QVariant();
}

QModelIndex LadspaModel::index(int row, int column,
			       const QModelIndex &parent) const
{
  if (!parent.isValid())
    return createIndex(row, column, -1);

  return createIndex(row, column, parent.row());
}

QModelIndex LadspaModel::parent(const QModelIndex &child) const
{
  if (child.isValid() && child.internalId() != -1)
    return index(child.internalId(), 0);

  return QModelIndex();
}

int LadspaModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 1;
}


LadspaDialog::LadspaDialog()
  : vbox(this)
  , selectedLib(-1)
  , selectedDesc(-1)
{
  char *error;
  std::string ladspadir, ladspapath, qs;
  void *so_handle;
  LADSPA_Descriptor_Function ladspa_dsc_func;

  unsigned l1;
  int colon, lastcolon;
  glob_t globbuf;

  setWindowTitle(tr("Ladspa Browser"));
  vbox.setMargin(10);
  vbox.setSpacing(10);
  const char *_lp = getenv("LADSPA_PATH");
  if (!_lp) {
    qWarning("LADSPA_PATH not set, assuming LADSPA_PATH=" LADSPA_PATH);
    ladspapath = LADSPA_PATH;
  } else {
    ladspapath = _lp;
    std::cerr << "LADSPA_PATH: " << ladspapath << std::endl;
  }
  colon = -1;
  do {
    lastcolon = colon;
    colon = ladspapath.find(":", lastcolon + 1);
    if (colon >= 0) {
      ladspadir = ladspapath.substr(lastcolon + 1, colon - lastcolon - 1);
    } else {
      ladspadir = (lastcolon) ? ladspapath.substr(lastcolon + 1, ladspapath.length() - lastcolon - 1)
                              : ladspapath;
    }
//    fprintf(stderr, "Searching for LADSPA plugins in %s\n", ladspadir.latin1());
    ladspadir += "/*.so";
    glob(ladspadir.c_str(), GLOB_MARK, NULL, &globbuf);
    for (l1 = 0; l1 < globbuf.gl_pathc; l1++) {
//      fprintf(stderr, "    found %s\n", globbuf.gl_pathv[l1]);
      if (!(so_handle = dlopen(globbuf.gl_pathv[l1], RTLD_LAZY))) {
        fprintf(stderr, "Error opening shared ladspa object %s.\n", globbuf.gl_pathv[l1]);
        continue;
      }
      ladspa_dsc_func =(LADSPA_Descriptor_Function)dlsym(so_handle, "ladspa_descriptor");
      if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error accessing ladspa descriptor in %s.\n", globbuf.gl_pathv[l1]);
        continue;
      }
      qs = globbuf.gl_pathv[l1];
      LadspaLib lib;
      lib.name = qs.substr(qs.rfind("/") + 1, qs.rfind(".") - qs.rfind("/") - 1).c_str();
      int d = 0;
      while (const LADSPA_Descriptor *desc = ladspa_dsc_func(d)) {
	lib.desc.append(desc);
	++d;
      }
      if (d)
	synthdata->ladspaLib.append(lib);
   }
    globfree(&globbuf);
  } while (colon >= 0);

  vbox.addWidget(ladspaView = new QTreeView());
  ladspaView->setModel(&ladspaModel);

  QHBoxLayout *searchBox = new QHBoxLayout();
  vbox.addLayout(searchBox);
  searchBox->setSpacing(10);
  searchLine = new QLineEdit();
  QObject::connect(searchLine, SIGNAL(textEdited(const QString &)),
		   this, SLOT(searchLineEdited(const QString &)));
  searchBox->addWidget(searchLine);
  QPushButton *searchButton = new QPushButton(tr("&Search"));
  searchBox->addWidget(searchButton);
  QObject::connect(searchButton, SIGNAL(clicked()), this, SLOT(searchClicked()));
  QObject::connect(searchLine, SIGNAL(returnPressed()), this, SLOT(searchClicked()));
  QFrame *labelFrame = new QFrame();
  vbox.addWidget(labelFrame);
  labelFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  QVBoxLayout *labelBox = new QVBoxLayout(labelFrame);
  labelBox->setSpacing(5);
  pluginLabel = new QLabel();
  labelBox->addWidget(pluginLabel);
  pluginLabel->setAlignment(/*Qt::WordBreak |*/ Qt::AlignLeft);
  pluginMaker = new QLabel();
  labelBox->addWidget(pluginMaker);
  pluginMaker->setAlignment(/*Qt::WordBreak |*/ Qt::AlignLeft);
  pluginCopyright = new QLabel();
  labelBox->addWidget(pluginCopyright);
  pluginCopyright->setAlignment(/*Qt::WordBreak |*/ Qt::AlignLeft);
  QObject::connect(ladspaView->selectionModel(),
		   SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
		   this,
		   SLOT(pluginHighlighted(const QItemSelection &, const QItemSelection &)));

  extCtrlPortsCheck = new QCheckBox(
          tr("&Export control ports as module ports"));
  vbox.addWidget(extCtrlPortsCheck);
  QHBoxLayout *buttonBox = new QHBoxLayout();
  vbox.addLayout(buttonBox);
  insertButton = new QPushButton(tr("&Create Plugin"));
  buttonBox->addWidget(insertButton);
  insertButton->setEnabled(false);
  QObject::connect(insertButton, SIGNAL(clicked()), this, SLOT(insertClicked()));
  buttonBox->addStretch();
  insertPolyButton = new QPushButton(tr("Create &Poly Plugin"));
  buttonBox->addWidget(insertPolyButton);
  insertPolyButton->setEnabled(false);
  QObject::connect(insertPolyButton, SIGNAL(clicked()), this,
          SLOT(insertPolyClicked()));
}

LadspaDialog::~LadspaDialog() {
}

void LadspaDialog::insertClicked()
{
  if (selectedDesc != -1)
    emit createLadspaModule(selectedLib, selectedDesc, false, extCtrlPortsCheck->isChecked());
}

void LadspaDialog::insertPolyClicked()
{
  if (selectedDesc != -1)
    emit createLadspaModule(selectedLib, selectedDesc, true, extCtrlPortsCheck->isChecked());
}

void LadspaDialog::pluginHighlighted(const QItemSelection &selected, const QItemSelection &)
{
  selectedLib = selectedDesc = -1;
  if (selected.indexes().count() > 0) {
    const QModelIndex mi = selected.indexes().at(0);
    if (mi.internalId() != -1) {
      selectedLib = mi.internalId();
      selectedDesc = mi.row();
    }
  }
  insertButton->setEnabled(selectedDesc != -1);
  insertPolyButton->setEnabled(selectedDesc != -1);
  if (selectedDesc != -1) {
    const LADSPA_Descriptor * desc =
      synthdata->ladspaLib.at(selectedLib).desc.at(selectedDesc);
    pluginLabel->setText(tr("Label: ") + desc->Label);
    pluginMaker->setText(tr("Author: ") + desc->Maker);
    pluginCopyright->setText(tr("Copyright: ") + desc->Copyright);
  } else {
    pluginLabel->setText("");
    pluginMaker->setText("");
    pluginCopyright->setText("");
  }
}

void LadspaDialog::search(bool select)
{
  if (!synthdata->ladspaLib.count())
    return;

  int lib = 0, desc = 0;
  int _lib, _desc;

  if (selectedDesc != -1) {
    lib = selectedLib;
    desc = selectedDesc;
  }
  _lib = lib;
  _desc = desc;

  QModelIndex parent = ladspaModel.index(lib, 0);

  do {
    if (++desc >= synthdata->ladspaLib.at(lib).desc.count()) {
      desc = 0;
      if (++lib >= synthdata->ladspaLib.count())
	lib = 0;
      parent = ladspaModel.index(lib, 0);
    }
    const LadspaLib &libr = synthdata->ladspaLib.at(lib);
    if (QString(libr.desc.at(desc)->Name).contains(searchLine->text(), Qt::CaseInsensitive)) {
      if (select) {
	QModelIndex mi = ladspaModel.index(desc, 0, parent);
	ladspaView->scrollTo(mi);
	ladspaView->selectionModel()->select(mi, QItemSelectionModel::ClearAndSelect);
	return;
      }
      ladspaView->setRowHidden(lib, QModelIndex(), false);
      ladspaView->setRowHidden(desc, parent, false);
      ladspaView->expand(parent);
    } else
      if (!select)
	ladspaView->setRowHidden(desc, parent, true);
  } while(_lib != lib  ||  _desc != desc);
}

void LadspaDialog::searchClicked()
{
  search(true);
}

void LadspaDialog::searchLineEdited(const QString &)
{
  for (int lib = 0; lib < synthdata->ladspaLib.count(); ++lib)
    ladspaView->setRowHidden(lib, QModelIndex(), true);
  search(false);
}
