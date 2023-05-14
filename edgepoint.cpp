#include "edgepoint.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "mygraphicsitem.h"
#include <QGraphicsScene>
#include <QDebug>
#include "mypixmapitem.h"

EdgePoint::EdgePoint(PointType type, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    m_type(type)
{
    this->setFlags(QGraphicsItem::ItemIsSelectable |
                   QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsFocusable |
                   QGraphicsItem::ItemSendsScenePositionChanges);
}

QRectF EdgePoint::boundingRect() const
{
    return QRectF(-2,-2,4,4);
}

void EdgePoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(QColor(255, 85, 0));
    painter->drawRect(-2,-2,4,4);
}

void EdgePoint::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->buttons()==Qt::LeftButton){
        MyGraphicsItem *parent = static_cast<MyGraphicsItem *>(this->parentItem());
        switch (m_type) {
        case Outside:
            QGraphicsItem::mouseMoveEvent(event);
            parent->outPos=this->pos();
            this->scene()->update();
            emit outsideChanged();
            break;
        case Inside:
            QGraphicsItem::mouseMoveEvent(event);
            parent->inPos=this->pos();
            this->scene()->update();
            emit insideChanged();
            break;
        }
    }
}

//限制功能点的活动区域
QVariant EdgePoint::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) //控件发生移动
    {
        QPointF newPos = value.toPointF(); //即将要移动的位置，这个坐标是在采样器中的坐标
        MyGraphicsItem *sampler = static_cast<MyGraphicsItem *>(this->parentItem());
        MyPixmapItem *pixmapItem = static_cast<MyPixmapItem *>(sampler->parentItem());
        int width = pixmapItem->pixmap().width();
        int height = pixmapItem->pixmap().height();
        QRectF rect(0, 0, qMin(sampler->pos().x(),width-sampler->pos().x()), qMin(sampler->pos().y(),height-sampler->pos().y())); //要限制的区域
        if (!rect.contains(newPos)) //是否在区域内
        {
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return newPos;
        }
    }
    return QGraphicsItem::itemChange(change, value);
}
