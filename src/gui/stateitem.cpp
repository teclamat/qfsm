#include "stateitem.hpp"

#include "Machine.h"
#include "State.h"
#include "TransitionInfo.h"
#include "common.hpp"
#include "scene.hpp"

#include <QBrush>
#include <QMarginsF>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QStringList>

#include <cmath>

namespace qfsm::gui {

constexpr int SHADOW_SIZE = 4;

static const QPen SELECTION_PEN{ QColor{ Qt::red }, 1 };
static const QPen NORMAL_PEN{ QColor{ Qt::black }, 1 };
static const QBrush BACKGROUND_BRUSH{ Qt::white };
static const QMarginsF FINAL_STATE_MARGIN{ 4.0, 4.0, 4.0, 4.0 };
static const QMarginsF SELECTION_MARGIN{ 2.5, 2.5, 2.5, 2.5 };

StateItem::StateItem(State* a_state)
  : QGraphicsItem{}
  , m_state{ a_state }
{
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
  setZValue(zets::state);

  const State::VisualData& data = m_state->visualData();
  m_outlinePen.setColor(data.outlineColor);
  m_outlinePen.setWidth(data.outlineWidth);
  m_radius = data.radius;
  // setPos(data.x - m_radius, data.y - m_radius);
  setPos(data.x, data.y);
}

QPainterPath StateItem::shape() const
{
  QPainterPath path{};
  const double radius = static_cast<double>(m_radius);
  path.addEllipse({}, radius, radius);
  return path;
}

QRectF StateItem::boundingRect() const
{
  const double boundsSize = static_cast<double>(m_radius + SHADOW_SIZE);
  return QRectF{ QPointF{ -boundsSize, -boundsSize }, QPointF{ boundsSize, boundsSize } };
}

void StateItem::paint(QPainter* a_painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  // qDebug() << "Drawing item" << m_state->getStateName() << "at position" << pos();
  const Scene* graphicsScene = qobject_cast<const Scene*>(scene());
  if (!graphicsScene) {
    return;
  }

  const Machine* machine = graphicsScene->machine();
  if (!machine) {
    return;
  }

  // const double size = static_cast<double>(2 * m_radius) + 0.5;
  const double radius = static_cast<double>(m_radius);
  const QRectF shapeRect{ QPointF{ -radius, -radius }, QPointF{ radius, radius } };
  const bool isMachineTextType = machine->getType() == TransitionType::Text;
  const bool splitStateArea =
      !isMachineTextType && (graphicsScene->options().showMooreOutputs || graphicsScene->options().showStateEncoding);

  a_painter->setRenderHints(QPainter::Antialiasing);
  a_painter->setFont(machine->getSFont());
  a_painter->setPen(NORMAL_PEN);
  a_painter->save();

  if (graphicsScene && graphicsScene->options().showShadows) {
    a_painter->setPen(Qt::NoPen);
    a_painter->setBrush({ graphicsScene->options().shadowColor });
    a_painter->drawEllipse(shapeRect.translated(SHADOW_SIZE, SHADOW_SIZE));
  }

  a_painter->setPen(m_outlinePen);
  a_painter->setBrush(BACKGROUND_BRUSH);
  a_painter->drawEllipse(shapeRect);
  if (m_state->isFinalState()) {
    a_painter->drawEllipse(shapeRect.marginsRemoved(FINAL_STATE_MARGIN));
  }

  a_painter->restore();

  QRectF topArea{ shapeRect };
  QRectF bottomArea{};
  if (splitStateArea) {
    topArea.setCoords(shapeRect.x(), shapeRect.y() + 8.0, shapeRect.right(), -2.0);
    bottomArea.setCoords(shapeRect.x(), 2.0, shapeRect.right(), shapeRect.bottom() - 8.0);
  }

  QStringList name{ m_state->getStateName() };
  if (isMachineTextType) {
    const QString entryActionText = m_state->getEntryActions();
    if (!entryActionText.isEmpty()) {
      name << QString{ "Entry: %1" }.arg(entryActionText);
    }
    const QString exitActionText = m_state->getExitActions();
    if (!exitActionText.isEmpty()) {
      name << QString{ "Exit: %1" }.arg(exitActionText);
    }
  }
  a_painter->drawText(topArea, Qt::AlignCenter | Qt::TextWordWrap | Qt::TextWrapAnywhere, name.join("\n"));

  if (splitStateArea) {
    const double padding = m_state->isFinalState() ? 9.0 : 5.0;
    a_painter->drawLine(QPointF{ shapeRect.left() + padding, 0.0 }, QPointF{ shapeRect.right() - padding, 0.0 });

    QStringList stateTextList{};
    if (graphicsScene->options().showMooreOutputs) {
      stateTextList << m_state->getMooreOutputsStr();
    }
    if (graphicsScene->options().showStateEncoding) {
      stateTextList << m_state->getCodeStr();
    }
    stateTextList.removeAll({});

    if (!stateTextList.isEmpty()) {
      a_painter->drawText(bottomArea, Qt::AlignCenter | Qt::TextWordWrap | Qt::TextWrapAnywhere,
                          stateTextList.join("/"));
    }
  }

  const bool isPrinter = a_painter->device()->devType() == QInternal::Printer;
  if (isSelected() && !isPrinter) {
    a_painter->setPen(SELECTION_PEN);
    a_painter->drawRect(shapeRect.marginsAdded(SELECTION_MARGIN));
  }
}

void StateItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* a_event)
{
  const double x = std::round(pos().x());
  const double y = std::round(pos().y());
  setPos(x, y);
  m_state->visualData().x = x + static_cast<double>(m_radius);
  m_state->visualData().y = y + static_cast<double>(m_radius);

  QGraphicsItem::mouseReleaseEvent(a_event);
}

} // namespace qfsm::gui
