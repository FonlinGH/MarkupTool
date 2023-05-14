#include "selectmergemapdialog.h"

SelectMergeMapDialog::SelectMergeMapDialog(QString type,QWidget *parent) :
    QDialog(parent),
    choice(0)
{
    origin=new QRadioButton("原图");
    transform=new QRadioButton("变换图");
    filted=new QRadioButton("滤波图");
    ok=new QPushButton("合并",this);
    cancle=new QPushButton("取消",this);
    layout=new QGridLayout(this);

    setWindowTitle("选择合并对象");
    resize(300,100);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
    origin->setChecked(true);
    layout->setMargin(30);
    layout->setSpacing(10);

    switch (type.toInt()) {
    case 110:
        layout->addWidget(origin,0,0,1,2);
        layout->addWidget(transform,1,0,1,2);
        layout->addWidget(ok,2,0);
        layout->addWidget(cancle,2,1);
        break;
    case 111:
        layout->addWidget(origin,0,0,1,2);
        layout->addWidget(transform,1,0,1,2);
        layout->addWidget(filted,2,0,1,2);
        layout->addWidget(ok,3,0);
        layout->addWidget(cancle,3,1);
        break;
    case 101:
        layout->addWidget(origin,0,0,1,2);
        layout->addWidget(filted,1,0,1,2);
        layout->addWidget(ok,2,0);
        layout->addWidget(cancle,2,1);
        break;
    default:
        break;
    }
    connect(ok,&QPushButton::clicked,this,[=](){
        if(origin->isChecked()){
            choice=1;
        }
        else if(transform->isChecked()){
            choice=2;
        }
        else if(filted->isChecked()){
            choice=3;
        }
        this->close();
    });
    connect(cancle,&QPushButton::clicked,this,&QDialog::close);
}

SelectMergeMapDialog::~SelectMergeMapDialog()
{
    delete origin;
    delete transform;
    delete filted;
}
