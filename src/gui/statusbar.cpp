#include "statusbar.hpp"

#include "literals.hpp"

#include <QLabel>

namespace qfsm::gui {

StatusBar::StatusBar(QWidget* a_parent)
  : QStatusBar{ a_parent }
  , m_zoom{ new QLabel{ this } }
  , m_selected{ new QLabel{ this } }
  , m_position{ new QLabel{ this } }
{
  m_zoom->setAlignment(Qt::AlignCenter);
  m_selected->setAlignment(Qt::AlignCenter);
  m_position->setAlignment(Qt::AlignRight);
  m_position->setContentsMargins(0, 0, 10, 0);

  addWidget(m_zoom, 1);
  addWidget(m_selected, 1);
  addWidget(m_position, 2);
}

void StatusBar::reset()
{
  onZoomChanged(1.0);
  onSelectionChanged(0);
  onPositionChanged(QPointF{});
}

void StatusBar::onZoomChanged(double a_zoomFactor)
{
  const auto zoomPercent = static_cast<int>(a_zoomFactor * 100.0);
  m_zoom->setText(tr("Zoom: %1%").arg(zoomPercent));
}

void StatusBar::onSelectionChanged(int a_selectionCount)
{
  m_selected->setText(tr("Selected: %1").arg(a_selectionCount));
}

void StatusBar::onPositionChanged(const QPointF& a_position)
{
  m_position->setText(QString{ u"X: %1  Y: %2"_qs }.arg(a_position.x()).arg(a_position.y()));
}

} // namespace qfsm::gui
