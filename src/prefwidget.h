/**
 * preferences dialog of AMS 
 */

#ifndef PREFWIDGET_H
#define PREFWIDGET_H

#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <QTextStream>

#include <alsa/asoundlib.h>


class PrefWidget : public QDialog
{
    Q_OBJECT

    QTabWidget *tabWidget;
    QLabel *colorBackgroundLabel;
    QLabel *colorModuleBackgroundLabel;
    QLabel *colorModuleBorderLabel;
    QLabel *colorModuleFontLabel;
    QLabel *colorPortFontLabel;
    QLabel *colorCableLabel;
    QLabel *colorJackLabel;

    QButtonGroup* midiModeButtons;
    QButtonGroup* moduleMoveButtons;
    QCheckBox* enableGrid;
    QLabel *gridMeshLabel;
    QLineEdit* gridMeshEditline;
    QCheckBox* rememberGeometry;
    QCheckBox* hideRecentFiles;
   
    void setupColorTab();
    void setupMidiTab();
    void setupEditingTab();

  public:
    PrefWidget(QWidget* parent = 0);
    void setBackgroundColor(QColor);
    void setModuleBackgroundColor(QColor);
    void setModuleBorderColor(QColor);
    void setModuleFontColor(QColor);
    void setPortFontColor(QColor);
    void setCableColor(QColor);
    void setJackColor(QColor);
    QColor getBackgroundColor();
    QColor getModuleBackgroundColor();
    QColor getModuleBorderColor();
    QColor getModuleFontColor();
    QColor getPortFontColor();
    QColor getCableColor();
    QColor getJackColor();
    void setMidiControllerMode(int);
    int getMidiControllerMode();
    void setModuleMoveMode(int);
    int getModuleMoveMode();
    void setEnableModuleGrid(bool hide);
    bool getEnableModuleGrid();
    void setGridMesh(int mesh);
    int getGridMesh();
    void setRememberGeometry(bool remember);
    bool getRememberGeometry();
    void setHideRecentFiles(bool hide);
    bool getHideRecentFiles();

  signals:
    void prefChanged();

  public slots:
    void colorBackgroundClicked();
    void colorModuleBackgroundClicked();
    void colorModuleBorderClicked();
    void colorModuleFontClicked();
    void colorPortFontClicked();
    void colorCableClicked();
    void defaultcolorClicked();
    void colorJackClicked();
    void enableGridToggled(bool checked);
};
  

#endif
