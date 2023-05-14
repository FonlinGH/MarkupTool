#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "opencv2/opencv.hpp"
#include <QLabel>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QThread>
#include "autosampletask.h"
#include <QSplitter>

using namespace cv;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //将Mat转换成QImage
    QImage matToImage(Mat mat);
    //将QImage转换成Mat
    Mat fromImage(QImage image);
    //对文件名为filename的图片进行边缘检测后返回qimage
    QImage myCanny(int threshold);

protected:

signals:
    //用于启动子线程中的工作
    void operate();

private slots:
    void on_actionopen_triggered();
    void on_actionsave_triggered();
    void on_actionexit_triggered();
    void on_actioncanny_triggered();
    void on_actionlog_triggered();
    void on_actionsplit_triggered();
    void on_actionmerge_triggered();
    void on_actionlaplace_triggered();
    void on_actionsobel_triggered();
    void on_actiongamma_triggered();
    void on_sliderForBound_valueChanged(int value);
    void on_sliderForBound_2_valueChanged(int value);
    void on_actionshowPanel_triggered();
    void on_actionhidePanel_triggered();
    void on_sliderForGamma_valueChanged(int value);
    void on_actionmedian_triggered();
    void on_actionhighpass_triggered();
    void on_comboBoxHighpassType_currentIndexChanged(int index);
    void on_sliderHighpassRadius_valueChanged(int value);
    void on_actionabout_triggered();
    void on_actionscreenshot_triggered();
    void on_actionsaveGrayImg_triggered();
    void on_actionsetcolor_triggered();
    void on_spinBox_valueChanged(int arg1);
    void on_spinBox_2_valueChanged(int arg1);
    void on_spinBox_3_valueChanged(int arg1);
    void on_spinBox_4_valueChanged(int arg1);
    void on_spinBox_5_valueChanged(int arg1);
    void on_spinBox_6_valueChanged(int arg1);
    void on_spinBox_7_valueChanged(int arg1);
    void on_spinBox_8_valueChanged(int arg1);
    void on_spinBox_9_valueChanged(int arg1);
    void on_spinBox_10_valueChanged(int arg1);
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_actionRectSampler_2_triggered();
    void on_actionAutoSampler_triggered();
    void on_actioncustom_triggered();
    void on_changeMarkColor_clicked();
    void on_sliderTransparence_valueChanged(int value);
    void on_actionopenEdgeimg_triggered();
    void on_MainWindow_customContextMenuRequested(const QPoint &pos);
    //sliderForV拖动响应函数
    void sliderForVSlot(int value);
    //sliderForThreshold1拖动响应函数
    void sliderForThreshold1Slot(int value);
    //sliderForThreshold2拖动响应函数
    void sliderForThreshold2Slot(int value);
    //右键点击响应函数
    void rightButtonClickSlot(int x,int y);
    //右键拖曳响应函数，左上角坐标(lx,ty)，右下角坐标(rx,by)
    void rightButtonDragSlot(int lx,int ty,int rx,int by);
    //右键拖曳保留区域点响应函数，左上角坐标(lx,ty)，右下角坐标(rx,by)
    void rightButtonDragCtrlSlot(int lx,int ty,int rx,int by);
    //鼠标位于显示图像内的响应函数，用于显示坐标
    void mouseMoveSlot(int x,int y);
    //鼠标右键按下拖曳选点响应函数
    void mouseMoveWithRightButtonSlot(int x,int y);
    //对鼠标轨迹上的点取消标记
    void mouseMoveWithRightButtonAndShiftSlot(int x,int y);
    //捕获graphicsView文件拖曳信号，打开新文件
    void openFile(QString file_name);            

private:
    Ui::MainWindow *ui;
    //状态栏文本
    QLabel *status_label;
    //状态栏进度条
    QProgressBar *progress;
    //“把图片拖到此处打开”
    QLabel *welcome_label;
    //容纳welcome_label的布局
    QHBoxLayout *m_layout;
    //当前打开图片的路径
    QString fileName;
    //原图像
    QImage origin_img;
    //原灰度图像
    QImage gray_img;
    //经过图像变换的图片
    QImage transform_img;
    //经过图像滤波的图片
    QImage filted_img;
    //经过边缘检测的图片
    QImage edge_img;
    //表示合并状态
    bool merge_stat;
    //标志检测方式，0为canny，1为laplace，2为sobel，3为log变换，4为gamma变换，5为高通滤波，6为采样器，7为自定义检测结果，-1表示无参数面板
    int method;
    //true表示参数面板不可显示
    bool close_panel;
    //标注颜色
    QColor mark_color;
    //背景透明度，0到100
    int transparence;
    //标记哪幅图像用于合并
    int choice;
    //决定是否允许人工标记
    bool allowMark;
    //右键菜单项
    QMenu *m_menu;
    QAction *m_open;
    QAction *m_openEdgeimg;
    QAction *m_saveEdgeimg;
    QAction *m_saveImg;
    QAction *m_changeColor;
    QAction *m_merge;
    QAction *m_split;
    QAction *m_exit;

};
#endif // MAINWINDOW_H
