#ifndef SRC_GUI_STATEITEM_HPP
#define SRC_GUI_STATEITEM_HPP

#include "common.hpp"

#include <QGraphicsItem>
#include <QPainter>
#include <QPen>

class State;

/// @namespace qfsm::gui
namespace qfsm::gui {

class StateItem : public QGraphicsItem {
 public:
  explicit StateItem(State* a_state);
  ~StateItem() = default;

  const State* state() const { return m_state; }
  int radius() const { return m_radius; }

 public:
  int type() const override { return STATE_TYPE; }
  QPainterPath shape() const override;
  QRectF boundingRect() const override;
  void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

 private:
  void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;

 private:
  State* m_state;

  QPen m_outlinePen{};
  int m_radius{};
};

} // namespace qfsm::gui

#endif // SRC_GUI_STATEITEM_HPP
