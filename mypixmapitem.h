#ifndef MYPIXMAPITEM_H
#define MYPIXMAPITEM_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

class MyPixmapItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit MyPixmapItem(QGraphicsPixmapItem *parent = nullptr);

//    //视图边界，为了窗口内始终看得见图像
//    qreal left_bound;
//    qreal right_bound;
//    qreal top_bound;
//    qreal bottom_bound;

protected:
    virtual QRectF boundingRect() const override;
    //virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    //记录拖曳起始位置
    QPointF start;

signals:
    //右键单击信号
    void rightButtonClick(int x,int y);
    //右键拖曳信号
    void rightButtonDrag(int lx,int ty,int rx,int by);
    //右键拖曳保留区域点的信号
    void rightButtonDragCtrl(int lx,int ty,int rx,int by);
    //鼠标右键被按下并移动信号
    void mouseMoveWithRightButton(int x,int y);
    //鼠标右键和shift键被按下并移动鼠标的信号
    void mouseMoveWithRightButtonAndShift(int x,int y);

public slots:
};

#endif // MYPIXMAPITEM_H
