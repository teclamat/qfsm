#include "transitionitem.hpp"

#include "Machine.h"
#include "StateOld.h"
#include "Transition.h"
#include "common.hpp"
#include "controlhandleitem.hpp"
#include "scene.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPen>
#include <QRectF>

#include <algorithm>
#include <cmath>

namespace qfsm::gui {

constexpr double PI = 3.1415926536;
constexpr double ARROW_ANGLE = 20.0;
constexpr double ARROW_LENGTH = 15.0;
constexpr double CPOINT_SIZE = 4.0;

static const QPen NORMAL_PEN{ QColor{ Qt::black }, 1 };
static const QPen SELECTION_PEN{ QColor{ Qt::red }, 1 };
static const QPen CONTROL_LINE_PEN{ QColor{ Qt::red }, 1, Qt::DotLine };
static const QBrush BLACK_BRUSH{ QColor{ Qt::black } };
static const QBrush WHITE_BRUSH{ QColor{ Qt::white } };
static const QBrush GREEN_BRUSH{ QColor{ Qt::green } };
static const QBrush RED_BRUSH{ QColor{ Qt::red } };

static const QPainterPathStroker SHAPE_STROKER{ QPen{ {}, 4 } };

const auto setupRectItem = [](ControlHandleItem* a_rectItem, const QPointF& a_center,
                              bool isControlPoint = false) -> void {
  a_rectItem->setCenter(a_center);
  a_rectItem->setPen(NORMAL_PEN);
  a_rectItem->setBrush(isControlPoint ? RED_BRUSH : GREEN_BRUSH);
};

TransitionItem::TransitionItem(Transition* a_transition)
  : QGraphicsItem{}
  , m_transition{ a_transition }
{
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
  setZValue(zets::transition);

  const Transition::VisualData& data = m_transition->visualData();
  m_line << data.begin << data.cp1 << data.cp2 << data.end;
  m_straight = data.straight;

  const QPointF position = m_line.boundingRect().topLeft();
  m_line.translate(-position);
  setPos(position);

  m_handle[handle::start] = new ControlHandleItem{ this, handle::start };
  m_handle[handle::control1] = new ControlHandleItem{ this, handle::control1 };
  m_handle[handle::control2] = new ControlHandleItem{ this, handle::control2 };
  m_handle[handle::end] = new ControlHandleItem{ this, handle::end };
  setupRectItem(m_handle[handle::start], m_line[handle::start]);
  setupRectItem(m_handle[handle::control1], m_line[handle::control1], true);
  setupRectItem(m_handle[handle::control2], m_line[handle::control2], true);
  setupRectItem(m_handle[handle::end], m_line[handle::end]);
}

QRectF TransitionItem::boundingRect() const
{
  return m_line.boundingRect().marginsAdded({ 5.0, 5.0, 5.0, 5.0 });
}

void TransitionItem::paint(QPainter* a_painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  const Scene* graphicsScene = qobject_cast<Scene*>(scene());
  const Machine* machine = graphicsScene ? graphicsScene->machine() : nullptr;
  const State* phantomState = machine ? static_cast<const State*>(machine->getPhantomState()) : nullptr;
  const State* startState = m_transition->getStart();
  const State* endState = m_transition->getEnd();

  const bool invalidTransition =
      (startState == nullptr) || (endState == nullptr) || (startState == phantomState) || (endState == phantomState);

  QPainterPath path{ m_line[handle::start] };
  path.cubicTo(m_line[handle::control1], m_line[handle::control2], m_line[handle::end]);

  a_painter->setRenderHint(QPainter::Antialiasing);

  a_painter->setPen(invalidTransition ? SELECTION_PEN : NORMAL_PEN);
  a_painter->drawPath(path);

  drawArrow(a_painter);

  if (isSelected()) {
    for (ControlHandleItem* item : m_handle) {
      item->setVisible(true);
    }
    if (!m_straight) {
      a_painter->setPen(CONTROL_LINE_PEN);
      a_painter->setBrush(Qt::NoBrush);
      a_painter->drawLine(m_line[handle::start], m_line[handle::control1]);
      a_painter->drawLine(m_line[handle::end], m_line[handle::control2]);
    }
  } else {
    for (ControlHandleItem* item : m_handle) {
      item->setVisible(false);
    }
  }
}

void TransitionItem::drawArrow(QPainter* a_painter)
{
  const QPointF p1 = pointAt(0.9);
  const QPointF p2 = pointAt(1.0);
  const double mg = ((p2.x() - p1.x()) != 0.0) ? ((p1.y() - p2.y()) / (p2.x() - p1.x()))
                                               : ((p2.y() < p1.y() ? 1.0 : -1.0) * 100000.0);

  double phi = std::atan(mg);
  if (p2.x() < p1.x()) {
    phi = phi + PI;
  }
  const double phil = phi - (ARROW_ANGLE / 180.0 * PI);
  const double phir = phi + (ARROW_ANGLE / 180.0 * PI);

  const QPointF arrowLeft{ p2.x() - ARROW_LENGTH * std::cos(phil), p2.y() + ARROW_LENGTH * std::sin(phil) };
  const QPointF arrowRight{ p2.x() - ARROW_LENGTH * std::cos(phir), p2.y() + ARROW_LENGTH * std::sin(phir) };
  const QPointF arrowMiddle{ p2.x() - 0.66 * ARROW_LENGTH * std::cos(phi),
                             p2.y() + 0.66 * ARROW_LENGTH * std::sin(phi) };

  const QPolygonF arrow{ { arrowLeft, m_line[handle::end], arrowRight } };
  // if (type > 2) {
  //   arrow << arrowMiddle;
  // }

  a_painter->setBrush(BLACK_BRUSH);
  a_painter->setPen(Qt::NoPen);
  a_painter->drawPolygon(arrow);
}

QPointF TransitionItem::pointAt(double a_t) const
{
  QPointF point{};

  const double t3 = a_t * a_t * a_t;
  const double tInv = 1.0 - a_t;
  const double tInv2 = tInv * tInv;
  const double tInv3 = tInv2 * tInv;
  const double cp1mod = 3 * tInv2 * a_t;
  const double cp2mod = 3 * tInv * a_t * a_t;

  const QPointF& start = m_line[0];
  const QPointF& end = m_line[3];
  const QPointF& cp1 = m_line[1];
  const QPointF& cp2 = m_line[2];

  point.rx() = tInv3 * start.x() + cp1mod * cp1.x() + cp2mod * cp2.x() + t3 * end.x();
  point.ry() = tInv3 * start.y() + cp1mod * cp1.y() + cp2mod * cp2.y() + t3 * end.y();

  return point;
}

void TransitionItem::handleMoved(int a_handleType, const QPointF& a_offset)
{
  prepareGeometryChange();

  m_line[a_handleType] += a_offset;

  const bool controlHandleMoved = (a_handleType == handle::control1) || (a_handleType == handle::control2);

  if (controlHandleMoved) {
    m_straight = false;
  } else {
    const int nearHandle = (a_handleType == handle::start) ? handle::control1 : handle::control2;
    if (m_straight) {
      const int farHandle = (nearHandle == handle::control1) ? handle::control2 : handle::control1;
      const QPointF nearHandleOffset{ a_offset * 2.0 / 3.0 };
      const QPointF farHandleOffset{ a_offset / 3.0 };
      moveHandle(nearHandle, nearHandleOffset);
      moveHandle(farHandle, farHandleOffset);
    } else {
      moveHandle(nearHandle, a_offset);
    }
  }
}

QPainterPath TransitionItem::shape() const
{
  QPainterPath path{ m_line[0] };
  path.cubicTo(m_line[1], m_line[2], m_line[3]);
  return SHAPE_STROKER.createStroke(path).simplified();
}

void TransitionItem::moveHandle(int a_handleType, const QPointF& a_offset)
{
  m_line[a_handleType] += a_offset;
  m_handle[a_handleType]->moveBy(a_offset.x(), a_offset.y());
}

} // namespace qfsm::gui
