#include "view.hpp"

#include "MainWindow.h"
#include "StatusBar.h"
#include "actionsmanager.hpp"
#include "scene.hpp"
#include "stateitem.hpp"
#include "transitionitem.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QDebug>

namespace qfsm::gui {

View::View(QMainWindow* a_parent)
  : QGraphicsView{ a_parent }
  , m_scene{ new Scene{ a_parent } }
  , m_window{ qobject_cast<MainWindow*>(a_parent) }
  , m_status{ m_window->getStatusBar() }
  , m_contextState{ new QMenu{ this } }
{
  using Action = ActionsManager::Action;
  using Group = ActionsManager::Group;

  setScene(m_scene);
  setDragMode(QGraphicsView::RubberBandDrag);
  setMouseTracking(true);

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
  m_selectedStates = 0;
  m_selectedTransitions = 0;
  for (QGraphicsItem* item : items) {
    if (qgraphicsitem_cast<StateItem*>(item)) {
      ++m_selectedStates;
    } else if (qgraphicsitem_cast<TransitionItem*>(item)) {
      ++m_selectedTransitions;
    }
  }
  m_status->setSelected(items.count());
  m_window->actionsManager()->update();
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

void View::mousePressEvent(QMouseEvent* a_event) {
  QGraphicsView::mousePressEvent(a_event);
}

void View::mouseMoveEvent(QMouseEvent* a_event)
{
  // qDebug() << "Mouse moved to" << a_event->pos() << mapToScene(a_event->pos());
  const QPointF position = mapToScene(a_event->pos());
  m_status->setPosition(position.x(), position.y());

  QGraphicsView::mouseMoveEvent(a_event);
}

} // namespace qfsm::gui
