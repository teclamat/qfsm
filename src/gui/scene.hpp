#ifndef SRC_GUI_SCENE_HPP
#define SRC_GUI_SCENE_HPP

#include <QGraphicsScene>
#include <QMainWindow>
#include <QRgb>

class MainWindow;
class Machine;

namespace qfsm::gui {

class Scene : public QGraphicsScene {
  Q_OBJECT
 public:
  struct Options {
    bool showShadows;
    QRgb shadowColor;
    bool showMooreOutputs;
    bool showStateEncoding;
  };

  explicit Scene(QMainWindow* a_parent = nullptr);
  ~Scene() = default;

  const Options& options() const { return m_options; }
  const Machine* machine() const;

 public slots:
  void readOptions();
  void selectAll();

 private:
  void mousePressEvent(QGraphicsSceneMouseEvent*) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*) override;

 private:
  MainWindow* m_window;

  Options m_options{};
};

} // namespace qfsm::gui

#endif // SRC_GUI_SCENE_HPP
