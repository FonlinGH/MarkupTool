#include "mygraphicsitem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include "mypixmapitem.h"
#include <QPolygonF>
#include <QKeyEvent>

MyGraphicsItem::MyGraphicsItem(QGraphicsItem *parent) :
    QGraphicsItem(parent),
    outPos(50.5, 50.5),
    inPos(5.5, 5.5),
    paint_color(0, 255, 255)//默认浅蓝色
{
    out_edge = new EdgePoint(EdgePoint::Outside, this);
    in_edge = new EdgePoint(EdgePoint::Inside, this);

    this->setFlags(QGraphicsItem::ItemIsSelectable |
                   QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsFocusable |
                   QGraphicsItem::ItemSendsScenePositionChanges);

    connect(out_edge, &EdgePoint::outsideChanged, this, [=](){
        emit outsideChanged(2*outPos.x(),2*outPos.y());
    });
    connect(in_edge, &EdgePoint::insideChanged, this, [=](){
        emit insideChanged(2*inPos.x(),2*inPos.y());
    });
}

QRectF MyGraphicsItem::boundingRect() const
{
    return QRectF(-outPos.x(),-outPos.y(),2*outPos.x(),2*outPos.y());
}

void MyGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(paint_color);
    painter->drawRect(QRectF(-outPos.x(),-outPos.y(),2*outPos.x(),2*outPos.y()));
    painter->drawRect(QRectF(-inPos.x(),-inPos.y(),2*inPos.x(),2*inPos.y()));

    painter->setBrush(QBrush(Qt::red));
    painter->drawEllipse(QRectF(-1,-1,2.0,2.0));

    out_edge->setPos(outPos);
    in_edge->setPos(inPos);
}

void MyGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

    if ( event->buttons() == Qt::LeftButton ) {
//        //以这种方式实现移动精度不高
//        QPointF offset = event->pos() - event->lastPos();
//        moveBy(offset.x(), offset.y());
        this->scene()->update();
        emit centerMove(this->pos().x(), this->pos().y());
    }
    QGraphicsItem::mouseMoveEvent(event);
}

//限制采样器活动区域
QVariant MyGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) //控件发生移动
    {
        QPointF newPos = value.toPointF(); //即将要移动的位置，这个坐标是在pixmap中的坐标
        MyPixmapItem *parent = static_cast<MyPixmapItem *>(this->parentItem());
        QRectF rect(outPos.x(), outPos.y(), parent->pixmap().width()-2*outPos.x(), parent->pixmap().height()-2*outPos.y()); //要限制的区域
        if (!rect.contains(newPos)) //如果超出区域，重新设置控件位置
        {
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return newPos;
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void MyGraphicsItem::keyPressEvent(QKeyEvent *event)
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
