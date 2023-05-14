#include "autosampletask.h"
#include <QDebug>
#include <QRect>
#include <QThread>

AutoSampleTask::AutoSampleTask(QImage img,int in_width,int in_height,int out_width,int out_height,int rect_width,int rect_height,int interval,QPoint center_sampler,QPoint center_rect,QString prefix,QObject *parent) :
    QObject(parent),
    img(img),
    in_width(in_width),
    in_height(in_height),
    out_width(out_width),
    out_height(out_height),
    rect_width(rect_width),
    rect_height(rect_height),
    interval(interval),
    center_sampler(center_sampler),
    center_rect(center_rect),
    prefix(prefix),
    width(img.width()-out_width),
    height(img.height()-out_height),
    border_width((out_width-in_width)/2),
    border_height((out_height-in_height)/2),
    left(0,0,center_rect.x()-rect_width/2,img.height()),
    top(0,0,img.width(),center_rect.y()-rect_height/2),
    right(center_rect.x()+rect_width/2,0,img.width()-center_rect.x()-rect_width/2,img.height()),
    bottom(0,center_rect.y()+rect_height/2,img.width(),img.height()-center_rect.y()-rect_height/2),
    total((img.width() - out_width)/interval * (img.height()- out_height)/interval),
    count(0)
{

}

void AutoSampleTask::working()
{
    //qDebug()<<"subThread:"<<QThread::currentThread();
    for (int i = 0; i < width; i=i+interval) {
        for (int j = 0; j < height; j=j+interval) {
            QRect rect(i,j,out_width,out_height);
            if(left.contains(rect) || top.contains(rect) || right.contains(rect) || bottom.contains(rect)){
                QImage outside,inside;
                outside = img.copy(i,j,out_width,out_height);
                if(in_width>0 && in_height>0){
                    inside = img.copy(i+border_width,j+border_height,in_width,in_height);
                    for (int x = border_width; x < (out_width+in_width)/2; ++x) {
                        for (int y = border_height; y < (out_height+in_height)/2; ++y) {
                            outside.setPixel(x,y,0xFF000000);
                        }
                    }
                }
                int center_x = i+out_width/2;
                int center_y = j+out_height/2;
                QString str=QString("%1_%2_%3").arg(prefix).arg(center_x).arg(center_y);
                outside.save(str+"_B.png");
                if(!inside.isNull())inside.save(str+"_C.png");
                emit process((qreal)count++/total*100);
            }
        }
    }
    emit finish(count);
}
