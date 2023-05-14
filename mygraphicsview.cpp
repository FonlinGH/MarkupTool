#include "mygraphicsview.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDebug>

MyGraphicsView::MyGraphicsView(QWidget *parent) :
    QGraphicsView(parent),
    pixmapItem(new MyPixmapItem()),
    scene(new QGraphicsScene()),
    rectSampler(new MyGraphicsItem(pixmapItem)),
    rectangle(new MyRectangle(11, 11, pixmapItem)),
    allowMove(false)
{
    //隐藏水平、垂直滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //使用抗锯齿渲染，让图像更平滑
    setRenderHint(QPainter::Antialiasing);

    //设置场景大小,参数x，y决定左上角的位置
    scene->setSceneRect(-10000, -10000, 20000, 20000);

    //设置图像显示位置
    pixmapItem->setPos(0,0);

    //将图元和当前对象加入到场景
    scene->addItem(pixmapItem);
    this->setScene(scene);

    //隐藏采样器和保留区域
    rectSampler->hide();
    rectangle->hide();

    //显示当前对象
    this->show();

    //传递信号到主窗口
    connect(pixmapItem, &MyPixmapItem::rightButtonClick, this, &MyGraphicsView::rightButtonClick);
    connect(pixmapItem, &MyPixmapItem::rightButtonDrag, this, &MyGraphicsView::rightButtonDrag);
    connect(pixmapItem, &MyPixmapItem::rightButtonDragCtrl, this, &MyGraphicsView::rightButtonDragCtrl);
    connect(pixmapItem, &MyPixmapItem::mouseMoveWithRightButton, this, &MyGraphicsView::mouseMoveWithRightButton);
    connect(pixmapItem, &MyPixmapItem::mouseMoveWithRightButtonAndShift, this, &MyGraphicsView::mouseMoveWithRightButtonAndShift);
    connect(rectSampler, &MyGraphicsItem::centerMove, this, &MyGraphicsView::centerMove);
    connect(rectSampler, &MyGraphicsItem::outsideChanged, this, &MyGraphicsView::outsideChanged);
    connect(rectSampler, &MyGraphicsItem::insideChanged, this, &MyGraphicsView::insideChanged);
    connect(rectangle, &MyRectangle::centerMove, this, &MyGraphicsView::rectangleCenterMove);
}

MyGraphicsView::~MyGraphicsView()
{
    //销毁当前对象时，销毁成员变量
    delete pixmapItem;
    scene->deleteLater();
}

void MyGraphicsView::initPixmap(QImage img)
{
    //将img设置到图元
    QPixmap pix=QPixmap::fromImage(img);
    pixmapItem->setPixmap(pix);
    //设置图像显示位置
    pixmapItem->setPos(0,0);
    //让相机对准图片中心
    centerOn(pixmapItem);
    //恢复缩放比例
    QMatrix m(1,matrix().m12(),matrix().m21(),1,matrix().dx(),matrix().dy());
    setMatrix(m);
    //隐藏采样器
    rectSampler->hide();
    rectangle->hide();
//    //根据图片大小设置视图边界
//    pixmapItem->left_bound=-(viewport()->rect().width()/2-pixmapItem->pixmap().width()/4);
//    pixmapItem->right_bound=pixmapItem->pixmap().width()*3/4+viewport()->rect().width()/2;
//    pixmapItem->top_bound=-(viewport()->rect().height()/2-pixmapItem->pixmap().height()/4);
//    pixmapItem->bottom_bound=pixmapItem->pixmap().height()*3/4+viewport()->rect().height()/2;
}

void MyGraphicsView::setPixmap(QImage img)
{
    //将img设置到图元
    QPixmap pix=QPixmap::fromImage(img);
    pixmapItem->setPixmap(pix);
}

QPixmap MyGraphicsView::getPixmap()
{
    return pixmapItem->pixmap();
}

void MyGraphicsView::reloadPixmap()
{
    if(pixmapItem->pixmap().isNull()){
        return;
    }
    //设置图像显示位置
    pixmapItem->setPos(0,0);
    centerOn(pixmapItem);
}

void MyGraphicsView::zoom(qreal factor)
{
    //防止缩得太小或放得太大
    qreal t = transform().scale(factor, factor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (t < 0.07 || t > 100)
        return ;

    scale(factor, factor);
}

void MyGraphicsView::addRectSampler()
{
    centerOn(pixmapItem);
    rectSampler->setPos(pixmapItem->pixmap().width()/2.0,pixmapItem->pixmap().height()/2.0);
    int width = pixmapItem->pixmap().width();
    int height= pixmapItem->pixmap().height();
    rectSampler->outPos.setX(width>=101?50.5:width/2.0);
    rectSampler->outPos.setY(height>=101?50.5:height/2.0);
    rectSampler->inPos.setX(width>=11?5.5:width/2.0);
    rectSampler->inPos.setY(height>=11?5.5:height/2.0);
    scene->update();
    rectSampler->show();
    rectangle->hide();
}

QSize MyGraphicsView::getSamplerOutSize()
{
    return QSize(rectSampler->outPos.x()*2, rectSampler->outPos.y()*2);
}

QSize MyGraphicsView::getSamplerInSize()
{
    return QSize(rectSampler->inPos.x()*2, rectSampler->inPos.y()*2);
}

QPointF MyGraphicsView::getSamplerLocation()
{
    return rectSampler->pos();
}

void MyGraphicsView::setSamplerLocation(QPointF pos)
{
    rectSampler->setPos(pos);
}

void MyGraphicsView::setSamplerInsideWidth(int value)
{
    rectSampler->inPos.setX(value/2.0);
    scene->update();
}

void MyGraphicsView::setSamplerInsideHeight(int value)
{
    rectSampler->inPos.setY(value/2.0);
    scene->update();
}

void MyGraphicsView::setSamplerOutsideWidth(int value)
{
    rectSampler->outPos.setX(value/2.0);
    scene->update();
}

void MyGraphicsView::setSamplerOutsideHeight(int value)
{
    rectSampler->outPos.setY(value/2.0);
    scene->update();
}

void MyGraphicsView::addRectangle()
{
    centerOn(pixmapItem);
    rectangle->setPos(pixmapItem->pixmap().width()/2.0,pixmapItem->pixmap().height()/2.0);
    rectangle->width  = 11;
    rectangle->height = 11;
    scene->update();
    rectangle->show();
}

QPointF MyGraphicsView::getRectangleLocation()
{
    return rectangle->pos();
}

void MyGraphicsView::setRectangleLocation(QPointF pos)
{
    rectangle->setPos(pos);
}

QSize MyGraphicsView::getRectangleSize()
{
    return QSize(rectangle->width, rectangle->height);
}

void MyGraphicsView::setRectangleWidth(int value)
{
    rectangle->width = value;
    scene->update();
}

void MyGraphicsView::setRectangleHeight(int value)
{
    rectangle->height = value;
    scene->update();
}

//当滑轮滚动时触发该函数，进行图像缩放
void MyGraphicsView::wheelEvent(QWheelEvent *event)
{
    //当滑轮滚动时，获取其滚动量
    QPoint amount=event->angleDelta();
    //正值表示放大，负值表示缩小
    amount.y()>0?zoom(1.1):zoom(0.9);
}

//当鼠标移动时触发该函数
void MyGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    //拖动图像过程中不更新左下角坐标
    if(event->buttons()!=Qt::LeftButton){
        //获取鼠标指针在图像中的坐标
        QPointF pos=pixmapItem->mapFromScene(mapToScene(event->pos()));
        int x=pos.x();
        int y=pos.y();
        int width=pixmapItem->pixmap().width();
        int height=pixmapItem->pixmap().height();
        if(x>=0 && x<width && y>=0 &&y<height){
            emit mouseMove(x,y);
        }
    }
    //移动图像位置
    else if(event->buttons()==Qt::LeftButton && allowMove){
        QPointF offset = mapToScene(event->pos()) - lastPos;
        pixmapItem->moveBy(offset.x(), offset.y());
        lastPos = mapToScene(event->pos());
        scene->update();
    }

    //调用父类的鼠标事件让事件能够传递到pixmapItem
    QGraphicsView::mouseMoveEvent(event);
}

void MyGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    //双击窗口复位
    if(!pixmapItem->pixmap().isNull()&&event->button()==Qt::LeftButton){
        //设置图像显示位置
        pixmapItem->setPos(0,0);
        //恢复缩放比例
        QMatrix m(1,matrix().m12(),matrix().m21(),1,matrix().dx(),matrix().dy());
        setMatrix(m);
        //让相机对准图片中心
        centerOn(pixmapItem);
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void MyGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    //如果拖进窗口的文件类型是png、jpg、jpeg、bmp，接受这类文件
    if(!event->mimeData()->urls()[0].fileName().right(3).compare("png")||!event->mimeData()->urls()[0].fileName().right(3).compare("jpg")||!event->mimeData()->urls()[0].fileName().right(3).compare("jpeg")||!event->mimeData()->urls()[0].fileName().right(3).compare("bmp")){
        event->accept();
    }
    else{
        event->ignore();//否则不接受鼠标事件
    }

    //QGraphicsView::dragEnterEvent(event);
}

void MyGraphicsView::dropEvent(QDropEvent *event){
    //从event中获取文件路径
    const QMimeData *data=event->mimeData();
    //向主窗口传递信号
    QString file_name=data->urls()[0].toLocalFile();
    emit dragFile(file_name);

    //QGraphicsView::dropEvent(event);
}
