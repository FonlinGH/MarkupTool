#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include "mygraphicsview.h"
#include "opencv2/opencv.hpp"
#include <QDebug>
#include <QImageReader>
#include <QStackedWidget>
#include "aboutdialog.h"
#include <QColorDialog>
#include "selectmergemapdialog.h"

using namespace cv;
using namespace std;

//重要的状态：merge_stat、allowMark、method
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      status_label(new QLabel()),
      progress(new QProgressBar()),
      welcome_label(new QLabel()),
      merge_stat(false),
      method(-1),
      close_panel(false),
      mark_color(255,0,255),//默认紫色
      transparence(0),//默认背景不透明
      allowMark(false)
{
    ui->setupUi(this);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    //设置窗口大小
    this->resize(1400,900);
    //将status_label设置到状态栏
    ui->statusbar->addWidget(status_label);
    ui->statusbar->addWidget(progress);
    progress->hide();
    //显示“把图片拖到此处打开”
    QFont font("楷体",20,QFont::Bold);
    welcome_label->setFont(font);
    welcome_label->setText("把图片拖到此处打开");
    welcome_label->setAlignment(Qt::AlignCenter);
    welcome_label->setStyleSheet("color:gray;");
    welcome_label->resize(260,30);
    welcome_label->setGeometry(this->width()/2-welcome_label->width()/2,this->height()/2-welcome_label->height()/2,welcome_label->width(),welcome_label->height());
    //将m_layout装进graphicsView，然后把welcome_label放进m_layout，设置居中对齐
    m_layout = new QHBoxLayout(ui->graphicsView);
    m_layout->addWidget(welcome_label);
    m_layout->setAlignment(welcome_label, Qt::AlignCenter);
    //隐藏参数面板stackedwidget
    ui->stackedWidget->hide();

    //构造右键菜单项
    m_menu = new QMenu(this);
    m_open = new QAction("打开",this);
    m_openEdgeimg = new QAction("导入边缘检测图",this);
    m_saveEdgeimg = new QAction("保存边缘图像（黑白）",this);
    m_saveImg = new QAction("保存当前图像",this);
    m_changeColor = new QAction("设置标注颜色",this);
    m_merge = new QAction("合并",this);
    m_split = new QAction("拆分",this);
    m_exit = new QAction("退出",this);

    m_menu->addAction(m_open);
    m_menu->addAction(m_openEdgeimg);
    m_menu->addAction(m_saveEdgeimg);
    m_menu->addAction(m_saveImg);
    m_menu->addAction(m_changeColor);
    m_menu->addAction(m_merge);
    m_menu->addAction(m_split);
    m_menu->addAction(m_exit);

    connect(m_open, &QAction::triggered, this, &MainWindow::on_actionopen_triggered);
    connect(m_openEdgeimg, &QAction::triggered, this, &MainWindow::on_actionopenEdgeimg_triggered);
    connect(m_saveEdgeimg, &QAction::triggered, this, &MainWindow::on_actionsave_triggered);
    connect(m_saveImg, &QAction::triggered, this, &MainWindow::on_actionscreenshot_triggered);
    connect(m_changeColor, &QAction::triggered, this, &MainWindow::on_actionsetcolor_triggered);
    connect(m_merge, &QAction::triggered, this, &MainWindow::on_actionmerge_triggered);
    connect(m_split, &QAction::triggered, this, &MainWindow::on_actionsplit_triggered);
    connect(m_exit, &QAction::triggered, this, &MainWindow::on_actionexit_triggered);

    connect(ui->sliderForV,&QSlider::valueChanged,this,&MainWindow::sliderForVSlot);
    connect(ui->sliderForThreshold1,&QSlider::valueChanged,this,&MainWindow::sliderForThreshold1Slot);
    connect(ui->sliderForThreshold2,&QSlider::valueChanged,this,&MainWindow::sliderForThreshold2Slot);
    connect(ui->graphicsView,&MyGraphicsView::rightButtonClick,this,&MainWindow::rightButtonClickSlot);
    connect(ui->graphicsView,&MyGraphicsView::rightButtonDrag,this,&MainWindow::rightButtonDragSlot);
    connect(ui->graphicsView,&MyGraphicsView::rightButtonDragCtrl,this,&MainWindow::rightButtonDragCtrlSlot);
    connect(ui->graphicsView,&MyGraphicsView::mouseMove,this,&MainWindow::mouseMoveSlot);
    connect(ui->graphicsView,&MyGraphicsView::mouseMoveWithRightButton,this,&MainWindow::mouseMoveWithRightButtonSlot);
    connect(ui->graphicsView,&MyGraphicsView::mouseMoveWithRightButtonAndShift,this,&MainWindow::mouseMoveWithRightButtonAndShiftSlot);
    connect(ui->graphicsView,&MyGraphicsView::dragFile,this,&MainWindow::openFile);
    connect(ui->graphicsView,&MyGraphicsView::centerMove,this,[=](int x, int y){
        ui->spinBox_5->setValue(x);
        ui->spinBox_6->setValue(y);
    });
    connect(ui->graphicsView,&MyGraphicsView::outsideChanged,this,[=](int width, int height){
        ui->spinBox_3->setValue(width);
        ui->spinBox_4->setValue(height);
    });
    connect(ui->graphicsView,&MyGraphicsView::insideChanged,this,[=](int width, int height){
        ui->spinBox->setValue(width);
        ui->spinBox_2->setValue(height);
    });
    connect(ui->graphicsView,&MyGraphicsView::rectangleCenterMove,this,[=](int x, int y){
        ui->spinBox_9->setValue(x);
        ui->spinBox_10->setValue(y);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
    delete status_label;
    delete welcome_label;
}

QImage MainWindow::matToImage(Mat mat)
{
    //判断Mat的类型，从而返回不同类型的img
    switch (mat.type()) {
        case CV_8UC1:{
            //qDebug()<<"using CV_8UC1";
            QImage img((uchar*)mat.data,mat.cols,mat.rows,mat.cols*1,QImage::Format_Grayscale8);
            return img;
        }
        case CV_8UC3:{
            QImage img((uchar*)mat.data,mat.cols,mat.rows,mat.cols*3,QImage::Format_RGB888);
            return img.rgbSwapped();
        }
        case CV_8UC4:{
            QImage img((uchar*)mat.data,mat.cols,mat.rows,mat.cols*4,QImage::Format_ARGB32);
            return img;
        }
        default:{
            QImage img;
            return img;
        }
    }
}

Mat MainWindow::fromImage(QImage image)
{
    //根据image类型转换成mat
    switch (image.format()) {
    case QImage::Format_ARGB32:  //8UC4
        return Mat(image.height(),image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
    case QImage::Format_RGB888:   //8UC3
        return Mat(image.height(),image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
    case QImage::Format_Grayscale8:   //8UC1
        return Mat(image.height(),image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
    default:
        break;

    }
    return Mat();
}

//点击菜单栏->打开
void MainWindow::on_actionopen_triggered()
{
    //获取图片路径，加载图片到graphicsView
    fileName = QFileDialog::getOpenFileName(this,"打开一张图片",QCoreApplication::applicationDirPath(),"Images (*.png *.jpg *.jpeg *.bmp)");
    if(fileName.isEmpty()){
        return ;
    }
    QImageReader reader;
    QImage img;
    reader.setDecideFormatFromContent(true);
    reader.setFileName(fileName);
    if(!reader.canRead()||!reader.read(&img))
        return;
    //隐藏参数面板再显示图像
    ui->stackedWidget->hide();
    qApp->processEvents();
    ui->graphicsView->initPixmap(img);
    //设置处理标记，说明当前图像是原图
    allowMark = false;
    merge_stat=false;
    welcome_label->hide();
    //保存图像到缓存,注意要用深拷贝
    origin_img = img.copy();
    if(img.format()!=QImage::Format_Grayscale8){
        img=img.convertToFormat(QImage::Format_Grayscale8);
    }
    gray_img=img.copy();
    //清除图像缓存
    transform_img=QImage();
    edge_img=QImage();
    filted_img=QImage();
    method=-1;
}

//点击菜单栏->保存边缘图像
void MainWindow::on_actionsave_triggered()
{
    //如果未打开图像，直接返回
    if(ui->graphicsView->getPixmap().isNull()){
        return ;
    }
    //如果当前显示图像未经过边缘检测，询问是否直接保存当前图像
    if(edge_img.isNull()&&QMessageBox::No==QMessageBox::question(this,"提示","当前图像未经过边缘检测，是否直接保存当前显示图像？")){
        return ;
    }
    //获取图片保存路径
    QFileInfo file_info(fileName);
    QString save_name=file_info.absolutePath();
    save_name+="/";
    save_name+=file_info.baseName().split("_")[0];
    save_name+=".png";
    save_name = QFileDialog::getSaveFileName(this,"保存边缘检测结果",save_name);

    //保存边缘检测结果
    if(!edge_img.isNull() && !save_name.isEmpty() && edge_img.save(save_name)){
        QMessageBox::information(this,"提示","保存成功");
    }
    //保存当前显示图像
    else if(!save_name.isEmpty() && ui->graphicsView->getPixmap().save(save_name)){
        QMessageBox::information(this,"提示","保存成功");
    }
}

//点击保存当前图像
void MainWindow::on_actionscreenshot_triggered()
{
    //如果当前无显示图像
    if(ui->graphicsView->getPixmap().isNull()){
        QMessageBox::information(this,"提示","当前无显示图像");
        return ;
    }
    //设置保存路径
    QString path=QCoreApplication::applicationDirPath();
    path.append("/");
    path.append(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    path.append(".png");
    //qDebug()<<path;
    QString get_save_path=QFileDialog::getSaveFileName(this,"保存当前显示图像",path);

    //执行保存
    if(!get_save_path.isEmpty() && ui->graphicsView->getPixmap().save(get_save_path)){
        QMessageBox::information(this,"提示","保存成功");
    }
}

//点击保存当前图像的灰度图
void MainWindow::on_actionsaveGrayImg_triggered()
{
    //如果当前无显示图像
    if(ui->graphicsView->getPixmap().isNull()){
        QMessageBox::information(this,"提示","当前无显示图像");
        return ;
    }
    //设置图片保存路径
    QFileInfo file_info(fileName);
    QString save_name=file_info.absolutePath()+"/"+file_info.baseName()+"_Gray.png";
    //qDebug()<<save_name;
    save_name = QFileDialog::getSaveFileName(this,"保存当前图像的灰度图",save_name);
    //执行保存
    if(!save_name.isEmpty() && ui->graphicsView->getPixmap().toImage().convertToFormat(QImage::Format_Grayscale8).save(save_name)){
        QMessageBox::information(this,"提示","保存成功");
    }
}

//点击菜单栏->退出
void MainWindow::on_actionexit_triggered()
{
    this->close();
}

//点击边缘检测->canny
void MainWindow::on_actioncanny_triggered()
{
    //如果未打开图片，直接返回
    if(gray_img.isNull()){
        return ;
    }
    //对图像进行边缘检测并将结果显示到graphicsView中
    Mat src,t,dst;
    //选择图像来源，优先次序为filted_img，transform_img，gray_img
    if(!filted_img.isNull()){
        t=fromImage(filted_img);
        t.copyTo(src);
    }
    else if(!transform_img.isNull()){
        t=fromImage(transform_img);
        t.copyTo(src);
    }
    else{
        t=fromImage(gray_img);
        t.copyTo(src);
    }
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }

    Canny(src,dst,ui->sliderForThreshold1->value(),ui->sliderForThreshold2->value());
    QImage img=matToImage(dst);

    //设置处理标记，说明当前图像是经过边缘检测的
    allowMark = true;
    //保存图像到缓存,注意要用深拷贝
    edge_img=img.copy(0,0,img.width(),img.height());
    //设置合并状态
    merge_stat=false;
    //设置检测方法，显示参数面板
    method=0;
    ui->stackedWidget->setCurrentIndex(method);
    if(!close_panel){
        ui->stackedWidget->show();
    }
    //等参数面板显示后再显示图像
    ui->graphicsView->initPixmap(img);
}

//点击边缘检测->laplace
void MainWindow::on_actionlaplace_triggered()
{
    //如果未打开图片，直接返回
    if(gray_img.isNull()){
        return ;
    }
    //对图像进行边缘检测并将结果显示到graphicsView中
    Mat src,t,dst;
    //选择图像来源，优先次序为filted_img，transform_img，gray_img
    if(!filted_img.isNull()){
        t=fromImage(filted_img);
        t.copyTo(src);
    }
    else if(!transform_img.isNull()){
        t=fromImage(transform_img);
        t.copyTo(src);
    }
    else{
        t=fromImage(gray_img);
        t.copyTo(src);
    }
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }
    Laplacian(src,dst,src.depth());
    for (int x = 0; x < dst.rows; ++x) {
        for (int y = 0; y < dst.cols; ++y) {
            if(dst.at<uchar>(x,y)>ui->sliderForBound->value()){
                dst.at<uchar>(x,y)=255;
            }
            else{
                dst.at<uchar>(x,y)=0;
            }
        }
    }
    QImage img=matToImage(dst);
    //设置处理标记，说明当前图像是经过边缘检测的
    allowMark = true;
    //保存图像到缓存,注意要用深拷贝
    edge_img=img.copy(0,0,img.width(),img.height());
    //设置合并状态
    merge_stat=false;
    //设置检测方式
    method=1;
    ui->stackedWidget->setCurrentIndex(method);
    if(!close_panel){
        ui->stackedWidget->show();
    }
    //等参数面板显示后再显示图像
    ui->graphicsView->initPixmap(img);
}

//点击边缘检测->sobel
void MainWindow::on_actionsobel_triggered()
{
    //如果未打开图片，直接返回
    if(gray_img.isNull()){
        return ;
    }
    //对图像进行边缘检测并将结果显示到graphicsView中
    Mat src,t,dst;
    //选择图像来源，优先次序为filted_img，transform_img，gray_img
    if(!filted_img.isNull()){
        t=fromImage(filted_img);
        t.copyTo(src);
    }
    else if(!transform_img.isNull()){
        t=fromImage(transform_img);
        t.copyTo(src);
    }
    else{
        t=fromImage(gray_img);
        t.copyTo(src);
    }
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }
    //先计算xy方向上的边缘检测图
    Mat sobel_x,sobel_y;
    Sobel(src,sobel_x,CV_64F,1,0);
    Sobel(src,sobel_y,CV_64F,0,1);
    convertScaleAbs(sobel_x,sobel_x);
    convertScaleAbs(sobel_y,sobel_y);
    //两者加权平均
    addWeighted(sobel_x,0.5,sobel_y,0.5,0,dst);
    for (int x = 0; x < dst.rows; ++x) {
        for (int y = 0; y < dst.cols; ++y) {
            if(dst.at<uchar>(x,y)>ui->sliderForBound_2->value()){
                dst.at<uchar>(x,y)=255;
            }
            else{
                dst.at<uchar>(x,y)=0;
            }
        }
    }
    QImage img=matToImage(dst);
    //设置处理标记，说明当前图像是经过边缘检测的
    allowMark = true;
    //保存图像到缓存,注意要用深拷贝
    edge_img=img.copy(0,0,img.width(),img.height());
    //设置合并状态
    merge_stat=false;
    //设置检测方式
    method=2;
    ui->stackedWidget->setCurrentIndex(method);
    if(!close_panel){
        ui->stackedWidget->show();
    }
    //等参数面板显示后再显示图像
    ui->graphicsView->initPixmap(img);
}

//sliderForV值改变响应函数
void MainWindow::sliderForVSlot(int value)
{
    //修改lcd值
    ui->lcdForV->display(value);
    //对图像做新的log变换
    Mat src,dst;
    /*从qimage构造mat有两种方式
     * 1.src=fromImage(QImage(filename))
     *用该方式构造后续对src的处理会报错
     *
     * 2.src=fromImage(gray_img)
     * 从已有的qimage构造就不会
     */
    src=fromImage(gray_img);
    src.convertTo(src, CV_32FC1);  //转化为32位浮点型
    src = src*value + 1;             //计算 r*v+1
    log(src, src);               //计算log(1+r*v),底数为e
    src=src/log(value);//底数换成v
    //归一化处理
    normalize(src, dst, 0, 255, NORM_MINMAX,CV_8UC1);
    //保存图像到缓存,注意要用深拷贝
    QImage img=matToImage(dst);
    transform_img=img.copy(0,0,img.width(),img.height());
    //显示log变换后的图像
    ui->graphicsView->setPixmap(transform_img);
}

void MainWindow::sliderForThreshold1Slot(int value)
{
//    //不允许下阈值超过上阈值
//    if(value>=ui->sliderForThreshold2->value()){
//        value=ui->sliderForThreshold2->value()-1;
//        ui->sliderForThreshold1->setValue(value);
//    }
    double threshold1=value/10.0;
    //修改lcd值
    ui->lcdForThreshold1->display(threshold1);
    //对图像进行边缘检测并将结果显示到graphicsView中
    Mat src,t,dst;
    //选择图像来源，优先次序为filted_img，transform_img，gray_img
    if(!filted_img.isNull()){
        t=fromImage(filted_img);
        t.copyTo(src);
    }
    else if(!transform_img.isNull()){
        t=fromImage(transform_img);
        t.copyTo(src);
    }
    else{
        t=fromImage(gray_img);
        t.copyTo(src);
    }
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }
    Canny(src,dst,threshold1,ui->sliderForThreshold2->value());
    QImage img=matToImage(dst);
    //保存图像到缓存,注意要用深拷贝
    edge_img=img.copy(0,0,img.width(),img.height());

    //如果是在图像合并状态下拖动myslider调节阈值，需要对检测结果做紫色标注
    if(merge_stat){
        QImage base_img;
        //根据choice选择用于合并的图像
        switch (choice) {
        case 1:
            base_img=origin_img;
            break;
        case 2:
            base_img=transform_img;
            break;
        case 3:
            base_img=filted_img;
            break;
        default:
            base_img=origin_img;
            break;
        }
        //将base_img和边缘检测图合并为merge_mat
        Mat merge_mat;
        vector<Mat> channels;
        //将base_img转为4通道，并分离到channels
        split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
        int x=base_img.width();
        int y=base_img.height();
        //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
        //注意mat和qimage的坐标关系，刚好相反
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if(edge_img.pixel(i,j)==0xFFFFFFFF){
                    channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                    channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                    channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
                }
            }
        }
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
    }
    //否则直接显示检测结果
    else{
        ui->graphicsView->setPixmap(edge_img);
    }
}

void MainWindow::sliderForThreshold2Slot(int value){
//    //不允许上阈值低于下阈值
//    if(value<=ui->sliderForThreshold1->value()){
//        value=ui->sliderForThreshold1->value()+1;
//        ui->sliderForThreshold2->setValue(value);
//    }
    //修改lcd值
    ui->lcdForThreshold2->display(value);
    //对图像进行边缘检测并将结果显示到graphicsView中
    Mat src,t,dst;
    //选择图像来源，优先次序为filted_img，transform_img，gray_img
    if(!filted_img.isNull()){
        t=fromImage(filted_img);
        t.copyTo(src);
    }
    else if(!transform_img.isNull()){
        t=fromImage(transform_img);
        t.copyTo(src);
    }
    else{
        t=fromImage(gray_img);
        t.copyTo(src);
    }
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }
    Canny(src,dst,ui->lcdForThreshold1->value(),value);
    QImage img=matToImage(dst);
    //保存图像到缓存,注意要用深拷贝
    edge_img=img.copy(0,0,img.width(),img.height());

    //如果是在图像合并状态下拖动myslider调节阈值，需要对检测结果做紫色标注
    if(merge_stat){
        QImage base_img;
        //根据choice选择用于合并的图像
        switch (choice) {
        case 1:
            base_img=origin_img;
            break;
        case 2:
            base_img=transform_img;
            break;
        case 3:
            base_img=filted_img;
            break;
        default:
            base_img=origin_img;
            break;
        }
        //将base_img和边缘检测图合并为merge_mat
        Mat merge_mat;
        vector<Mat> channels;
        //将base_img转为4通道，并分离到channels
        split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
        int x=base_img.width();
        int y=base_img.height();
        //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
        //注意mat和qimage的坐标关系，刚好相反
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if(edge_img.pixel(i,j)==0xFFFFFFFF){
                    channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                    channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                    channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
                }
            }
        }
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
    }
    //否则直接显示检测结果
    else{
        ui->graphicsView->setPixmap(edge_img);
    }
}

void MainWindow::rightButtonClickSlot(int x, int y)
{
//    //如果当前是合并状态
//    if(merge_stat){
//        //获取当前显示图像，拆分为4通道(这里如果先定义一个mat，接受fromimage的返回值，再split会报错)
//        vector<Mat> channels;
//        split(fromImage(ui->graphicsView->getPixmap().toImage().convertToFormat(QImage::Format_ARGB32)),channels);
//        //取消紫色标注
//        if(edge_img.pixel(x,y)==0xFFFFFFFF){
//            //获取该点原来的颜色
//            QImage base_img;
//            //判断原图是哪张图
//            if(transform_img.isNull()){
//                base_img=origin_img;
//            }
//            else{
//                base_img=transform_img;
//            }
//            //恢复三通道原来的值
//            channels.at(0).at<uchar>(y,x)=base_img.pixel(x,y);//B通道
//            channels.at(1).at<uchar>(y,x)=base_img.pixel(x,y);//G通道
//            channels.at(2).at<uchar>(y,x)=base_img.pixel(x,y);//R通道
//        }
//        //对点击处标注紫色
//        else{
//            channels.at(0).at<uchar>(y,x)=255;//B通道
//            channels.at(1).at<uchar>(y,x)=0;//G通道
//            channels.at(2).at<uchar>(y,x)=255;//R通道
//        }
//        //合并4通道，显示结果
//        Mat merge_mat;
//        merge(channels,merge_mat);
//        ui->graphicsView->setPixmap(matToImage(merge_mat));
//        //对缓存edge_img做修改
//        edge_img.setPixel(x,y,0x00FFFFFF ^ edge_img.pixel(x,y));
//    }
//    //否则直接对边缘检测图操作
//    else{
//        edge_img.setPixel(x,y,0x00FFFFFF ^ edge_img.pixel(x,y));
//        ui->graphicsView->setPixmap(edge_img);
//    }
    if(!allowMark){
        return;
    }
    edge_img.setPixel(x,y,0x00FFFFFF ^ edge_img.pixel(x,y));
    if(merge_stat){
        QImage base_img;
        //根据choice选择用于合并的图像
        switch (choice) {
        case 1:
            base_img=origin_img;
            break;
        case 2:
            base_img=transform_img;
            break;
        case 3:
            base_img=filted_img;
            break;
        default:
            base_img=origin_img;
            break;
        }
        //将base_img和边缘检测图合并为merge_mat
        Mat merge_mat;
        vector<Mat> channels;
        //将base_img转为4通道，并分离到channels
        split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
        int x=base_img.width();
        int y=base_img.height();
        //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
        //注意mat和qimage的坐标关系，刚好相反
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if(edge_img.pixel(i,j)==0xFFFFFFFF){
                    channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                    channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                    channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
                }
            }
        }
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
    }
    else{
        ui->graphicsView->setPixmap(edge_img);
    }
}

void MainWindow::rightButtonDragSlot(int lx, int ty, int rx, int by)
{
    if(!allowMark){
        return;
    }
    //修改边缘检测图edge_img，将区域内的像素全部置黑
    for (int x = lx; x <= rx; ++x) {
        for (int y = ty; y <= by; ++y) {
            edge_img.setPixel(x,y,0xFF000000);
        }
    }
    //如果当前是合并状态，将边缘检测图与base_img重新合成显示
    if(merge_stat){
        QImage base_img;
        //根据choice选择用于合并的图像
        switch (choice) {
        case 1:
            base_img=origin_img;
            break;
        case 2:
            base_img=transform_img;
            break;
        case 3:
            base_img=filted_img;
            break;
        default:
            base_img=origin_img;
            break;
        }
        //将base_img和边缘检测图合并为merge_mat
        Mat merge_mat;
        vector<Mat> channels;
        //将base_img转为4通道，并分离到channels
        split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
        int x=base_img.width();
        int y=base_img.height();
        //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
        //注意mat和qimage的坐标关系，刚好相反
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if(edge_img.pixel(i,j)==0xFFFFFFFF){
                    channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                    channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                    channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
                }
            }
        }
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
    }
    //否则直接显示修改后的边缘检测图
    else{
        ui->graphicsView->setPixmap(edge_img);
    }
}

void MainWindow::rightButtonDragCtrlSlot(int lx, int ty, int rx, int by)
{
    if(!allowMark){
        return;
    }
    //qDebug()<<lx<<ty<<rx<<by;
    //修改边缘检测图edge_img，将区域外的像素全部置黑
    Mat edge,mask;
    edge=fromImage(edge_img);
    mask=Mat::zeros(edge.rows,edge.cols,edge.type());
    for (int x = lx; x <= rx; ++x) {
        for (int y = ty; y <= by; ++y) {
            mask.at<uchar>(y,x)=255;
        }
    }
    bitwise_and(edge,mask,edge);
    QImage img=matToImage(edge);
    edge_img=img.copy(0,0,img.width(),img.height());
    //如果当前是合并状态，将边缘检测图与base_img重新合成显示
    if(merge_stat){
        QImage base_img;
        //根据choice选择用于合并的图像
        switch (choice) {
        case 1:
            base_img=origin_img;
            break;
        case 2:
            base_img=transform_img;
            break;
        case 3:
            base_img=filted_img;
            break;
        default:
            base_img=origin_img;
            break;
        }
        //将base_img和边缘检测图合并为merge_mat
        Mat merge_mat;
        vector<Mat> channels;
        //将base_img转为4通道，并分离到channels
        split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
        int x=base_img.width();
        int y=base_img.height();
        //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
        //注意mat和qimage的坐标关系，刚好相反
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if(edge_img.pixel(i,j)==0xFFFFFFFF){
                    channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                    channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                    channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
                }
            }
        }
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
    }
    //否则直接显示修改后的边缘检测图
    else{
        ui->graphicsView->setPixmap(edge_img);
    }
}

void MainWindow::mouseMoveSlot(int x, int y)
{
    status_label->setText("rows="+QString::number(y)+",cols="+QString::number(x));
}

void MainWindow::mouseMoveWithRightButtonSlot(int x, int y)
{
    if(!allowMark){
        return;
    }
    //如果当前是合并状态
    if(merge_stat){
        //获取当前显示图像，拆分为4通道(这里如果先定义一个mat，接受fromimage的返回值，再split会报错)
        vector<Mat> channels;
        split(fromImage(ui->graphicsView->getPixmap().toImage().convertToFormat(QImage::Format_ARGB32)),channels);
        //对点击处标注紫色
        channels.at(0).at<uchar>(y,x)=mark_color.blue();//B通道
        channels.at(1).at<uchar>(y,x)=mark_color.green();//G通道
        channels.at(2).at<uchar>(y,x)=mark_color.red();//R通道
        //合并4通道，显示结果
        Mat merge_mat;
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
        //对缓存edge_img做修改
        edge_img.setPixel(x,y,0xFFFFFFFF);
    }
    //否则直接对边缘检测图操作
    else{
        edge_img.setPixel(x,y,0xFFFFFFFF);
        ui->graphicsView->setPixmap(edge_img);
    }
}

void MainWindow::mouseMoveWithRightButtonAndShiftSlot(int x, int y)
{
//    //如果当前是合并状态
//    if(merge_stat){
//        //获取当前显示图像，拆分为4通道(这里如果先定义一个mat，接受fromimage的返回值，再split会报错)
//        vector<Mat> channels;
//        split(fromImage(ui->graphicsView->getPixmap().toImage().convertToFormat(QImage::Format_ARGB32)),channels);
//        //取消标注
//        //获取该点原来的颜色
//        QImage base_img;
//        //判断原图是哪张图
//        if(transform_img.isNull()){
//            base_img=origin_img;
//        }
//        else{
//            base_img=transform_img;
//        }
//        //恢复三通道原来的值
//        channels.at(0).at<uchar>(y,x)=base_img.pixel(x,y);//B通道
//        channels.at(1).at<uchar>(y,x)=base_img.pixel(x,y);//G通道
//        channels.at(2).at<uchar>(y,x)=base_img.pixel(x,y);//R通道
//        //合并4通道，显示结果
//        Mat merge_mat;
//        merge(channels,merge_mat);
//        ui->graphicsView->setPixmap(matToImage(merge_mat));
//        //对缓存edge_img做修改
//        edge_img.setPixel(x,y,0xFF000000);
//    }
//    //否则直接对边缘检测图操作
//    else{
//        edge_img.setPixel(x,y,0xFF000000);
//        ui->graphicsView->setPixmap(edge_img);
//    }
    if(!allowMark){
        return;
    }
    //对(x,y)八邻域的像素取消标记
    for (int i = x-1; i <= x+1; ++i) {
        for (int j = y-1; j <= y+1; ++j) {
            edge_img.setPixel(i,j,0xFF000000);
        }
    }

    if(merge_stat){
        QImage base_img;
        //根据choice选择用于合并的图像
        switch (choice) {
        case 1:
            base_img=origin_img;
            break;
        case 2:
            base_img=transform_img;
            break;
        case 3:
            base_img=filted_img;
            break;
        default:
            base_img=origin_img;
            break;
        }
        //将base_img和边缘检测图合并为merge_mat
        Mat merge_mat;
        vector<Mat> channels;
        //将base_img转为4通道，并分离到channels
        split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
        int x=base_img.width();
        int y=base_img.height();
        //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
        //注意mat和qimage的坐标关系，刚好相反
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if(edge_img.pixel(i,j)==0xFFFFFFFF){
                    channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                    channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                    channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
                }
            }
        }
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
    }
    else{
        ui->graphicsView->setPixmap(edge_img);
    }
}

void MainWindow::openFile(QString file_name)
{
    fileName=file_name;
    QImageReader reader;
    QImage img;
    reader.setDecideFormatFromContent(true);
    reader.setFileName(fileName);
    if(!reader.canRead()||!reader.read(&img))
        return;

    merge_stat=false;
    welcome_label->hide();

    //清除图像缓存
    transform_img=QImage();
    edge_img=QImage();
    filted_img=QImage();
    //隐藏参数面板
    ui->stackedWidget->hide();
    qApp->processEvents();
    ui->graphicsView->initPixmap(img);
    //保存图像到缓存,注意要用深拷贝
    origin_img = img.copy();
    if(img.format()!=QImage::Format_Grayscale8){
        img=img.convertToFormat(QImage::Format_Grayscale8);
    }
    gray_img=img.copy();
    //设置处理标记，说明当前图像是原图
    allowMark = false;
    method=-1;
}

//点击图像变换->log变换
void MainWindow::on_actionlog_triggered()
{
    //如果未打开图片，直接返回
    if(gray_img.isNull()){
        return ;
    }
    //对图像做log变换
    Mat src,dst;
    /*从qimage构造mat有两种方式
     * 1.src=fromImage(QImage(filename))
     *用该方式构造后续对src的处理会报错
     *
     * 2.src=fromImage(gray_img)
     * 从已有的qimage构造就不会
     */
    src=fromImage(gray_img);
    src.convertTo(src, CV_32FC1);  //转化为32位浮点型
    src = src*ui->sliderForV->value() + 1;             //计算 r*v+1
    log(src, src);               //计算log(1+r*v),底数为e
    src=src/log(ui->sliderForV->value());//底数换成v
    //归一化处理
    normalize(src, dst, 0, 255, NORM_MINMAX,CV_8UC1);
    //保存图像到缓存,注意要用深拷贝
    QImage img=matToImage(dst);
    transform_img=img.copy(0,0,img.width(),img.height());
    //设置检测方法，显示参数面板
    method=3;
    ui->stackedWidget->setCurrentIndex(method);
    if(!close_panel){
        ui->stackedWidget->show();
    }
    merge_stat=false;
    //显示log变换后的图像
    ui->graphicsView->initPixmap(transform_img);
    allowMark = false;
}

//点击拆分合并->合并
void MainWindow::on_actionmerge_triggered()
{
    QImage base_img;
    //检测是否存在源图
    if(gray_img.isNull()){
        QMessageBox::information(this,"提示","请打开一张原图");
        return ;
    }
    //检测是否存在边缘检测图
    if(edge_img.isNull()){
        QMessageBox::information(this,"提示","源图像未经过边缘检测");
        return ;
    }
    //构造type用于显示对话框样式
    QString type="1";
    if(transform_img.isNull()){
        type+="0";
    }
    else{
        type+="1";
    }
    if(filted_img.isNull()){
        type+="0";
    }
    else{
        type+="1";
    }
    //如果只有原图，无需选择
    if(type=="100"){
        base_img=origin_img;
    }
    //否则弹出选择框，根据选择结果进行图像合并
    else{
        SelectMergeMapDialog dialog(type,this);
        dialog.exec();
        if(dialog.choice==0){
            return;
        }
        else if(dialog.choice==1){
            base_img=origin_img;
        }
        else if(dialog.choice==2){
            base_img=transform_img;
        }
        else if(dialog.choice==3){
            base_img=filted_img;
        }
        choice=dialog.choice;
    }
    //将base_img和边缘检测图合并为merge_mat
    Mat merge_mat;
    vector<Mat> channels;
    //将base_img转为4通道，并分离到channels
    split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
    int x=base_img.width();
    int y=base_img.height();
    //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
    //注意mat和qimage的坐标关系，刚好相反
    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            if(edge_img.pixel(i,j)==0xFFFFFFFF){
                channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
            }
        }
    }
    merge(channels,merge_mat);
    ui->graphicsView->setPixmap(matToImage(merge_mat));
    allowMark = true;
    //设置合并状态
    merge_stat=true;
}

//点击拆分合并->拆分
void MainWindow::on_actionsplit_triggered()
{
    if(merge_stat){
        ui->graphicsView->setPixmap(edge_img);
        allowMark = true;
        merge_stat=false;
    }
    else{
        QMessageBox::information(this,"提示","需要先合并才能拆分");
    }
}

//点击图像变换->gamma
void MainWindow::on_actiongamma_triggered()
{
    //如果未打开图片，直接返回
    if(gray_img.isNull()){
        return ;
    }
    //对图像做gamma变换
    Mat src,dst;
    /*从qimage构造mat有两种方式
     * 1.src=fromImage(QImage(filename))
     *用该方式构造后续对src的处理会报错
     *
     * 2.src=fromImage(gray_img)
     * 从已有的qimage构造就不会
     */
    src=fromImage(gray_img);
    //转灰度图
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }
    src.convertTo(src, CV_64F);  //转化为64位浮点型
    //计算 r^gamma
    for (int i = 0; i < src.rows; ++i) {
        for (int j = 0; j < src.cols; ++j) {
            src.at<double>(i,j)=pow(src.at<double>(i,j),ui->lcdForGamma->value());
        }
    }
    //归一化处理
    normalize(src, dst, 0, 255, NORM_MINMAX,CV_8UC1);
    //保存图像到缓存,注意要用深拷贝
    QImage img=matToImage(dst);
    transform_img=img.copy(0,0,img.width(),img.height());
    //设置检测方法，显示参数面板
    method=4;
    ui->stackedWidget->setCurrentIndex(method);
    if(!close_panel){
        ui->stackedWidget->show();
    }
    merge_stat=false;
    //显示gamma变换后的图像
    ui->graphicsView->initPixmap(transform_img);
    allowMark = false;
}


void MainWindow::on_sliderForGamma_valueChanged(int value)
{
    double gamma=value/10.0;
    ui->lcdForGamma->display(gamma);
    //对图像做gamma变换
    Mat src,dst;
    /*从qimage构造mat有两种方式
     * 1.src=fromImage(QImage(filename))
     *用该方式构造后续对src的处理会报错
     *
     * 2.src=fromImage(gray_img)
     * 从已有的qimage构造就不会
     */
    src=fromImage(gray_img);
    //转灰度图
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }
    src.convertTo(src, CV_64F);  //转化为64位浮点型
    //计算 r^gamma
    for (int i = 0; i < src.rows; ++i) {
        for (int j = 0; j < src.cols; ++j) {
            src.at<double>(i,j)=pow(src.at<double>(i,j),gamma);
        }
    }
    //归一化处理
    normalize(src, dst, 0, 255, NORM_MINMAX,CV_8UC1);
    //保存图像到缓存,注意要用深拷贝
    QImage img=matToImage(dst);
    transform_img=img.copy(0,0,img.width(),img.height());
    //显示gamma变换后的图像
    ui->graphicsView->setPixmap(transform_img);
}

//laplace边缘检测阈值发生变化
void MainWindow::on_sliderForBound_valueChanged(int value)
{
    //设置lcd数值
    ui->lcdForV_2->display(value);
    //对图像进行边缘检测并将结果显示到graphicsView中
    Mat src,t,dst;
    //选择图像来源，优先次序为filted_img，transform_img，gray_img
    if(!filted_img.isNull()){
        t=fromImage(filted_img);
        t.copyTo(src);
    }
    else if(!transform_img.isNull()){
        t=fromImage(transform_img);
        t.copyTo(src);
    }
    else{
        t=fromImage(gray_img);
        t.copyTo(src);
    }
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }
    Laplacian(src,dst,src.depth());
    for (int x = 0; x < dst.rows; ++x) {
        for (int y = 0; y < dst.cols; ++y) {
            if(dst.at<uchar>(x,y)>value){
                dst.at<uchar>(x,y)=255;
            }
            else{
                dst.at<uchar>(x,y)=0;
            }
        }
    }
    //    Mat kernel = (Mat_<int>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    //    filter2D(src, dst, src.depth(), kernel);
    QImage img=matToImage(dst);
    //保存图像到缓存,注意要用深拷贝
    edge_img=img.copy(0,0,img.width(),img.height());

    if(merge_stat){
        QImage base_img;
        //根据choice选择用于合并的图像
        switch (choice) {
        case 1:
            base_img=origin_img;
            break;
        case 2:
            base_img=transform_img;
            break;
        case 3:
            base_img=filted_img;
            break;
        default:
            base_img=origin_img;
            break;
        }
        //将base_img和边缘检测图合并为merge_mat
        Mat merge_mat;
        vector<Mat> channels;
        //将base_img转为4通道，并分离到channels
        split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
        int x=base_img.width();
        int y=base_img.height();
        //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
        //注意mat和qimage的坐标关系，刚好相反
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if(edge_img.pixel(i,j)==0xFFFFFFFF){
                    channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                    channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                    channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
                }
            }
        }
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
    }
    //否则直接显示检测结果
    else{
        ui->graphicsView->setPixmap(edge_img);
    }
}

//sobel边缘检测阈值发生变化
void MainWindow::on_sliderForBound_2_valueChanged(int value)
{
    //设置lcd数值
    ui->lcdForV_3->display(value);
    //对图像进行边缘检测并将结果显示到graphicsView中
    Mat src,t,dst;
    //选择图像来源，优先次序为filted_img，transform_img，gray_img
    if(!filted_img.isNull()){
        t=fromImage(filted_img);
        t.copyTo(src);
    }
    else if(!transform_img.isNull()){
        t=fromImage(transform_img);
        t.copyTo(src);
    }
    else{
        t=fromImage(gray_img);
        t.copyTo(src);
    }
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }
    //先计算xy方向上的边缘检测图
    Mat sobel_x,sobel_y;
    Sobel(src,sobel_x,CV_64F,1,0);
    Sobel(src,sobel_y,CV_64F,0,1);
    convertScaleAbs(sobel_x,sobel_x);
    convertScaleAbs(sobel_y,sobel_y);
    //两者加权平均
    addWeighted(sobel_x,0.5,sobel_y,0.5,0,dst);
    for (int x = 0; x < dst.rows; ++x) {
        for (int y = 0; y < dst.cols; ++y) {
            if(dst.at<uchar>(x,y)>value){
                dst.at<uchar>(x,y)=255;
            }
            else{
                dst.at<uchar>(x,y)=0;
            }
        }
    }
    QImage img=matToImage(dst);
    //保存图像到缓存,注意要用深拷贝
    edge_img=img.copy(0,0,img.width(),img.height());

    if(merge_stat){
        QImage base_img;
        //根据choice选择用于合并的图像
        switch (choice) {
        case 1:
            base_img=origin_img;
            break;
        case 2:
            base_img=transform_img;
            break;
        case 3:
            base_img=filted_img;
            break;
        default:
            base_img=origin_img;
            break;
        }
        //将base_img和边缘检测图合并为merge_mat
        Mat merge_mat;
        vector<Mat> channels;
        //将base_img转为4通道，并分离到channels
        split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
        int x=base_img.width();
        int y=base_img.height();
        //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
        //注意mat和qimage的坐标关系，刚好相反
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if(edge_img.pixel(i,j)==0xFFFFFFFF){
                    channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                    channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                    channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
                }
            }
        }
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
    }
    //否则直接显示检测结果
    else{
        ui->graphicsView->setPixmap(edge_img);
    }
}

//点击显示参数面板
void MainWindow::on_actionshowPanel_triggered()
{
    if(method==-1){
        return ;
    }
    close_panel=false;
    ui->stackedWidget->show();
    //将当前显示图像重新加载到窗口中心
    ui->graphicsView->reloadPixmap();
}

//点击关闭参数面板
void MainWindow::on_actionhidePanel_triggered()
{
    ui->stackedWidget->hide();
    close_panel=true;
    //将当前显示图像重新加载到窗口中心(在隐藏参数面板后graphicsView的尺寸参数并未立即更新，需要调用qApp->processEvents()刷新界面)
    qApp->processEvents();
    ui->graphicsView->reloadPixmap();
}

//点击中值滤波
void MainWindow::on_actionmedian_triggered()
{
    if(gray_img.isNull()){
        return;
    }
    Mat src,dst;
    if(transform_img.isNull()){
        src=fromImage(gray_img);
    }
    else{
        src=fromImage(transform_img);
    }
    if(src.type()!=CV_8UC1){
        src.convertTo(src,CV_8UC1);
    }
    medianBlur(src,dst,3);
    QImage img=matToImage(dst);
    //保存图像到缓存,注意要用深拷贝
    filted_img=img.copy(0,0,img.width(),img.height());
    //中值滤波无参数设置，隐藏参数面板
    method=-1;
    ui->stackedWidget->hide();
    qApp->processEvents();
    merge_stat=false;
    //显示中值滤波后的图像
    ui->graphicsView->initPixmap(filted_img);
    allowMark = false;
}

//点击高通滤波
void MainWindow::on_actionhighpass_triggered()
{
    if(gray_img.isNull()){
        return;
    }
    Mat src,dst;
    if(transform_img.isNull()){
        src=fromImage(gray_img);
    }
    else{
        src=fromImage(transform_img);
    }

    //高通滤波，增强边缘
    src.convertTo(src,CV_32FC1);
    Mat f_complex_c2;
    dft(src,f_complex_c2,DFT_COMPLEX_OUTPUT);

    //将f_complex_c2低频区域的值归零，保留高频区域的值
    //计算滤波半径，图像中心位置
    int radius=f_complex_c2.cols>f_complex_c2.rows?(f_complex_c2.rows/2)*(ui->lcdHighpassRadius->value()/100.0):(f_complex_c2.cols/2)*(ui->lcdHighpassRadius->value()/100.0);
    int cx=f_complex_c2.cols/2;
    int cy=f_complex_c2.rows/2;
    //将低频移至中心
    Mat temp;
    //这里用的是浅拷贝，对part图像的交换操作将影响f_complex_c2
    Mat part1(f_complex_c2,Rect(0,0,cx,cy));
    Mat part2(f_complex_c2,Rect(cx,0,cx,cy));
    Mat part3(f_complex_c2,Rect(0,cy,cx,cy));
    Mat part4(f_complex_c2,Rect(cx,cy,cx,cy));
    part1.copyTo(temp);
    part4.copyTo(part1);
    temp.copyTo(part4);

    part2.copyTo(temp);
    part3.copyTo(part2);
    temp.copyTo(part3);
    //根据参数面板选择相应滤波方式
    //理想高通滤波
    if(ui->comboBoxHighpassType->currentIndex()==0){
        for (int i = 0; i < f_complex_c2.rows; ++i) {
            for (int j = 0; j < f_complex_c2.cols; ++j) {
                //计算点到中心的距离
                double d=pow(abs(float(i-cy)),2)+pow(abs(float(j-cx)),2);
                if(sqrt(d)<radius){
                    f_complex_c2.at<Vec2f>(i,j)=0;
                }
            }
        }
    }
    //巴特沃斯高通滤波
    else if(ui->comboBoxHighpassType->currentIndex()==1){
        for (int i = 0; i < f_complex_c2.rows; ++i) {
            for (int j = 0; j < f_complex_c2.cols; ++j) {
                f_complex_c2.at<Vec2f>(i,j)=f_complex_c2.at<Vec2f>(i,j)*(1.0-(1.0 / (1.0 + pow(sqrt(pow(i - cy, 2.0) + pow(j - cx, 2.0)) / radius, 4.0))));
            }
        }
    }
    //高斯高通滤波
    else if(ui->comboBoxHighpassType->currentIndex()==2){
        for (int i = 0; i < f_complex_c2.rows; ++i) {
            for (int j = 0; j < f_complex_c2.cols; ++j) {
                f_complex_c2.at<Vec2f>(i,j)=f_complex_c2.at<Vec2f>(i,j)*(1.0-exp(-(pow(i - cy, 2.0) + pow(j - cx, 2.0)) / 2*pow(radius, 2.0)));
            }
        }
    }

    //将低频中心移回原来位置
    Mat temp_;
    //这里用的是浅拷贝，对part图像的交换操作将影响f_complex_c2
    Mat part1_(f_complex_c2,Rect(0,0,cx,cy));
    Mat part2_(f_complex_c2,Rect(cx,0,cx,cy));
    Mat part3_(f_complex_c2,Rect(0,cy,cx,cy));
    Mat part4_(f_complex_c2,Rect(cx,cy,cx,cy));
    part1_.copyTo(temp_);
    part4_.copyTo(part1_);
    temp_.copyTo(part4_);

    part2_.copyTo(temp_);
    part3_.copyTo(part2_);
    temp_.copyTo(part3_);
    //傅里叶逆变换，只取实部
    Mat f_real_c1;
    dft(f_complex_c2,f_real_c1,DFT_REAL_OUTPUT + DFT_SCALE + DFT_INVERSE);
    f_real_c1.convertTo(dst,CV_8UC1);

    QImage img=matToImage(dst);
    //保存图像到缓存,注意要用深拷贝
    filted_img=img.copy(0,0,img.width(),img.height());
    //设置合并状态
    merge_stat=false;
    //设置检测方法，显示参数面板
    method=5;
    ui->stackedWidget->setCurrentIndex(method);
    if(!close_panel){
        ui->stackedWidget->show();
    }
    //等参数面板显示后再显示图像
    ui->graphicsView->initPixmap(img);
    allowMark = false;
}

//滤波器类型发生改变
void MainWindow::on_comboBoxHighpassType_currentIndexChanged(int index)
{
    Mat src,dst;
    if(transform_img.isNull()){
        src=fromImage(gray_img);
    }
    else{
        src=fromImage(transform_img);
    }

    //高通滤波，增强边缘
    src.convertTo(src,CV_32FC1);
    Mat f_complex_c2;
    dft(src,f_complex_c2,DFT_COMPLEX_OUTPUT);

    //将f_complex_c2低频区域的值归零，保留高频区域的值
    //计算滤波半径，图像中心位置
    int radius=f_complex_c2.cols>f_complex_c2.rows?(f_complex_c2.rows/2)*(ui->lcdHighpassRadius->value()/100.0):(f_complex_c2.cols/2)*(ui->lcdHighpassRadius->value()/100.0);
    int cx=f_complex_c2.cols/2;
    int cy=f_complex_c2.rows/2;
    //将低频移至中心
    Mat temp;
    //这里用的是浅拷贝，对part图像的交换操作将影响f_complex_c2
    Mat part1(f_complex_c2,Rect(0,0,cx,cy));
    Mat part2(f_complex_c2,Rect(cx,0,cx,cy));
    Mat part3(f_complex_c2,Rect(0,cy,cx,cy));
    Mat part4(f_complex_c2,Rect(cx,cy,cx,cy));
    part1.copyTo(temp);
    part4.copyTo(part1);
    temp.copyTo(part4);

    part2.copyTo(temp);
    part3.copyTo(part2);
    temp.copyTo(part3);
    //根据参数面板选择相应滤波方式
    //理想高通滤波
    if(index==0){
        for (int i = 0; i < f_complex_c2.rows; ++i) {
            for (int j = 0; j < f_complex_c2.cols; ++j) {
                //计算点到中心的距离
                double d=pow(abs(float(i-cy)),2)+pow(abs(float(j-cx)),2);
                if(sqrt(d)<radius){
                    f_complex_c2.at<Vec2f>(i,j)=0;
                }
            }
        }
    }
    //巴特沃斯高通滤波
    else if(index==1){
        for (int i = 0; i < f_complex_c2.rows; ++i) {
            for (int j = 0; j < f_complex_c2.cols; ++j) {
                f_complex_c2.at<Vec2f>(i,j)=f_complex_c2.at<Vec2f>(i,j)*(1.0-(1.0 / (1.0 + pow(sqrt(pow(i - cy, 2.0) + pow(j - cx, 2.0)) / radius, 4.0))));
            }
        }
    }
    //高斯高通滤波
    else if(ui->comboBoxHighpassType->currentIndex()==2){
        for (int i = 0; i < f_complex_c2.rows; ++i) {
            for (int j = 0; j < f_complex_c2.cols; ++j) {
                f_complex_c2.at<Vec2f>(i,j)=f_complex_c2.at<Vec2f>(i,j)*(1.0-exp(-(pow(i - cy, 2.0) + pow(j - cx, 2.0)) / 2*pow(radius, 2.0)));
            }
        }
    }

    //将低频中心移回原来位置
    Mat temp_;
    //这里用的是浅拷贝，对part图像的交换操作将影响f_complex_c2
    Mat part1_(f_complex_c2,Rect(0,0,cx,cy));
    Mat part2_(f_complex_c2,Rect(cx,0,cx,cy));
    Mat part3_(f_complex_c2,Rect(0,cy,cx,cy));
    Mat part4_(f_complex_c2,Rect(cx,cy,cx,cy));
    part1_.copyTo(temp_);
    part4_.copyTo(part1_);
    temp_.copyTo(part4_);

    part2_.copyTo(temp_);
    part3_.copyTo(part2_);
    temp_.copyTo(part3_);
    //傅里叶逆变换，只取实部
    Mat f_real_c1;
    dft(f_complex_c2,f_real_c1,DFT_REAL_OUTPUT + DFT_SCALE + DFT_INVERSE);
    f_real_c1.convertTo(dst,CV_8UC1);

    QImage img=matToImage(dst);
    //保存图像到缓存,注意要用深拷贝
    filted_img=img.copy(0,0,img.width(),img.height());
    //显示高通滤波后的图像
    ui->graphicsView->setPixmap(filted_img);
}

//滤波半径发生变换
void MainWindow::on_sliderHighpassRadius_valueChanged(int value)
{
    //设置lcd数值
    ui->lcdHighpassRadius->display(value);
    Mat src,dst;
    if(transform_img.isNull()){
        src=fromImage(gray_img);
    }
    else{
        src=fromImage(transform_img);
    }

    //高通滤波，增强边缘
    src.convertTo(src,CV_32FC1);
    Mat f_complex_c2;
    dft(src,f_complex_c2,DFT_COMPLEX_OUTPUT);

    //将f_complex_c2低频区域的值归零，保留高频区域的值
    //计算滤波半径，图像中心位置
    int radius=f_complex_c2.cols>f_complex_c2.rows?(f_complex_c2.rows/2)*(value/100.0):(f_complex_c2.cols/2)*(value/100.0);
    int cx=f_complex_c2.cols/2;
    int cy=f_complex_c2.rows/2;
    //将低频移至中心
    Mat temp;
    //这里用的是浅拷贝，对part图像的交换操作将影响f_complex_c2
    Mat part1(f_complex_c2,Rect(0,0,cx,cy));
    Mat part2(f_complex_c2,Rect(cx,0,cx,cy));
    Mat part3(f_complex_c2,Rect(0,cy,cx,cy));
    Mat part4(f_complex_c2,Rect(cx,cy,cx,cy));
    part1.copyTo(temp);
    part4.copyTo(part1);
    temp.copyTo(part4);

    part2.copyTo(temp);
    part3.copyTo(part2);
    temp.copyTo(part3);
    //根据参数面板选择相应滤波方式
    //理想高通滤波
    if(ui->comboBoxHighpassType->currentIndex()==0){
        for (int i = 0; i < f_complex_c2.rows; ++i) {
            for (int j = 0; j < f_complex_c2.cols; ++j) {
                //计算点到中心的距离
                double d=pow(abs(float(i-cy)),2)+pow(abs(float(j-cx)),2);
                if(sqrt(d)<radius){
                    f_complex_c2.at<Vec2f>(i,j)=0;
                }
            }
        }
    }
    //巴特沃斯高通滤波
    else if(ui->comboBoxHighpassType->currentIndex()==1){
        for (int i = 0; i < f_complex_c2.rows; ++i) {
            for (int j = 0; j < f_complex_c2.cols; ++j) {
                f_complex_c2.at<Vec2f>(i,j)=f_complex_c2.at<Vec2f>(i,j)*(1.0-(1.0 / (1.0 + pow(sqrt(pow(i - cy, 2.0) + pow(j - cx, 2.0)) / radius, 4.0))));
            }
        }
    }
    //高斯高通滤波
    else if(ui->comboBoxHighpassType->currentIndex()==2){
        for (int i = 0; i < f_complex_c2.rows; ++i) {
            for (int j = 0; j < f_complex_c2.cols; ++j) {
                f_complex_c2.at<Vec2f>(i,j)=f_complex_c2.at<Vec2f>(i,j)*(1.0-exp(-(pow(i - cy, 2.0) + pow(j - cx, 2.0)) / 2*pow(radius, 2.0)));
            }
        }
    }

    //将低频中心移回原来位置
    Mat temp_;
    //这里用的是浅拷贝，对part图像的交换操作将影响f_complex_c2
    Mat part1_(f_complex_c2,Rect(0,0,cx,cy));
    Mat part2_(f_complex_c2,Rect(cx,0,cx,cy));
    Mat part3_(f_complex_c2,Rect(0,cy,cx,cy));
    Mat part4_(f_complex_c2,Rect(cx,cy,cx,cy));
    part1_.copyTo(temp_);
    part4_.copyTo(part1_);
    temp_.copyTo(part4_);

    part2_.copyTo(temp_);
    part3_.copyTo(part2_);
    temp_.copyTo(part3_);
    //傅里叶逆变换，只取实部
    Mat f_real_c1;
    dft(f_complex_c2,f_real_c1,DFT_REAL_OUTPUT + DFT_SCALE + DFT_INVERSE);
    f_real_c1.convertTo(dst,CV_8UC1);

    QImage img=matToImage(dst);
    //保存图像到缓存,注意要用深拷贝
    filted_img=img.copy(0,0,img.width(),img.height());
    //显示高通滤波后的图像
    ui->graphicsView->setPixmap(filted_img);
}

//点击帮助->关于
void MainWindow::on_actionabout_triggered()
{
    AboutDialog about(this);
    about.exec();
    //about.show();
}

//点击设置标注颜色
void MainWindow::on_actionsetcolor_triggered()
{
    QColor temp = QColorDialog::getColor(mark_color,this,"选择一种标注颜色");
    if(temp.isValid()){
        mark_color=temp;
        ui->changeMarkColor->setText(mark_color.name());
    }
    else{
        return;
    }
    //如果在合并状态下，刷新
    if(merge_stat){
        QImage base_img;
        //根据choice选择用于合并的图像
        switch (choice) {
        case 1:
            base_img=origin_img;
            break;
        case 2:
            base_img=transform_img;
            break;
        case 3:
            base_img=filted_img;
            break;
        default:
            base_img=origin_img;
            break;
        }
        //将base_img和边缘检测图合并为merge_mat
        Mat merge_mat;
        vector<Mat> channels;
        //将base_img转为4通道，并分离到channels
        split(fromImage(base_img.convertToFormat(QImage::Format_ARGB32)),channels);
        int x=base_img.width();
        int y=base_img.height();
        //在base_img的基础上对检测到的边缘标注紫色，紫色ARGB值为（255，255，0，255）
        //注意mat和qimage的坐标关系，刚好相反
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                if(edge_img.pixel(i,j)==0xFFFFFFFF){
                    channels.at(0).at<uchar>(j,i)=mark_color.blue();//B通道
                    channels.at(1).at<uchar>(j,i)=mark_color.green();//G通道
                    channels.at(2).at<uchar>(j,i)=mark_color.red();//R通道
                }
            }
        }
        merge(channels,merge_mat);
        ui->graphicsView->setPixmap(matToImage(merge_mat));
    }
    //如果在自定义检测结果情况下
    if(method == 7){
        QImage temp = edge_img.convertToFormat(QImage::Format_ARGB32);
        //根据透明度计算alpha通道值
        int alpha = 255 - transparence/100.0*255;
        for (int y = 0; y < temp.height(); ++y) {
            QRgb *row = (QRgb*)temp.scanLine(y);
            for (int x = 0; x < temp.width(); ++x) {
                if(temp.pixelColor(x, y) == QColor("#ffffff"))temp.setPixelColor(x, y, mark_color);
                else((unsigned char*)&row[x])[3] = alpha;
            }
        }

        ui->graphicsView->setPixmap(temp);
    }
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    if(arg1>ui->spinBox_3->value()){
        ui->spinBox->setValue(ui->spinBox_3->value());
        //ui->graphicsView->setSamplerInsideWidth(ui->spinBox_3->value());
    }
    else if(arg1<0){
        ui->spinBox->setValue(0);
        //ui->graphicsView->setSamplerInsideWidth(0);
    }
    else{
        ui->graphicsView->setSamplerInsideWidth(arg1);
    }
}

void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
    if(arg1>ui->spinBox_4->value()){
        ui->spinBox_2->setValue(ui->spinBox_4->value());
        //ui->graphicsView->setSamplerInsideHeight(ui->spinBox_4->value());
    }
    else if(arg1<0){
        ui->spinBox_2->setValue(0);
        //ui->graphicsView->setSamplerInsideHeight(0);
    }
    else{
        ui->graphicsView->setSamplerInsideHeight(arg1);
    }
}

void MainWindow::on_spinBox_3_valueChanged(int arg1)
{
    if(arg1<ui->spinBox->value()){
        ui->spinBox_3->setValue(ui->spinBox->value());
        //ui->graphicsView->setSamplerOutsideWidth(ui->spinBox->value());
    }
    else if(arg1<1){
        ui->spinBox_3->setValue(1);
        //ui->graphicsView->setSamplerOutsideWidth(1);
    }
    else if(ui->graphicsView->getSamplerLocation().x()-arg1/2<0 || ui->graphicsView->getSamplerLocation().x()+arg1/2>ui->graphicsView->getPixmap().width()){
        ui->spinBox_3->setValue(arg1-1);
        //ui->graphicsView->setSamplerOutsideWidth(arg1-1);
    }
    else{
        ui->graphicsView->setSamplerOutsideWidth(arg1);
    }
}

void MainWindow::on_spinBox_4_valueChanged(int arg1)
{
    if(arg1<ui->spinBox_2->value()){
        ui->spinBox_4->setValue(ui->spinBox_2->value());
        //ui->graphicsView->setSamplerOutsideHeight(ui->spinBox_2->value());
    }
    else if(arg1<1){
        ui->spinBox_4->setValue(1);
        //ui->graphicsView->setSamplerOutsideHeight(1);
    }
    else if(ui->graphicsView->getSamplerLocation().y()-arg1/2<0 || ui->graphicsView->getSamplerLocation().y()+arg1/2>ui->graphicsView->getPixmap().height()){
        ui->spinBox_4->setValue(arg1-1);
        //ui->graphicsView->setSamplerOutsideHeight(arg1-1);
    }
    else{
        ui->graphicsView->setSamplerOutsideHeight(arg1);
    }
}

void MainWindow::on_spinBox_5_valueChanged(int arg1)
{    
    if(arg1-ui->spinBox_3->value()/2 < 0){
        ui->spinBox_5->setValue(arg1+1);
    }
    else if(arg1+ui->spinBox_3->value()/2 > ui->graphicsView->getPixmap().width()){
        ui->spinBox_5->setValue(arg1-1);
    }
    else{
        ui->graphicsView->setSamplerLocation(QPointF(arg1,ui->spinBox_6->value()));
    }
}

void MainWindow::on_spinBox_6_valueChanged(int arg1)
{
    if(arg1-ui->spinBox_4->value()/2 < 0){
        ui->spinBox_6->setValue(arg1+1);
    }
    else if(arg1+ui->spinBox_4->value()/2 > ui->graphicsView->getPixmap().height()){
        ui->spinBox_6->setValue(arg1-1);
    }
    else{
        ui->graphicsView->setSamplerLocation(QPointF(ui->spinBox_5->value(),arg1));
    }
}

//点击截取并保存
void MainWindow::on_pushButton_clicked()
{
    int in_width = ui->spinBox->value();
    int in_height = ui->spinBox_2->value();
    int out_width = ui->spinBox_3->value();
    int out_height = ui->spinBox_4->value();
    int center_x = ui->spinBox_5->value();
    int center_y = ui->spinBox_6->value();
    QImage outside,inside;
    outside = ui->graphicsView->getPixmap().copy(center_x-out_width/2,center_y-out_height/2,out_width,out_height).toImage();
    if(in_width>0 && in_height>0){
        inside = ui->graphicsView->getPixmap().copy(center_x-in_width/2,center_y-in_height/2,in_width,in_height).toImage();
        for (int x = (out_width-in_width)/2; x < (out_width+in_width)/2; ++x) {
            for (int y = (out_height-in_height)/2; y < (out_height+in_height)/2; ++y) {
                outside.setPixel(x,y,0xFF000000);
            }
        }
    }

    //获取图片保存路径
    QFileInfo file_info(fileName);
    QString save_name=file_info.absolutePath();
    save_name = save_name + "/" + file_info.baseName().split("_")[0] + "_"+QString::number(center_x)+"_"+QString::number(center_y);
    save_name=QFileDialog::getSaveFileName(this,"选择保存位置",save_name);
    //qDebug()<<save_name;
    if(!save_name.isEmpty() && !inside.isNull() && outside.save(save_name+"_B.png") && inside.save(save_name+"_C.png")){
        QMessageBox::information(this,"提示","保存成功!\n外围区域保存到："+save_name+"_B.png"+"\n中心区域保存到："+save_name+"_C.png");
    }
    else if(!save_name.isEmpty() && inside.isNull() && outside.save(save_name+"_B.png")){
        QMessageBox::information(this,"提示","保存成功!\n外围区域保存到："+save_name+"_B.png\n无中心区域");
    }
}

//点击矩形采样
void MainWindow::on_actionRectSampler_2_triggered()
{
    //如果未打开图片直接返回
    if(ui->graphicsView->getPixmap().isNull()){
        return;
    }

    //设置检测方法，显示参数面板
    method=6;
    ui->stackedWidget->setCurrentIndex(method);
    if(!close_panel){
        ui->stackedWidget->show();
    }
    //隐藏自动检测参数
    ui->groupBox_15->hide();
    allowMark = false;

    //显示采样器
    ui->graphicsView->addRectSampler();
    //初始化面板参数
    QSize outSize = ui->graphicsView->getSamplerOutSize();
    QSize inSize  = ui->graphicsView->getSamplerInSize();
    ui->spinBox->setValue(inSize.width());
    ui->spinBox_2->setValue(inSize.height());
    ui->spinBox_3->setValue(outSize.width());
    ui->spinBox_4->setValue(outSize.height());
    //初始化中心坐标
    QPointF location = ui->graphicsView->getSamplerLocation();
    ui->spinBox_5->setValue(location.x());
    ui->spinBox_6->setValue(location.y());
}

//点击自动采样
void MainWindow::on_actionAutoSampler_triggered()
{
    //如果未打开图片直接返回
    if(ui->graphicsView->getPixmap().isNull()){
        return;
    }

    //设置检测方法，显示参数面板
    method=6;
    ui->stackedWidget->setCurrentIndex(method);
    if(!close_panel){
        ui->stackedWidget->show();
    }
    //显示自动检测参数
    ui->groupBox_15->show();
    allowMark = false;

    //显示采样器
    ui->graphicsView->addRectSampler();
    //显示保留框
    ui->graphicsView->addRectangle();
    //初始化面板参数
    QSize outSize = ui->graphicsView->getSamplerOutSize();
    QSize inSize  = ui->graphicsView->getSamplerInSize();
    ui->spinBox->setValue(inSize.width());
    ui->spinBox_2->setValue(inSize.height());
    ui->spinBox_3->setValue(outSize.width());
    ui->spinBox_4->setValue(outSize.height());
    //初始化中心坐标
    QPointF location = ui->graphicsView->getSamplerLocation();
    ui->spinBox_5->setValue(location.x());
    ui->spinBox_6->setValue(location.y());
    //初始化自动检测参数
    QSize size = ui->graphicsView->getRectangleSize();
    ui->spinBox_7->setValue(size.width());
    ui->spinBox_8->setValue(size.height());
    QPointF center = ui->graphicsView->getRectangleLocation();
    ui->spinBox_9->setValue(center.x());
    ui->spinBox_10->setValue(center.y());
}

void MainWindow::on_spinBox_9_valueChanged(int arg1)
{
    if(arg1-ui->spinBox_7->value()/2 < 0){
        ui->spinBox_9->setValue(arg1+1);
    }
    else if(arg1+ui->spinBox_7->value()/2 > ui->graphicsView->getPixmap().width()){
        ui->spinBox_9->setValue(arg1-1);
    }
    else{
        ui->graphicsView->setRectangleLocation(QPointF(arg1,ui->spinBox_10->value()));
    }
}

void MainWindow::on_spinBox_10_valueChanged(int arg1)
{
    if(arg1-ui->spinBox_8->value()/2 < 0){
        ui->spinBox_10->setValue(arg1+1);
    }
    else if(arg1+ui->spinBox_8->value()/2 > ui->graphicsView->getPixmap().width()){
        ui->spinBox_10->setValue(arg1-1);
    }
    else{
        ui->graphicsView->setRectangleLocation(QPointF(ui->spinBox_9->value(),arg1));
    }
}

void MainWindow::on_spinBox_7_valueChanged(int arg1)
{
    if(ui->graphicsView->getRectangleLocation().x()-arg1/2<0 || ui->graphicsView->getRectangleLocation().x()+arg1/2>ui->graphicsView->getPixmap().width()){
        ui->spinBox_7->setValue(arg1-1);
    }
    else{
        ui->graphicsView->setRectangleWidth(arg1);
    }
}

void MainWindow::on_spinBox_8_valueChanged(int arg1)
{
    if(ui->graphicsView->getRectangleLocation().y()-arg1/2<0 || ui->graphicsView->getRectangleLocation().y()+arg1/2>ui->graphicsView->getPixmap().height()){
        ui->spinBox_8->setValue(arg1-1);
    }
    else{
        ui->graphicsView->setRectangleHeight(arg1);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
//    int in_width = ui->spinBox->value();
//    int in_height = ui->spinBox_2->value();
//    int out_width = ui->spinBox_3->value();
//    int out_height = ui->spinBox_4->value();
//    int border_width = (out_width-in_width)/2;
//    int border_height = (out_height-in_height)/2;
//    int width = ui->graphicsView->getPixmap().width() - ui->spinBox_3->value();
//    int height= ui->graphicsView->getPixmap().height()- ui->spinBox_4->value();
//    QRect left(0,0,ui->spinBox_9->value()-ui->spinBox_7->value()/2,ui->graphicsView->getPixmap().height());
//    QRect top(0,0,ui->graphicsView->getPixmap().width(),ui->spinBox_10->value()-ui->spinBox_8->value()/2);
//    QRect right(ui->spinBox_9->value()+ui->spinBox_7->value()/2,0,ui->graphicsView->getPixmap().width()-ui->spinBox_9->value()-ui->spinBox_7->value()/2,ui->graphicsView->getPixmap().height());
//    QRect bottom(0,ui->spinBox_10->value()+ui->spinBox_8->value()/2,ui->graphicsView->getPixmap().width(),ui->graphicsView->getPixmap().height()-ui->spinBox_10->value()-ui->spinBox_8->value()/2);
//    //获取图片保存路径
//    QFileInfo file_info(fileName);
//    QString save_name=file_info.absolutePath();
//    save_name=QFileDialog::getExistingDirectory(this,"选择保存位置",save_name);
//    if(save_name.isEmpty()){
//        return;
//    }
//    QString dir = save_name;
//    save_name+="/";
//    save_name+=file_info.baseName().split("_")[0];
//    //qDebug()<<save_name;

//    progress->show();
//    progress->setValue(0);
//    int total = width/ui->spinBox_11->value() * height/ui->spinBox_11->value();
//    int count=0;
//    for (int i = 0; i < width; i=i+ui->spinBox_11->value()) {
//        for (int j = 0; j < height; j=j+ui->spinBox_11->value()) {
//            QRect rect(i,j,out_width,out_height);
//            if(left.contains(rect) || top.contains(rect) || right.contains(rect) || bottom.contains(rect)){
//                QImage outside,inside;
//                outside = ui->graphicsView->getPixmap().copy(i,j,out_width,out_height).toImage();
//                if(in_width>0 && in_height>0){
//                    inside = ui->graphicsView->getPixmap().copy(i+border_width,j+border_height,in_width,in_height).toImage();
//                    for (int x = border_width; x < (out_width+in_width)/2; ++x) {
//                        for (int y = border_height; y < (out_height+in_height)/2; ++y) {
//                            outside.setPixel(x,y,0xFF000000);
//                        }
//                    }
//                }
//                int center_x = i+out_width/2;
//                int center_y = j+out_height/2;
//                QString str=QString("%1_%2_%3").arg(save_name).arg(center_x).arg(center_y);
//                outside.save(str+"_B.png");
//                if(!inside.isNull())inside.save(str+"_C.png");
//                count++;
//                progress->setValue((qreal)count/total*100);
//                qApp->processEvents();
//            }
//        }
//    }
//    progress->hide();
//    QMessageBox::information(this,"提示",QString("共采样%1对图像\n保存在%2").arg(count).arg(dir));

    //qDebug()<<"mainThread:"<<QThread::currentThread();
    //获取图片保存路径
    QFileInfo file_info(fileName);
    QString save_name=file_info.absolutePath();
    save_name=QFileDialog::getExistingDirectory(this,"选择保存位置",save_name);
    if(save_name.isEmpty()){
        return;
    }
    QString dir = save_name;
    save_name+="/";
    save_name+=file_info.baseName().split("_")[0];
    ui->pushButton_2->setEnabled(false);

    //创建线程
    QThread *thread = new QThread();

    //创建任务类
    AutoSampleTask *task = new AutoSampleTask(ui->graphicsView->getPixmap().toImage(),
                                              ui->spinBox->value(),
                                              ui->spinBox_2->value(),
                                              ui->spinBox_3->value(),
                                              ui->spinBox_4->value(),
                                              ui->spinBox_7->value(),
                                              ui->spinBox_8->value(),
                                              ui->spinBox_11->value(),
                                              QPoint(ui->spinBox_5->value(),ui->spinBox_6->value()),
                                              QPoint(ui->spinBox_9->value(),ui->spinBox_10->value()),
                                              save_name);

    //将任务放到子线程
    task->moveToThread(thread);

    progress->show();
    progress->setValue(0);
    connect(task, &AutoSampleTask::process, this, [=](int percent){
        progress->setValue(percent);
    });
    connect(task, &AutoSampleTask::finish, this, [=](int count){
        progress->hide();
        QMessageBox::information(this,"提示",QString("共采样%1对图像\n保存在%2").arg(count).arg(dir));
        ui->pushButton_2->setEnabled(true);

        delete task;

        thread->quit();
        thread->wait();
        delete thread;
    });
    connect(this, &MainWindow::operate, task, &AutoSampleTask::working);

    //启动线程
    thread->start();
    emit operate();
}

//点击自定义检测结果
void MainWindow::on_actioncustom_triggered()
{
    if(gray_img.isNull() && edge_img.isNull()){
        return;
    }
    if(edge_img.isNull()){
        if(QMessageBox::Yes == QMessageBox::question(this, "提示", "检测到当前图像没有进行边缘检测，是否把原图作为检测结果？")){
            edge_img = origin_img.copy();
            if(QImage::Format_Grayscale8 != edge_img.format()){
                edge_img = edge_img.convertToFormat(QImage::Format_Grayscale8);
                for (int y = 0; y < edge_img.height(); ++y) {
                    for (int x = 0; x < edge_img.width(); ++x) {
                        if(edge_img.pixelColor(x, y) != QColor("#000000"))edge_img.setPixel(x, y, 0xFFFFFFFF);
                    }
                }
            }
        }
        else{
            return;
        }
    }

    QImage temp = edge_img.convertToFormat(QImage::Format_ARGB32);
    //根据透明度计算alpha通道值
    int alpha = 255 - transparence/100.0*255;
    for (int y = 0; y < temp.height(); ++y) {
        QRgb *row = (QRgb*)temp.scanLine(y);
        for (int x = 0; x < temp.width(); ++x) {
            if(temp.pixelColor(x, y) == QColor("#ffffff"))temp.setPixelColor(x, y, mark_color);
            else((unsigned char*)&row[x])[3] = alpha;
        }
    }

    //设置处理标记，不允许人工修改
    allowMark = false;
    //设置合并状态
    merge_stat=false;
    //设置检测方法，显示参数面板
    method=7;
    ui->stackedWidget->setCurrentIndex(method);
    if(!close_panel){
        ui->stackedWidget->show();
    }
    //等参数面板显示后再显示图像
    ui->graphicsView->initPixmap(temp);
}

void MainWindow::on_changeMarkColor_clicked()
{
    QColor color = QColorDialog::getColor(mark_color,this,"选择一种标注颜色");
    if(color.isValid()){
        mark_color=color;
        ui->changeMarkColor->setText(mark_color.name());
    }
    else{
        return;
    }

    QImage temp = edge_img.convertToFormat(QImage::Format_ARGB32);
    //根据透明度计算alpha通道值
    int alpha = 255 - transparence/100.0*255;
    for (int y = 0; y < temp.height(); ++y) {
        QRgb *row = (QRgb*)temp.scanLine(y);
        for (int x = 0; x < temp.width(); ++x) {
            if(temp.pixelColor(x, y) == QColor("#ffffff"))temp.setPixelColor(x, y, mark_color);
            else((unsigned char*)&row[x])[3] = alpha;
        }
    }

    ui->graphicsView->setPixmap(temp);
}

void MainWindow::on_sliderTransparence_valueChanged(int value)
{
    transparence = value;
    ui->lcdTransparence->display(value);

    QImage temp = edge_img.convertToFormat(QImage::Format_ARGB32);
    //根据透明度计算alpha通道值
    int alpha = 255 - transparence/100.0*255;
    for (int y = 0; y < temp.height(); ++y) {
        QRgb *row = (QRgb*)temp.scanLine(y);
        for (int x = 0; x < temp.width(); ++x) {
            if(temp.pixelColor(x, y) == QColor("#ffffff"))temp.setPixelColor(x, y, mark_color);
            else((unsigned char*)&row[x])[3] = alpha;
        }
    }

    ui->graphicsView->setPixmap(temp);
}

//点击导入已有边缘检测图
void MainWindow::on_actionopenEdgeimg_triggered()
{
    //获取图片路径，加载图片到graphicsView
    QString str = QFileDialog::getOpenFileName(this,"导入边缘检测图",QCoreApplication::applicationDirPath(),"Images (*.png *.jpg *.jpeg *.bmp)");
    if(str.isEmpty()){
        return ;
    }
    //加载图像到edge_img
    QImageReader reader;
    reader.setDecideFormatFromContent(true);
    reader.setFileName(str);
    if(!reader.canRead()||!reader.read(&edge_img))
        return;
    //如果已有原图，但导入的边缘检测图与原图大小不一致，属于不合理操作，给出提示
    if(!origin_img.isNull() && origin_img.size()!=edge_img.size()){
        QMessageBox::warning(this, "警告", "检测到原图和边缘检测图大小不一致，请确认是否选择正确");
        edge_img = QImage();
        return;
    }
    //将导入的边缘检测图转换为灰度图
    edge_img = edge_img.convertToFormat(QImage::Format_Grayscale8);
    //非黑置白，对不是背景的像素设置为边缘像素
    for (int y = 0; y < edge_img.height(); ++y) {
        for (int x = 0; x < edge_img.width(); ++x) {
            if(edge_img.pixelColor(x, y) != QColor("#000000"))edge_img.setPixel(x, y, 0xFFFFFFFF);
        }
    }

    //隐藏参数面板再显示图像
    ui->stackedWidget->hide();
    qApp->processEvents();
    ui->graphicsView->initPixmap(edge_img);
    //设置处理标记，说明当前图像是边缘检测图
    allowMark = true;
    merge_stat=false;
    welcome_label->hide();
    method=-1;
}

//右键菜单
void MainWindow::on_MainWindow_customContextMenuRequested(const QPoint &pos)
{
    if(!allowMark){
        m_menu->exec(cursor().pos());
    }
}
