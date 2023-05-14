#ifndef AUTOSAMPLETASK_H
#define AUTOSAMPLETASK_H

#include <QImage>
#include <QObject>
#include <QPoint>

class AutoSampleTask : public QObject
{
    Q_OBJECT
public:
    explicit AutoSampleTask(QImage img,int in_width,int in_height,int out_width,int out_height,int rect_width,int rect_height,int interval,QPoint center_sampler,QPoint center_rect,QString prefix,QObject *parent = nullptr);

    QImage img;
    int in_width;
    int in_height;
    int out_width;
    int out_height;
    int rect_width;
    int rect_height;
    int interval;
    QPoint center_sampler;
    QPoint center_rect;
    QString prefix;

    int width;
    int height;
    int border_width;
    int border_height;
    QRect left;
    QRect top;
    QRect right;
    QRect bottom;

    int total;
    int count;

signals:
    void process(int percent);
    void finish(int count);

public slots:
    void working();
};

#endif // AUTOSAMPLETASK_H
