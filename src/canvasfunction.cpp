#include <qpoint.h>
#include <qbrush.h>
#include <qpen.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "function.h"
#include "canvasfunction.h"

static const qreal SCALE = 5.0 * FUNCTION_SCALE;


CanvasPoint::CanvasPoint(class CanvasFunction &canvasFunction, int p)
  : QGraphicsEllipseItem(-0.07 * FUNCTION_SCALE, -0.07 * FUNCTION_SCALE,
			 0.14 * FUNCTION_SCALE, 0.14 * FUNCTION_SCALE)
  , canvasFunction(canvasFunction)
  , p(p)
{
  setPen(QPen(canvasFunction.color(), 0.1));
  setBrush(canvasFunction.color());
  setFlag(ItemIsMovable);
}

void CanvasPoint::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  canvasFunction.mouseMoveEvent(p, event);
}
void CanvasPoint::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  canvasFunction.mousePressEvent(p, event);
}

void CanvasFunction::mouseMoveEvent(int p, QGraphicsSceneMouseEvent *event)
{
  qreal x = event->scenePos().x();
  qreal y = event->scenePos().y();

  x = qMin(x, SCALE);
  x = qMax(x, -SCALE);
  y = qMin(y, SCALE);
  y = qMax(y, -SCALE);

  switch (*function.mode) {
  case 0:
    {
      if (p > 0)
	x = qMax(x, canvasPoints.at(p - 1)->scenePos().x());

      if (p < function.pointCount - 1)
	x = qMin(x, canvasPoints.at(p + 1)->scenePos().x());

      QPointF newPos(x, y);
      setPoint(p, newPos);
    }
    break;
  case 1:
    {
      qreal delta = x - canvasPoints.at(p)->scenePos().x();
      for (int _p = 0; _p < function.pointCount; ++_p) {
	QPointF newPos = canvasPoints.at(_p)->scenePos();
	newPos.setX(newPos.x() + delta);
	newPos.setX(qMin(newPos.x(), SCALE));
	newPos.setX(qMax(newPos.x(), -SCALE));
	setPoint(_p, newPos);
      }
    } break;
  case 2:
    {
      qreal delta = y - canvasPoints.at(p)->scenePos().y();
      for (int _p = 0; _p < function.pointCount; ++_p) {
	QPointF newPos = canvasPoints.at(_p)->scenePos();
	newPos.setY(newPos.y() + delta);
	newPos.setY(qMin(newPos.y(), SCALE));
	newPos.setY(qMax(newPos.y(), -SCALE));
	setPoint(_p, newPos);
      }
    } break;
  case 3:
    if (pos0[p].x() < -0.000001 || pos0[p].x() > 0.000001) {
      qreal delta = x / pos0[p].x();
      for (int _p = 0; _p < function.pointCount; ++_p) {
	QPointF newPos = canvasPoints.at(_p)->scenePos();
	newPos.setX(pos0[_p].x() * delta);
	newPos.setX(qMin(newPos.x(), SCALE));
	newPos.setX(qMax(newPos.x(), -SCALE));
	setPoint(_p, newPos);
      }
    } break;
  case 4:
    if (pos0[p].y() < -0.000001 || pos0[p].y() > 0.000001) {
      qreal delta = y / pos0[p].y();
      for (int _p = 0; _p < function.pointCount; ++_p) {
	QPointF newPos = canvasPoints.at(_p)->scenePos();
	newPos.setY(pos0[_p].y() * delta);
	newPos.setY(qMin(newPos.y(), SCALE));
	newPos.setY(qMax(newPos.y(), -SCALE));
	setPoint(_p, newPos);
      }
    } break;
  }
}

void CanvasFunction::mousePressEvent(int , QGraphicsSceneMouseEvent *)
{
  for (int p = 0; p < function.pointCount; ++p)
    switch (*function.mode) {
    default:
      pos0[p] = canvasPoints.at(p)->scenePos();
      break;
    case 5:
      {
	QPointF newPos(p - 5, 0);
	setPoint(p, newPos *= FUNCTION_SCALE);
      }
      break;
    case 6:
      {
	QPointF newPos(p - 5, -(p - 5));
	setPoint(p, newPos *= FUNCTION_SCALE);
      }
      break;
    }

}

CanvasFunction::CanvasFunction(Function &function, int index)
  : function(function)
  , index(index)
  , pos0(function.pointCount)
{
  int l1;

  for (l1 = 0; l1 < function.pointCount; l1++) {
    CanvasPoint *p = new CanvasPoint(*this, l1);
    canvasPoints.append(p);
    function.scene()->addItem(p);
  }

  for (l1 = 0; l1 < function.pointCount - 1; l1++) {
    QGraphicsLineItem *canvasLine = function.scene()->addLine(QLineF(), QPen(color(), 0.05 * FUNCTION_SCALE));
    canvasLines.append(canvasLine);
  }
}

CanvasFunction::~CanvasFunction() {

}

#include <iostream>
void CanvasFunction::setPoint(int p)
{
  canvasPoints.at(p)->setPos(function.point[index][p]);
  /*std::cout << __PRETTY_FUNCTION__ << index << "[" << p << "] = " <<
    function.point[index][p].x << ", " << function.point[index][p].y <<std::endl;*/

  if (p > 0)
    canvasLines.at(p - 1)->setLine(QLineF(function.point[index][p - 1], function.point[index][p]));

  if (p < function.pointCount - 1)
    canvasLines.at(p)->setLine(QLineF(function.point[index][p], function.point[index][p + 1]));
}

void CanvasFunction::setPoint(int p, QPointF &pos)
{
  function.point[index][p].x = pos.x() / FUNCTION_SCALE;
  function.point[index][p].y = pos.y() / FUNCTION_SCALE;
  setPoint(p);
}

void CanvasFunction::setZ(int z)
{
  int p;
  for (p = 0; p < function.pointCount; ++p) {
    canvasPoints.at(p)->setZValue(z);
    canvasPoints.at(p)->setVisible(z >= 20);
  }
  for (p = 0; p < function.pointCount - 1; ++p)
    canvasLines.at(p)->setZValue(z);
}
