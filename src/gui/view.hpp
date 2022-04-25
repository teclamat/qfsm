#ifndef SRC_GUI_VIEW_HPP
#define SRC_GUI_VIEW_HPP

#include <QGraphicsView>
#include <QList>

class MainWindow;
class QMenu;
class QAction;
class QMainWindow;

enum class DocumentMode;

namespace qfsm::gui {

class Scene;
class StateItem;
class TransitionItem;

class View : public QGraphicsView {
  Q_OBJECT
 public:
  explicit View(QMainWindow* a_parent = nullptr);
  ~View() = default;

  Scene* scene() { return m_scene; };

  QList<StateItem*> selectedStates() const;
  QList<TransitionItem*> selectedTransitions() const;
  int selectedStatesCount() const { return m_selectedStatesCount; }
  int selectedTransitionsCount() const { return m_selectedTransitionsCount; }

 public slots:
  void onModeChanged(DocumentMode a_mode);
  void zoomIn();
  void zoomOut();
  void zoomReset();

 signals:
  void zoomChanged(double a_zoomFactor);
  void selectionChanged(int a_selectionCount);
  void positionChanged(const QPointF& a_position);

 private:
  void zoomChange(bool a_fromWheel = false);

 private slots:
  void onSelectionChanged();

 private:
  void contextMenuEvent(QContextMenuEvent*) override;
  void mousePressEvent(QMouseEvent*) override;
  void mouseMoveEvent(QMouseEvent*) override;
  void wheelEvent(QWheelEvent*) override;

 private:
  Scene* m_scene;
  MainWindow* m_window;
  QMenu* m_contextState;
  int m_zoomLevel;

  int m_selectedStatesCount{};
  int m_selectedTransitionsCount{};
};

} // namespace qfsm::gui

#endif // SRC_GUI_VIEW_HPP
