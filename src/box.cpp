#include "box.h"

Box::Box(QWidget *parent, const QString &name)
  : QWidget(parent)
{
  setObjectName(name);
  setAutoFillBackground(true);
}
