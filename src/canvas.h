#ifndef CANVAS_H
#define CANVAS_H

#include <stdio.h>
#include <stdlib.h>
#include <QGraphicsScene>
#include <qmatrix.h>

#define CANVAS_GRID_COLOR_LIGHT 0xC0C0C0
#define CANVAS_GRID_COLOR_DARK  0x707070
#define CANVAS_COLOR_BG         0x141450

class Canvas:public QGraphicsScene {
  Q_OBJECT private:
    float zoom, dx, dy, w, h;
    int border_l, border_r, border_b, border_t, scale;
    QMatrix matrix;

  public:
     Canvas(QObject * parent = 0, const char *name = 0);
    ~Canvas();
    void setZoom(float p_zoom);
    void setMatrix(QMatrix p_matrix);
    void setGrid(int p_border_l, int p_border_r, int p_border_b,
                 int p_border_t, int p_scale, int p_w, int p_h, float p_dx,
                 float p_dy);

  protected:
     virtual void drawBackground(QPainter * r, const QRectF &);

};

#endif
