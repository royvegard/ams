#include <qpainter.h>
#include <QVBoxLayout>
#include <qpushbutton.h>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTextEdit>
#include "textedit.h"

TextEdit::TextEdit(QWidget* parent, const char *name)
  : Box(parent, name)
{
  QVBoxLayout *layout = new QVBoxLayout;
  textEdit = new QTextEdit(this);
  textEdit->setPalette(QPalette(QColor(77, 70, 64), QColor(250, 250, 250)));
  layout->addWidget(textEdit);
  QPushButton *removeButton = new QPushButton(tr("&Remove Comment"), this);
  QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removeThisTextEdit()));
  layout->addWidget(removeButton);
  setLayout(layout);
//  synthdata->incTextEditCount();
//  textEditID = synthdata->getTextEditID();
//  synthdata->textEditList.append(this);
  textEditID = 0;  // TODO assign this
  setPalette(QPalette(QColor(77, 70, 64), QColor(77, 70, 64)));
  setGeometry(TEXTEDIT_NEW_X, TEXTEDIT_NEW_Y, TEXTEDIT_DEFAULT_WIDTH, TEXTEDIT_DEFAULT_HEIGHT);
  sizeDrag = false;
}

TextEdit::~TextEdit() {

//  synthdata->textEditListList.removeRef(this);
//  synthdata->decTextEditCount();
}

void TextEdit::paintEvent(QPaintEvent *) {

  QPainter p(this);
  int l1;

  for (l1 = 0; l1 < 4; l1++) {
    p.setPen(QColor(195 + 20*l1, 195 + 20*l1, 195 + 20*l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
}

void TextEdit::mousePressEvent(QMouseEvent *ev)
{
    switch (ev->button()) {
        case Qt::MidButton:
            sizeDrag = true;
            mousePressPos = ev->pos();
            ev->accept();
        default:
            ev->ignore();
            break;
    }
}

void TextEdit::mouseReleaseEvent(QMouseEvent *ev)
{
    switch (ev->button()) {
        case Qt::MidButton:
            sizeDrag = false;
            ev->accept();
        default:
            ev->ignore();
            break;
    }
}

void TextEdit::mouseMoveEvent(QMouseEvent *ev)
{
    if (sizeDrag) {
        emit sizeDragged(ev->pos());
        ev->accept();
    }
    else
        ev->ignore();
}

void TextEdit::removeThisTextEdit() {

  emit removeTextEdit();
}
