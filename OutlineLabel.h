#ifndef OUTLINELABEL_H
#define OUTLINELABEL_H

#include <QtCore>
#include <QWidget>
#include <QLabel>
#include <QPen>
#include <QPainter>
#include <QPaintEvent>

class OutlineLabel : public QLabel {
  Q_OBJECT;

  public:
    OutlineLabel(QWidget *parent = 0);
    ~OutlineLabel();
    void paintEvent(QPaintEvent *event);

};

#endif
