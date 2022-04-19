#include "actionsmanager.hpp"

#include "MainControl.h"
#include "MainWindow.h"
#include "UndoBuffer.h"
#include "optionsmanager.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QMainWindow>
#include <QMimeData>

namespace qfsm::gui {

ActionsManager::ActionsManager(QMainWindow* a_window)
  : QObject{ a_window }
  , m_window{ qobject_cast<MainWindow*>(a_window) }
  , m_modeGroup{ new QActionGroup{ this } }
{
  setupIcons();

  OptionsManager* options = m_window->options();

  QAction* action{};

  // File

  action = m_actions[Group::File][Action::New] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("newdoc")]);
  action->setShortcut(QKeySequence::New);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileNew);

  action = m_actions[Group::File][Action::Open] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("open")]);
  action->setShortcut(QKeySequence::Open);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileOpen);

  action = m_actions[Group::File][Action::Save] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("save")]);
  action->setShortcut(QKeySequence::Save);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileSave);

  action = m_actions[Group::File][Action::SaveAs] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("saveas")]);
  action->setShortcut(QKeySequence::SaveAs);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileSaveAs);

  action = m_actions[Group::File][Action::Print] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("print")]);
  action->setShortcut(QKeySequence::Print);
  connect(action, &QAction::triggered, m_window, &MainWindow::filePrint);

  action = m_actions[Group::File][Action::NewWindow] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("newwindow")]);
  connect(action, &QAction::triggered, m_window->control(), qOverload<>(&MainControl::newWindow));

  action = m_actions[Group::File][Action::Close] = new QAction{ this };
  action->setShortcut(QKeySequence::Close);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileClose);

  action = m_actions[Group::File][Action::Quit] = new QAction{ this };
  action->setShortcut(QKeySequence::Quit);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileQuit);

  // Edit

  action = m_actions[Group::Edit][Action::Undo] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("undo")]);
  action->setShortcut(QKeySequence::Undo);
  connect(action, &QAction::triggered, m_window, &MainWindow::editUndo);

  action = m_actions[Group::Edit][Action::Cut] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("cut")]);
  action->setShortcut(QKeySequence::Cut);
  connect(action, &QAction::triggered, m_window, &MainWindow::editCut);

  action = m_actions[Group::Edit][Action::Copy] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("copy")]);
  action->setShortcut(QKeySequence::Copy);
  connect(action, &QAction::triggered, m_window, &MainWindow::editCopy);

  action = m_actions[Group::Edit][Action::Paste] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("paste")]);
  action->setShortcut(QKeySequence::Paste);
  connect(action, &QAction::triggered, m_window, &MainWindow::editPaste);

  action = m_actions[Group::Edit][Action::Delete] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("delete")]);
  action->setShortcut(QKeySequence::Delete);
  connect(action, &QAction::triggered, m_window, &MainWindow::editDelete);

  action = m_actions[Group::Edit][Action::SelectAll] = new QAction{ this };
  action->setShortcut(QKeySequence::SelectAll);
  connect(action, &QAction::triggered, m_window, &MainWindow::editSelectAll);

  action = m_actions[Group::Edit][Action::ClearSelect] = new QAction{ this };
  action->setShortcut(Qt::CTRL | Qt::Key_D);
  connect(action, &QAction::triggered, m_window, &MainWindow::editDeselectAll);

  action = m_actions[Group::Edit][Action::Options] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("options")]);
  connect(action, &QAction::triggered, m_window, &MainWindow::editOptions);

  // View

  action = m_actions[Group::View][Action::Codes] = new QAction{ this };
  action->setCheckable(true);
  action->setChecked(options->value<bool>(option::Group::View, option::stateEncoding));
  connect(action, &QAction::triggered, m_window, &MainWindow::viewStateEncoding);

  action = m_actions[Group::View][Action::MooreOut] = new QAction{ this };
  action->setCheckable(true);
  action->setChecked(options->value<bool>(option::Group::View, option::mooreOutputs));
  action->setShortcut(Qt::CTRL | Qt::Key_M);
  connect(action, &QAction::triggered, m_window, &MainWindow::viewMooreOutputs);

  action = m_actions[Group::View][Action::MealyIn] = new QAction{ this };
  action->setCheckable(true);
  action->setChecked(options->value<bool>(option::Group::View, option::mealyInputs));
  connect(action, &QAction::triggered, m_window, &MainWindow::viewMealyInputs);

  action = m_actions[Group::View][Action::MealyOut] = new QAction{ this };
  action->setCheckable(true);
  action->setChecked(options->value<bool>(option::Group::View, option::mealyOutputs));
  connect(action, &QAction::triggered, m_window, &MainWindow::viewMealyInputs);

  action = m_actions[Group::View][Action::Shadows] = new QAction{ this };
  action->setCheckable(true);
  action->setChecked(options->value<bool>(option::Group::View, option::shadows));
  connect(action, &QAction::triggered, m_window, &MainWindow::viewShadows);

  action = m_actions[Group::View][Action::Grid] = new QAction{ this };
  action->setCheckable(true);
  action->setChecked(options->value<bool>(option::Group::View, option::grid));
  connect(action, &QAction::triggered, m_window, &MainWindow::viewGrid);

  action = m_actions[Group::View][Action::IoView] = new QAction{ this };
  action->setCheckable(true);
  action->setChecked(options->value<bool>(option::Group::View, option::ioView));
  connect(action, &QAction::triggered, m_window, &MainWindow::viewIOView);

  action = m_actions[Group::View][Action::Select] = new QAction{ this };
  action->setCheckable(true);
  action->setChecked(true);
  action->setActionGroup(m_modeGroup);
  action->setIcon(m_icons[QStringLiteral("select")]);
  action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
  connect(action, &QAction::triggered, m_window, &MainWindow::editSelect);

  action = m_actions[Group::View][Action::Pan] = new QAction{ this };
  action->setCheckable(true);
  action->setActionGroup(m_modeGroup);
  action->setIcon(m_icons[QStringLiteral("panning")]);
  action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_P);
  connect(action, &QAction::triggered, m_window, &MainWindow::viewPan);

  action = m_actions[Group::View][Action::ZoomIn] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("zoomin")]);
  action->setShortcut(QKeySequence::ZoomIn);
  connect(action, &QAction::triggered, m_window, &MainWindow::viewZoomIn);

  action = m_actions[Group::View][Action::ZoomOut] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("zoomout")]);
  action->setShortcut(QKeySequence::ZoomOut);
  connect(action, &QAction::triggered, m_window, &MainWindow::viewZoomOut);

  action = m_actions[Group::View][Action::Zoom100] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("zoom100")]);
  action->setShortcut(Qt::CTRL | Qt::Key_0);
  connect(action, &QAction::triggered, m_window, &MainWindow::viewZoom100);

  // Machine

  action = m_actions[Group::Machine][Action::Simulate] = new QAction{ this };
  action->setCheckable(true);
  action->setActionGroup(m_modeGroup);
  action->setIcon(m_icons[QStringLiteral("run")]);
  action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_I);
  connect(action, &QAction::triggered, m_window, &MainWindow::machineSimulate);

  action = m_actions[Group::Machine][Action::Check] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::machineICheck);

  action = m_actions[Group::Machine][Action::Correct] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::machineCorrectCodes);

  action = m_actions[Group::Machine][Action::Edit] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::machineEdit);

  // State

  action = m_actions[Group::State][Action::Add] = new QAction{ this };
  action->setCheckable(true);
  action->setActionGroup(m_modeGroup);
  action->setIcon(m_icons[QStringLiteral("addstate")]);
  action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
  connect(action, &QAction::triggered, m_window, &MainWindow::stateNew);

  action = m_actions[Group::State][Action::Initial] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::stateSetInitial);

  action = m_actions[Group::State][Action::Final] = new QAction{ this };
  action->setShortcut(Qt::CTRL | Qt::Key_E);
  connect(action, &QAction::triggered, m_window, &MainWindow::stateSetFinal);

  action = m_actions[Group::State][Action::Edit] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::stateEdit);

  // Transition

  action = m_actions[Group::Transition][Action::Add] = new QAction{ this };
  action->setCheckable(true);
  action->setActionGroup(m_modeGroup);
  action->setIcon(m_icons[QStringLiteral("addtransition")]);
  action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_T);
  connect(action, &QAction::triggered, m_window, &MainWindow::transNew);

  action = m_actions[Group::Transition][Action::Straight] = new QAction{ this };
  action->setIcon(m_icons[QStringLiteral("straighten")]);
  action->setShortcut(Qt::CTRL | Qt::Key_T);
  connect(action, &QAction::triggered, m_window, &MainWindow::transStraighten);

  action = m_actions[Group::Transition][Action::Edit] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::transEdit);

  // Help

  action = m_actions[Group::Help][Action::Manual] = new QAction{ this };
  action->setShortcut(Qt::Key_F1);
  connect(action, &QAction::triggered, m_window, &MainWindow::helpManual);

  action = m_actions[Group::Help][Action::About] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::helpAbout);

  action = m_actions[Group::Help][Action::AboutQt] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::helpAboutQt);

  setupNames();
}

QAction* ActionsManager::action(Group a_group, Action a_action)
{
  return m_actions.value(a_group).value(a_action);
}

void ActionsManager::setEnabled(Group a_group, bool a_enabled)
{
  ActionGroup& group = m_actions[a_group];
  for (auto actionIt = group.begin(); actionIt != group.end(); ++actionIt) {
    actionIt.value()->setEnabled(a_enabled);
  }
}

void ActionsManager::setEnabled(Group a_group, Action a_action, bool a_enabled)
{
  QAction* actionPtr = action(a_group, a_action);
  if (actionPtr) {
    actionPtr->setEnabled(a_enabled);
  }
}

void ActionsManager::setEnabled(Group a_group, QList<Action> a_actions, bool a_enabled)
{
  ActionGroup& group = m_actions[a_group];
  for (Action action : a_actions) {
    QAction* actionPtr = group.value(action);
    if (actionPtr) {
      actionPtr->setEnabled(a_enabled);
    }
  }
}

void ActionsManager::update()
{
  const QClipboard* clipboard = qApp->clipboard();
  const bool hasProject = m_window->project() && m_window->project()->isValid();
  const bool hasUndoActions = hasProject && !m_window->project()->undoBuffer()->isEmpty();
  const bool hasClipboardData = clipboard->mimeData() && clipboard->mimeData()->hasFormat("text/qfsm-objects");
  const int selectedStatesCount = m_window->view()->selectedStates();
  const int selectedTransitionsCount = m_window->view()->selectedTransitions();
  const int selectedItemsCount = selectedStatesCount + selectedTransitionsCount;

  setEnabled(Group::Transition, Action::Straight, selectedTransitionsCount > 0);
  setEnabled(Group::Transition, Action::Edit, selectedTransitionsCount == 1);
  setEnabled(Group::State, Action::Final, selectedStatesCount > 0);
  setEnabled(Group::State, { Action::Initial, Action::Edit }, selectedStatesCount == 1);
  setEnabled(Group::Edit, { Action::Cut, Action::Copy, Action::Delete }, selectedItemsCount > 0);
  setEnabled(Group::Edit, Action::Paste, hasProject && hasClipboardData);
  setEnabled(Group::Edit, Action::Undo, hasUndoActions);
}

void ActionsManager::setupNames()
{
  m_actions[Group::File][Action::New]->setText(tr("&New..."));
  m_actions[Group::File][Action::New]->setToolTip(tr("Creates a new file"));
  m_actions[Group::File][Action::Open]->setText(tr("&Open..."));
  m_actions[Group::File][Action::Open]->setToolTip(tr("Opens a file"));
  m_actions[Group::File][Action::Save]->setText(tr("&Save"));
  m_actions[Group::File][Action::Save]->setToolTip(tr("Saves this file"));
  m_actions[Group::File][Action::SaveAs]->setText(tr("Save &As..."));
  m_actions[Group::File][Action::Print]->setText(tr("&Print..."));
  m_actions[Group::File][Action::Print]->setToolTip(tr("Prints this file"));
  m_actions[Group::File][Action::NewWindow]->setText(tr("New &Window"));
  m_actions[Group::File][Action::Close]->setText(tr("&Close"));
  m_actions[Group::File][Action::Quit]->setText(tr("&Quit"));

  m_actions[Group::Edit][Action::Undo]->setText(tr("U&ndo"));
  m_actions[Group::Edit][Action::Undo]->setToolTip(tr("Undo last action"));
  m_actions[Group::Edit][Action::Cut]->setText(tr("C&ut"));
  m_actions[Group::Edit][Action::Cut]->setToolTip(tr("Cuts Selection"));
  m_actions[Group::Edit][Action::Copy]->setText(tr("&Copy"));
  m_actions[Group::Edit][Action::Copy]->setToolTip(tr("Copies Selection"));
  m_actions[Group::Edit][Action::Paste]->setText(tr("&Paste"));
  m_actions[Group::Edit][Action::Paste]->setToolTip(tr("Pastes the clipboard"));
  m_actions[Group::Edit][Action::Delete]->setText(tr("De&lete"));
  m_actions[Group::Edit][Action::SelectAll]->setText(tr("Select &All"));
  m_actions[Group::Edit][Action::ClearSelect]->setText(tr("&Clear selection"));
  m_actions[Group::Edit][Action::Options]->setText(tr("&Options"));

  m_actions[Group::View][Action::Codes]->setText(tr("State &Codes"));
  m_actions[Group::View][Action::MooreOut]->setText(tr("Moo&re Outputs"));
  m_actions[Group::View][Action::MealyIn]->setText(tr("Mealy I&nputs"));
  m_actions[Group::View][Action::MealyOut]->setText(tr("Mea&ly Outputs"));
  m_actions[Group::View][Action::Shadows]->setText(tr("&Shadows"));
  m_actions[Group::View][Action::Grid]->setText(tr("&Grid"));
  m_actions[Group::View][Action::IoView]->setText(tr("&IO View"));
  m_actions[Group::View][Action::Select]->setText(tr("&Select"));
  m_actions[Group::View][Action::Select]->setToolTip(tr("Select objects"));
  m_actions[Group::View][Action::Pan]->setText(tr("&Pan view"));
  m_actions[Group::View][Action::ZoomIn]->setText(tr("Zoom &In"));
  m_actions[Group::View][Action::ZoomIn]->setToolTip(tr("Zooms into the view"));
  m_actions[Group::View][Action::ZoomOut]->setText(tr("Zoom &Out"));
  m_actions[Group::View][Action::ZoomOut]->setToolTip(tr("Zoom out of the view"));
  m_actions[Group::View][Action::Zoom100]->setText(tr("Zoom &100%"));

  m_actions[Group::Machine][Action::Simulate]->setText(tr("&Simulate..."));
  m_actions[Group::Machine][Action::Simulate]->setToolTip(tr("Simulates this machine"));
  m_actions[Group::Machine][Action::Check]->setText(tr("&Integrity Check"));
  m_actions[Group::Machine][Action::Correct]->setText(tr("&Auto correct State Codes..."));
  m_actions[Group::Machine][Action::Edit]->setText(tr("&Edit..."));

  m_actions[Group::State][Action::Add]->setText(tr("&New"));
  m_actions[Group::State][Action::Add]->setToolTip(tr("Add new states"));
  m_actions[Group::State][Action::Initial]->setText(tr("Set &Initial State"));
  m_actions[Group::State][Action::Final]->setText(tr("&Toggle Final State"));
  m_actions[Group::State][Action::Edit]->setText(tr("&Edit..."));

  m_actions[Group::Transition][Action::Add]->setText(tr("&New"));
  m_actions[Group::Transition][Action::Add]->setToolTip(tr("Add new transitions"));
  m_actions[Group::Transition][Action::Straight]->setText(tr("Straighten"));
  m_actions[Group::Transition][Action::Straight]->setToolTip(tr("Straightens selected transitions"));
  m_actions[Group::Transition][Action::Edit]->setText(tr("&Edit..."));

  m_actions[Group::Help][Action::Manual]->setText(tr("Qfsm &Manual..."));
  m_actions[Group::Help][Action::About]->setText(tr("&About..."));
  m_actions[Group::Help][Action::AboutQt]->setText(tr("About &Qt..."));
}

void ActionsManager::setupIcons()
{
  addIcon(QStringLiteral("addstate"));
  addIcon(QStringLiteral("addtransition"));
  addIcon(QStringLiteral("copy"));
  addIcon(QStringLiteral("cut"));
  addIcon(QStringLiteral("delete"));
  addIcon(QStringLiteral("newdoc"));
  addIcon(QStringLiteral("newwindow"));
  addIcon(QStringLiteral("open"));
  addIcon(QStringLiteral("options"));
  addIcon(QStringLiteral("panning"));
  addIcon(QStringLiteral("paste"));
  addIcon(QStringLiteral("print"));
  addIcon(QStringLiteral("run"));
  addIcon(QStringLiteral("save"));
  addIcon(QStringLiteral("saveas"));
  addIcon(QStringLiteral("select"));
  addIcon(QStringLiteral("straighten"));
  addIcon(QStringLiteral("undo"));
  addIcon(QStringLiteral("zoom100"));
  addIcon(QStringLiteral("zoomin"));
  addIcon(QStringLiteral("zoomout"));
}

void ActionsManager::addIcon(const QString& a_iconName)
{
  QIcon& icon = m_icons[a_iconName];
  // icon.addFile(QStringLiteral(":/icons/%1.png").arg(a_iconName));
  icon.addFile(QStringLiteral(":/icons/sc_%1.png").arg(a_iconName), { 16, 16 });
  // icon.addFile(QStringLiteral(":/icons/lc_%1.png").arg(a_iconName), { 24, 24 });
}

} // namespace qfsm::gui
