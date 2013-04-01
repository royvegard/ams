#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "box.h"

#define TEXTEDIT_DEFAULT_WIDTH                200
#define TEXTEDIT_DEFAULT_HEIGHT               170 
#define TEXTEDIT_NEW_X                         50
#define TEXTEDIT_NEW_Y                         50

class TextEdit: public Box
{
  Q_OBJECT

  bool sizeDrag;
      
public: 
  int textEditID;
  class QTextEdit *textEdit;

private:
  QPoint mousePressPos;

public:
  TextEdit(QWidget* parent=0, const char *name=0);
  virtual  ~TextEdit();
      
protected:
  virtual void paintEvent(class QPaintEvent *ev);
  virtual void mousePressEvent (class QMouseEvent* );
  virtual void mouseReleaseEvent (QMouseEvent* );
  virtual void mouseMoveEvent (QMouseEvent* );

signals:
  void dragged(QPoint pos);
  void sizeDragged(const QPoint&);
  void removeTextEdit();
                        
public slots: 
  virtual void removeThisTextEdit();
};
  
#endif
