#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <vector>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QUrl>
#include <QDragMoveEvent>
#include <QGraphicsScene>
#include "mygraphicsitem.h"
#include <QGraphicsSceneMouseEvent>
#include "mypixmapitem.h"
#include "myrectangle.h"

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyGraphicsView(QWidget *parent = nullptr);
    ~MyGraphicsView();
    //首次设置图片
    void initPixmap(QImage img);
    //设置图片并显示
    void setPixmap(QImage img);
    //获取当前控件的图片
    QPixmap getPixmap();
    //将当前显示图像重新加载到窗口中心
    void reloadPixmap();
    //缩放函数
    void zoom(qreal factor);
    //加入采样器
    void addRectSampler();
    //获取采样器大小
    QSize getSamplerOutSize();
    QSize getSamplerInSize();
    //获取采样器在pixmapitem的坐标
    QPointF getSamplerLocation();
    //设置采样器在pixmapitem的坐标
    void setSamplerLocation(QPointF pos);
    //设置采样器内外宽高
    void setSamplerInsideWidth(int value);
    void setSamplerInsideHeight(int value);
    void setSamplerOutsideWidth(int value);
    void setSamplerOutsideHeight(int value);
    //添加保留区域
    void addRectangle();
    //获取保留区域坐标
    QPointF getRectangleLocation();
    //设置保留区域坐标
    void setRectangleLocation(QPointF pos);
    //获取保留区域大小
    QSize getRectangleSize();
    //设置保留区域大小
    void setRectangleWidth(int value);
    void setRectangleHeight(int value);


//重写事件响应函数
protected:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event){
        //按下鼠标左键要移动图像时，记录按下位置
        if(event->button()==Qt::LeftButton && !rectSampler->isUnderMouse() && !rectangle->isUnderMouse() && !rectSampler->in_edge->isUnderMouse() && !rectSampler->out_edge->isUnderMouse()){
            allowMove = true;
            lastPos = mapToScene(event->pos());
        }
        QGraphicsView::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event){allowMove = false;QGraphicsView::mouseReleaseEvent(event);}
    void mouseDoubleClickEvent(QMouseEvent *event);
    //拖动图片到窗口内打开
    void dragEnterEvent(QDragEnterEvent *event);
    //QGraphicsView默认的dragMoveEvent中ignore了拖拽事件，导致信号传递终止，重写该方法使信号继续传递
    void dragMoveEvent(QDragMoveEvent *event){event->accept();}
    void dropEvent(QDropEvent *event);

private:
    MyPixmapItem *pixmapItem;
    QGraphicsScene *scene;
    //矩形采样器
    MyGraphicsItem *rectSampler;
    //保留区域
    MyRectangle *rectangle;
    //记录上次鼠标位置
    QPointF lastPos;
    //标记图像是否可移动
    bool allowMove;

signals:
    //右键单击信号
    void rightButtonClick(int x,int y);
    //右键拖曳信号
    void rightButtonDrag(int lx,int ty,int rx,int by);
    //右键拖曳保留区域点的信号
    void rightButtonDragCtrl(int lx,int ty,int rx,int by);
    //鼠标移动信号
    void mouseMove(int x,int y);
    //鼠标右键被按下并移动信号
    void mouseMoveWithRightButton(int x,int y);
    //鼠标右键和shift键被按下并移动鼠标的信号
    void mouseMoveWithRightButtonAndShift(int x,int y);
    //文件拖曳信号
    void dragFile(QString file_name);
    //采样器移动信号
    void centerMove(int x, int y);
    //采样器大小发生变化
    void outsideChanged(int width, int height);
    void insideChanged(int width, int height);
    //保留区域移动信号
    void rectangleCenterMove(int x, int y);

public slots:
};

#endif // MYGRAPHICSVIEW_H
