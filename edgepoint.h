#ifndef EDGEPOINT_H
#define EDGEPOINT_H

#include <QObject>
#include <QGraphicsItem>

class EdgePoint : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    enum PointType{Outside, Inside};
    explicit EdgePoint(PointType type, QGraphicsItem *parent = nullptr);

protected:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    PointType m_type;

signals:
    void outsideChanged();
    void insideChanged();

public slots:
};

#endif // EDGEPOINT_H
