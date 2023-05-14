#ifndef SELECTMERGEMAPDIALOG_H
#define SELECTMERGEMAPDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QRadioButton>
#include <QPushButton>

class SelectMergeMapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectMergeMapDialog(QString type,QWidget *parent = nullptr);
    ~SelectMergeMapDialog();
    //选择结果，0未选择，1为原图，2为变换图，3为滤波图
    int choice;

private:
    QGridLayout *layout;
    QRadioButton *origin;
    QRadioButton *transform;
    QRadioButton *filted;
    QPushButton *ok;
    QPushButton *cancle;

};

#endif // SELECTMERGEMAPDIALOG_H
