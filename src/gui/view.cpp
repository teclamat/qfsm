#include "view.hpp"

#include "MainWindow.h"
#include "actionsmanager.hpp"
#include "common.hpp"
#include "scene.hpp"
#include "stateitem.hpp"
#include "transitionitem.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QDebug>

#include <algorithm>
#include <array>
#include <cstddef>

namespace qfsm::gui {

constexpr int ZOOM_LEVELS = 11;
constexpr int MAXIMUM_ZOOM_LEVEL = ZOOM_LEVELS - 1;
constexpr int INITIAL_ZOOM_LEVEL = 4;
constexpr std::array<double, static_cast<std::size_t>(ZOOM_LEVELS)> ZOOM_FACTORS{ .25, .5,   .75, .9,  1.0, 1.25,
                                                                                  1.5, 1.75, 2.0, 3.0, 4.0 };

View::View(QMainWindow* a_parent)
  : QGraphicsView{ a_parent }
  , m_scene{ new Scene{ a_parent } }
  , m_window{ qobject_cast<MainWindow*>(a_parent) }
  , m_contextState{ new QMenu{ this } }
  , m_zoomLevel{ INITIAL_ZOOM_LEVEL }
{
  using Action = ActionsManager::Action;
  using Group = ActionsManager::Group;

  setScene(m_scene);
  setDragMode(QGraphicsView::RubberBandDrag);
  setMouseTracking(true);

  zoomReset();

  ActionsManager* manager = m_window->actionsManager();

  m_contextState->addAction(manager->action(Group::Edit, Action::Undo));
  m_contextState->addSeparator();
  m_contextState->addAction(manager->action(Group::Edit, Action::Cut));
  m_contextState->addAction(manager->action(Group::Edit, Action::Copy));
  m_contextState->addAction(manager->action(Group::Edit, Action::Delete));
  m_contextState->addSeparator();
  m_contextState->addAction(manager->action(Group::State, Action::Initial));
  m_contextState->addAction(manager->action(Group::State, Action::Final));
  m_contextState->addSeparator();
  m_contextState->addAction(manager->action(Group::State, Action::Edit));

  connect(m_scene, &Scene::selectionChanged, this, &View::onSelectionChanged);
}

QList<StateItem*> View::selectedStates() const
{
  QList<StateItem*> items{};
  for (QGraphicsItem* item : m_scene->selectedItems()) {
    if (item->type() == STATE_TYPE) {
      items << qgraphicsitem_cast<StateItem*>(item);
    }
  }
  return items;
}

QList<TransitionItem*> View::selectedTransitions() const
{
  QList<TransitionItem*> items{};
  for (QGraphicsItem* item : m_scene->selectedItems()) {
    if (item->type() == TRANSITION_TYPE) {
      items << qgraphicsitem_cast<TransitionItem*>(item);
    }
  }
  return items;
}

void View::zoomIn()
{
  if (m_zoomLevel < MAXIMUM_ZOOM_LEVEL) {
    ++m_zoomLevel;
    zoomChange();
  }
}

void View::zoomOut()
{
  if (m_zoomLevel > 0) {
    --m_zoomLevel;
    zoomChange();
  }
}

void View::zoomReset()
{
  m_zoomLevel = INITIAL_ZOOM_LEVEL;
  zoomChange();
}

void View::zoomChange(bool a_fromWheel)
{
  const double zoomFactor = ZOOM_FACTORS[static_cast<std::size_t>(m_zoomLevel)];
  const ViewportAnchor anchor = transformationAnchor();
  setTransformationAnchor(a_fromWheel ? QGraphicsView::AnchorUnderMouse : QGraphicsView::AnchorViewCenter);
  setTransform(QTransform::fromScale(zoomFactor, zoomFactor));
  setTransformationAnchor(anchor);

  emit zoomChanged(zoomFactor);
}

void View::onModeChanged(DocumentMode a_mode)
{
  switch (a_mode) {
    case DocumentMode::Select:
      setDragMode(DragMode::RubberBandDrag);
      viewport()->setCursor(Qt::CursorShape::ArrowCursor);
      break;
    case DocumentMode::Pan:
      setDragMode(DragMode::ScrollHandDrag);
      break;
    case DocumentMode::NewState:
      [[fallthrough]];
    case DocumentMode::NewTransition:
      setDragMode(DragMode::NoDrag);
      viewport()->setCursor(Qt::CursorShape::CrossCursor);
      break;
    default:
      setDragMode(DragMode::NoDrag);
      viewport()->setCursor(Qt::CursorShape::ArrowCursor);
      break;
  }
}

void View::onSelectionChanged()
{
  QList<QGraphicsItem*> items = m_scene->selectedItems();

  m_selectedStatesCount = 0;
  m_selectedTransitionsCount = 0;
  for (QGraphicsItem* item : items) {
    if (qgraphicsitem_cast<StateItem*>(item)) {
      ++m_selectedStatesCount;
    } else if (qgraphicsitem_cast<TransitionItem*>(item)) {
      ++m_selectedTransitionsCount;
    }
  }

  emit selectionChanged(items.size());
}

void View::contextMenuEvent(QContextMenuEvent* a_event)
{
  const QPoint& menuPosition = a_event->globalPos();
  const int selectedItemsCount = m_scene->selectedItems().count();

  StateItem* state = qgraphicsitem_cast<StateItem*>(itemAt(a_event->pos()));
  if (state) {
    if (!state->isSelected()) {
      m_scene->clearSelection();
      state->setSelected(true);
    }
    m_contextState->popup(menuPosition);
  } else {
    m_window->contextCommon()->popup(menuPosition);
  }
}

void View::mousePressEvent(QMouseEvent* a_event)
{
  QGraphicsView::mousePressEvent(a_event);
}

void View::mouseMoveEvent(QMouseEvent* a_event)
{
  emit positionChanged(mapToScene(a_event->pos()));

  QGraphicsView::mouseMoveEvent(a_event);
}

void View::wheelEvent(QWheelEvent* a_event)
{
  const int angle = a_event->angleDelta().y();
  if (angle > 0) {
    ++m_zoomLevel;
  } else {
    --m_zoomLevel;
  }
  m_zoomLevel = std::clamp(m_zoomLevel, 0, MAXIMUM_ZOOM_LEVEL);
  zoomChange(true);
}

} // namespace qfsm::gui
