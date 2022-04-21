/*
Copyright (C) 2000,2001 Stefan Duffner

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "MainWindow.h"
#include "DrawArea.h"
#include "Edit.h"
#include "ExportAHDL.h"
#include "ExportEPS.h"
#include "ExportIODescription.h"
#include "ExportKISS.h"
#include "ExportPNG.h"
#include "ExportRagel.h"
#include "ExportSCXML.h"
#include "ExportSMC.h"
#include "ExportSVG.h"
#include "ExportStateTable.h"
#include "ExportVHDL.h"
#include "ExportVVVV.h"
#include "ExportVerilog.h"
#include "FileIO.h"
#include "ICheck.h"
#include "IOInfo.h"
#include "ImportGraphviz.h"
#include "PrintManager.h"
#include "Project.h"
#include "ScrollView.h"
#include "Selection.h"
#include "Simulator.h"
#include "StateManager.h"
#include "StatusBar.h"
#include "TableBuilderASCII.h"
#include "TableBuilderHTML.h"
#include "TableBuilderLatex.h"
#include "TransitionInfo.h"
#include "UndoBuffer.h"

#include "info.hpp"
#include "literals.hpp"
#include "maincontrol.hpp"
#include "optionsmanager.hpp"

#include "gui/actionsmanager.hpp"
#include "gui/error.hpp"
#include "gui/stateitem.hpp"
#include "gui/transitionitem.hpp"
#include "gui/view.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QMenuBar>
#include <QMimeData>
#include <QObject>
#include <QSettings>
#include <QUrl>

#include <fstream>
#include <sstream>
#include <string>

constexpr auto QFSM_MIME_DATA_TYPE = "text/qfsm-objects";

/**
 * Constructor.
 * Initialises the mainwindow with all its menus.
 */
MainWindow::MainWindow(QObject* a_parent)
  : QMainWindow{}
  , m_control{ qobject_cast<qfsm::MainControl*>(a_parent) }
  , m_statusBar{ new StatusBar{ this } }
  , m_optionsManager{ new qfsm::OptionsManager{ this } }
  , m_actionsManager{ new qfsm::gui::ActionsManager{ this } }
  , m_view{ new qfsm::gui::View{ this } }
  , m_mainView{ new ScrollView(this) }
{
  m_mainView->setVisible(false);
  // m_graphicsScene = new QGraphicsScene{ this };
  // m_graphicsView = new QGraphicsView{ m_graphicsScene, this };
  // m_graphicsView->setViewport(new QOpenGLWidget{});
  setCentralWidget(m_view);
  m_view->scene()->setSceneRect(0, 0, 800, 600);
  // setCentralWidget(m_mainView);
  setAcceptDrops(true);

  appIcon.addFile(":/icons/qfsm32.png", { 32, 32 });
  appIcon.addFile(":/icons/qfsm48.png", { 48, 48 });
  setWindowIcon(appIcon);

  createToolBar();

  /*
  qDebug("%d", IOInfo::isBinaryType("ANY 01101|0|101"));
  qDebug("%d", IOInfo::isBinaryType("DEF"));
  qDebug("%d", IOInfo::isBinaryType("ANY01101|0|101"));
  qDebug("%d", IOInfo::isBinaryType("ANY  01101101"));
  qDebug("%d", IOInfo::isBinaryType("ANY"));
  qDebug("%d", IOInfo::isBinaryType("01101|0|101"));
  qDebug("%d", IOInfo::isBinaryType("01101101"));
  */
  /*
  qDebug("%d", IOInfo::isASCIIType("01101|0|101"));
  qDebug("%d", IOInfo::isASCIIType("transition1"));
  qDebug("%d", IOInfo::isASCIIType("\\011"));
  qDebug("%d", IOInfo::isASCIIType("This is a test."));
  qDebug("%d", IOInfo::isASCIIType("abcde"));
  qDebug("%d", IOInfo::isASCIIType("abce\\n"));
  */
  using Group = qfsm::gui::ActionsManager::Group;

  QMenu* menu_import = m_actionsManager->menu(Group::Import);
#ifdef GRAPHVIZ_FOUND
  id_import_graphviz = menu_import->insertItem(tr("&Graphviz..."), this, SLOT(fileImportGraphviz()));
#else
  menu_import->setEnabled(false);
#endif

  QMenuBar* mainMenuBar = menuBar();
  mainMenuBar->addMenu(m_actionsManager->menu(Group::File));
  mainMenuBar->addMenu(m_actionsManager->menu(Group::Edit));
  mainMenuBar->addMenu(m_actionsManager->menu(Group::View));
  mainMenuBar->addMenu(m_actionsManager->menu(Group::Machine));
  mainMenuBar->addMenu(m_actionsManager->menu(Group::State));
  mainMenuBar->addMenu(m_actionsManager->menu(Group::Transition));
  mainMenuBar->addMenu(m_actionsManager->menu(Group::Help));

  // Context Menu: State
  // cmenu_state = new QMenu(this);
  // id_csundo = cmenu_state->addAction(*undoset, tr("U&ndo"), this, SLOT(editUndo()), Qt::CTRL | Qt::Key_Z);
  // cmenu_state->addSeparator();
  // id_cscut = cmenu_state->addAction(*cutset, tr("C&ut"), this, SLOT(editCut()), Qt::CTRL | Qt::Key_X);
  // id_cscopy = cmenu_state->addAction(*copyset, tr("&Copy"), this, SLOT(editCopy()), Qt::CTRL | Qt::Key_C);
  // id_csdelete = cmenu_state->addAction(tr("De&lete"), this, SLOT(editDelete()), Qt::Key_Delete);
  // cmenu_state->addSeparator();
  // id_ceditstate = cmenu_state->addAction(tr("&Edit..."), this, SLOT(stateEdit()));
  // id_csetinitial = cmenu_state->addAction(tr("Set &Initial State"), this, SLOT(stateSetInitial()));
  // id_csetend = cmenu_state->addAction(tr("&Toggle Final State"), this, SLOT(stateSetFinal()));

  // Context Menu: Transition
  // cmenu_trans = new QMenu(this);
  // cmenu_trans->setMouseTracking(true);
  // id_ctundo = cmenu_trans->addAction(*undoset, tr("U&ndo"), this, SLOT(editUndo()), Qt::CTRL | Qt::Key_Z);
  // cmenu_trans->addSeparator();
  // id_ctcut = cmenu_trans->addAction(*cutset, tr("C&ut"), this, SLOT(editCut()), Qt::CTRL | Qt::Key_X);
  // id_ctcopy = cmenu_trans->addAction(*copyset, tr("&Copy"), this, SLOT(editCopy()), Qt::CTRL | Qt::Key_C);
  // id_ctdelete = cmenu_trans->addAction(tr("De&lete"), this, SLOT(editDelete()), Qt::Key_Delete);
  // cmenu_trans->addSeparator();
  // id_cedittrans = cmenu_trans->addAction(tr("&Edit..."), this, SLOT(transEdit()));
  // id_ctrans_straight = cmenu_trans->addAction(*transstraightenset, tr("&Straighten"), this, SLOT(transStraighten()),
  //                                             Qt::CTRL | Qt::Key_T);

  // Context Menu: ScrollView
  // cmenu_sview = menu_edit;

  setStatusBar(m_statusBar);

  m_stateManager = new StateManager(this);
  machinemanager = new MachineManager(this);
  m_transitionManager = new TransitionManager(this);
  fileio = new FileIO(this);
  printmanager = new PrintManager(this);

  mb_changed = new QMessageBox("qfsm", tr("The file has been changed. Do you want to save it?"),
                               QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                               QMessageBox::Cancel | QMessageBox::Escape);
  mb_changed->setButtonText(QMessageBox::Yes, tr("Yes"));
  mb_changed->setButtonText(QMessageBox::No, tr("No"));
  mb_changed->setButtonText(QMessageBox::Cancel, tr("Cancel"));

  fileio->loadOptions(&doc_options);

  tabwidgetdialog = new OptionsDlg(this);

  opt_general = new OptGeneralDlgImpl(tabwidgetdialog);
  opt_general->init();

  opt_display = new OptDisplayDlgImpl(tabwidgetdialog);
  opt_display->init(&doc_options);

  opt_printing = new OptPrintingDlgImpl(tabwidgetdialog);
  opt_printing->init(&doc_options);

  tabwidgetdialog->ui.tabs->addTab(opt_general, tr("&General"));
  tabwidgetdialog->ui.tabs->addTab(opt_display, tr("&Display"));
  tabwidgetdialog->ui.tabs->addTab(opt_printing, tr("&Printing"));

  ahdl_export = new ExportAHDLDlgImpl(this);
  ahdl_export->init(&doc_options);
  ver_export = new ExportVerilogDlgImpl(this);
  ver_export->init(&doc_options);
  vhdl_export = new ExportVHDLDlgImpl(this);
  vhdl_export->init(&doc_options, NULL);
  statetable_export = new ExportStateTableDlgImpl(this);
  statetable_export->init(&doc_options);
  ragel_export = new ExportRagelDlgImpl(this);
  ragel_export->init(&doc_options);
  testbench_export = new ExportTestbenchDlgImpl(this);
  testbench_export->init(&doc_options, NULL);
  vvvv_export = new ExportVVVVDlgImpl(this);
  vvvv_export->init(&doc_options, NULL);

  view_io = new IOViewDlgImpl(this);
  simulator = new Simulator(this);
  ichecker = new ICheck(this);

  shift_pressed = false;
  control_pressed = false;
  m_isCutOperation = false;

  connect(this, &MainWindow::modeChanged, m_actionsManager, &qfsm::gui::ActionsManager::update);
  connect(this, &MainWindow::modeChanged, m_view, &qfsm::gui::View::onModeChanged);

  setMode(DocumentMode::Select);
  updateAll(); // MenuBar();

  //  connect(cmenu_state, SIGNAL(aboutToHide()), m_mainView,
  //  SLOT(contextMenuHiding())); connect(cmenu_trans, SIGNAL(aboutToHide()),
  //  m_mainView, SLOT(contextMenuHiding())); connect(cmenu_sview,
  //  SIGNAL(aboutToHide()), m_mainView, SLOT(contextMenuHiding()));
  // connect(this, SIGNAL(allSelected()), m_mainView->getDrawArea(), SLOT(allSelected()));
  connect(this, SIGNAL(objectsPasted()), m_mainView->getDrawArea(), SLOT(objectsPasted()));
  connect(this, SIGNAL(quitWindow(MainWindow*)), m_control, SLOT(quitWindow(MainWindow*)));
  connect(this, SIGNAL(escapePressed()), m_mainView->getDrawArea(), SLOT(escapePressed()));
  connect(m_mainView->getDrawArea(), SIGNAL(zoomedToPercentage(int)), m_statusBar, SLOT(setZoom(int)));
  connect(this, SIGNAL(updateStatusZoom(int)), m_mainView->getDrawArea(), SIGNAL(zoomedToPercentage(int)));
  connect(fileio, SIGNAL(statusMessage(QString)), this, SLOT(statusMessage(QString)));
  connect(fileio, SIGNAL(setWaitCursor()), this, SLOT(setWaitCursor()));
  connect(fileio, SIGNAL(setPreviousCursor()), this, SLOT(setPreviousCursor()));
  connect(view_io, SIGNAL(closing()), this, SLOT(viewIOView()));
  connect(vvvv_export, SIGNAL(updateCode()), this, SLOT(updateVVVV()));
}

/// Destructor
MainWindow::~MainWindow()
{
  fileio->saveOptions(&doc_options);

  delete m_mainView;
  if (m_project)
    delete m_project;

  delete m_stateManager;
  delete machinemanager;
  delete m_transitionManager;
  delete printmanager;
  delete fileio;
  delete m_statusBar;
  delete simulator;
  delete ichecker;

  delete mb_changed;

  delete ahdl_export;
  delete ver_export;
  delete vhdl_export;
  delete statetable_export;
  delete ragel_export;
  delete testbench_export;
  delete vvvv_export;
}

QMenu* MainWindow::contextCommon()
{
  return m_actionsManager->menu(qfsm::gui::ActionsManager::Group::Edit);
}

/// Creates the toolbar with its buttons
void MainWindow::createToolBar()
{
  using Group = qfsm::gui::ActionsManager::Group;
  using Action = qfsm::gui::ActionsManager::Action;

  toolbar = addToolBar("Main Toolbar");
  toolbar->setMovable(true);

  toolbar->addAction(m_actionsManager->action(Group::File, Action::New));
  toolbar->addAction(m_actionsManager->action(Group::File, Action::Open));
  toolbar->addAction(m_actionsManager->action(Group::File, Action::Save));
  toolbar->addAction(m_actionsManager->action(Group::File, Action::Print));
  toolbar->addSeparator();
  toolbar->addAction(m_actionsManager->action(Group::Edit, Action::Undo));
  toolbar->addAction(m_actionsManager->action(Group::Edit, Action::Cut));
  toolbar->addAction(m_actionsManager->action(Group::Edit, Action::Copy));
  toolbar->addAction(m_actionsManager->action(Group::Edit, Action::Paste));
  toolbar->addSeparator();
  toolbar->addAction(m_actionsManager->action(Group::View, Action::Select));
  toolbar->addAction(m_actionsManager->action(Group::View, Action::Pan));
  toolbar->addAction(m_actionsManager->action(Group::State, Action::Add));
  toolbar->addAction(m_actionsManager->action(Group::Transition, Action::Add));
  toolbar->addAction(m_actionsManager->action(Group::Machine, Action::Simulate));
  toolbar->addSeparator();
  toolbar->addAction(m_actionsManager->action(Group::View, Action::ZoomIn));
  toolbar->addAction(m_actionsManager->action(Group::View, Action::ZoomOut));
  toolbar->addAction(m_actionsManager->action(Group::Transition, Action::Straight));
}

/// Called when a key is pressed
void MainWindow::keyPressEvent(QKeyEvent* k)
{
  if (k->key() == Qt::Key_Shift)
    shift_pressed = true;
  else if (k->key() == Qt::Key_Control)
    control_pressed = true;
  else if (k->key() == Qt::Key_Escape) {
    emit escapePressed();
  }
}

/// Called when a key is released
void MainWindow::keyReleaseEvent(QKeyEvent* k)
{
  if (k->key() == Qt::Key_Shift)
    shift_pressed = false;
  if (k->key() == Qt::Key_Control)
    control_pressed = false;
}

/// Called when this window is about to close
void MainWindow::closeEvent(QCloseEvent* e)
{
  //  fileQuit();
  doc_options.applyOptions(this);

  aboutToClose = true;
  if (m_project && m_project->hasChanged()) {
    switch (mb_changed->exec()) {
      case QMessageBox::Yes:
        if (!fileSave()) {
          aboutToClose = false;
          e->ignore();
          return;
        }
        break;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
        e->ignore();
        aboutToClose = false;
        return;
        break;
    }
  }
  //  e->accept();
  emit quitWindow(this);
}

/// Called when this window receives the focus
void MainWindow::focusInEvent(QFocusEvent* a_event)
{
  if (a_event->gotFocus() && (a_event->reason() != Qt::FocusReason::PopupFocusReason)) {
    m_actionsManager->updatePaste();
  }
}

/// Called when a drag & drop item enters the window
void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
  QString format;

  format = e->mimeData()->formats().first();

  qDebug() << "Format:" << format;
  if (format != "text/qfsm-objects" && format != "text/uri-list" && !e->mimeData()->hasUrls()) {
    return;
  }

  if (e->source() != this)
    e->acceptProposedAction();
  else
    e->ignore();
}

/// Called when a drag & drop item is dropped
void MainWindow::dropEvent(QDropEvent* e)
{
  QString format, path, data;
  const QMimeData* mm;
  QList<QUrl> urls;
  format = e->mimeData()->formats().first();

  qDebug() << "Format:" << format;

  if (format != "text/qfsm-objects" && format != "text/uri-list" && !e->mimeData()->hasUrls())
    return;

  e->acceptProposedAction();

  mm = e->mimeData();
  if (mm->hasUrls()) {
    path = mm->urls().first().path();
    if (path.right(4) != ".fsm") {
      statusMessage(tr("Invalid file type."));
      return;
    }

    // workaround for windows drag&drop bugs
#ifdef WIN32
    if (path[0] == '/')
      path = path.right(path.length() - 1);
#endif

    fileOpen(path);
  } else {
    if (e->source() == this)
      return;
    if (!m_project)
      return;
    if (!m_project->machine())
      return;

    m_mainView->getDrawArea()->getSelection()->deselectAll(m_project->machine());
    data = QString(mm->data("text/qfsm-objects"));

    if (qfsm::edit::paste(m_project, m_mainView->getDrawArea()->getSelection(), data)) {
      emit objectsPasted();
      m_project->setChanged();
    }

    int count = m_mainView->getDrawArea()->getSelection()->count();
    statusMessage(tr("%n object(s) pasted.", nullptr, count), 2000);

    m_mainView->widget()->repaint();
    updateAll();
  }

  //  data=QString(mm->data("text/qfsm-objects"));
}

/**
 * Sets the current mode and updates the menus.
 */
void MainWindow::setMode(DocumentMode a_mode)
{
  if (m_mode == a_mode) {
    return;
  }
  m_mode = a_mode;
  emit modeChanged(m_mode);
}

/// Repaints the scroll view
void MainWindow::repaintViewport()
{
  m_mainView->widget()->repaint();
}

/// Updates all menus.
void MainWindow::updateMenuBar()
{
  m_actionsManager->update();

  int numstates, numtrans;

  // id_import->setEnabled(true);
  // id_import_graphviz->setEnabled(true);

  if (m_project) {
    // id_save->setEnabled(true);
    // id_saveas->setEnabled(true);
    // id_print->setEnabled(true);
    // id_export->setEnabled(true);
    // id_close->setEnabled(true);
    // id_selectall->setEnabled(true);
    // id_deselectall->setEnabled(true);
    // id_newstate->setEnabled(true);
    // id_newtrans->setEnabled(true);
    if (m_project->machine() && m_project->machine()->getType() == Ascii)
      ;
    // id_export_ragel->setEnabled(true);
    else
      ;
    // id_export_ragel->setEnabled(false);
    if (m_project->machine() && m_project->machine()->getType() == Text) {

      // id_export_ahdl->setEnabled(false);
      // id_export_vhdl->setEnabled(false);
      // id_export_verilog->setEnabled(false);
      // id_export_kiss->setEnabled(false);
      // id_export_vvvv->setEnabled(true);
      // id_export_scxml->setEnabled(true);
      // id_export_smc->setEnabled(true);

      // id_viewstateenc->setEnabled(false);
      // id_viewmoore->setEnabled(false);
      // tbmachinesim->setEnabled(false);
    } else {

      // id_export_ahdl->setEnabled(true);
      // id_export_vhdl->setEnabled(true);
      // id_export_verilog->setEnabled(true);
      // id_export_kiss->setEnabled(true);
      // id_export_vvvv->setEnabled(false);
      // id_export_scxml->setEnabled(false);
      // id_export_smc->setEnabled(false);

      // id_viewstateenc->setEnabled(true);
      // id_viewmoore->setEnabled(true);
      // tbmachinesim->setEnabled(true);
    }
    // id_viewmealyin->setEnabled(true);
    // id_viewmealyout->setEnabled(true);
    // id_viewgrid->setEnabled(true);
    // id_ioview->setEnabled(true);
    // id_viewshadows->setEnabled(true);

    // id_zoom->setEnabled(true);
    // id_zoomin->setEnabled(true);
    // id_zoomout->setEnabled(true);
    // id_zoom100->setEnabled(true);
    // id_select->setEnabled(true);
    // id_pan->setEnabled(true);
    // tbselect->setEnabled(true);
    // tbpan->setEnabled(true);
    // tbzoom->setEnabled(true);
    // tbzoomin->setEnabled(true);
    // tbzoomout->setEnabled(true);

    // id_machineedit->setEnabled(true);
    // id_correctcodes->setEnabled(true);
    // id_machineicheck->setEnabled(true);
    // tbsave->setEnabled(true);
    // tbprint->setEnabled(true);
    // tbstatenew->setEnabled(true);
    // tbtransnew->setEnabled(true);
  } else {
    // id_save->setEnabled(false);
    // id_saveas->setEnabled(false);
    // id_print->setEnabled(false);
    // id_export->setEnabled(false);
    // id_close->setEnabled(false);
    // id_selectall->setEnabled(false);
    // id_deselectall->setEnabled(false);
    // id_newstate->setEnabled(false);
    // id_newtrans->setEnabled(false);
    // id_viewstateenc->setEnabled(false);
    // id_viewmoore->setEnabled(false);
    // id_viewmealyin->setEnabled(false);
    // id_viewmealyout->setEnabled(false);
    // id_viewgrid->setEnabled(false);
    // id_viewshadows->setEnabled(false);
    // id_ioview->setEnabled(false);

    // id_zoom->setEnabled(false);
    // id_zoomin->setEnabled(false);
    // id_zoomout->setEnabled(false);
    // id_zoom100->setEnabled(false);
    // id_select->setEnabled(false);
    // id_pan->setEnabled(false);
    // tbselect->setEnabled(false);
    // tbpan->setEnabled(false);
    // tbzoom->setEnabled(false);
    // tbzoomin->setEnabled(false);
    // tbzoomout->setEnabled(false);

    // id_machineedit->setEnabled(false);
    // id_correctcodes->setEnabled(false);
    // id_machineicheck->setEnabled(false);
    // tbsave->setEnabled(false);
    // tbprint->setEnabled(false);
    // tbstatenew->setEnabled(false);
    // tbtransnew->setEnabled(false);
    // tbmachinesim->setEnabled(false);
  }

  numtrans = m_mainView->getDrawArea()->getSelection()->countTransitions();
  numstates = m_mainView->getDrawArea()->getSelection()->countStates();

  if (m_project && m_project->machine() && m_project->machine()->getType() != Text &&
      m_project->machine()->getNumStates() > 0) {
    // id_machinesim->setEnabled(true);
    // tbmachinesim->setEnabled(true);
  } else {
    // id_machinesim->setEnabled(false);
    // tbmachinesim->setEnabled(false);
  }

  if (numtrans) {
    // id_trans_straight->setEnabled(true);
    // tbtransstraighten->setEnabled(true);
    // id_cedittrans->setEnabled(true);
    // id_ctrans_straight->setEnabled(true);
  } else {
    // id_trans_straight->setEnabled(false);
    // tbtransstraighten->setEnabled(false);
    // id_cedittrans->setEnabled(false);
    // id_ctrans_straight->setEnabled(false);
  }

  if (numstates > 0) {
    // id_setend->setEnabled(true);
    // id_csetend->setEnabled(true);
  } else {
    // id_setend->setEnabled(false);
    // id_csetend->setEnabled(false);
  }
  if (numstates == 1) {
    // id_setinitial->setEnabled(true);
    // id_editstate->setEnabled(true);
  } else {
    // id_setinitial->setEnabled(false);
    // id_editstate->setEnabled(false);
  }

  if (numtrans == 1) {
    // id_edittrans->setEnabled(true);
  } else {
    // id_edittrans->setEnabled(false);
  }

  if (numstates + numtrans > 0) {
    // id_delete->setEnabled(true);
    // id_cut->setEnabled(true);
    // id_copy->setEnabled(true);
    // id_csdelete->setEnabled(true);
    // id_cscut->setEnabled(true);
    // id_cscopy->setEnabled(true);
    // id_ctdelete->setEnabled(true);
    // id_ctcut->setEnabled(true);
    // id_ctcopy->setEnabled(true);
    // tbcut->setEnabled(true);
    // tbcopy->setEnabled(true);
  } else {
    // id_delete->setEnabled(false);
    // id_cut->setEnabled(false);
    // id_copy->setEnabled(false);
    // id_csdelete->setEnabled(false);
    // id_cscut->setEnabled(false);
    // id_cscopy->setEnabled(false);
    // id_ctdelete->setEnabled(false);
    // id_ctcut->setEnabled(false);
    // id_ctcopy->setEnabled(false);
    // tbcut->setEnabled(false);
    // tbcopy->setEnabled(false);
  }

  //  updatePaste();

  // if (doc_options.getViewStateEncoding())
  // id_viewstateenc->setChecked(true);
  // else
  // id_viewstateenc->setChecked(false);

  // if (doc_options.getViewMoore())
  //   id_viewmoore->setChecked(true);
  // else
  //   id_viewmoore->setChecked(false);

  // if (doc_options.getViewMealyIn())
  //   id_viewmealyin->setChecked(true);
  // else
  //   id_viewmealyin->setChecked(false);

  // if (doc_options.getViewMealyOut())
  //   id_viewmealyout->setChecked(true);
  // else
  //   id_viewmealyout->setChecked(false);

  // if (doc_options.getViewGrid())
  //   id_viewgrid->setChecked(true);
  // else
  //   id_viewgrid->setChecked(false);

  // if (doc_options.getViewIOView())
  //   id_ioview->setChecked(true);
  // else
  //   id_ioview->setChecked(false);

  // if (doc_options.getStateShadows())
  //   id_viewshadows->setChecked(true);
  // else
  //   id_viewshadows->setChecked(false);

  if (m_project && !m_project->undoBuffer()->isEmpty()) {
    // id_undo->setEnabled(true);
    // id_csundo->setEnabled(true);
    // id_ctundo->setEnabled(true);
    // tbundo->setEnabled(true);
  } else {
    // id_undo->setEnabled(false);
    // id_csundo->setEnabled(false);
    // id_ctundo->setEnabled(false);
    // tbundo->setEnabled(false);
  }

  if (m_mode == DocumentMode::Simulating) {
    // id_undo->setEnabled(false);
    // id_csundo->setEnabled(false);
    // id_ctundo->setEnabled(false);
    // id_cut->setEnabled(false);
    // id_copy->setEnabled(false);
    // id_paste->setEnabled(false);
    // id_delete->setEnabled(false);
    // id_cscut->setEnabled(false);
    // id_cscopy->setEnabled(false);
    // id_csdelete->setEnabled(false);
    // id_ctcut->setEnabled(false);
    // id_ctcopy->setEnabled(false);
    // id_ctdelete->setEnabled(false);

    // id_select->setEnabled(false);
    // id_pan->setEnabled(false);
    // id_newstate->setEnabled(false);
    // id_newtrans->setEnabled(false);
    // id_zoom->setEnabled(false);

    // id_selectall->setEnabled(false);
    // id_deselectall->setEnabled(false);
    // id_machineedit->setEnabled(false);
    // id_editstate->setEnabled(false);
    // id_setinitial->setEnabled(false);
    // id_ceditstate->setEnabled(false);
    // id_csetinitial->setEnabled(false);
    // id_setend->setEnabled(false);
    // id_edittrans->setEnabled(false);
    // id_trans_straight->setEnabled(false);
    // id_cedittrans->setEnabled(false);
    // id_ctrans_straight->setEnabled(false);

    // tbselect->setEnabled(false);
    // tbpan->setEnabled(false);
    // tbzoom->setEnabled(false);
    // tbundo->setEnabled(false);
    // tbcut->setEnabled(false);
    // tbcopy->setEnabled(false);
    // tbstatenew->setEnabled(false);
    // tbtransnew->setEnabled(false);
    // tbtransstraighten->setEnabled(false);
  }
}

/// Updates the title bar.
void MainWindow::updateTitleBar()
{
  QString title{ u"Qfsm"_qs };
  if (m_project && !fileio->getActFilePath().isEmpty()) {
    title += u" - "_qs + fileio->getActFileName();
    if (m_project->hasChanged()) {
      title += tr(" (modified)");
    }
  }
  setWindowTitle(title);
}

/// Updates the status bar
void MainWindow::updateStatusBar()
{
  // int selected;
  // int scale;

  // if (m_project) {
  //   selected = m_mainView->getDrawArea()->getSelection()->count();
  //   scale = int(m_mainView->getDrawArea()->getScale() * 100 + 0.5);
  // } else {
  //   selected = -1;
  //   scale = -1;
  // }

  // m_statusBar->setSelected(selected);
  // emit updateStatusZoom(scale);
}

/// Updates menu, title bar and status bar
void MainWindow::updateAll()
{
  updateMenuBar();
  updateTitleBar();
  // updateStatusBar();
  if (vvvv_export->isVisible())
    updateVVVV();
}

/// Refreshes the MRU file list
void MainWindow::refreshMRU()
{
  QMenu* recentFilesMenu = m_actionsManager->menu(qfsm::gui::ActionsManager::Group::Recent);
  recentFilesMenu->clear();

  for (const QString& recentFile : m_optionsManager->recentsList()) {
    recentFilesMenu->addAction(recentFile, [&]() { fileOpen(recentFile); });
  }
}

/// Shows the context menu for a state
void MainWindow::showContextState()
{
  // cmenu_state->popup(QCursor::pos());
}

/// Shows the context menu for a transition
void MainWindow::showContextTrans()
{
  // cmenu_trans->popup(QCursor::pos());
}

/// Shows the context menu for the scrollview
void MainWindow::showContext()
{
  // cmenu_sview->popup(QCursor::pos());
}

/// Sends a message @a s to the status bar
void MainWindow::statusMessage(const QString& a_message)
{
  m_statusBar->showMessage(a_message);
}

/// Sends a message @a s for time @a t to the status bar
void MainWindow::statusMessage(const QString& a_message, int a_timeout)
{
  m_statusBar->showMessage(a_message, a_timeout);
}

/// Creates a new file
void MainWindow::fileNew()
{
  int result;
  bool sim = false;

  if (m_mode == DocumentMode::Simulating)
    sim = true;

  if (m_project && m_project->hasChanged()) {
    switch (mb_changed->exec()) {
      case QMessageBox::Yes:
        if (!fileSave())
          return;
        break;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
        return;
        break;
    }
  }
  qfsm::Project* p = new qfsm::Project{ this };

  result = machinemanager->addMachine(p);
  if (result) {
    if (sim)
      simulator->closeDlg();

    if (m_project) {
      delete m_project;
      m_project = NULL;
    }
    m_project = p;
    fileio->setActFilePath({});

    statusMessage(tr("%1 created.").arg(m_project->machine()->getName()), 2000);
  } else {
    p->deleteLater();
    return;
  }

  setMode(DocumentMode::Select);
  m_mainView->getDrawArea()->reset();
  m_mainView->widget()->repaint();

  updateAll();
}

void MainWindow::fileOpen()
{
  QStringList projectFiles = fileio->selectProjectFiles();
  if (projectFiles.isEmpty()) {
    return;
  }
  fileOpen(projectFiles.takeFirst());
  for (const QString& projectFile : projectFiles) {
    m_control->newWindow(projectFile);
  }
}

/// Opens an existing file.
void MainWindow::fileOpen(const QString& a_fileName)
{
  if (m_project) {
    if (m_project->hasChanged()) {
      switch (mb_changed->exec()) {
        case QMessageBox::Yes:
          if (!fileSave())
            return;
          break;
        case QMessageBox::No:
          break;
        case QMessageBox::Cancel:
          return;
          break;
      }
    }

    m_view->scene()->clear();
    m_project->deleteLater();
    m_project = nullptr;
  }

  if (a_fileName.isEmpty()) {
    return;
  }

  qfsm::Project* newProject = fileio->openFileXML(a_fileName);
  if (newProject) {
    m_project = newProject;
    m_project->undoBuffer()->clear();

    statusMessage(tr("File %1 loaded.").arg(fileio->getActFileName()), 2000);

    for (GState* state : m_project->machine()->getSList()) {
      qfsm::gui::StateItem* stateItem = new qfsm::gui::StateItem{ state };
      m_view->scene()->addItem(stateItem);
      for (GTransition* transition : state->tlist) {
        qfsm::gui::TransitionItem* transitionItem = new qfsm::gui::TransitionItem{ transition };
        m_view->scene()->addItem(transitionItem);
      }
    }

    if (m_mode == DocumentMode::Simulating) {
      if (!simulator->startSimulation(m_project->machine()))
        setMode(DocumentMode::Select);
    }

    m_optionsManager->addRecentsEntry(fileio->getActFilePath());
  } else {
    const QString& fileName = a_fileName.isEmpty() ? fileio->getActFilePath() : a_fileName;
    qfsm::gui::msg::info(tr("File `%1` could not be opened").arg(fileName));
    m_statusBar->clearMessage();
    if (!a_fileName.isEmpty()) {
      m_optionsManager->removeRecentsEntry(a_fileName);
    }
  }

  m_actionsManager->update();
  updateTitleBar();
}

// /// Opens a file from the MRU file list with the name @a fileName
// void MainWindow::fileOpenRecent(QString fileName)
// {
//   qfsm::Project* p;

//   if (m_project && m_project->hasChanged()) {
//     switch (mb_changed->exec()) {
//       case QMessageBox::Yes:
//         if (!fileSave())
//           return;
//         break;
//       case QMessageBox::No:
//         break;
//       case QMessageBox::Cancel:
//         return;
//         break;
//     }
//   }

//   p = fileio->openFileXML(fileName);
//   if (p) {
//     if (m_project) {
//       delete m_project;
//       m_project = NULL;
//     }
//     statusMessage(tr("File") + " " + fileio->getActFileName() + " " + tr("loaded."), 2000);
//     m_project = p;
//     p->undoBuffer()->clear();

//     updateAll();
//     m_mainView->updateBackground();
//     m_mainView->getDrawArea()->resetState();
//     // m_mainView->getDrawArea()->updateCanvasSize();
//     m_mainView->updateSize();
//     if (m_mode == DocumentMode::Simulating) {
//       if (!simulator->startSimulation(m_project->machine()))
//         setMode(DocumentMode::Select);
//     } else
//       m_mainView->widget()->repaint();

//     m_optionsManager->addRecentsEntry(fileio->getActFilePath());
//   } else {
//     qfsm::gui::msg::info(tr("File %1 could not be opened").arg(fileName));
//     m_statusBar->clearMessage();
//     m_optionsManager->removeRecentsEntry(fileName);
//   }
// }

/// Saves the current file.
bool MainWindow::fileSave()
{
  bool result = false;

  if (m_project) {
    const bool saveas = fileio->getActFileName().isEmpty();
    // QCursor oldcursor1 = cursor();
    // QCursor oldcursor2 = m_mainView->viewport()->cursor();
    // setCursor(Qt::WaitCursor);
    // m_mainView->viewport()->setCursor(Qt::WaitCursor);

    result = fileio->saveFile(m_project);
    if (result) {
      statusMessage(tr("File %1 saved.").arg(fileio->getActFileName()), 2000);
      m_project->undoBuffer()->clear();
      if (saveas) {
        m_optionsManager->addRecentsEntry(fileio->getActFilePath());
      }
    }

    updateAll();

    // setCursor(oldcursor1);
    // m_mainView->viewport()->setCursor(oldcursor2);
  }

  return result;
}

/// Saves the current file with a new name.
bool MainWindow::fileSaveAs()
{
  if (m_project) {
    bool result;
    QCursor oldcursor1 = cursor();
    QCursor oldcursor2 = m_mainView->viewport()->cursor();
    setCursor(Qt::WaitCursor);
    m_mainView->viewport()->setCursor(Qt::WaitCursor);

    result = fileio->saveFileAs(m_project);

    if (result) {
      statusMessage(tr("File %1 saved.").arg(fileio->getActFileName()), 2000);
      m_project->undoBuffer()->clear();
      m_optionsManager->addRecentsEntry(fileio->getActFilePath());
    }

    updateAll();

    setCursor(oldcursor1);
    m_mainView->viewport()->setCursor(oldcursor2);

    return result;
  }

  return false;
}

/// Imports a graphviz file
void MainWindow::fileImportGraphviz()
{
  qfsm::Project* p;

  if (m_project && m_project->hasChanged()) {
    switch (mb_changed->exec()) {
      case QMessageBox::Yes:
        if (!fileSave())
          return;
        break;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
        return;
        break;
    }
  }

  ImportGraphviz* imp = new ImportGraphviz(&doc_options);
  p = fileio->importFile(imp, m_mainView);
  delete imp;

  if (p) {
    if (m_project) {
      delete m_project;
      m_project = NULL;
    }
    statusMessage(tr("File %1 imported.").arg(fileio->getActFileName()), 2000);
    m_project = p;
    p->undoBuffer()->clear();

    updateAll();
    m_mainView->updateBackground();
    m_mainView->getDrawArea()->resetState();
    // m_mainView->getDrawArea()->updateCanvasSize();
    if (m_mode == DocumentMode::Simulating) {
      if (!simulator->startSimulation(m_project->machine()))
        setMode(DocumentMode::Select);
    } else {
      // m_mainView->widget()->repaint();
      // m_mainView->getDrawArea()->zoomReset();
      m_mainView->updateSize();
    }
  } else if (!fileio->getActImportFilePath().isNull()) {
    qfsm::gui::msg::info(tr("File %1 could not be opened").arg(fileio->getActFileName()));
    m_statusBar->clearMessage();
  }
}

/// Exports the current diagram to an EPS file
bool MainWindow::fileExportEPS()
{
  if (m_project) {
    bool result;

    m_project->machine()->updateDefaultTransitions();

    ExportEPS* exp = new ExportEPS(&doc_options);
    result = fileio->exportFile(m_project, exp, m_mainView);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current diagram to a SVG file
bool MainWindow::fileExportSVG()
{
  if (m_project) {
    bool result;

    m_project->machine()->updateDefaultTransitions();

    ExportSVG* exp = new ExportSVG(&doc_options);
    result = fileio->exportFile(m_project, exp, m_mainView);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current diagram to a PNG file
bool MainWindow::fileExportPNG()
{
  if (m_project) {
    bool result;

    m_project->machine()->updateDefaultTransitions();

    ExportPNG* exp = new ExportPNG(&doc_options);
    result = fileio->exportFile(m_project, exp, m_mainView);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to an AHDL tdf file
bool MainWindow::fileExportAHDL()
{
  if (m_project) {
    bool result;

    switch (ahdl_export->exec()) {
      case QDialog::Accepted:
        doc_options.applyOptions(this);
        break;
      case QDialog::Rejected:
        return true;
        break;
    }

    m_project->machine()->updateDefaultTransitions();
    ExportAHDL* exp = new ExportAHDL(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to a VHDL file
bool MainWindow::fileExportVHDL()
{
  if (m_project) {
    bool result;
    QString errorMessage;
    QStringList invalidNames;

    vhdl_export->init(&doc_options, m_project->machine());
    switch (vhdl_export->exec()) {
      case QDialog::Accepted:
        doc_options.applyOptions(this);
        break;
      case QDialog::Rejected:
        return true;
        break;
    }

    m_project->machine()->updateDefaultTransitions();
    ExportVHDL* exp = new ExportVHDL(&doc_options);

    result = exp->checkMachineNames(m_project->machine(), &doc_options, &invalidNames);

    if (!result) {
      errorMessage = tr("Export of file %1 failed!").arg(fileio->getActExportFileName()) + "\n\n" +
                     tr("The following identifiers do not match the VHDL syntax:") + "\n";

      errorMessage += invalidNames.join("\n");

      qfsm::gui::msg::warn(errorMessage);

      statusMessage(tr("Export of file %1 failed").arg(fileio->getActExportFileName()), 2000);
      delete exp;
      return false;
    }

    if (!m_project || !exp)
      return false;

    if (!exp->validateMachine(m_project->machine()))
      return false;

    QString path_entity = vhdl_export->getEntityPath();
    QString path_arch = vhdl_export->getArchitecturePath();

    QFile ftmp(path_entity);
    if (ftmp.exists()) {
      if (qfsm::gui::msg::warn(tr("File %1 exists. Do you want to overwrite it?").arg(path_entity),
                               qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) != QMessageBox::Ok) {
        delete exp;
        return false;
      }
    }

    std::ofstream fout_entity(path_entity.toStdString());

    if (!fout_entity) {
      qfsm::gui::msg::warn(tr("Unable to write file %1!").arg(path_entity));
      delete exp;
      return false;
    }

    if (doc_options.getVHDLSepFiles()) {
      ftmp.setFileName(path_arch);
      if (ftmp.exists()) {
        if (qfsm::gui::msg::warn(tr("File %1 exists. Do you want to overwrite it?").arg(path_arch),
                                 qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) != QMessageBox::Ok) {
          delete exp;
          return false;
        }
      }

      std::ofstream fout_architecture(path_arch.toStdString());
      if (!fout_architecture) {
        qfsm::gui::msg::warn(tr("Unable to write file %1!").arg(path_arch));
        delete exp;
        return false;
      }
      exp->init(&fout_entity, &fout_architecture, m_project->machine(), path_entity, NULL);
      exp->doExport();
    } else {
      exp->init(&fout_entity, &fout_entity, m_project->machine(), path_entity, NULL);
      exp->doExport();
    }

    //  result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports IO Description
bool MainWindow::fileExportIODescription()
{
  bool result;
  if (m_project) {
    m_project->machine()->updateDefaultTransitions();
    ExportIODescription* exp = new ExportIODescription(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports VHDL Testbench
bool MainWindow::fileExportTestbench()
{
  if (m_project) {
    bool result;
    int dialog_result;
    QString errorMessage;
    QStringList invalidNames;
    QString base_dir_name, testbench_dir_name, testvector_dir_name, package_dir_name, logfile_dir_name;
    QDir testbenchDir, testvectorDir, packageDir, logfileDir;
    std::ofstream *testbench_out, *testvector_out, *package_out;

    testbench_export->init(&doc_options, m_project->machine());

    while (true) {
      dialog_result = testbench_export->exec();

      if (dialog_result == QDialog::Accepted) {
        doc_options.applyOptions(this);

        base_dir_name = doc_options.getTestbenchBaseDirectory();
        testbench_dir_name =
            doc_options.getTestbenchVHDLPath().left(doc_options.getTestbenchVHDLPath().lastIndexOf("/") + 1);
        testvector_dir_name =
            doc_options.getTestvectorASCIIPath().left(doc_options.getTestvectorASCIIPath().lastIndexOf("/") + 1);
        package_dir_name =
            doc_options.getTestpackageVHDLPath().left(doc_options.getTestpackageVHDLPath().lastIndexOf("/") + 1);
        logfile_dir_name =
            doc_options.getTestbenchLogfilePath().left(doc_options.getTestbenchLogfilePath().lastIndexOf("/") + 1);

        testbenchDir.setPath(base_dir_name + testbench_dir_name);
        testvectorDir.setPath(base_dir_name + testvector_dir_name);
        packageDir.setPath(base_dir_name + package_dir_name);
        logfileDir.setPath(base_dir_name + logfile_dir_name);

        if (m_project->machine()->getNumOutputs() > 0) {
          QMessageBox::critical(this, "qfsm",
                                tr("The current version does not create a "
                                   "testbench for a Mealy-type FSM!"),
                                QMessageBox::Ok);
          return false;
        }

        if (!testbenchDir.exists()) {
          dialog_result = QMessageBox::question(
              NULL, "qfsm", tr("%1 does not exist\n\nCreate it?").arg(base_dir_name + testbench_dir_name),
              QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

          if (dialog_result == QMessageBox::No)
            continue;

          result = testbenchDir.mkpath(base_dir_name + testbench_dir_name);

          if (!result) {
            QMessageBox::critical(NULL, "qfsm",
                                  tr("Error creating directory %1!").arg(base_dir_name + testbench_dir_name),
                                  QMessageBox::Ok, QMessageBox::Ok);
            continue;
          }
        }

        if (!testvectorDir.exists()) {
          dialog_result = QMessageBox::question(
              NULL, "qfsm", tr("%1 does not exist\n\nCreate it?").arg(base_dir_name + testvector_dir_name),
              QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

          if (dialog_result == QMessageBox::No)
            continue;

          result = testvectorDir.mkpath(base_dir_name + testvector_dir_name);

          if (!result) {
            QMessageBox::critical(NULL, "qfsm",
                                  tr("Error creating directory %1!").arg(base_dir_name + testvector_dir_name),
                                  QMessageBox::Ok, QMessageBox::Ok);
            continue;
          }
        }

        if (!packageDir.exists()) {
          dialog_result = QMessageBox::question(
              NULL, "qfsm", tr("%1 does not exist\n\nCreate it?").arg(base_dir_name + package_dir_name),
              QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

          if (dialog_result == QMessageBox::No)
            continue;

          result = packageDir.mkpath(base_dir_name + package_dir_name);

          if (!result) {
            QMessageBox::critical(NULL, "qfsm",
                                  tr("Error creating directory %1!").arg(base_dir_name + package_dir_name),
                                  QMessageBox::Ok, QMessageBox::Ok);
            continue;
          }
        }

        if (!logfileDir.exists()) {
          dialog_result = QMessageBox::question(
              NULL, "qfsm", tr("%1 does not exist\n\nCreate it?").arg(base_dir_name + logfile_dir_name),
              QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

          if (dialog_result == QMessageBox::No)
            continue;

          result = logfileDir.mkpath(base_dir_name + logfile_dir_name);

          if (!result) {
            QMessageBox::critical(NULL, "qfsm",
                                  tr("Error creating directory %1!").arg(base_dir_name + logfile_dir_name),
                                  QMessageBox::Ok, QMessageBox::Ok);
            continue;
          }
        }

        QFile ftmp(base_dir_name + doc_options.getTestbenchVHDLPath());
        if (ftmp.exists()) {
          if (qfsm::gui::msg::warn(
                  tr("File %1 exists. Do you want to overwrite it?").arg(doc_options.getTestbenchVHDLPath()),
                  qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) != QMessageBox::Ok)
            return false;
        }
        ftmp.setFileName(base_dir_name + doc_options.getTestvectorASCIIPath());
        if (ftmp.exists()) {
          if (qfsm::gui::msg::warn(
                  tr("File %1 exists. Do you want to overwrite it?").arg(doc_options.getTestvectorASCIIPath()),
                  qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) != QMessageBox::Ok)
            return false;
        }
        ftmp.setFileName(base_dir_name + doc_options.getTestpackageVHDLPath());
        if (ftmp.exists()) {
          if (qfsm::gui::msg::warn(
                  tr("File %1 exists. Do you want to overwrite it?").arg(doc_options.getTestpackageVHDLPath()),
                  qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) != QMessageBox::Ok)
            return false;
        }

        testbench_out = new std::ofstream((base_dir_name + doc_options.getTestbenchVHDLPath()).toLatin1().data());
        testvector_out = new std::ofstream((base_dir_name + doc_options.getTestvectorASCIIPath()).toLatin1().data());
        package_out = new std::ofstream((base_dir_name + doc_options.getTestpackageVHDLPath()).toLatin1().data());

        if (!testbench_out) {
          qfsm::gui::msg::warn(tr("Unable to open file %1!").arg(doc_options.getTestbenchVHDLPath()));
          return false;
        }
        if (!testvector_out) {
          qfsm::gui::msg::warn(tr("Unable to open file %1!").arg(doc_options.getTestvectorASCIIPath()));
          return false;
        }
        if (!package_out) {
          qfsm::gui::msg::warn(tr("Unable to open file %1!").arg(doc_options.getTestpackageVHDLPath()));
          return false;
        }
        break;
      } else
        return true;
    }

    result = ExportVHDL::checkMachineNames(m_project->machine(), &doc_options, &invalidNames);
    if (!result) {
      errorMessage = tr("Export of file %1 failed!").arg(fileio->getActExportFileName()) + "\n\n" +
                     tr("The following identifiers do not match the VHDL syntax:") + "\n";
      errorMessage += invalidNames.join("\n");
      qfsm::gui::msg::warn(errorMessage);

      statusMessage(tr("Export of file %1 failed.").arg(fileio->getActExportFileName()), 2000);
      delete testvector_out;
      delete testbench_out;
      delete package_out;
      return false;
    }

    m_project->machine()->updateDefaultTransitions();

    ExportTestbenchVHDL* exportTestbench = new ExportTestbenchVHDL(&doc_options);
    ExportTestvectorASCII* exportTestvector = new ExportTestvectorASCII(&doc_options);

    exportTestbench->init(testbench_out, package_out, m_project->machine(), doc_options.getTestbenchVHDLPath(), NULL);
    exportTestvector->init(testvector_out, m_project->machine(), doc_options.getTestvectorASCIIPath(), NULL);

    exportTestbench->doExport();
    exportTestvector->doExport();

    delete exportTestbench;
    delete exportTestvector;
    delete testvector_out;
    delete testbench_out;
    delete package_out;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to a Verilog HDL file
bool MainWindow::fileExportVerilog()
{
  if (m_project) {
    bool result;

    switch (ver_export->exec()) {
      case QDialog::Accepted:
        doc_options.applyOptions(this);
        break;
      case QDialog::Rejected:
        return true;
        break;
    }

    m_project->machine()->updateDefaultTransitions();
    ExportVerilog* exp = new ExportVerilog(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to a KISS file
bool MainWindow::fileExportKISS()
{
  if (m_project) {
    bool result;

    m_project->machine()->updateDefaultTransitions();
    ExportKISS* exp = new ExportKISS(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to the 'vvvv Automata code' window
bool MainWindow::fileExportVVVV()
{
  if (m_project) {
    bool result = true;

    m_project->machine()->updateDefaultTransitions();
    updateVVVV();
    vvvv_export->show();
    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to a SCXML file
bool MainWindow::fileExportSCXML()
{
  if (m_project) {
    bool result;

    m_project->machine()->updateDefaultTransitions();
    ExportSCXML* exp = new ExportSCXML(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

void MainWindow::updateVVVV()
{
  doc_options.applyOptions(this);

  ExportVVVV* exp = new ExportVVVV(&doc_options);
  // string exp_output;
  std::ostringstream ostr;
  // result = fileio->exportFile(m_project, exp);
  exp->init(&ostr, m_project->machine());
  exp->doExport();
  vvvv_export->setText(ostr.str().c_str());
  // qDebug("%s" ,str.str().c_str());
  delete exp;
}

/// Exports the current file to an ASCII state table
bool MainWindow::fileExportSTASCII()
{
  if (m_project) {
    bool result;

    switch (statetable_export->exec()) {
      case QDialog::Accepted:
        // opt_display->init(&doc_options);
        doc_options.applyOptions(this);
        break;
      case QDialog::Rejected:
        return true;
        break;
    }
    QCursor oldcursor1 = cursor();
    QCursor oldcursor2 = m_mainView->viewport()->cursor();
    setCursor(Qt::WaitCursor);
    m_mainView->viewport()->setCursor(Qt::WaitCursor);

    TableBuilderASCII* tb = new TableBuilderASCII(this, m_project->machine(), &doc_options);
    ExportStateTable* exp = new ExportStateTable(&doc_options, tb);
    result = fileio->exportFile(m_project, exp);
    delete exp;
    delete tb;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    setCursor(oldcursor1);
    m_mainView->viewport()->setCursor(oldcursor2);

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to a Latex state table
bool MainWindow::fileExportSTLatex()
{
  if (m_project) {
    bool result;

    switch (statetable_export->exec()) {
      case QDialog::Accepted:
        // opt_display->init(&doc_options);
        doc_options.applyOptions(this);
        break;
      case QDialog::Rejected:
        return true;
        break;
    }
    QCursor oldcursor1 = cursor();
    QCursor oldcursor2 = m_mainView->viewport()->cursor();
    setCursor(Qt::WaitCursor);
    m_mainView->viewport()->setCursor(Qt::WaitCursor);

    TableBuilderLatex* tb = new TableBuilderLatex(this, m_project->machine(), &doc_options);
    ExportStateTable* exp = new ExportStateTable(&doc_options, tb);
    result = fileio->exportFile(m_project, exp);
    delete exp;
    delete tb;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    setCursor(oldcursor1);
    m_mainView->viewport()->setCursor(oldcursor2);

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to a HTML state table
bool MainWindow::fileExportSTHTML()
{
  if (m_project) {
    bool result;

    switch (statetable_export->exec()) {
      case QDialog::Accepted:
        doc_options.applyOptions(this);
        break;
      case QDialog::Rejected:
        return true;
        break;
    }

    QCursor oldcursor1 = cursor();
    QCursor oldcursor2 = m_mainView->viewport()->cursor();
    setCursor(Qt::WaitCursor);
    m_mainView->viewport()->setCursor(Qt::WaitCursor);

    TableBuilderHTML* tb = new TableBuilderHTML(this, m_project->machine(), &doc_options);
    ExportStateTable* exp = new ExportStateTable(&doc_options, tb);
    result = fileio->exportFile(m_project, exp);
    delete exp;
    delete tb;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    setCursor(oldcursor1);
    m_mainView->viewport()->setCursor(oldcursor2);

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to a Ragel file
bool MainWindow::fileExportRagel()
{
  if (m_project) {
    bool result;

    switch (ragel_export->exec()) {
      case QDialog::Accepted:
        doc_options.applyOptions(this);
        break;
      case QDialog::Rejected:
        return true;
        break;
    }

    m_project->machine()->updateDefaultTransitions();
    ExportRagel* exp = new ExportRagel(&doc_options);
    result = fileio->exportFile(m_project, exp);

    if (!result)
      return false;

    QFileInfo fi(fileio->getActExportFileName());
    bool create_action_file = doc_options.getRagelCreateAction();
    QString act_file = fileio->getExportDir() + "/" + fi.baseName() + "_actions.rl";
    if (create_action_file) {
      QFile ftmp(act_file);
      if (ftmp.exists()) {
        if (qfsm::gui::msg::warn(tr("File %1 exists. Do you want to overwrite it?").arg(act_file),
                                 qfsm::gui::msg::Button::Ok | qfsm::gui::msg::Button::Cancel) != QMessageBox::Ok)
          create_action_file = false;
      }
    }

    if (create_action_file)
      exp->writeActionFile(act_file.toStdString().c_str(), fileio->getActExportFileName().toStdString().c_str());

    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Exports the current file to a SCXML file
bool MainWindow::fileExportSMC()
{
  if (m_project) {
    bool result;

    m_project->machine()->updateDefaultTransitions();
    ExportSMC* exp = new ExportSMC(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result) {
      statusMessage(tr("File %1 exported.").arg(fileio->getActExportFileName()), 2000);
    }

    updateAll();
    return result;
  }
  return false;
}

/// Prints the current file.
void MainWindow::filePrint()
{
  QCursor oldcursor1 = cursor();
  QCursor oldcursor2 = m_mainView->viewport()->cursor();
  setCursor(Qt::WaitCursor);
  m_mainView->viewport()->setCursor(Qt::WaitCursor);

  printmanager->print();

  setCursor(oldcursor1);
  m_mainView->viewport()->setCursor(oldcursor2);
}

/// Called when 'File->Quit' is clicked
void MainWindow::fileQuit()
{
  close();
}

/// Closes the current file.
bool MainWindow::fileClose()
{
  if (m_project) {
    if (m_project->hasChanged()) {
      switch (mb_changed->exec()) {
        case QMessageBox::Yes:
          if (!fileSave())
            return false;
          break;
        case QMessageBox::No:
          break;
        case QMessageBox::Cancel:
          return false;
          break;
      }
    }

    simulator->closeDlg();
    m_view->scene()->clear();
    m_project->deleteLater();
    m_project = nullptr;

    statusMessage(tr("File %1 closed.").arg(fileio->getActFileName()), 2000);

    setMode(DocumentMode::Select);

    return true;
  }
  return false;
}

/// Undo the last action.
void MainWindow::editUndo()
{
  if (m_project) {
    m_project->undoBuffer()->undo();
    m_view->scene()->update();
    m_actionsManager->update();
  }
}

/// Cuts the selected objects from the current machine and puts it on the
/// clipboard
void MainWindow::editCut()
{
  const int selectionCount = m_mainView->getDrawArea()->getSelection()->count();

  m_isCutOperation = true;
  editCopy();
  editDelete();
  m_isCutOperation = false;

  statusMessage(tr("%n object(s) cut.", nullptr, selectionCount), 2000);

  m_actionsManager->updatePaste();
}

/// Copies the selected objects to the clipboard
void MainWindow::editCopy()
{
  static const QString qfsmDataFormat{ QFSM_MIME_DATA_TYPE };

  const QString data = qfsm::edit::copy(m_project);
  if (data.isEmpty()) {
    return;
  }

  QClipboard* clipboard = QApplication::clipboard();

  QMimeData* mimeData = new QMimeData{};
  mimeData->setData(qfsmDataFormat, data.toLocal8Bit());
  mimeData->setText(data);

  clipboard->setMimeData(mimeData);

  if (!m_isCutOperation) {
    const int selectionCount = m_mainView->getDrawArea()->getSelection()->count();
    statusMessage(tr("%n object(s) copied.", nullptr, selectionCount), 2000);
    m_actionsManager->updatePaste();
  }
}

/// Pastes the objects on the clipboard into the current machine
void MainWindow::editPaste()
{
  static const QString qfsmDataFormat{ QFSM_MIME_DATA_TYPE };

  if (m_project == nullptr) {
    return;
  }

  QClipboard* clipboard = QApplication::clipboard();

  const QMimeData* mimeData = clipboard->mimeData();
  if (mimeData == nullptr || !mimeData->hasFormat(qfsmDataFormat)) {
    return;
  }

  const QString data{ mimeData->data(qfsmDataFormat) };
  if (data.isEmpty()) {
    return;
  }

  Selection* selection = m_mainView->getDrawArea()->getSelection();
  selection->deselectAll(m_project->machine());

  if (qfsm::edit::paste(m_project, selection, data)) {
    emit objectsPasted();
    m_project->setChanged();
  }

  const int selectionCount = selection->count();
  statusMessage(tr("%n object(s) pasted.", nullptr, selectionCount), 2000);
  m_mainView->widget()->repaint();
  updateAll();
}

/// Delete the selected objects.
void MainWindow::editDelete()
{
  Selection* selection = m_mainView->getDrawArea()->getSelection();
  const int selectionCount = selection ? selection->count() : 0;

  const bool result = qfsm::edit::deleteSelection(m_project, selection);
  if (!result) {
    return;
  }

  if (!m_isCutOperation) {
    statusMessage(tr("%n object(s) deleted.", nullptr, selectionCount), 2000);
  }

  m_project->setChanged();
  updateAll();
  m_mainView->widget()->repaint();
}

/// Set select mode.
void MainWindow::editSelect()
{
  setMode(DocumentMode::Select);
}

/// Called when 'Edit->Select all' is clicked
void MainWindow::editSelectAll()
{
  m_view->scene()->selectAll();
  m_actionsManager->update();
}

/// Called when 'Edit->Deselect all' is clicked
void MainWindow::editDeselectAll()
{
  m_view->scene()->clearSelection();
  m_actionsManager->update();
}

/// Edit options.
void MainWindow::editOptions()
{
  if (tabwidgetdialog->exec()) {
    doc_options.applyOptions(this);
    fileio->saveOptions(&doc_options);
    updateAll();
    m_mainView->widget()->repaint();
  }
}

/// Initiate a drag operation and process the drop result
bool MainWindow::runDragOperation(bool a_forceCopy)
{
  static const QString qfsmDataFormat{ QFSM_MIME_DATA_TYPE };
  const QString data = qfsm::edit::copy(m_project);
  bool result = false;

  if (!data.isEmpty()) {
    QMimeData* mimeData = new QMimeData{};
    mimeData->setData(qfsmDataFormat, data.toLocal8Bit());
    mimeData->setText(data);

    QDrag* drag = new QDrag{ this };
    drag->setMimeData(mimeData);

    const Qt::DropAction dropAction = drag->exec(Qt::MoveAction | Qt::CopyAction);

    switch (dropAction) {
      case Qt::IgnoreAction: {
        qDebug("Drag action ignored");
        break;
      }
      case Qt::CopyAction:
        [[fallthrough]];
      case Qt::LinkAction: {
        qDebug("Drag action finished");
        result = true;
        break;
      }
      case Qt::MoveAction:
        [[fallthrough]];
      case Qt::TargetMoveAction: {
        qDebug("Drag action finished, deleting data");
        editDelete();
        result = true;
        break;
      }
    }
  }
  return result;
}

/// Toggle view state encoding
void MainWindow::viewStateEncoding()
{
  m_optionsManager->toggleValue(qfsm::option::Group::View, qfsm::option::stateEncoding);

  // doc_options.setViewStateEncoding(!doc_options.getViewStateEncoding());
  // updateAll();
  // m_mainView->widget()->repaint();
}

/// Toggle view moore outputs.
void MainWindow::viewMooreOutputs()
{
  m_optionsManager->toggleValue(qfsm::option::Group::View, qfsm::option::mooreOutputs);

  // doc_options.setViewMoore(!doc_options.getViewMoore());
  // updateAll();
  // m_mainView->widget()->repaint();
}

/// Toggle view mealy outputs.
void MainWindow::viewMealyOutputs()
{
  m_optionsManager->toggleValue(qfsm::option::Group::View, qfsm::option::mealyOutputs);
  // doc_options.setViewMealyOut(!doc_options.getViewMealyOut());
  // updateAll();
  // m_mainView->widget()->repaint();
}

/// Toggle view mealy inputs.
void MainWindow::viewMealyInputs()
{
  m_optionsManager->toggleValue(qfsm::option::Group::View, qfsm::option::mealyInputs);
  // doc_options.setViewMealyIn(!doc_options.getViewMealyIn());
  // updateAll();
  // m_mainView->widget()->repaint();
}

/// Toggle view grid.
void MainWindow::viewGrid()
{
  const bool value = m_optionsManager->toggleValue(qfsm::option::Group::View, qfsm::option::grid);

  statusMessage(value ? tr("Grid is on.") : tr("Grid is off."), 2000);

  m_view->scene()->update();

  // updateAll();
  // m_mainView->widget()->repaint();
}

/// Toggle view shadows.
void MainWindow::viewShadows()
{
  m_optionsManager->toggleValue(qfsm::option::Group::View, qfsm::option::shadows);
  // doc_options.setStateShadows(!doc_options.getStateShadows());
  // updateAll();
  // m_mainView->widget()->repaint();
}

/// Set panning mode
void MainWindow::viewPan()
{
  //  QDragEnterEvent*ev=new QDragEnterEvent(QPoint(0,0),Qt::CopyAction,new
  //  QMimeData(),Qt::LeftButton,Qt::NoModifier); QPaintEvent*ev=new
  //  QPaintEvent(QRect(0,0,100,100)); QApplication::postEvent(m_mainView,ev);
  // m_mainView->widget()->repaint();

  setMode(DocumentMode::Pan);
}

/// Set zooming mode
void MainWindow::viewZoom()
{
  setMode(DocumentMode::Zooming);
}

/// Zoom in the view.
void MainWindow::viewZoomIn()
{
  //  QPoint middle(m_mainView->visibleWidth()/2, m_mainView->visibleHeight()/2);
  //  QPoint offset(m_mainView->contentsX(), m_mainView->contentsY());
  QPoint middle(m_mainView->width() / 2, m_mainView->height() / 2);
  QPoint offset = m_mainView->getDrawArea()->mapTo(m_mainView, QPoint(0, 0));
  middle -= offset;

  m_mainView->getDrawArea()->zoomIn(middle); // zoom->zoom(m_mainView, p, true);
}

/// Zoom out the view.
void MainWindow::viewZoomOut()
{
  //  QPoint middle(m_mainView->visibleWidth()/2, m_mainView->visibleHeight()/2);
  //  QPoint offset(m_mainView->contentsX(), m_mainView->contentsY());
  QPoint middle(m_mainView->width() / 2, m_mainView->height() / 2);
  QPoint offset = m_mainView->getDrawArea()->mapTo(m_mainView, QPoint(0, 0));
  middle -= offset;

  m_mainView->getDrawArea()->zoomOut(middle); // zoom->zoom(m_mainView, p, false);
}

/// Set zoom to 100%
void MainWindow::viewZoom100()
{
  m_mainView->getDrawArea()->zoomReset();
}

/// Toggle IO view
void MainWindow::viewIOView()
{
  const bool value = m_optionsManager->toggleValue(qfsm::option::Group::View, qfsm::option::ioView);
  if (value && m_project) {
    view_io->show();
  } else {
    view_io->hide();
  }
  // doc_options.setViewIOView(!doc_options.getViewIOView());

  // if (doc_options.getViewIOView() && m_project)
  //   view_io->show();
  // else
  //   view_io->hide();

  // updateMenuBar();
}

/// Update IOView text
void MainWindow::updateIOView(Machine* a_machine)
{
  if (a_machine) {
    view_io->updateIOList(a_machine);
    if (m_optionsManager->value<bool>(qfsm::option::Group::View, qfsm::option::ioView)) {
      view_io->show();
    } else {
      view_io->hide();
    }
  }
  // if (m != NULL) {
  //   view_io->updateIOList(m);

  //   if (doc_options.getViewIOView())
  //     view_io->show();
  //   else
  //     view_io->hide();

  //   updateMenuBar();
  // }
}

/// Edit the current machine.
void MainWindow::machineEdit()
{
  if (m_project && m_project->isValid()) {
    machinemanager->editMachine(m_project);
    updateAll();
    m_mainView->widget()->repaint();
  }
}

/// Automatically correct the state codes of the machine.
void MainWindow::machineCorrectCodes()
{
  if (m_project && m_project->isValid()) {
    m_project->machine()->correctCodes();
    updateAll();
    m_mainView->widget()->repaint();
  }
}

/// Simulate the current machine.
void MainWindow::machineSimulate()
{
  if (m_mode != DocumentMode::Simulating) {
    if (simulator->startSimulation(m_project->machine())) {
      setMode(DocumentMode::Simulating);
      m_mainView->widget()->repaint();
    }
  } else {
    simulator->stopSimulation();
    setMode(DocumentMode::Select);
  }
  updateAll();
}

/// Called when 'Machine->Check Integrity' is clicked
void MainWindow::machineICheck()
{
  if (m_project && m_project->isValid()) {
    const QCursor oldCursorWindow = cursor();
    const QCursor oldCursorViewport = m_mainView->viewport()->cursor();

    setCursor(Qt::WaitCursor);
    m_mainView->viewport()->setCursor(Qt::WaitCursor);

    statusMessage(tr("Checking machine..."));
    m_project->machine()->checkIntegrity(ichecker);
    statusMessage(tr("Check finished."), 2000);

    setCursor(oldCursorWindow);
    m_mainView->viewport()->setCursor(oldCursorViewport);
  }
}

/// Add new state to current machine.
void MainWindow::stateNew()
{
  setMode(DocumentMode::NewState);
}

/// Edit selected state.
void MainWindow::stateEdit()
{
  if (m_project == nullptr) {
    return;
  }

  GState* state = static_cast<GState*>(m_mainView->getDrawArea()->getContextObject());
  if (state == nullptr) {
    state = m_mainView->getDrawArea()->getSelection()->getSList().front();
  }

  if (state) {
    m_stateManager->editState(state);
  }
}

/// Set selected state as initial state.
void MainWindow::stateSetInitial()
{
  if (!m_project || !m_project->isValid()) {
    return;
  }

  GState* state = static_cast<GState*>(m_mainView->getDrawArea()->getContextObject());
  if (state == nullptr) {
    state = m_mainView->getDrawArea()->getSelection()->getSList().front();
  }

  if (state) {
    m_stateManager->setInitialState(m_project->machine(), state);
    updateAll();
    m_mainView->widget()->repaint();
  }
}

/// Set selected state as final state.
void MainWindow::stateSetFinal()
{
  if (!m_project || !m_project->isValid()) {
    return;
  }

  m_stateManager->setFinalStates(m_project->machine(), m_mainView->getDrawArea()->getSelection()->getSList());

  updateAll();
  m_mainView->widget()->repaint();
}

/// Add new transition.
void MainWindow::transNew()
{
  setMode(DocumentMode::NewTransition);
}

/// Edit selected transition.
void MainWindow::transEdit()
{
  if (!m_project || !m_project->isValid()) {
    return;
  }

  GTransition* transition = static_cast<GTransition*>(m_mainView->getDrawArea()->getContextObject());
  if (transition == nullptr) {
    transition = m_mainView->getDrawArea()->getSelection()->getTList().front();
  }

  if (transition) {
    m_transitionManager->editTransition(m_project->machine(), transition);
  }
}

/// Straighten selected transitions.
void MainWindow::transStraighten()
{
  GTransition* t;
  //  int otype;
  t = NULL; //(GTransition*)m_mainView->getContextObject(otype);
  if (t) {
    m_project->undoBuffer()->changeTransition(t);
    t->straighten();
  } else
    m_transitionManager->straightenSelection(&m_mainView->getDrawArea()->getSelection()->getTList());

  m_project->setChanged();
  updateAll();
  m_mainView->widget()->repaint();
}

void MainWindow::helpManual()
{
  QDir dir;
  QString qfsmpath;

#ifdef Q_OS_WIN
  QString tmppath;
  QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Qfsm", QSettings::NativeFormat);
  tmppath = settings.value("Install_Dir", QVariant("-1")).toString();
  // qDebug("tmppath: %s", tmppath.latin1());
  if (tmppath == "-1")
    dir = QDir::current();
  else
    dir.cd(tmppath);
  dir.cd(QFSM_HELP_DIR);
  qfsmpath = dir.absolutePath();
#else
  qfsmpath = QFSM_HELP_DIR;
  dir = QDir(qfsmpath);
#endif
  QFileInfo fi(dir, "qfsm.html");
  QString helpfile = fi.absoluteFilePath();
  QDesktopServices::openUrl(QUrl::fromLocalFile(helpfile));
}

/// Displays a help dialog about qfsm.
void MainWindow::helpAbout()
{
  qfsm::info::about(this);
}

/// Displays a help dialog about Qt.
void MainWindow::helpAboutQt()
{
  QMessageBox::aboutQt(this, "qfsm");
}

/// Sets the wait cursor for the main window and the scrollview
void MainWindow::setWaitCursor()
{
  previous_viewcursor = m_mainView->viewport()->cursor();
  setCursor(Qt::WaitCursor);
  m_mainView->viewport()->setCursor(Qt::WaitCursor);
}

/// Restores the previous cursor for the main window and the scrollview
void MainWindow::setPreviousCursor()
{
  setCursor(Qt::ArrowCursor);
  m_mainView->viewport()->setCursor(previous_viewcursor);
}
