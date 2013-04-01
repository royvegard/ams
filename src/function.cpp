#include <cmath>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qbrush.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qpoint.h>
#include <qmatrix.h>
#include <QPolygon>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QGraphicsScene>

#include "function.h"
#include "canvasfunction.h"

static const qreal SCALE = 5.0 * FUNCTION_SCALE;


Function::Function(int p_functionCount, int *p_mode, int *p_editIndex,
                   tFunction & point, int p_pointCount, QWidget * parent,
                   const char *name)
:QGraphicsView(new QGraphicsScene(), parent), point(point)
{
    int l1;
    QString qs;

//?   scene()->setGrid(FUNCTION_BORDER_L, FUNCTION_BORDER_R, FUNCTION_BORDER_B, FUNCTION_BORDER_T, FUNCTION_SCALE,
//                                 FUNCTION_WIDTH, FUNCTION_HEIGHT, FUNCTION_GRID, FUNCTION_GRID);

    functionCount = p_functionCount;
    mode = p_mode;
    editIndex = p_editIndex;
    mousePressed = false, activeFunction = -1;
    activePoint = -1;
    zoom = 1.0;
    updateScale();
//   for (l1 = 0; l1 < MAX_FUNCTIONS; l1++) {
//     points[l1] = p_points[l1];
    setPointCount(MAX_POINTS);
    //    screenPoints[l1] = new QPolygon(MAX_POINTS);
    //  }
    setMinimumWidth(FUNCTION_MINIMUM_WIDTH);
    setMinimumHeight(FUNCTION_MINIMUM_HEIGHT);
    setPalette(QPalette
               (QColor(FUNCTION_COLOR_FG), QColor(FUNCTION_COLOR_BG)));
    colorTable[0] = QColor(FUNCTION_COLOR_1);
    colorTable[1] = QColor(FUNCTION_COLOR_2);
    colorTable[2] = QColor(FUNCTION_COLOR_3);
    colorTable[3] = QColor(FUNCTION_COLOR_4);
//  colorTable[4] = QColor(FUNCTION_COLOR_5);
//  colorTable[5] = QColor(FUNCTION_COLOR_6);
//  colorTable[6] = QColor(FUNCTION_COLOR_7);
//  colorTable[7] = QColor(FUNCTION_COLOR_8);
    setBackgroundBrush(QColor(FUNCTION_COLOR_BG));

    scene()->setSceneRect(-5 * FUNCTION_SCALE, -5 * FUNCTION_SCALE,
                          10 * FUNCTION_SCALE, 10 * FUNCTION_SCALE);

    for (l1 = 0; l1 < functionCount; l1++) {
        CanvasFunction *canvasFunction = new CanvasFunction(*this, l1);
        canvasFunctionList.append(canvasFunction);
        updateFunction(l1);
        qs = tr("Out %1").arg(l1);
        QGraphicsSimpleTextItem *canvasText =
            new QGraphicsSimpleTextItem(qs, 0, scene());
        canvasText->setPos(8 + 50 * l1, 4);
        //    canvasText->setColor(colorTable[l1]);
        canvasText->setFont(synthdata->bigFont);
        canvasText->setVisible(TRUE);
        canvasTextList.append(canvasText);
    }
}

Function::~Function()
{
}

void Function::updateFunction(int index)
{/*
   int l1, z;
   QPoint qp;

   *screenPoints[index] = matrix.map(*points[index]);
   if (*editIndex && (index == *editIndex - 1)) {
     z = 20;
   } else {
     z = 10;
   }
   f[0][index][0] = -1e30;
   f[0][index][pointCount+1] = 1e30;
   f[1][index][0] = 0;
   f[1][index][pointCount+1] = 0;
   for (l1 = 0; l1 < pointCount; l1++) {
     qp = screenPoints[index]->point(l1);
     f[0][index][l1 + 1] = (points[index]->point(l1).x() - FUNCTION_CENTER_X) / FUNCTION_SCALE;
     f[1][index][l1 + 1] = (double)(FUNCTION_HEIGHT - points[index]->point(l1).y() - FUNCTION_CENTER_Y) / (double)FUNCTION_SCALE;
     canvasFunctionList.at(index)->setPoint(l1, qp.x(), qp.y(), z);
   }
    //  repaint(false);*/
}

void Function::setPointCount(int count)
{
    pointCount = count;
}

QSize Function::sizeHint() const
{
    return QSize(FUNCTION_MINIMUM_WIDTH, FUNCTION_MINIMUM_HEIGHT);
}

QSizePolicy Function::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
                       QSizePolicy::MinimumExpanding);
}

void Function::resizeEvent(QResizeEvent *)
{
    updateScale();
    //  scene()->setSceneRect(0, 0, zoom * width(), zoom * height());
    //  redrawGrid();
    redrawFunction();
    //  updateContents();
}

void Function::redrawFunction()
{
    int l1;

    if (canvasFunctionList.count() == functionCount) {
        for (l1 = 0; l1 < functionCount; l1++) {
            updateFunction(l1);
        }
    }
}

void Function::drawBackground(QPainter * painter, const QRectF & rect)
{
    QString s;
    QFont f;
    f.setPointSizeF(250.0 / zoom);
    painter->setFont(f);
    {
        qreal y = ceil(rect.topLeft().y() * zoom / FUNCTION_SCALE);
        y *= FUNCTION_SCALE / zoom;
        QLineF l(qMax(-SCALE, rect.topLeft().x()), y,
                 qMin(SCALE, rect.topRight().x()), y);
        while (l.y1() < rect.bottomLeft().y()) {
            painter->drawLine(l);
            painter->drawText(l.p1(), s.setNum(l.y1() / FUNCTION_SCALE));
            l.translate(0, (qreal) FUNCTION_SCALE / zoom);
        }
    }
    {
        qreal x = ceil(rect.topLeft().x() * zoom / FUNCTION_SCALE);
        x *= FUNCTION_SCALE / zoom;
        QLineF l(x, qMax(-SCALE, rect.topLeft().y()), x,
                 qMin(SCALE, rect.bottomLeft().y()));
        while (l.x1() < rect.bottomRight().x()) {
            painter->drawLine(l);
            painter->drawText(l.p1(), s.setNum(l.x1() / FUNCTION_SCALE));
            l.translate((qreal) FUNCTION_SCALE / zoom, 0);
        }
    }
}

void Function::redrawGrid()
{
    int l1, l2, ix0, iy0;
    QPoint qp_in[2], qp_out[2];
    QTransform invMatrix, zoomMatrix;

    if (matrix.isInvertible()) {
        invMatrix = matrix.inverted();
    } else {
        fprintf(stderr, "Function::redrawGrid: Could not invert Matrix.\n");
        return;
    }
    //contentsToViewport(0, 0, x0, y0);
    //qp_in[0] = QPoint(x0, y0);
    qp_in[0] = mapFromScene(0, 0);
    qp_out[0] = invMatrix.map(qp_in[0]);
    ix0 =  qp_out[0].x() / (FUNCTION_GRID * 2);
    iy0 =  qp_out[0].y() / (FUNCTION_GRID * 2);
    //  fprintf(stderr, "x0: %d  y0: %d qp.x(): %d, qp.y(): %d  ix0: %d  iy0: %d\n", x0, y0, qp_out[0].x(), qp_out[0].y(), ix0, iy0);
    qp_in[0].setX(0);
    qp_in[1].setX(FUNCTION_WIDTH);
    qp_in[1].setY(0);
    zoomMatrix = matrix;
    zoomMatrix.scale(1.0/zoom, 1.0/zoom);
    zoomMatrix.translate(-ix0 * zoom * 2 * FUNCTION_GRID, -iy0 * zoom * 2 * FUNCTION_GRID);
    for (l1 = 0; l1 <gridX.count(); l1++) {
        qp_in[0].setY(l1 * FUNCTION_GRID);
        for (l2 = 0; l2 < 2; l2++) {
            qp_out[l2] = zoomMatrix.map(qp_in[l2]);
        }
        gridX.at(l1)->setLine(qp_out[0].x(), qp_out[0].y(), qp_out[1].x(), qp_out[0].y());
    }
    qp_in[0].setY(0);
    qp_in[1].setY(FUNCTION_HEIGHT);
    qp_in[1].setX(0);
    for (l1 = 0; l1 <gridY.count(); l1++) {
        qp_in[0].setX(l1 * FUNCTION_GRID);
        for (l2 = 0; l2 < 2; l2++) {
            qp_out[l2] = zoomMatrix.map(qp_in[l2]);
        }
        gridY.at(l1)->setLine(qp_out[0].x(), qp_out[0].y(), qp_out[0].x(), qp_out[1].y());
    }
}

void Function::contentsMousePressEvent(QMouseEvent * ev)
{
   int l1, l2;
   QList<QGraphicsItem *> hitList;
   QTransform invMatrix;
   QPoint qp;
   bool foundHit;

   if (matrix.isInvertible()) {
     invMatrix = matrix.inverted();
     qp = invMatrix.map(ev->pos());
     emit mousePos(qp.x(), qp.y());
   }
   mousePressPos = ev->pos();
   mousePressed = true;
   foundHit = false;
   hitList = scene()->items(ev->pos());
   if (hitList.count()) {
     for (l1 = 0; l1 < functionCount; l1++) {
       for (l2 = 0; l2 < pointCount; l2++) {
         for(QList<QGraphicsItem *>::Iterator it=hitList.begin(); it!=hitList.end(); it++) {
           if (canvasFunctionList.at(l1)->canvasPoints.at(l2) == *it) {
 //            fprintf(stderr, "Hit %d %d\n", l1, l2);
             if (!*editIndex || (l1 == *editIndex - 1)) {
               activeFunction = l1;
               activePoint = l2;
 //              fprintf(stderr, "activeFunction: %d  activePoint: %d\n", l1, l2);
               foundHit = true;
               break;
             } else {
               activePoint = -1;
 //              fprintf(stderr, "activePoint: %d\n", l2);
             }
           }
         }
       }
       if (foundHit) {
         break;
       }
     }
     if (*mode == 3) {
       for (l1 = 0; l1 < pointCount; l1++) {
         deltaArray[l1] = point[activeFunction][l1].x - FUNCTION_CENTER_X;
       }
     } else if (*mode == 4) {
       for (l1 = 0; l1 < pointCount; l1++) {
         deltaArray[l1] = point[activeFunction][l1].y - FUNCTION_CENTER_Y;
       }
     }
   } else {
     activePoint = -1;
   }
}

void Function::contentsMouseReleaseEvent(QMouseEvent * ev)
{
   QPoint qp;
   int l1;

   mousePressed = false;
   if ((activeFunction >=0) && (activePoint >= 0)) {
     switch (*mode) {
       case 5:
         for (l1 = 0; l1 < pointCount; l1++) {
           qp = QPoint(l1 * FUNCTION_WIDTH / (pointCount - 1), FUNCTION_HEIGHT >> 1);
           point[activeFunction][l1].x = qp.x();
           point[activeFunction][l1].y = qp.y();
         }
         break;
       case 6:
         for (l1 = 0; l1 < pointCount; l1++) {
           qp = QPoint(l1 * FUNCTION_WIDTH / (pointCount - 1), (pointCount - 1 - l1) * FUNCTION_WIDTH / (pointCount - 1));
           point[activeFunction][l1].x = qp.x();
           point[activeFunction][l1].y = qp.y();
         }
         break;
     }
     updateFunction(activeFunction);
     //    repaintContents(false);
   }
}

void Function::contentsMouseMoveEvent(QMouseEvent * ev)
{
   QTransform invMatrix;
   QPoint qp;
   int l1, delta;
   float scaleFactor;

   if (matrix.isInvertible()) {
     invMatrix = matrix.inverted();
     qp = invMatrix.map(ev->pos());
     emit mousePos(qp.x(), qp.y());
     if (mousePressed && (activeFunction >=0) && (activePoint >= 0)) {
 //    fprintf(stderr, "mouseMoveEvent scale[0] = %f, scale[1] = %f\n", scale[0], scale[1]);
       switch (*mode) {
         case 0:
           if ((activePoint > 0) && (qp.x() < point[activeFunction][activePoint - 1].x)) {
             // a minimum dx of 1 corresponds to 0.002 V
             qp.setX(point[activeFunction][activePoint-1].x + 1);
           } else if ((activePoint < pointCount - 1) && (qp.x() >
                       point[activeFunction][activePoint + 1].x)) {
             qp.setX(point[activeFunction][activePoint+1].x - 1);
           }
           if (qp.x() < 0) qp.setX(0);
           if (qp.x() > FUNCTION_WIDTH) qp.setX(FUNCTION_WIDTH);
           if (qp.y() < 0) qp.setY(0);
           if (qp.y() > FUNCTION_HEIGHT) qp.setY(FUNCTION_HEIGHT);
           point[activeFunction][activePoint].x = qp.x();
           point[activeFunction][activePoint].y = qp.y();
           break;
         case 1:
           delta = qp.x() - point[activeFunction][activePoint].x;
           for (l1 = 0; l1 < pointCount; l1++) {
             qp = QPoint(point[activeFunction][l1].x + delta,
                     point[activeFunction][l1].y);
             if (qp.x() < 0) qp.setX(0);
             if (qp.x() > FUNCTION_WIDTH) qp.setX(FUNCTION_WIDTH);

             point[activeFunction][l1].x = qp.x();
             point[activeFunction][l1].y = qp.y();
           }
           break;
         case 2:
           delta = qp.y() - point[activeFunction][activePoint].y;
           for (l1 = 0; l1 < pointCount; l1++) {
             qp = QPoint(point[activeFunction][l1].x,
                     point[activeFunction][l1].y + delta);
             if (qp.y() < 0) qp.setY(0);
             if (qp.y() > FUNCTION_HEIGHT) qp.setY(FUNCTION_HEIGHT);
             point[activeFunction][l1].x = qp.x();
             point[activeFunction][l1].y = qp.y();
           }
           break;
         case 3:
           delta = qp.x() - FUNCTION_CENTER_X;
           scaleFactor = (deltaArray[activePoint] != 0)
                       ? (double)delta / (double)deltaArray[activePoint] : 1.0;
           for (l1 = 0; l1 < pointCount; l1++) {
             qp = QPoint((double)FUNCTION_CENTER_X + scaleFactor * (double)deltaArray[l1],
                     point[activeFunction][l1].y);
             if (qp.x() < 0) qp.setX(0);
             if (qp.x() > FUNCTION_WIDTH) qp.setX(FUNCTION_WIDTH);
             point[activeFunction][l1].x = qp.x();
             point[activeFunction][l1].y = qp.y();
           }
           break;
         case 4:
           delta = qp.y() - FUNCTION_CENTER_Y;
           scaleFactor = (deltaArray[activePoint] != 0)
                       ? (double)delta / (double)deltaArray[activePoint] : 1.0;
           for (l1 = 0; l1 < pointCount; l1++) {
             qp = QPoint(point[activeFunction][l1].x,
                         (double)FUNCTION_CENTER_Y + scaleFactor * (double)deltaArray[l1]);
             if (qp.y() < 0) qp.setY(0);
             if (qp.y() > FUNCTION_HEIGHT) qp.setY(FUNCTION_HEIGHT);
             point[activeFunction][l1].x = qp.x();
             point[activeFunction][l1].y = qp.y();
           }
           break;
       }
 // fprintf(stderr, "mouseMoveEvent points[%d]->point(%d) = %d %d\n", activeFunction, activePoint,
 //   points[activeFunction]->point(activePoint).x(), points[activeFunction]->point(activePoint).y());
       updateFunction(activeFunction);
 // fprintf(stderr, "mouseMoveEvent f[0][%d][%d+1] = %f f[1][%d][%d+1] = %f\n", activeFunction, activePoint, f[0][activeFunction][activePoint+1],
 //    activeFunction, activePoint, f[1][activeFunction][activePoint+1]);
 //    repaintContents(false);
     }
   } else {
     fprintf(stderr, "Matrix not invertible !\n");
   }
}

void Function::setZoom(float p_zoom)
{
    zoom = p_zoom;
    updateScale();
}

void Function::updateScale()
{
    double scale[2];

    scale[0] = (double) zoom *width() / (11 * FUNCTION_SCALE);
    scale[1] = (double) zoom *height() / (11 * FUNCTION_SCALE);
    matrix.reset();
    matrix.scale(scale[0], scale[1]);
    setTransform(matrix);
}

void Function::setPoint(int f_index, int p_index, int x, int y)
{
    FunctionPointT & pf = point[f_index][p_index];
    pf.x = (float) (x - FUNCTION_CENTER_X) / FUNCTION_SCALE;
    pf.y = (float) (y - FUNCTION_CENTER_Y) / FUNCTION_SCALE;
    canvasFunctionList.at(f_index)->setPoint(p_index);
}

QPoint Function::getPoint(int f_index, int p_index)
{
    FunctionPointT & pf = point[f_index][p_index];
    QPoint p;
    p.setX((int) (pf.x * FUNCTION_SCALE) + FUNCTION_CENTER_X);
    p.setY((int) (pf.y * FUNCTION_SCALE) + FUNCTION_CENTER_Y);
    return p;
}

void Function::highlightFunction(int index)
{
    for (int l1 = 0; l1 < functionCount; l1++)
        canvasFunctionList.at(l1)->
            setZ((index ? (index - 1 == l1 ? 20 : 10) : 20) + l1);
}
