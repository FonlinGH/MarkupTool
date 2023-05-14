#include "mypixmapitem.h"
#include <QDebug>
#include <QGraphicsScene>

MyPixmapItem::MyPixmapItem(QGraphicsPixmapItem *parent) : QGraphicsPixmapItem(parent)
{
    this->setFlags(QGraphicsItem::ItemIsSelectable |
                   QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsFocusable);
}

QRectF MyPixmapItem::boundingRect() const
{
    return QRectF(0, 0, pixmap().width(), pixmap().height());
}

//void MyPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
//{
//    Q_UNUSED(painter)
//    Q_UNUSED(option)
//    Q_UNUSED(widget)
//}

void MyPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //记录拖曳起始位置
    if(event->button()==Qt::RightButton){
        start = event->pos();
    }

    //QGraphicsPixmapItem::mousePressEvent(event);
}

void MyPixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
//    //当鼠标左键在按住状态下移动时,计算光标偏移量(这里不能用event->button()==Qt::LeftButton)
//    if(event->buttons()==Qt::LeftButton){
//        QPointF offset = event->pos() - event->lastPos();
//        moveBy(offset.x(), offset.y());
//        this->scene()->update();
//    }
    //当鼠标右键和shift被按下，鼠标移动过程中对轨迹上的点取消标注
    if(event->buttons()==Qt::RightButton && event->modifiers()==Qt::ShiftModifier){
        QRect rect(1, 1, pixmap().width()-2, pixmap().height()-2);
        if(rect.contains(x, y))emit mouseMoveWithRightButtonAndShift(x,y);
    }
    //当鼠标右键在按住状态下移动时,对轨迹上的点进行标注
    else if(event->buttons()==Qt::RightButton && event->modifiers()!=Qt::AltModifier && event->modifiers()!=Qt::ControlModifier){
        emit mouseMoveWithRightButton(x,y);
    }

    //QGraphicsPixmapItem::mouseMoveEvent(event);
}

void MyPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button()==Qt::RightButton){
        QPointF offset = event->pos() - start;
        if(qAbs(offset.x())>=1 || qAbs(offset.y())>=1){
            //qDebug()<<"11111";
            //获取区域左上角和右下角的坐标
            int larger_x,smaller_x,larger_y,smaller_y;
            event->pos().x()>start.x()?larger_x=event->pos().x():larger_x=start.x();
            event->pos().x()>start.x()?smaller_x=start.x():smaller_x=event->pos().x();
            event->pos().y()>start.y()?larger_y=event->pos().y():larger_y=start.y();
            event->pos().y()>start.y()?smaller_y=start.y():smaller_y=event->pos().y();
            //将区域约束到图像内
            larger_x  = qMin(larger_x, pixmap().width()-1);
            smaller_x = qMax(0, smaller_x);
            larger_y  = qMin(larger_y, pixmap().height()-1);
            smaller_y = qMax(0, smaller_y);
            if(event->modifiers()==Qt::AltModifier){
                //qDebug()<<smaller_x<<smaller_y<<larger_x<<larger_y;
                emit rightButtonDrag(smaller_x,smaller_y,larger_x,larger_y);
            }
            else if(event->modifiers() == Qt::ControlModifier){
                //qDebug()<<smaller_x<<smaller_y<<larger_x<<larger_y;
                emit rightButtonDragCtrl(smaller_x,smaller_y,larger_x,larger_y);
            }
        }
        else{
            int x = event->pos().x();
            int y = event->pos().y();
            //qDebug()<<x<<y;
            emit rightButtonClick(x,y);
        }
    }

    //QGraphicsPixmapItem::mouseReleaseEvent(event);
}
