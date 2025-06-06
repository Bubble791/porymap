#ifndef COLLISIONPIXMAPITEM_H
#define COLLISIONPIXMAPITEM_H

#include <QSpinBox>

#include "metatileselector.h"
#include "movementpermissionsselector.h"
#include "layoutpixmapitem.h"
#include "map.h"
#include "settings.h"

class CollisionPixmapItem : public LayoutPixmapItem {
    Q_OBJECT
public:
    CollisionPixmapItem(Layout *layout, QSpinBox * selectedCollision, QSpinBox * selectedElevation, MetatileSelector *metatileSelector, Settings *settings, qreal *opacity)
        : LayoutPixmapItem(layout, metatileSelector, settings){
        this->selectedCollision = selectedCollision;
        this->selectedElevation = selectedElevation;
        this->opacity = opacity;
        layout->setCollisionItem(this);
    }
    QSpinBox * selectedCollision;
    QSpinBox * selectedElevation;
    qreal *opacity;
    void updateMovementPermissionSelection(QGraphicsSceneMouseEvent *event);
    virtual void paint(QGraphicsSceneMouseEvent*);
    virtual void floodFill(QGraphicsSceneMouseEvent*);
    virtual void magicFill(QGraphicsSceneMouseEvent*);
    virtual void pick(QGraphicsSceneMouseEvent*);
    void draw(bool ignoreCache = false);

private:
    unsigned actionId_ = 0;
    QPoint previousPos;
    void updateSelection(QPoint pos);

signals:
    void mouseEvent(QGraphicsSceneMouseEvent *, CollisionPixmapItem *);
    void hoveredMapMovementPermissionChanged(int, int);
    void hoveredMapMovementPermissionCleared();

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent*);
    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
    void mousePressEvent(QGraphicsSceneMouseEvent*);
    void mouseMoveEvent(QGraphicsSceneMouseEvent*);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
};

#endif // COLLISIONPIXMAPITEM_H
