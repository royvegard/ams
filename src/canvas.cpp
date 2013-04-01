#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <qpainter.h>
#include <qstring.h>
#include <qfont.h>

#include "canvas.h"
#include "synthdata.h"


Canvas::Canvas(QObject * parent, const char *)
:QGraphicsScene(parent)
{
    zoom = 1.0;
}

Canvas::~Canvas()
{
}

void debugCatch()
{
}

void Canvas::drawBackground(QPainter * painter, const QRectF & clip)
{
    int l1;
    QPointF qp_in[2], qp_out[2];
    float zoom_dx, zoom_dy, val;
    QString qs;
    static int countt;

    std::cout << __FUNCTION__ << countt++ << std::endl;
    debugCatch();

    zoom_dx = dx / zoom;
    zoom_dy = dy / zoom;
    painter->fillRect(clip, QBrush(QColor(CANVAS_COLOR_BG)));

    for (l1 = 0; l1 <= w / zoom_dx; l1++) {
        val = ((float) l1 * zoom_dx - (float) w / 2.0) / (float) scale;
        if (floor(val) == ceil(val)) {
            painter->setPen(QColor(CANVAS_GRID_COLOR_LIGHT));
        }
        else {
            painter->setPen(QColor(CANVAS_GRID_COLOR_DARK));
        }
        qp_in[0].setX(l1 * zoom_dx);
        qp_in[0].setY(0);
        qp_in[1].setX(l1 * zoom_dx);
        qp_in[1].setY(h);
        qp_out[0] = matrix.map(qp_in[0]);
        qp_out[1] = matrix.map(qp_in[1]);
        painter->drawLine(qp_out[0], qp_out[1]);
        if (floor(val) == ceil(val)) {
            qs.sprintf("%6.0f", val);
            painter->setFont(synthdata->bigFont);
            painter->
                drawText(QPointF(qp_out[1].x() - 20, qp_out[1].y() + 15),
                         qs);
        }
    }
    for (l1 = 0; l1 <= h / zoom_dy; l1++) {
        val = -((float) l1 * zoom_dy - (float) w / 2.0) / (float) scale;
        if (floor(val) == ceil(val)) {
            painter->setPen(QColor(CANVAS_GRID_COLOR_LIGHT));
        }
        else {
            painter->setPen(QColor(CANVAS_GRID_COLOR_DARK));
        }
        qp_in[0].setY(l1 * zoom_dy);
        qp_in[0].setX(0);
        qp_in[1].setY(l1 * zoom_dy);
        qp_in[1].setX(w);
        qp_out[0] = matrix.map(qp_in[0]);
        qp_out[1] = matrix.map(qp_in[1]);
        painter->drawLine(qp_out[0], qp_out[1]);
        if (floor(val) == ceil(val)) {
            qs.sprintf("%7.0f", val);
            painter->setFont(synthdata->bigFont);
            painter->
                drawText(QPointF(qp_out[0].x() - 44, qp_out[0].y() + 4),
                         qs);
        }
    }
}

void Canvas::setZoom(float p_zoom)
{
    zoom = p_zoom;
}

void Canvas::setMatrix(QMatrix p_matrix)
{
    matrix = p_matrix;
}

void Canvas::setGrid(int p_border_l, int p_border_r, int p_border_b,
                     int p_border_t, int p_scale, int p_w, int p_h,
                     float p_dx, float p_dy)
{
    border_l = p_border_l;
    border_r = p_border_r;
    border_b = p_border_b;
    border_t = p_border_t;
    scale = p_scale;
    w = p_w;
    h = p_h;
    dx = p_dx;
    dy = p_dy;
}
