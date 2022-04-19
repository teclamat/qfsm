#ifndef SRC_GUI_CONTROLHANDLEITEM_HPP
#define SRC_GUI_CONTROLHANDLEITEM_HPP

#include <QGraphicsRectItem>
#include <QPointF>

namespace qfsm::gui {

class ControlHandleItem : public QGraphicsRectItem {
 public:
  ControlHandleItem(QGraphicsItem* a_parent, int a_type = -1);
  void setCenter(const QPointF& a_center);

 private:
  QVariant itemChange(GraphicsItemChange, const QVariant&) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;

 private:
  int m_type;

  QPointF m_center{};
};

} // namespace qfsm::gui

#endif // SRC_GUI_CONTROLHANDLEITEM_HPP
