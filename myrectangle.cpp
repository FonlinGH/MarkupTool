#include "myrectangle.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <mypixmapitem.h>
#include <QKeyEvent>

MyRectangle::MyRectangle(qreal w, qreal h, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    width(w),
    height(h)
{
    this->setFlags(QGraphicsItem::ItemIsSelectable |
                   QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsFocusable |
                   QGraphicsItem::ItemSendsScenePositionChanges);
}

QRectF MyRectangle::boundingRect() const
{
    return QRectF(-width/2.0,-height/2.0,width,height);
}

void MyRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(Qt::red);
    painter->drawRect(QRectF(-width/2.0,-height/2.0,width,height));
}

void MyRectangle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ( event->buttons() == Qt::LeftButton ) {
        this->scene()->update();
        emit centerMove(this->pos().x(), this->pos().y());
    }
    QGraphicsItem::mouseMoveEvent(event);
}

QVariant MyRectangle::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) // 控件发生移动
    {
        QPointF newPos = value.toPointF(); //即将要移动的位置，这个坐标是在pixmap中的坐标
        MyPixmapItem *parent = static_cast<MyPixmapItem *>(this->parentItem());
        QRectF rect(width/2.0, height/2.0, parent->pixmap().width()-width, parent->pixmap().height()-height); // 你要限制的区域
        if (!rect.contains(newPos)) // 是否在区域内
        {
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return newPos;
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void MyRectangle::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        moveBy(0, -1.0);
        break;
    case Qt::Key_Down:
        moveBy(0, 1.0);
        break;
    case Qt::Key_Left:
        moveBy(-1.0, 0);
        break;
    case Qt::Key_Right:
        moveBy(1.0, 0);
        break;
    default:
        break;
    }
    emit centerMove(this->pos().x(), this->pos().y());
}
