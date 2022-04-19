#ifndef SRC_GUI_VIEW_HPP
#define SRC_GUI_VIEW_HPP

#include <QGraphicsView>

class MainWindow;
class QMenu;
class QAction;
class QMainWindow;
class StatusBar;

enum class DocumentMode;

namespace qfsm::gui {

class Scene;

class View : public QGraphicsView {
  Q_OBJECT
 public:
  explicit View(QMainWindow* a_parent = nullptr);
  ~View() = default;

  Scene* scene() { return m_scene; };

  int selectedStates() const { return m_selectedStates; }
  int selectedTransitions() const { return m_selectedTransitions; }

public slots:
 void onModeChanged(DocumentMode a_mode);

private slots:
 void onSelectionChanged();

private:
 void contextMenuEvent(QContextMenuEvent*) override;
 void mousePressEvent(QMouseEvent*) override;
 void mouseMoveEvent(QMouseEvent*) override;

private:
 Scene* m_scene;
 MainWindow* m_window;
 StatusBar* m_status;
 QMenu* m_contextState;

 int m_selectedStates{};
 int m_selectedTransitions{};
};

} // namespace qfsm::gui

#endif // SRC_GUI_VIEW_HPP
