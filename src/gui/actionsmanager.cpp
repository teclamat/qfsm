#include "actionsmanager.hpp"

#include "MainWindow.h"
#include "TransitionInfo.h"
#include "UndoBuffer.h"
#include "literals.hpp"
#include "maincontrol.hpp"
#include "optionsmanager.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QMainWindow>
#include <QMenu>
#include <QMimeData>

namespace qfsm::gui {

ActionsManager::ActionsManager(QMainWindow* a_window)
  : QObject{ a_window }
  , m_window{ qobject_cast<MainWindow*>(a_window) }
  , m_modeGroup{ new QActionGroup{ this } }
{
  setupIcons();
  setupActions();
  setupMenus();
  setupNames();
}

QAction* ActionsManager::action(Group a_group, Action a_action)
{
  return m_actions.value(a_group).value(a_action);
}

QMenu* ActionsManager::menu(Group a_group)
{
  return m_menus.value(a_group);
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
  const bool notSimulating = m_window->mode() != DocumentMode::Simulating;
  const bool hasProject = m_window->project();
  const bool hasValidProject = hasProject && m_window->project()->isValid();
  const bool hasTextType = hasValidProject && (m_window->project()->machine()->getType() == TransitionType::Text);
  const bool hasUndoActions = hasProject && !m_window->project()->undoBuffer()->isEmpty();
  const int statesCount = hasValidProject ? m_window->project()->machine()->getNumStates() : 0;
  const int selectedStatesCount = m_window->view()->selectedStates();
  const int selectedTransitionsCount = m_window->view()->selectedTransitions();
  const int selectedItemsCount = selectedStatesCount + selectedTransitionsCount;

  m_menus[Group::Export]->setEnabled(hasProject);
  m_menus[Group::View]->setEnabled(hasProject);
  m_menus[Group::Machine]->setEnabled(hasProject);
  m_menus[Group::State]->setEnabled(hasProject && notSimulating);
  m_menus[Group::Transition]->setEnabled(hasProject && notSimulating);

  setEnabled(Group::File, { Action::Save, Action::SaveAs, Action::Print, Action::Close }, hasProject);
  setEnabled(Group::Edit, Action::Undo, notSimulating && hasUndoActions);
  setEnabled(Group::Edit, { Action::Cut, Action::Copy, Action::Delete }, notSimulating && (selectedItemsCount > 0));
  setEnabled(Group::Edit, { Action::SelectAll, Action::ClearSelect }, hasProject && notSimulating);
  setEnabled(Group::View, { Action::ZoomIn, Action::ZoomOut }, hasProject);
  setEnabled(Group::View, { Action::Select, Action::Pan }, hasProject && notSimulating);
  setEnabled(Group::View, { Action::Codes, Action::MooreOut }, !hasTextType);
  setEnabled(Group::Machine, Action::Simulate, hasValidProject && !hasTextType && (statesCount > 0));
  setEnabled(Group::Machine, Action::Edit, notSimulating);
  setEnabled(Group::State, Action::Add, hasProject);
  setEnabled(Group::State, Action::Final, selectedStatesCount > 0);
  setEnabled(Group::State, { Action::Initial, Action::Edit }, selectedStatesCount == 1);
  setEnabled(Group::Transition, Action::Add, hasProject);
  setEnabled(Group::Transition, Action::Straight, selectedTransitionsCount > 0);
  setEnabled(Group::Transition, Action::Edit, selectedTransitionsCount == 1);

  updatePaste();
}

void ActionsManager::updatePaste()
{
  const bool notSimulating = m_window->mode() != DocumentMode::Simulating;
  const QClipboard* clipboard = qApp->clipboard();
  const bool hasProject = m_window->project() && m_window->project()->isValid();
  const bool hasClipboardData = clipboard->mimeData() && clipboard->mimeData()->hasFormat(u"text/qfsm-objects"_qs);

  setEnabled(Group::Edit, Action::Paste, hasProject && notSimulating && hasClipboardData);
}

void ActionsManager::setupNames()
{
  m_actions[Group::File][Action::New]->setText(tr("New..."));
  m_actions[Group::File][Action::New]->setToolTip(tr("Creates a new file"));
  m_actions[Group::File][Action::Open]->setText(tr("Open..."));
  m_actions[Group::File][Action::Open]->setToolTip(tr("Opens a file"));
  m_actions[Group::File][Action::Save]->setText(tr("Save"));
  m_actions[Group::File][Action::Save]->setToolTip(tr("Saves this file"));
  m_actions[Group::File][Action::SaveAs]->setText(tr("Save As..."));
  m_actions[Group::File][Action::Print]->setText(tr("Print..."));
  m_actions[Group::File][Action::Print]->setToolTip(tr("Prints this file"));
  m_actions[Group::File][Action::NewWindow]->setText(tr("New Window"));
  m_actions[Group::File][Action::Close]->setText(tr("Close"));
  m_actions[Group::File][Action::Quit]->setText(tr("Quit"));

  m_actions[Group::Export][Action::ExportEPS]->setText(tr("EPS..."));
  m_actions[Group::Export][Action::ExportSVG]->setText(tr("SVG..."));
  m_actions[Group::Export][Action::ExportPNG]->setText(tr("PNG..."));
  m_actions[Group::Export][Action::ExportAHDL]->setText(tr("AHDL..."));
  m_actions[Group::Export][Action::ExportVHDL]->setText(tr("VHDL..."));
  m_actions[Group::Export][Action::ExportVerilog]->setText(tr("Verilog HDL..."));
  m_actions[Group::Export][Action::ExportKISS]->setText(tr("KISS..."));
  m_actions[Group::Export][Action::ExportTestbench]->setText(tr("VHDL Testbench"));
  m_actions[Group::Export][Action::ExportIoDesc]->setText(tr("I/O Description"));
  m_actions[Group::Export][Action::ExportSCXML]->setText(tr("SCXML"));
  m_actions[Group::Export][Action::ExportVVVV]->setText(tr("vvvv Automata code"));
  m_actions[Group::Export][Action::ExportSTASCII]->setText(tr("State Table (ASCII)..."));
  m_actions[Group::Export][Action::ExportSTLatex]->setText(tr("State Table (Latex)..."));
  m_actions[Group::Export][Action::ExportSTHTML]->setText(tr("State Table (HTML)..."));
  m_actions[Group::Export][Action::ExportRagel]->setText(tr("Ragel..."));
  m_actions[Group::Export][Action::ExportSMC]->setText(tr("SMC..."));

  m_actions[Group::Edit][Action::Undo]->setText(tr("Undo"));
  m_actions[Group::Edit][Action::Undo]->setToolTip(tr("Undo last action"));
  m_actions[Group::Edit][Action::Cut]->setText(tr("Cut"));
  m_actions[Group::Edit][Action::Cut]->setToolTip(tr("Cuts Selection"));
  m_actions[Group::Edit][Action::Copy]->setText(tr("Copy"));
  m_actions[Group::Edit][Action::Copy]->setToolTip(tr("Copies Selection"));
  m_actions[Group::Edit][Action::Paste]->setText(tr("Paste"));
  m_actions[Group::Edit][Action::Paste]->setToolTip(tr("Pastes the clipboard"));
  m_actions[Group::Edit][Action::Delete]->setText(tr("Delete"));
  m_actions[Group::Edit][Action::SelectAll]->setText(tr("Select All"));
  m_actions[Group::Edit][Action::ClearSelect]->setText(tr("Clear selection"));
  m_actions[Group::Edit][Action::Options]->setText(tr("Options"));

  m_actions[Group::View][Action::Codes]->setText(tr("State Codes"));
  m_actions[Group::View][Action::MooreOut]->setText(tr("Moore Outputs"));
  m_actions[Group::View][Action::MealyIn]->setText(tr("Mealy Inputs"));
  m_actions[Group::View][Action::MealyOut]->setText(tr("Mealy Outputs"));
  m_actions[Group::View][Action::Shadows]->setText(tr("Shadows"));
  m_actions[Group::View][Action::Grid]->setText(tr("Grid"));
  m_actions[Group::View][Action::IoView]->setText(tr("IO View"));
  m_actions[Group::View][Action::Select]->setText(tr("Select"));
  m_actions[Group::View][Action::Select]->setToolTip(tr("Select objects"));
  m_actions[Group::View][Action::Pan]->setText(tr("Pan view"));
  m_actions[Group::View][Action::ZoomIn]->setText(tr("Zoom In"));
  m_actions[Group::View][Action::ZoomIn]->setToolTip(tr("Zooms into the view"));
  m_actions[Group::View][Action::ZoomOut]->setText(tr("Zoom Out"));
  m_actions[Group::View][Action::ZoomOut]->setToolTip(tr("Zoom out of the view"));
  m_actions[Group::View][Action::Zoom100]->setText(tr("Zoom 100%"));

  m_actions[Group::Machine][Action::Simulate]->setText(tr("Simulate..."));
  m_actions[Group::Machine][Action::Simulate]->setToolTip(tr("Simulates this machine"));
  m_actions[Group::Machine][Action::Check]->setText(tr("Integrity Check"));
  m_actions[Group::Machine][Action::Correct]->setText(tr("Auto correct State Codes..."));
  m_actions[Group::Machine][Action::Edit]->setText(tr("Edit..."));

  m_actions[Group::State][Action::Add]->setText(tr("New"));
  m_actions[Group::State][Action::Add]->setToolTip(tr("Add new states"));
  m_actions[Group::State][Action::Initial]->setText(tr("Set Initial State"));
  m_actions[Group::State][Action::Final]->setText(tr("Toggle Final State"));
  m_actions[Group::State][Action::Edit]->setText(tr("Edit..."));

  m_actions[Group::Transition][Action::Add]->setText(tr("New", "neuter"));
  m_actions[Group::Transition][Action::Add]->setToolTip(tr("Add new transitions"));
  m_actions[Group::Transition][Action::Straight]->setText(tr("Straighten"));
  m_actions[Group::Transition][Action::Straight]->setToolTip(tr("Straightens selected transitions"));
  m_actions[Group::Transition][Action::Edit]->setText(tr("Edit..."));

  m_actions[Group::Help][Action::Manual]->setText(tr("Qfsm Manual..."));
  m_actions[Group::Help][Action::About]->setText(tr("About..."));
  m_actions[Group::Help][Action::AboutQt]->setText(tr("About Qt..."));

  m_menus[Group::Recent]->setTitle(tr("Open Recent"));
  m_menus[Group::Import]->setTitle(tr("Import"));
  m_menus[Group::Export]->setTitle(tr("Export"));
  m_menus[Group::File]->setTitle(tr("File"));
  m_menus[Group::Edit]->setTitle(tr("Edit"));
  m_menus[Group::View]->setTitle(tr("View"));
  m_menus[Group::Machine]->setTitle(tr("Machine"));
  m_menus[Group::State]->setTitle(tr("State"));
  m_menus[Group::Transition]->setTitle(tr("Transition"));
  m_menus[Group::Help]->setTitle(tr("Help"));
}

void ActionsManager::setupIcons()
{
  addIcon(u"addstate"_qs);
  addIcon(u"addtransition"_qs);
  addIcon(u"copy"_qs);
  addIcon(u"cut"_qs);
  addIcon(u"delete"_qs);
  addIcon(u"newdoc"_qs);
  addIcon(u"newwindow"_qs);
  addIcon(u"open"_qs);
  addIcon(u"options"_qs);
  addIcon(u"panning"_qs);
  addIcon(u"paste"_qs);
  addIcon(u"print"_qs);
  addIcon(u"run"_qs);
  addIcon(u"save"_qs);
  addIcon(u"saveas"_qs);
  addIcon(u"select"_qs);
  addIcon(u"straighten"_qs);
  addIcon(u"undo"_qs);
  addIcon(u"zoom100"_qs);
  addIcon(u"zoomin"_qs);
  addIcon(u"zoomout"_qs);
}

void ActionsManager::setupActions()
{
  OptionsManager* options = m_window->options();

  QAction* action{};

  // File

  action = m_actions[Group::File][Action::New] = new QAction{ this };
  action->setIcon(m_icons[u"newdoc"_qs]);
  action->setShortcut(QKeySequence::New);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileNew);

  action = m_actions[Group::File][Action::Open] = new QAction{ this };
  action->setIcon(m_icons[u"open"_qs]);
  action->setShortcut(QKeySequence::Open);
  connect(action, &QAction::triggered, [this](bool) { m_window->fileOpen(); });

  action = m_actions[Group::File][Action::Save] = new QAction{ this };
  action->setIcon(m_icons[u"save"_qs]);
  action->setShortcut(QKeySequence::Save);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileSave);

  action = m_actions[Group::File][Action::SaveAs] = new QAction{ this };
  action->setIcon(m_icons[u"saveas"_qs]);
  action->setShortcut(QKeySequence::SaveAs);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileSaveAs);

  action = m_actions[Group::File][Action::Print] = new QAction{ this };
  action->setIcon(m_icons[u"print"_qs]);
  action->setShortcut(QKeySequence::Print);
  connect(action, &QAction::triggered, m_window, &MainWindow::filePrint);

  action = m_actions[Group::File][Action::NewWindow] = new QAction{ this };
  action->setIcon(m_icons[u"newwindow"_qs]);
  connect(action, &QAction::triggered, m_window->control(), qOverload<>(&MainControl::newWindow));

  action = m_actions[Group::File][Action::Close] = new QAction{ this };
  action->setShortcut(QKeySequence::Close);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileClose);

  action = m_actions[Group::File][Action::Quit] = new QAction{ this };
  action->setShortcut(QKeySequence::Quit);
  connect(action, &QAction::triggered, m_window, &MainWindow::fileQuit);

  // Export
  action = m_actions[Group::Export][Action::ExportEPS] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportEPS);

  action = m_actions[Group::Export][Action::ExportSVG] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportSVG);

  action = m_actions[Group::Export][Action::ExportPNG] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportPNG);

  action = m_actions[Group::Export][Action::ExportAHDL] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportAHDL);

  action = m_actions[Group::Export][Action::ExportVHDL] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportVHDL);

  action = m_actions[Group::Export][Action::ExportVerilog] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportVerilog);

  action = m_actions[Group::Export][Action::ExportKISS] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportKISS);

  action = m_actions[Group::Export][Action::ExportTestbench] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportTestbench);

  action = m_actions[Group::Export][Action::ExportIoDesc] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportIODescription);

  action = m_actions[Group::Export][Action::ExportSCXML] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportSCXML);

  action = m_actions[Group::Export][Action::ExportVVVV] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportVVVV);

  action = m_actions[Group::Export][Action::ExportSTASCII] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportSTASCII);

  action = m_actions[Group::Export][Action::ExportSTLatex] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportSTLatex);

  action = m_actions[Group::Export][Action::ExportSTHTML] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportSTHTML);

  action = m_actions[Group::Export][Action::ExportRagel] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportRagel);

  action = m_actions[Group::Export][Action::ExportSMC] = new QAction{ this };
  connect(action, &QAction::triggered, m_window, &MainWindow::fileExportSMC);

  // Edit

  action = m_actions[Group::Edit][Action::Undo] = new QAction{ this };
  action->setIcon(m_icons[u"undo"_qs]);
  action->setShortcut(QKeySequence::Undo);
  connect(action, &QAction::triggered, m_window, &MainWindow::editUndo);

  action = m_actions[Group::Edit][Action::Cut] = new QAction{ this };
  action->setIcon(m_icons[u"cut"_qs]);
  action->setShortcut(QKeySequence::Cut);
  connect(action, &QAction::triggered, m_window, &MainWindow::editCut);

  action = m_actions[Group::Edit][Action::Copy] = new QAction{ this };
  action->setIcon(m_icons[u"copy"_qs]);
  action->setShortcut(QKeySequence::Copy);
  connect(action, &QAction::triggered, m_window, &MainWindow::editCopy);

  action = m_actions[Group::Edit][Action::Paste] = new QAction{ this };
  action->setIcon(m_icons[u"paste"_qs]);
  action->setShortcut(QKeySequence::Paste);
  connect(action, &QAction::triggered, m_window, &MainWindow::editPaste);

  action = m_actions[Group::Edit][Action::Delete] = new QAction{ this };
  action->setIcon(m_icons[u"delete"_qs]);
  action->setShortcut(QKeySequence::Delete);
  connect(action, &QAction::triggered, m_window, &MainWindow::editDelete);

  action = m_actions[Group::Edit][Action::SelectAll] = new QAction{ this };
  action->setShortcut(QKeySequence::SelectAll);
  connect(action, &QAction::triggered, m_window, &MainWindow::editSelectAll);

  action = m_actions[Group::Edit][Action::ClearSelect] = new QAction{ this };
  action->setShortcut(Qt::CTRL | Qt::Key_D);
  connect(action, &QAction::triggered, m_window, &MainWindow::editDeselectAll);

  action = m_actions[Group::Edit][Action::Options] = new QAction{ this };
  action->setIcon(m_icons[u"options"_qs]);
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
  action->setIcon(m_icons[u"select"_qs]);
  action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
  connect(action, &QAction::triggered, m_window, &MainWindow::editSelect);

  action = m_actions[Group::View][Action::Pan] = new QAction{ this };
  action->setCheckable(true);
  action->setActionGroup(m_modeGroup);
  action->setIcon(m_icons[u"panning"_qs]);
  action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_P);
  connect(action, &QAction::triggered, m_window, &MainWindow::viewPan);

  action = m_actions[Group::View][Action::ZoomIn] = new QAction{ this };
  action->setIcon(m_icons[u"zoomin"_qs]);
  action->setShortcut(QKeySequence::ZoomIn);
  connect(action, &QAction::triggered, m_window, &MainWindow::viewZoomIn);

  action = m_actions[Group::View][Action::ZoomOut] = new QAction{ this };
  action->setIcon(m_icons[u"zoomout"_qs]);
  action->setShortcut(QKeySequence::ZoomOut);
  connect(action, &QAction::triggered, m_window, &MainWindow::viewZoomOut);

  action = m_actions[Group::View][Action::Zoom100] = new QAction{ this };
  action->setIcon(m_icons[u"zoom100"_qs]);
  action->setShortcut(Qt::CTRL | Qt::Key_0);
  connect(action, &QAction::triggered, m_window, &MainWindow::viewZoom100);

  // Machine

  action = m_actions[Group::Machine][Action::Simulate] = new QAction{ this };
  action->setCheckable(true);
  action->setActionGroup(m_modeGroup);
  action->setIcon(m_icons[u"run"_qs]);
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
  action->setIcon(m_icons[u"addstate"_qs]);
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
  action->setIcon(m_icons[u"addtransition"_qs]);
  action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_T);
  connect(action, &QAction::triggered, m_window, &MainWindow::transNew);

  action = m_actions[Group::Transition][Action::Straight] = new QAction{ this };
  action->setIcon(m_icons[u"straighten"_qs]);
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
}

void ActionsManager::setupMenus()
{
  QMenu* menu{};

  // Recents
  menu = m_menus[Group::Recent] = new QMenu{ m_window };
  connect(menu, &QMenu::aboutToShow, m_window, &MainWindow::refreshMRU);

  // Import
  m_menus[Group::Import] = new QMenu{ m_window };

  // Export
  menu = m_menus[Group::Export] = new QMenu{ m_window };
  menu->addAction(m_actions[Group::Export][Action::ExportEPS]);
  menu->addAction(m_actions[Group::Export][Action::ExportSVG]);
  menu->addAction(m_actions[Group::Export][Action::ExportPNG]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Export][Action::ExportAHDL]);
  menu->addAction(m_actions[Group::Export][Action::ExportVHDL]);
  menu->addAction(m_actions[Group::Export][Action::ExportVerilog]);
  menu->addAction(m_actions[Group::Export][Action::ExportKISS]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Export][Action::ExportTestbench]);
  menu->addAction(m_actions[Group::Export][Action::ExportIoDesc]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Export][Action::ExportSCXML]);
  menu->addAction(m_actions[Group::Export][Action::ExportVVVV]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Export][Action::ExportSTASCII]);
  menu->addAction(m_actions[Group::Export][Action::ExportSTLatex]);
  menu->addAction(m_actions[Group::Export][Action::ExportSTHTML]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Export][Action::ExportRagel]);
  menu->addAction(m_actions[Group::Export][Action::ExportSMC]);

  // File
  menu = m_menus[Group::File] = new QMenu{ m_window };
  menu->addAction(m_actions[Group::File][Action::New]);
  menu->addAction(m_actions[Group::File][Action::Open]);
  menu->addMenu(m_menus[Group::Recent]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::File][Action::Save]);
  menu->addAction(m_actions[Group::File][Action::SaveAs]);
  menu->addSeparator();
  menu->addMenu(m_menus[Group::Import]);
  menu->addMenu(m_menus[Group::Export]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::File][Action::Print]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::File][Action::NewWindow]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::File][Action::Close]);
  menu->addAction(m_actions[Group::File][Action::Quit]);

  // Edit
  menu = m_menus[Group::Edit] = new QMenu{ m_window };
  menu->addAction(m_actions[Group::Edit][Action::Undo]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Edit][Action::Cut]);
  menu->addAction(m_actions[Group::Edit][Action::Copy]);
  menu->addAction(m_actions[Group::Edit][Action::Paste]);
  menu->addAction(m_actions[Group::Edit][Action::Delete]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Edit][Action::SelectAll]);
  menu->addAction(m_actions[Group::Edit][Action::ClearSelect]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Edit][Action::Options]);

  // View
  menu = m_menus[Group::View] = new QMenu{ m_window };
  menu->addAction(m_actions[Group::View][Action::Codes]);
  menu->addAction(m_actions[Group::View][Action::MooreOut]);
  menu->addAction(m_actions[Group::View][Action::MealyIn]);
  menu->addAction(m_actions[Group::View][Action::MealyOut]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::View][Action::Shadows]);
  menu->addAction(m_actions[Group::View][Action::Grid]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::View][Action::IoView]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::View][Action::Select]);
  menu->addAction(m_actions[Group::View][Action::Pan]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::View][Action::ZoomIn]);
  menu->addAction(m_actions[Group::View][Action::ZoomOut]);
  menu->addAction(m_actions[Group::View][Action::Zoom100]);

  // Machine
  menu = m_menus[Group::Machine] = new QMenu{ m_window };
  menu->addAction(m_actions[Group::Machine][Action::Simulate]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Machine][Action::Check]);
  menu->addAction(m_actions[Group::Machine][Action::Correct]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Machine][Action::Edit]);

  // State
  menu = m_menus[Group::State] = new QMenu{ m_window };
  menu->addAction(m_actions[Group::State][Action::Add]);
  menu->addAction(m_actions[Group::State][Action::Initial]);
  menu->addAction(m_actions[Group::State][Action::Final]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::State][Action::Edit]);

  // Transition
  menu = m_menus[Group::Transition] = new QMenu{ m_window };
  menu->addAction(m_actions[Group::Transition][Action::Add]);
  menu->addAction(m_actions[Group::Transition][Action::Straight]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Transition][Action::Edit]);

  // Help
  menu = m_menus[Group::Help] = new QMenu{ m_window };
  menu->addAction(m_actions[Group::Help][Action::Manual]);
  menu->addSeparator();
  menu->addAction(m_actions[Group::Help][Action::About]);
  menu->addAction(m_actions[Group::Help][Action::AboutQt]);
}

void ActionsManager::addIcon(const QString& a_iconName)
{
  QIcon& icon = m_icons[a_iconName];
  // icon.addFile(u":/icons/%1.png"_qs.arg(a_iconName));
  icon.addFile(u":/icons/sc_%1.png"_qs.arg(a_iconName), { 16, 16 });
  // icon.addFile(u":/icons/lc_%1.png"_qs.arg(a_iconName), { 24, 24 });
}

} // namespace qfsm::gui
