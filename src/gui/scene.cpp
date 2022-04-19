#include "scene.hpp"

#include "MainWindow.h"
#include "Project.h"
#include "optionsmanager.hpp"

#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QSettings>

namespace qfsm::gui {

Scene::Scene(QMainWindow* a_parent)
  : QGraphicsScene{ a_parent }
  , m_window{ qobject_cast<MainWindow*>(a_parent) }
{
  readOptions();
}

const Machine* Scene::machine() const
{
  Project* project = m_window->project();
  return project && project->isValid() ? project->machine() : nullptr;
}

void Scene::selectAll()
{
  for (QGraphicsItem* item : items()) {
    item->setSelected(true);
  }
}

void Scene::readOptions()
{
  using Group = option::Group;

  const OptionsManager* options = m_window->options();
  m_options.showShadows = options->value<bool>(Group::View, option::shadows);
  m_options.shadowColor = options->value<QRgb>(Group::View, option::shadowColor);
  m_options.showMooreOutputs = options->value<bool>(Group::View, option::mooreOutputs);
  m_options.showStateEncoding = options->value<bool>(Group::View, option::stateEncoding);

  update();
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent* a_event)
{
  if (a_event->button() == Qt::MouseButton::RightButton) {
    a_event->accept();
    return;
  }

  QGraphicsScene::mousePressEvent(a_event);
}

void Scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* a_event)
{
  if (a_event->button() == Qt::MouseButton::RightButton) {
    a_event->accept();
    return;
  }
  QGraphicsScene::mouseDoubleClickEvent(a_event);
}

} // namespace qfsm::gui
