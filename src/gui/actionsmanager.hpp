#ifndef SRC_GUI_ACTIONSMANAGER_HPP
#define SRC_GUI_ACTIONSMANAGER_HPP

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

class QAction;
class QActionGroup;
class QMainWindow;
class MainWindow;

namespace qfsm::gui {

class ActionsManager : public QObject {
  Q_OBJECT
 public:
  enum class Group { File, Edit, View, Machine, State, Transition, Help };
  enum class Action {
    New,
    Open,
    Save,
    SaveAs,
    Print,
    NewWindow,
    Close,
    Quit,
    Undo,
    Cut,
    Copy,
    Paste,
    Delete,
    SelectAll,
    ClearSelect,
    Options,
    Codes,
    MooreOut,
    MealyIn,
    MealyOut,
    Shadows,
    Grid,
    IoView,
    Select,
    Pan,
    ZoomIn,
    ZoomOut,
    Zoom100,
    Simulate,
    Correct,
    Check,
    Add,
    Edit,
    Initial,
    Final,
    Straight,
    Manual,
    About,
    AboutQt
  };

  using ActionGroup = QHash<Action, QAction*>;
  using ActionMap = QHash<Group, ActionGroup>;
  using IconMap = QHash<QString, QIcon>;

  ActionsManager(QMainWindow* a_window = nullptr);
  ~ActionsManager() = default;

  QAction* action(Group a_group, Action a_action);

  void setEnabled(Group a_group, bool a_enabled = true);
  void setEnabled(Group a_group, Action a_action, bool a_enabled = true);
  void setEnabled(Group a_group, QList<Action> a_actions, bool a_enabled = true);

  void update();
  void setupNames();

 private:
  void setupIcons();
  void addIcon(const QString& a_iconName);

 private:
  MainWindow* m_window;
  QActionGroup* m_modeGroup;

  ActionMap m_actions{};
  IconMap m_icons{};
};

} // namespace qfsm::gui

#endif // SRC_GUI_ACTIONSMANAGER_HPP
