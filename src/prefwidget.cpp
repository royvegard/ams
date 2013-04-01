#include <qwidget.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <QFileDialog>
#include <qcolordialog.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qlayout.h>
#include <QDialogButtonBox>
#include <QDir>
#include <QGridLayout>
#include <QTextStream>
#include <alsa/asoundlib.h>

#include "synthdata.h"
#include "prefwidget.h"



PrefWidget::PrefWidget(QWidget* parent): QDialog(parent)
{
    setWindowTitle(tr("Edit preferences"));
    tabWidget = new QTabWidget(this);

    setupColorTab();
    setupMidiTab();
    setupEditingTab();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel |
            QDialogButtonBox::Apply);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QPushButton* applyButton = buttonBox->button(QDialogButtonBox::Apply);
    connect(applyButton, SIGNAL(pressed()), this, SIGNAL(prefChanged()));

    QBoxLayout* baseLayout = new QVBoxLayout();
    baseLayout->addWidget(tabWidget);
    baseLayout->addWidget(buttonBox);
    setLayout(baseLayout);
}


void PrefWidget::setupColorTab()
{
    QWidget *colorWidget = new QWidget(this);
    QVBoxLayout *colorBox = new QVBoxLayout(colorWidget);


    QLabel* label = new QLabel(tr("&Background"));
    colorBackgroundLabel = new QLabel();
    colorBackgroundLabel->setMinimumWidth(80);
    colorBackgroundLabel->setAutoFillBackground(true);
    colorBackgroundLabel->setPalette(Qt::black);
    QPushButton* button = new QPushButton(tr("Change..."));
    connect(button, SIGNAL(clicked()), this,
            SLOT(colorBackgroundClicked()));
    label->setBuddy(button);

    QHBoxLayout* lineLayout = new QHBoxLayout();
    lineLayout->addWidget(label);
    lineLayout->addStretch();
    lineLayout->addWidget(colorBackgroundLabel);
    lineLayout->addWidget(button);
    colorBox->addLayout(lineLayout);

    label = new QLabel(tr("&Module background"));
    colorModuleBackgroundLabel = new QLabel();
    colorModuleBackgroundLabel->setMinimumWidth(80);
    colorModuleBackgroundLabel->setAutoFillBackground(true);
    colorModuleBackgroundLabel->setPalette(Qt::black);
    button = new QPushButton(tr("Change..."));
    connect(button, SIGNAL(clicked()), this,
            SLOT(colorModuleBackgroundClicked()));
    label->setBuddy(button);

    lineLayout = new QHBoxLayout();
    lineLayout->addWidget(label);
    lineLayout->addStretch();
    lineLayout->addWidget(colorModuleBackgroundLabel);
    lineLayout->addWidget(button);
    colorBox->addLayout(lineLayout);

    label = new QLabel(tr("M&odule border"));
    colorModuleBorderLabel = new QLabel();
    colorModuleBorderLabel->setMinimumWidth(80);
    colorModuleBorderLabel->setAutoFillBackground(true);
    colorModuleBorderLabel->setPalette(Qt::black);
    button = new QPushButton(tr("Change..."));
    connect(button, SIGNAL(clicked()), this,
            SLOT(colorModuleBorderClicked()));
    label->setBuddy(button);

    lineLayout = new QHBoxLayout();
    lineLayout->addWidget(label);
    lineLayout->addStretch();
    lineLayout->addWidget(colorModuleBorderLabel);
    lineLayout->addWidget(button);
    colorBox->addLayout(lineLayout);

    label = new QLabel(tr("Module &font"));
    colorModuleFontLabel = new QLabel();
    colorModuleFontLabel->setMinimumWidth(80);
    colorModuleFontLabel->setAutoFillBackground(true);
    colorModuleFontLabel->setPalette(Qt::black);
    button = new QPushButton(tr("Change..."));
    connect(button, SIGNAL(clicked()), this,
            SLOT(colorModuleFontClicked()));
    label->setBuddy(button);

    lineLayout = new QHBoxLayout();
    lineLayout->addWidget(label);
    lineLayout->addStretch();
    lineLayout->addWidget(colorModuleFontLabel);
    lineLayout->addWidget(button);
    colorBox->addLayout(lineLayout);

    label = new QLabel(tr("Po&rt font"));
    colorPortFontLabel = new QLabel();
    colorPortFontLabel->setMinimumWidth(80);
    colorPortFontLabel->setAutoFillBackground(true);
    colorPortFontLabel->setPalette(Qt::black);
    button = new QPushButton(tr("Change..."));
    connect(button, SIGNAL(clicked()), this,
            SLOT(colorPortFontClicked()));
    label->setBuddy(button);

    lineLayout = new QHBoxLayout();
    lineLayout->addWidget(label);
    lineLayout->addStretch();
    lineLayout->addWidget(colorPortFontLabel);
    lineLayout->addWidget(button);
    colorBox->addLayout(lineLayout);

    label = new QLabel(tr("&Cables"));
    colorCableLabel = new QLabel();
    colorCableLabel->setMinimumWidth(80);
    colorCableLabel->setAutoFillBackground(true);
    colorCableLabel->setPalette(Qt::black);
    button = new QPushButton(tr("Change..."));
    connect(button, SIGNAL(clicked()), this,
            SLOT(colorCableClicked()));
    label->setBuddy(button);

    lineLayout = new QHBoxLayout();
    lineLayout->addWidget(label);
    lineLayout->addStretch();
    lineLayout->addWidget(colorCableLabel);
    lineLayout->addWidget(button);
    colorBox->addLayout(lineLayout);

    label = new QLabel(tr("&Jacks"));
    colorJackLabel = new QLabel();
    colorJackLabel->setMinimumWidth(80);
    colorJackLabel->setAutoFillBackground(true);
    colorJackLabel->setPalette(Qt::black);
    button = new QPushButton(tr("Change..."));
    connect(button, SIGNAL(clicked()), this,
            SLOT(colorJackClicked()));
    label->setBuddy(button);

    lineLayout = new QHBoxLayout();
    lineLayout->addWidget(label);
    lineLayout->addStretch();
    lineLayout->addWidget(colorJackLabel);
    lineLayout->addWidget(button);
    colorBox->addLayout(lineLayout);


    QPushButton *defaultColorButton = new QPushButton(tr("&Default colors"));
    connect(defaultColorButton, SIGNAL(clicked()), this,
            SLOT(defaultcolorClicked()));

    lineLayout = new QHBoxLayout();
    lineLayout->addWidget(defaultColorButton);
    lineLayout->addStretch();
    colorBox->addLayout(lineLayout);

    colorBox->addStretch();


    tabWidget->addTab(colorWidget, tr("&Colors"));
}


void PrefWidget::setupMidiTab()
{
    QWidget *midiWidget = new QWidget(this);
    QVBoxLayout *midiBox = new QVBoxLayout(midiWidget);

    /*MIDI mode radio buttons*/
    QGroupBox *midiModeGroup = new QGroupBox(tr("MIDI controller mode"));
    midiModeButtons = new QButtonGroup();

    QRadioButton *jumpsRadioButton = new QRadioButton(
            tr("&Avoid parameter jumps"));
    jumpsRadioButton->setChecked (true);
    midiModeButtons->addButton(jumpsRadioButton, 0); //FIXME: MAGIC

    QRadioButton *initRadioButton = new QRadioButton(
            tr("&Init MIDI controller"));
    midiModeButtons->addButton(initRadioButton, 1); //FIXME: MAGIC

    QRadioButton *followRadioButton = new QRadioButton(
            tr("&Follow MIDI controller"));
    midiModeButtons->addButton(followRadioButton, 2); //FIXME: MAGIC

    QVBoxLayout *midiModeLayout = new QVBoxLayout;
    midiModeLayout->addWidget(jumpsRadioButton);
    midiModeLayout->addWidget(initRadioButton);
    midiModeLayout->addWidget(followRadioButton);
    midiModeGroup->setLayout(midiModeLayout);
    midiBox->addWidget(midiModeGroup);

    midiBox->addStretch();


    tabWidget->addTab(midiWidget, tr("&MIDI"));
}


/*Editing tab*/
void PrefWidget::setupEditingTab()
{
    QWidget *editingWidget = new QWidget(this);
    QVBoxLayout *editingBox = new QVBoxLayout(editingWidget);

    /*Module movement mode radio buttons*/
    QGroupBox *moduleMoveGroup = new QGroupBox(tr("Module movement"));
    moduleMoveButtons = new QButtonGroup();

    QRadioButton *standardRadioButton = new QRadioButton(
            tr("&Standard"));
    standardRadioButton->setChecked (true);
    moduleMoveButtons->addButton(standardRadioButton, 0); //FIXME: MAGIC

    QRadioButton *noborderRadioButton = new QRadioButton(
            tr("&No topleft border"));
    moduleMoveButtons->addButton(noborderRadioButton, 1); //FIXME: MAGIC

    QVBoxLayout *moduleMoveLayout = new QVBoxLayout;
    moduleMoveLayout->addWidget(standardRadioButton);
    moduleMoveLayout->addWidget(noborderRadioButton);
    moduleMoveGroup->setLayout(moduleMoveLayout);
    editingBox->addWidget(moduleMoveGroup);

    /*Module position grid*/
    QGroupBox *modulePositionGroup = new QGroupBox(tr("Module position"));

    enableGrid = new QCheckBox(
            tr("Enable module &position grid"));
    connect(enableGrid, SIGNAL(toggled(bool)),
            this, SLOT(enableGridToggled(bool)));

    gridMeshLabel = new QLabel(tr("&Grid mesh size"));
    gridMeshEditline = new QLineEdit();
    gridMeshEditline->setMaxLength(3);
    QIntValidator* meshValidator = new QIntValidator(2, 200,  //MAGIC
                            gridMeshEditline);
    gridMeshEditline->setValidator(meshValidator);
    gridMeshEditline->setToolTip(
            tr("Valid mesh size range is %1 to %2 pixel")
            .arg(meshValidator->bottom())
            .arg(meshValidator->top()));
    gridMeshLabel->setBuddy(gridMeshEditline);

    QHBoxLayout* gridMeshLayout = new QHBoxLayout();
    gridMeshLayout->addWidget(gridMeshLabel);
    gridMeshLayout->addWidget(gridMeshEditline);

    QVBoxLayout *modulePositionLayout = new QVBoxLayout;
    modulePositionLayout->addWidget(enableGrid);
    modulePositionLayout->addLayout(gridMeshLayout);
    modulePositionGroup->setLayout(modulePositionLayout);
    editingBox->addWidget(modulePositionGroup);

    /*checkboxes*/
    rememberGeometry = new QCheckBox(tr("&Remember window geometry"));
    rememberGeometry->setToolTip(tr("Save and restore main window "
                "position and size for next ams session."));
    editingBox->addWidget(rememberGeometry);

    hideRecentFiles = new QCheckBox(tr("&Hide recently used files"));
    hideRecentFiles->setToolTip(tr("Do not show list of recently used "
                "files in file menu."));
    editingBox->addWidget(hideRecentFiles);

    editingBox->addStretch();

    tabWidget->addTab(editingWidget, tr("&Editing"));
}


void PrefWidget::defaultcolorClicked()
{
    colorBackgroundLabel->setPalette(QColor(COLOR_MAINWIN_BG));
    colorModuleBackgroundLabel->setPalette(QColor(COLOR_MODULE_BG));
    colorModuleBorderLabel->setPalette(QColor(195, 195, 195));
    colorModuleFontLabel->setPalette(QColor(255, 255, 255));
    colorCableLabel->setPalette(QColor(180, 180, 180));
    colorJackLabel->setPalette(QColor(250, 200, 50));
}

void PrefWidget::colorBackgroundClicked()
{
    QColor tmp;

    tmp = QColorDialog::getColor(getBackgroundColor());
    if (tmp.isValid())
        colorBackgroundLabel->setPalette(tmp);
}

void PrefWidget::colorModuleBackgroundClicked()
{
    QColor tmp;

    tmp = QColorDialog::getColor(getModuleBackgroundColor());
    if (tmp.isValid())
        colorModuleBackgroundLabel->setPalette(tmp);
}

void PrefWidget::colorModuleBorderClicked()
{
    QColor tmp;

    tmp = QColorDialog::getColor(getModuleBorderColor());
    if (tmp.isValid())
        colorModuleBorderLabel->setPalette(tmp);
}

void PrefWidget::colorModuleFontClicked()
{
    QColor tmp;

    tmp = QColorDialog::getColor(getModuleFontColor());
    if (tmp.isValid())
        colorModuleFontLabel->setPalette(tmp);
}

void PrefWidget::colorPortFontClicked()
{
    QColor tmp;

    tmp = QColorDialog::getColor(getPortFontColor());
    if (tmp.isValid())
        colorPortFontLabel->setPalette(tmp);
}

void PrefWidget::colorCableClicked()
{
    QColor tmp;

    tmp = QColorDialog::getColor(getCableColor());
    if (tmp.isValid())
        colorCableLabel->setPalette(tmp);
}

void PrefWidget::colorJackClicked()
{
    QColor tmp;

    tmp = QColorDialog::getColor(getJackColor());
    if (tmp.isValid())
        colorJackLabel->setPalette(tmp);
}

void PrefWidget::setMidiControllerMode(int mode)
{
    QAbstractButton* button = midiModeButtons->button(mode);
    if (button != NULL)
        button->setChecked(true);
}

int PrefWidget::getMidiControllerMode()
{
    return midiModeButtons->checkedId();
}

void PrefWidget::setModuleMoveMode(int mode)
{
    QAbstractButton* button = moduleMoveButtons->button(mode);
    if (button != NULL)
        button->setChecked(true);
}

int PrefWidget::getModuleMoveMode()
{
    return moduleMoveButtons->checkedId();
}


void PrefWidget::setEnableModuleGrid(bool enable)
{
    enableGrid->setChecked(enable);
}

bool PrefWidget::getEnableModuleGrid()
{
    return enableGrid->isChecked();
}

void PrefWidget::setGridMesh(int mesh)
{
    gridMeshEditline->setText(QString::number(mesh));
}

int PrefWidget::getGridMesh()
{
    return gridMeshEditline->text().toInt();
}

void PrefWidget::setRememberGeometry(bool remember)
{
    rememberGeometry->setChecked(remember);
}

bool PrefWidget::getRememberGeometry()
{
    return rememberGeometry->isChecked();
}

void PrefWidget::setHideRecentFiles(bool hide)
{
    hideRecentFiles->setChecked(hide);
}

bool PrefWidget::getHideRecentFiles()
{
    return hideRecentFiles->isChecked();
}

void PrefWidget::setBackgroundColor(QColor color)
{
    colorBackgroundLabel->setPalette(color);
}

QColor PrefWidget::getBackgroundColor()
{
    return colorBackgroundLabel->palette().color(QPalette::Background);
}

void PrefWidget::setModuleBackgroundColor(QColor color)
{
    colorModuleBackgroundLabel->setPalette(color);
}

QColor PrefWidget::getModuleBackgroundColor()
{
    return colorModuleBackgroundLabel->palette().color(QPalette::Background);
}

void PrefWidget::setModuleBorderColor(QColor color)
{
    colorModuleBorderLabel->setPalette(color);
}

QColor PrefWidget::getModuleBorderColor()
{
    return colorModuleBorderLabel->palette().color(QPalette::Background);
}

void PrefWidget::setModuleFontColor(QColor color)
{
    colorModuleFontLabel->setPalette(color);
}

QColor PrefWidget::getModuleFontColor()
{
    return colorModuleFontLabel->palette().color(QPalette::Background);
}

void PrefWidget::setPortFontColor(QColor color)
{
    colorPortFontLabel->setPalette(color);
}

QColor PrefWidget::getPortFontColor()
{
    return colorPortFontLabel->palette().color(QPalette::Background);
}

void PrefWidget::setCableColor(QColor color)
{
    colorCableLabel->setPalette(color);
}

QColor PrefWidget::getCableColor()
{
    return colorCableLabel->palette().color(QPalette::Background);
}

void PrefWidget::setJackColor(QColor color)
{
    colorJackLabel->setPalette(color);
}

QColor PrefWidget::getJackColor()
{
    return colorJackLabel->palette().color(QPalette::Background);
}

void PrefWidget::enableGridToggled(bool checked)
{
    gridMeshLabel->setEnabled(checked);
    gridMeshEditline->setEnabled(checked);
}
