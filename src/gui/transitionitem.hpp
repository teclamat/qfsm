#ifndef SRC_GUI_TRANSITIONITEM_HPP
#define SRC_GUI_TRANSITIONITEM_HPP

#include "common.hpp"

#include <QGraphicsItem>
#include <QPointF>
#include <QPolygonF>

#include <array>

class Transition;

namespace qfsm::gui {

class ControlHandleItem;

class TransitionItem : public QGraphicsItem {
 public:
  explicit TransitionItem(Transition* a_transition);
  ~TransitionItem() = default;

  void handleMoved(int a_type, const QPointF& a_position);

 public:
  int type() const override { return TRANSITION_TYPE; };
  QRectF boundingRect() const override;
  QPainterPath shape() const override;
  void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

 private:
  void moveHandle(int a_handleType, const QPointF& a_offset);
  void drawArrow(QPainter*);
  QPointF pointAt(double a_t) const;

 private:
  Transition* m_transition;
  ControlHandleItem* m_startRect;
  ControlHandleItem* m_endRect;
  ControlHandleItem* m_cp1Rect;
  ControlHandleItem* m_cp2Rect;

  QPolygonF m_line{};
  std::array<ControlHandleItem*, 4> m_handle{};
  bool m_straight{ true };
};

} // namespace qfsm::gui

#endif // SRC_GUI_TRANSITIONITEM_HPP
