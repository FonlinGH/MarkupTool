#ifndef MYGRAPHICSITEM_H
#define MYGRAPHICSITEM_H

#include <QObject>
#include <QGraphicsItem>
#include "edgepoint.h"

class MyGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    explicit MyGraphicsItem(QGraphicsItem *parent = nullptr);

    //调整外矩形框大小的点
    EdgePoint *out_edge;
    //调整内矩形框大小的点
    EdgePoint *in_edge;

    QPointF outPos;
    QPointF inPos;

protected:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void keyPressEvent(QKeyEvent *event) override;

private:    
    //绘制用的颜色
    QColor paint_color;

signals:
    void centerMove(int x, int y);
    void outsideChanged(int width, int height);
    void insideChanged(int width, int height);

public slots:
};

#endif // MYGRAPHICSITEM_H
