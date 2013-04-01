#ifndef CANVASFUNCTION_H
#define CANVASFUNCTION_H

#include <qcolor.h>
#include <QVector>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>

#include "function.h"


class CanvasPoint: public QGraphicsEllipseItem {
  class CanvasFunction &canvasFunction;
  int p;

  void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
  void mousePressEvent(QGraphicsSceneMouseEvent * event);

public:
  CanvasPoint(CanvasFunction &, int p);

};

class CanvasFunction
{
  friend class CanvasPoint;
  Function &function;
  const int index;

  QColor color() {
    return function.colorTable[index];
  }
  void mouseMoveEvent(int p, QGraphicsSceneMouseEvent * event);
  void mousePressEvent(int p, QGraphicsSceneMouseEvent * event);
  QVector<QPointF> pos0;
    
public:  
  QList<CanvasPoint*> canvasPoints;
  QList<QGraphicsLineItem*> canvasLines;

public:
  CanvasFunction(Function &function, int index);
  ~CanvasFunction();

  void setPoint(int p);
  void setPoint(int p, QPointF &pos);
  void setZ(int z);

};
  
#endif
