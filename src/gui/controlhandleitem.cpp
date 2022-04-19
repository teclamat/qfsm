#include "controlhandleitem.hpp"

#include "GState.h"
// #include "State.h"
#include "common.hpp"
#include "stateitem.hpp"
#include "transitionitem.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMarginsF>
#include <QSizeF>

namespace qfsm::gui {

static const QMarginsF CONTROL_MARGINS{ 3.0, 3.0, 3.0, 3.0 };

ControlHandleItem::ControlHandleItem(QGraphicsItem* a_parent, int a_type)
  : QGraphicsRectItem{ a_parent }
  , m_type{ a_type }
{
  setFlags(QGraphicsItem::ItemIsMovable);
  setZValue(zets::controlHandle);
}

void ControlHandleItem::setCenter(const QPointF& a_center)
{
  m_center = a_center;
  setRect(QRectF{ m_center, QSizeF{ 0.0, 0.0 } }.marginsAdded(CONTROL_MARGINS));
}

QVariant ControlHandleItem::itemChange(GraphicsItemChange a_change, const QVariant& a_value)
{
  if (a_change == GraphicsItemChange::ItemPositionHasChanged) {
    TransitionItem* transitionItem = qgraphicsitem_cast<TransitionItem*>(parentItem());
    if (transitionItem) {
      transitionItem->handleMoved(m_type, m_center + a_value.toPointF());
    }
  }
  return QGraphicsRectItem::itemChange(a_change, a_value);
}

void ControlHandleItem::mouseMoveEvent(QGraphicsSceneMouseEvent* a_event)
{
  QPointF offset = a_event->pos() - a_event->buttonDownPos(Qt::LeftButton);

  const bool isStartOrEndHandle = (m_type == handle::start) || (m_type == handle::end);
  if (isStartOrEndHandle) {
    for (const QGraphicsItem* item : scene()->items(a_event->scenePos())) {
      if (item->type() != STATE_TYPE) {
        continue;
      }
      const StateItem* state = qgraphicsitem_cast<const StateItem*>(item);
      const QPointF statePos = state->pos();
      QPointF edgePosition{};
      GState::circleEdge(statePos.x(), statePos.y(), state->radius(), a_event->scenePos().x(), a_event->scenePos().y(),
                         edgePosition.rx(), edgePosition.ry());
      offset = mapFromScene(edgePosition) - m_center;
      break;
    }
  }

  setPos(pos() + offset);

  TransitionItem* transitionItem = qgraphicsitem_cast<TransitionItem*>(parentItem());
  if (transitionItem) {
    transitionItem->handleMoved(m_type, offset);
  }
}

} // namespace qfsm::gui
