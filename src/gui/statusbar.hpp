#ifndef GUI_STATUSBAR_HPP
#define GUI_STATUSBAR_HPP

#include <QPointF>
#include <QStatusBar>

class QLabel;
class QWidget;

namespace qfsm::gui {

class StatusBar : public QStatusBar {
  Q_OBJECT
 public:
  StatusBar(QWidget* a_parent = nullptr);
  ~StatusBar() = default;

  void reset();

 public slots:
  void onZoomChanged(double a_zoomFactor);
  void onSelectionChanged(int a_selectionCount);
  void onPositionChanged(const QPointF& a_position);

 private:
  QLabel* m_zoom;
  QLabel* m_selected;
  QLabel* m_position;
};

} // namespace qfsm::gui

#endif // GUI_STATUSBAR_HPP
