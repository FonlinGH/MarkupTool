#ifndef MYRECTANGLE_H
#define MYRECTANGLE_H

#include <QObject>
#include <QGraphicsItem>

class MyRectangle : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    explicit MyRectangle(qreal w = 11, qreal h = 11, QGraphicsItem *parent = nullptr);
    qreal width;
    qreal height;

protected:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void centerMove(int x,int y);

public slots:
};

#endif // MYRECTANGLE_H
