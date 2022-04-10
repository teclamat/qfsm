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

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <QPixmap>
#include <QUrl>
#include <fstream>
// #include <q3textstream.h>
// #include <q3toolbar.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qobject.h>
#include <qsettings.h>
#include <qtoolbutton.h>
#include <sstream>
#include <string>

#include "AppInfo.h"
#include "DocStatus.h"
#include "DrawArea.h"
#include "Error.h"
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
#include "ImportGraphviz.h"
#include "MainControl.h"
#include "MainWindow.h"
#include "MimeMachine.h"
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

#include "../pics/c_mag.xpm"
#include "../pics/editcopy.xpm"
#include "../pics/editcopyoff.xpm"
#include "../pics/editcut.xpm"
#include "../pics/editcutoff.xpm"
#include "../pics/editpaste.xpm"
#include "../pics/editpasteoff.xpm"
#include "../pics/editundo.xpm"
#include "../pics/editundooff.xpm"
#include "../pics/filenew.xpm"
#include "../pics/fileopen.xpm"
#include "../pics/fileprint.xpm"
#include "../pics/fileprintoff.xpm"
#include "../pics/filesave.xpm"
#include "../pics/filesaveoff.xpm"
#include "../pics/machinesim.xpm"
#include "../pics/machinesimoff.xpm"
#include "../pics/pan.xpm"
#include "../pics/panoff.xpm"
#include "../pics/qfsm_64.xpm"
#include "../pics/select.xpm"
#include "../pics/selectoff.xpm"
#include "../pics/statenew.xpm"
#include "../pics/statenewoff.xpm"
#include "../pics/transnew.xpm"
#include "../pics/transnewoff.xpm"
#include "../pics/transstraighten.xpm"
#include "../pics/transstraightenoff.xpm"
#include "../pics/zoom.xpm"
#include "../pics/zoomin.xpm"
#include "../pics/zoomoff.xpm"
#include "../pics/zoomout.xpm"

#include "IOInfo.h"

// using namespace std;

/**
 * Constructor.
 * Initialises the mainwindow with all its menus.
 */
MainWindow::MainWindow(QObject* a_parent)
  : QMainWindow{}
  , m_control{ qobject_cast<qfsm::MainControl*>(a_parent) }
  , m_mainView{ new ScrollView(this) }
  , m_menuBar{ menuBar() }
  , m_project{ nullptr }
{
  setCentralWidget(m_mainView);
  setAcceptDrops(true);

  QPixmap paicon((const char**)qfsm_64_xpm);
  setWindowIcon(paicon);

  createToolBar();

  // m_menuBar = menuBar();
  // m_menuBar=new QMenuBar(this);
  // menu_mru = new QMenu(this);

  menu_import = new QMenu(this);
#ifdef GRAPHVIZ_FOUND
  id_import_graphviz = menu_import->insertItem(tr("&Graphviz..."), this, SLOT(fileImportGraphviz()));
#else
  menu_import->setEnabled(false);
#endif

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

  // File -> Export
  menu_export = new QMenu(this);
  // menu_export->setMouseTracking(true);
  menu_export->addAction(tr("E&PS..."), this, &MainWindow::fileExportEPS);
  menu_export->addAction(tr("&SVG..."), this, &MainWindow::fileExportSVG);
  menu_export->addAction(tr("&PNG..."), this, &MainWindow::fileExportPNG);
  menu_export->addSeparator();
  id_export_ahdl = menu_export->addAction(tr("&AHDL..."), this, &MainWindow::fileExportAHDL);
  id_export_vhdl = menu_export->addAction(tr("&VHDL..."), this, &MainWindow::fileExportVHDL);
  id_export_verilog = menu_export->addAction(tr("V&erilog HDL..."), this, &MainWindow::fileExportVerilog);
  id_export_kiss = menu_export->addAction(tr("&KISS"), this, &MainWindow::fileExportKISS);
  menu_export->addSeparator();
  menu_export->addAction(tr("VHDL &Testbench"), this, &MainWindow::fileExportTestbench);
  menu_export->addAction(tr("I/O &Description"), this, &MainWindow::fileExportIODescription);
  menu_export->addSeparator();
  id_export_scxml = menu_export->addAction(tr("SC&XML"), this, &MainWindow::fileExportSCXML);
  id_export_vvvv = menu_export->addAction(tr("vvvv A&utomata code"), this, &MainWindow::fileExportVVVV);
  menu_export->addSeparator();
  menu_export->addAction(tr("State Table (ASC&II)..."), this, &MainWindow::fileExportSTASCII);
  menu_export->addAction(tr("State Table (&Latex)..."), this, &MainWindow::fileExportSTLatex);
  menu_export->addAction(tr("State Table (&HTML)..."), this, &MainWindow::fileExportSTHTML);
  menu_export->addSeparator();
  id_export_ragel = menu_export->addAction(tr("&Ragel..."), this, &MainWindow::fileExportRagel);
  id_export_smc = menu_export->addAction(tr("SM&C..."), this, &MainWindow::fileExportSMC);

  // File
  menu_file = new QMenu(this);
  // menu_file->setMouseTracking(true);
  menu_file->addAction(*pnew, tr("&New..."), this, &MainWindow::fileNew, Qt::CTRL | Qt::Key_N);
  id_open = menu_file->addAction(*popen, tr("&Open..."), this, &MainWindow::fileOpen, Qt::CTRL | Qt::Key_O);
  menu_mru = menu_file->addMenu(tr("Open &Recent"));
  menu_file->addSeparator();
  id_save = menu_file->addAction(*saveset, tr("&Save"), this, &MainWindow::fileSave, Qt::CTRL | Qt::Key_S);
  id_saveas = menu_file->addAction(tr("Save &As..."), this, &MainWindow::fileSaveAs);
  menu_file->addSeparator();
  id_import = menu_file->addMenu(menu_import);
  id_import->setText(tr("&Import"));
  id_export = menu_file->addMenu(menu_export);
  id_export->setText(tr("&Export"));
  menu_file->addSeparator();
  id_print = menu_file->addAction(*printset, tr("&Print..."), this, &MainWindow::filePrint, Qt::CTRL | Qt::Key_P);
  menu_file->addSeparator();
  menu_file->addAction(tr("New &Window"), m_control, qOverload<>(&qfsm::MainControl::newWindow));
  menu_file->addSeparator();
  id_close = menu_file->addAction(tr("&Close"), this, &MainWindow::fileClose, Qt::CTRL | Qt::Key_W);
  menu_file->addAction(tr("&Quit"), this, &MainWindow::fileQuit, Qt::CTRL | Qt::Key_Q);

  // Edit
  menu_edit = new QMenu(this);
  // menu_edit->setCheckable(true);
  menu_edit->setMouseTracking(true);
  id_undo = menu_edit->addAction(*undoset, tr("U&ndo"), this, &MainWindow::editUndo, Qt::CTRL | Qt::Key_Z);
  menu_edit->addSeparator();
  id_cut = menu_edit->addAction(*cutset, tr("C&ut"), this, &MainWindow::editCut, Qt::CTRL | Qt::Key_X);
  id_copy = menu_edit->addAction(*copyset, tr("&Copy"), this, &MainWindow::editCopy, Qt::CTRL | Qt::Key_C);
  id_paste = menu_edit->addAction(*pasteset, tr("&Paste"), this, &MainWindow::editPaste, Qt::CTRL | Qt::Key_V);
  id_delete = menu_edit->addAction(tr("De&lete"), this, &MainWindow::editDelete, Qt::Key_Delete);
  menu_edit->addSeparator();
  id_select =
      menu_edit->addAction(*selset, tr("&Select"), this, &MainWindow::editSelect, Qt::CTRL | Qt::SHIFT | Qt::Key_S);
  id_selectall = menu_edit->addAction(tr("Select &All"), this, &MainWindow::editSelectAll, Qt::CTRL | Qt::Key_A);
  id_deselectall = menu_edit->addAction(tr("&Deselect All"), this, &MainWindow::editDeselectAll, Qt::CTRL | Qt::Key_D);
  menu_edit->addSeparator();
  menu_edit->addAction(tr("&Options"), this, SLOT(editOptions()));

  // View
  menu_view = new QMenu(this);
  // menu_view->setCheckable(true);
  menu_view->setMouseTracking(true);
  id_viewstateenc = menu_view->addAction(tr("State &Codes"), this, SLOT(viewStateEncoding()));
  id_viewmoore = menu_view->addAction(tr("Moo&re Outputs"), this, SLOT(viewMooreOutputs()), Qt::CTRL | Qt::Key_M);
  id_viewmealyin = menu_view->addAction(tr("Mealy I&nputs"), this, SLOT(viewMealyInputs()));
  id_viewmealyout = menu_view->addAction(tr("Mea&ly Outputs"), this, SLOT(viewMealyOutputs()));
  menu_view->addSeparator();
  id_viewshadows = menu_view->addAction(tr("&Shadows"), this, SLOT(viewShadows()));
  id_viewgrid = menu_view->addAction(tr("&Grid"), this, SLOT(viewGrid()));
  menu_view->addSeparator();
  id_ioview = menu_view->addAction(tr("&IO View"), this, SLOT(viewIOView()));
  menu_view->addSeparator();
  id_pan = menu_view->addAction(*panset, tr("&Pan view"), this, SLOT(viewPan()), Qt::CTRL | Qt::SHIFT| Qt::Key_P );
  id_zoom = menu_view->addAction(*zoomset, tr("&Zoom"), this, SLOT(viewZoom()), Qt::CTRL| Qt::SHIFT| Qt::Key_Z);
  id_zoomin = menu_view->addAction(*pzoomin, tr("Zoom &In"), this, SLOT(viewZoomIn()), Qt::CTRL | Qt::Key_I);
  id_zoomout = menu_view->addAction(*pzoomout, tr("Zoom &Out"), this, SLOT(viewZoomOut()), Qt::CTRL | Qt::Key_U);
  id_zoom100 = menu_view->addAction(tr("Zoom &100%"), this, SLOT(viewZoom100()), Qt::CTRL | Qt::Key_R);

  // Machine
  menu_machine = new QMenu(this);
  menu_machine->setMouseTracking(true);
  id_machineedit = menu_machine->addAction(tr("&Edit..."), this, SLOT(machineEdit()));
  id_correctcodes = menu_machine->addAction(tr("&Auto correct State Codes..."), this, SLOT(machineCorrectCodes()));
  id_machinesim = menu_machine->addAction(*machinesimset, tr("&Simulate..."), this, SLOT(machineSimulate()),
                                          Qt::CTRL | Qt::SHIFT| Qt::Key_I);
  id_machineicheck = menu_machine->addAction(tr("&Integrity Check"), this, SLOT(machineICheck()));

  // State
  menu_state = new QMenu(this);
  // menu_state->setCheckable(true);
  menu_state->setMouseTracking(true);
  id_newstate =
      menu_state->addAction(*statenewset, tr("&New"), this, SLOT(stateNew()), Qt::CTRL| Qt::SHIFT| Qt::Key_N );
  id_editstate = menu_state->addAction(tr("&Edit..."), this, SLOT(stateEdit()));
  id_setinitial = menu_state->addAction(tr("Set &Initial State"), this, SLOT(stateSetInitial()));
  id_setend = menu_state->addAction(tr("&Toggle Final State"), this, SLOT(stateSetFinal()), Qt::CTRL | Qt::Key_E);

  // Transition
  menu_trans = new QMenu(this);
  menu_trans->setMouseTracking(true);
  id_newtrans =
      menu_trans->addAction(*transnewset, tr("&New"), this, SLOT(transNew()), Qt::CTRL | Qt::SHIFT | Qt::Key_T);
  id_edittrans = menu_trans->addAction(tr("&Edit..."), this, SLOT(transEdit()));
  id_trans_straight = menu_trans->addAction(*transstraightenset, tr("&Straighten"), this, SLOT(transStraighten()),
                                            Qt::CTRL | Qt::Key_T);

  // Help
  /*  menu_help = new QMenu(this);
    menu_help->setMouseTracking(true);
    menu_help->insertItem(tr("&About..."), this, SLOT(helpAbout()));
    menu_help->insertSeparator();
    menu_help->insertItem(tr("About &Qt..."), this, SLOT(helpAboutQt()));
  */
  menu_help = new QMenu(this);
  menu_help->setMouseTracking(true);
  menu_help->addAction(tr("Qfsm &Manual..."), this, SLOT(helpManual()), Qt::Key_F1);
  menu_help->addSeparator();
  menu_help->addAction(tr("&About..."), this, SLOT(helpAbout()));
  menu_help->addAction(tr("About &Qt..."), this, SLOT(helpAboutQt()));

  m_menuBar->addMenu(menu_file)->setText(tr("&File"));
  m_menuBar->addMenu(menu_edit)->setText(tr("&Edit"));
  m_menuBar->addMenu(menu_view)->setText(tr("&View"));
  m_menuBar->addMenu(menu_machine)->setText(tr("&Machine"));
  m_menuBar->addMenu(menu_state)->setText(tr("&State"));
  m_menuBar->addMenu(menu_trans)->setText(tr("&Transition"));
  m_menuBar->addMenu(menu_help)->setText(tr("&Help"));

  // Context Menu: State
  cmenu_state = new QMenu(this);
  cmenu_state->setMouseTracking(true);
  id_csundo = cmenu_state->addAction(*undoset, tr("U&ndo"), this, SLOT(editUndo()), Qt::CTRL | Qt::Key_Z);
  cmenu_state->addSeparator();
  id_cscut = cmenu_state->addAction(*cutset, tr("C&ut"), this, SLOT(editCut()), Qt::CTRL | Qt::Key_X);
  id_cscopy = cmenu_state->addAction(*copyset, tr("&Copy"), this, SLOT(editCopy()), Qt::CTRL | Qt::Key_C);
  id_csdelete = cmenu_state->addAction(tr("De&lete"), this, SLOT(editDelete()), Qt::Key_Delete);
  cmenu_state->addSeparator();
  id_ceditstate = cmenu_state->addAction(tr("&Edit..."), this, SLOT(stateEdit()));
  id_csetinitial = cmenu_state->addAction(tr("Set &Initial State"), this, SLOT(stateSetInitial()));
  id_csetend = cmenu_state->addAction(tr("&Toggle Final State"), this, SLOT(stateSetFinal()));

  // Context Menu: Transition
  cmenu_trans = new QMenu(this);
  cmenu_trans->setMouseTracking(true);
  id_ctundo = cmenu_trans->addAction(*undoset, tr("U&ndo"), this, SLOT(editUndo()), Qt::CTRL | Qt::Key_Z);
  cmenu_trans->addSeparator();
  id_ctcut = cmenu_trans->addAction(*cutset, tr("C&ut"), this, SLOT(editCut()), Qt::CTRL | Qt::Key_X);
  id_ctcopy = cmenu_trans->addAction(*copyset, tr("&Copy"), this, SLOT(editCopy()), Qt::CTRL | Qt::Key_C);
  id_ctdelete = cmenu_trans->addAction(tr("De&lete"), this, SLOT(editDelete()), Qt::Key_Delete);
  cmenu_trans->addSeparator();
  id_cedittrans = cmenu_trans->addAction(tr("&Edit..."), this, SLOT(transEdit()));
  id_ctrans_straight = cmenu_trans->addAction(*transstraightenset, tr("&Straighten"), this, SLOT(transStraighten()),
                                              Qt::CTRL | Qt::Key_T);

  // Context Menu: ScrollView
  cmenu_sview = menu_edit;

  statusbar = new StatusBar(this);
  setStatusBar(statusbar);

  statemanager = new StateManager(this);
  machinemanager = new MachineManager(this);
  transmanager = new TransitionManager(this);
  fileio = new FileIO(this);
  printmanager = new PrintManager(this);

  mb_changed = new QMessageBox("qfsm", tr("The file has been changed. Do you want to save it?"),
                               QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                               QMessageBox::Cancel | QMessageBox::Escape);
  mb_changed->setButtonText(QMessageBox::Yes, tr("Yes"));
  mb_changed->setButtonText(QMessageBox::No, tr("No"));
  mb_changed->setButtonText(QMessageBox::Cancel, tr("Cancel"));

  fileio->loadOptions(&doc_options);
  fileio->loadMRU(m_control->getMRUList());

  tabwidgetdialog = new QDialog(this);
  tabwidgetdialog->resize(400, 300);
  tabwidgetdialog->setWindowTitle(tr("Qfsm Options"));
  // optionsDialog->setOkButton();
  // optionsDialog->setCancelButton();

  tabdialog = new QTabWidget(tabwidgetdialog);

  opt_general = new OptGeneralDlgImpl(tabdialog);
  opt_general->init();

  opt_display = new OptDisplayDlgImpl(tabdialog);
  opt_display->init(&doc_options);

  opt_printing = new OptPrintingDlgImpl(tabdialog);
  opt_printing->init(&doc_options);

  tabdialog->addTab(opt_general, tr("&General"));
  tabdialog->addTab(opt_display, tr("&Display"));
  tabdialog->addTab(opt_printing, tr("&Printing"));

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
  edit = new Edit(this);

  shift_pressed = false;
  control_pressed = false;
  bcut = false;

  setMode(DocStatus::Select);
  updateAll(); // MenuBar();

  connect(menu_mru, SIGNAL(aboutToShow()), this, SLOT(refreshMRU()));
  //  connect(cmenu_state, SIGNAL(aboutToHide()), m_mainView,
  //  SLOT(contextMenuHiding())); connect(cmenu_trans, SIGNAL(aboutToHide()),
  //  m_mainView, SLOT(contextMenuHiding())); connect(cmenu_sview,
  //  SIGNAL(aboutToHide()), m_mainView, SLOT(contextMenuHiding()));
  connect(this, SIGNAL(allSelected()), m_mainView->getDrawArea(), SLOT(allSelected()));
  connect(this, SIGNAL(objectsPasted()), m_mainView->getDrawArea(), SLOT(objectsPasted()));
  connect(this, SIGNAL(quitWindow(MainWindow*)), m_control, SLOT(quitWindow(MainWindow*)));
  connect(this, SIGNAL(escapePressed()), m_mainView->getDrawArea(), SLOT(escapePressed()));
  connect(m_mainView->getDrawArea(), SIGNAL(zoomedToPercentage(int)), statusbar, SLOT(setZoom(int)));
  connect(this, SIGNAL(updateStatusZoom(int)), m_mainView->getDrawArea(), SIGNAL(zoomedToPercentage(int)));
  connect(fileio, SIGNAL(sbMessage(QString)), this, SLOT(sbMessage(QString)));
  connect(m_menuBar, SIGNAL(triggered(QAction*)), this, SLOT(menuItemActivated(QAction*)));
  connect(menu_edit, SIGNAL(aboutToShow()), this, SLOT(editMenuAboutToShow()));
  connect(fileio, SIGNAL(setWaitCursor()), this, SLOT(setWaitCursor()));
  connect(fileio, SIGNAL(setPreviousCursor()), this, SLOT(setPreviousCursor()));
  connect(view_io, SIGNAL(closing()), this, SLOT(viewIOView()));
  connect(vvvv_export, SIGNAL(updateCode()), this, SLOT(updateVVVV()));
}

/// Destructor
MainWindow::~MainWindow()
{
  fileio->saveOptions(&doc_options);

  destroyToolBar();
  delete m_mainView;
  delete m_menuBar;
  delete menu_file;
  delete menu_import;
  delete menu_export;
  delete menu_edit;
  delete menu_view;
  delete menu_machine;
  delete menu_state;
  delete menu_trans;
  delete cmenu_state;
  delete cmenu_trans;
  if (m_project)
    delete m_project;

  delete statemanager;
  delete machinemanager;
  delete transmanager;
  delete printmanager;
  delete fileio;
  delete statusbar;
  delete tabdialog;
  delete simulator;
  delete edit;
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

/// Creates the toolbar with its buttons
void MainWindow::createToolBar()
{
  //  toolbar = new QToolBar("Main Toolbar", this);
  toolbar = addToolBar("Main Toolbar");
  toolbar->setMovable(true);

  pnew = new QPixmap((const char**)filenew);
  tbnew = toolbar->addAction(*pnew, tr("New File"), this, SLOT(fileNew()));
  tbnew->setToolTip(tr("Creates a new file"));

  // tbnew = new QToolButton(
  //     *pnew, tr("New File"), tr("Creates a new file"), this, SLOT(fileNew()), toolbar);
  // toolbar->addWidget(tbnew);

  popen = new QPixmap((const char**)fileopen);
  tbopen = toolbar->addAction(*popen, tr("Open File"), this, SLOT(fileOpen()));
  tbopen->setToolTip(tr("Opens a file"));

  QPixmap psave((const char**)filesave);
  QPixmap psaveoff((const char**)filesaveoff);
  saveset = new QIcon(psave);
  saveset->addPixmap(psaveoff, QIcon::Disabled);
  tbsave = toolbar->addAction(*saveset, tr("Save File"), this, SLOT(fileSave()));
  tbsave->setToolTip(tr("Saves this file"));

  QPixmap pprint((const char**)fileprint);
  QPixmap pprintoff((const char**)fileprintoff);
  printset = new QIcon(pprint);
  printset->addPixmap(pprintoff, QIcon::Disabled);
  tbprint = toolbar->addAction(*printset, tr("Print"), this, SLOT(filePrint()));
  tbprint->setToolTip(tr("Prints this file"));

  QPixmap pundo((const char**)editundo);
  QPixmap pundooff((const char**)editundooff);
  undoset = new QIcon(pundo);
  undoset->addPixmap(pundooff, QIcon::Disabled);
  tbundo = toolbar->addAction(*undoset, tr("Undo"), this, SLOT(editUndo()));
  tbundo->setToolTip(tr("Undo last action"));

  QPixmap pcut((const char**)editcut);
  QPixmap pcutoff((const char**)editcutoff);
  cutset = new QIcon(pcut);
  cutset->addPixmap(pcutoff, QIcon::Disabled);
  tbcut = toolbar->addAction(*cutset, tr("Cut"), this, SLOT(editCut()));
  tbcut->setToolTip(tr("Cuts Selection"));

  QPixmap pcopy((const char**)editcopy);
  QPixmap pcopyoff((const char**)editcopyoff);
  copyset = new QIcon(pcopy);
  copyset->addPixmap(pcopyoff, QIcon::Disabled);
  tbcopy = toolbar->addAction(*copyset, tr("Copy"), this, SLOT(editCopy()));
  tbcopy->setToolTip(tr("Copies Selection"));

  QPixmap ppaste((const char**)editpaste);
  QPixmap ppasteoff((const char**)editpasteoff);
  pasteset = new QIcon(ppaste);
  pasteset->addPixmap(ppasteoff, QIcon::Disabled);
  tbpaste = toolbar->addAction(*pasteset, tr("Paste"), this, SLOT(editPaste()));
  tbpaste->setToolTip(tr("Pastes the clipboard"));

  toolbar->addSeparator();

  QPixmap pselect((const char**)sel);
  QPixmap pselectoff((const char**)selectoff);
  selset = new QIcon(pselect);
  selset->addPixmap(pselectoff, QIcon::Disabled);
  tbselect = toolbar->addAction(*selset, tr("Select"), this, SLOT(editSelect()));
  tbselect->setToolTip(tr("Select objects"));
  tbselect->setCheckable(true);

  QPixmap ppan((const char**)pan);
  QPixmap ppanoff((const char**)panoff);
  panset = new QIcon(ppan);
  panset->addPixmap(ppanoff, QIcon::Disabled);
  tbpan = toolbar->addAction(*panset, tr("Pan"), this, SLOT(viewPan()));
  tbpan->setToolTip(tr("Pan view"));
  tbpan->setCheckable(true);

  QPixmap pzoom((const char**)zoom);
  QPixmap pzoomoff((const char**)zoomoff);
  zoomset = new QIcon(pzoom);
  zoomset->addPixmap(pzoomoff, QIcon::Disabled);
  tbzoom = toolbar->addAction(*zoomset, tr("Zoom"), this, SLOT(viewZoom()));
  tbzoom->setToolTip(tr("Switches to zoom mode"));
  tbzoom->setCheckable(true);

  QPixmap pstatenew((const char**)statenew);
  QPixmap pstatenewoff((const char**)statenewoff);
  statenewset = new QIcon(pstatenew);
  statenewset->addPixmap(pstatenewoff, QIcon::Disabled);
  tbstatenew = toolbar->addAction(*statenewset, tr("Add State"), this, SLOT(stateNew()));
  tbstatenew->setToolTip(tr("Add new states"));
  tbstatenew->setCheckable(true);

  QPixmap ptransnew((const char**)transnew);
  QPixmap ptransnewoff((const char**)transnewoff);
  transnewset = new QIcon(ptransnew);
  transnewset->addPixmap(ptransnewoff, QIcon::Disabled);
  tbtransnew = toolbar->addAction(*transnewset, tr("Add Transition"), this, SLOT(transNew()));
  tbtransnew->setToolTip(tr("Add new transitions"));
  tbtransnew->setCheckable(true);

  QPixmap pmachinesim((const char**)machinesim);
  QPixmap pmachinesimoff((const char**)machinesimoff);
  machinesimset = new QIcon(pmachinesim);
  machinesimset->addPixmap(pmachinesimoff, QIcon::Disabled);
  tbmachinesim = toolbar->addAction(*machinesimset, tr("Simulate"), this, SLOT(machineSimulate()));
  tbmachinesim->setToolTip(tr("Simulates this machine"));
  tbmachinesim->setCheckable(true);

  toolbar->addSeparator();

  pzoomin = new QPixmap((const char**)zoomin);
  tbzoomin = toolbar->addAction(*pzoomin, tr("Zoom In"), this, SLOT(viewZoomIn()));
  tbzoomin->setToolTip(tr("Zooms into the view"));

  pzoomout = new QPixmap((const char**)zoomout);
  tbzoomout = toolbar->addAction(*pzoomout, tr("Zoom Out"), this, SLOT(viewZoomOut()));
  tbzoomout->setToolTip(tr("Zoom out of the view"));

  QPixmap ptransstraighten((const char**)transstraighten);
  QPixmap ptransstraightenoff((const char**)transstraightenoff);
  transstraightenset = new QIcon(ptransstraighten);
  transstraightenset->addPixmap(ptransstraightenoff, QIcon::Disabled);
  tbtransstraighten =
      toolbar->addAction(*transstraightenset, tr("Straighten Transitions"), this, SLOT(transStraighten()));
  tbtransstraighten->setToolTip(tr("Straightens selected transitions"));

  zoomCursor = new QCursor(QPixmap((const char**)c_mag_xpm), 7, 7);
}

/// Destroys the toolbar
void MainWindow::destroyToolBar()
{
  delete popen;
  delete pnew;
  delete pzoomin;
  delete pzoomout;
  delete saveset;
  delete printset;
  delete undoset;
  delete cutset;
  delete copyset;
  delete pasteset;
  delete selset;
  delete panset;
  delete zoomset;
  delete statenewset;
  delete transnewset;
  delete transstraightenset;
  delete machinesimset;
  delete tbnew;
  delete tbopen;
  delete tbsave;
  delete tbprint;
  delete tbundo;
  delete tbcut;
  delete tbcopy;
  delete tbpaste;
  delete tbselect;
  delete tbpan;
  delete tbzoom;
  delete tbstatenew;
  delete tbtransnew;
  delete tbmachinesim;
  delete tbzoomin;
  delete tbzoomout;
  delete tbtransstraighten;
  delete toolbar;
  delete zoomCursor;
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
void MainWindow::focusInEvent(QFocusEvent* e)
{
  qDebug("test");
  if (e->gotFocus()) {
    if (e->reason() != Qt::PopupFocusReason)
      updatePaste();
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
      this->sbMessage("Invalid file type");
      return;
    }

    // workaround for windows drag&drop bugs
#ifdef WIN32
    if (path[0] == '/')
      path = path.right(path.length() - 1);
#endif

    fileOpenRecent(path);
  } else {
    if (e->source() == this)
      return;
    if (!m_project)
      return;
    if (!m_project->machine)
      return;

    m_mainView->getDrawArea()->getSelection()->deselectAll(m_project->machine);
    data = QString(mm->data("text/qfsm-objects"));

    if (edit->paste(m_mainView->getDrawArea()->getSelection(), m_project, m_project->machine, data)) {
      emit objectsPasted();
      m_project->setChanged();
    }

    int count = m_mainView->getDrawArea()->getSelection()->count();
    if (count == 1)
      statusbar->showMessage(QString::number(count) + " " + tr("object pasted."), 2000);
    else
      statusbar->showMessage(QString::number(count) + " " + tr("objects pasted."), 2000);

    m_mainView->widget()->repaint();
    updateAll();
  }

  //  data=QString(mm->data("text/qfsm-objects"));
}

/// Called when a menu item is activated
void MainWindow::menuItemActivated(QAction*)
{
  m_mainView->getDrawArea()->resetContext();
}

/// Called when the edit menu is about to show
void MainWindow::editMenuAboutToShow()
{
  m_mainView->getDrawArea()->resetContext();
}

/**
 * Sets the current mode and updates the menus.
 */
void MainWindow::setMode(int m)
{
  doc_status.setMode(m);

  switch (m) {
    case DocStatus::Select:
      id_select->setChecked(true);
      id_pan->setChecked(false);
      id_newstate->setChecked(false);
      id_newtrans->setChecked(false);
      id_zoom->setChecked(false);
      id_machinesim->setChecked(false);
      tbselect->setChecked(true);
      tbpan->setChecked(false);
      tbzoom->setChecked(false);
      tbstatenew->setChecked(false);
      tbtransnew->setChecked(false);
      tbmachinesim->setChecked(false);
      break;
    case DocStatus::Pan:
      id_select->setChecked(false);
      id_pan->setChecked(true);
      id_newstate->setChecked(false);
      id_newtrans->setChecked(false);
      id_zoom->setChecked(false);
      id_machinesim->setChecked(false);
      tbselect->setChecked(false);
      tbpan->setChecked(true);
      tbzoom->setChecked(false);
      tbstatenew->setChecked(false);
      tbtransnew->setChecked(false);
      tbmachinesim->setChecked(false);
      break;
    case DocStatus::NewState:
      id_select->setChecked(false);
      id_pan->setChecked(false);
      id_newstate->setChecked(true);
      id_newtrans->setChecked(false);
      id_zoom->setChecked(false);
      id_machinesim->setChecked(false);
      tbselect->setChecked(false);
      tbpan->setChecked(false);
      tbzoom->setChecked(false);
      tbstatenew->setChecked(true);
      tbtransnew->setChecked(false);
      tbmachinesim->setChecked(false);
      break;
    case DocStatus::NewTransition:
      id_select->setChecked(false);
      id_pan->setChecked(false);
      id_newstate->setChecked(false);
      id_newtrans->setChecked(true);
      id_zoom->setChecked(false);
      id_machinesim->setChecked(false);
      tbselect->setChecked(false);
      tbpan->setChecked(false);
      tbzoom->setChecked(false);
      tbstatenew->setChecked(false);
      tbtransnew->setChecked(true);
      tbmachinesim->setChecked(false);
      break;
    case DocStatus::Zooming:
      id_select->setChecked(false);
      id_pan->setChecked(false);
      id_newstate->setChecked(false);
      id_newtrans->setChecked(false);
      id_zoom->setChecked(true);
      id_machinesim->setChecked(false);
      tbselect->setChecked(false);
      tbpan->setChecked(false);
      tbzoom->setChecked(true);
      tbstatenew->setChecked(false);
      tbtransnew->setChecked(false);
      tbmachinesim->setChecked(false);
      break;
    case DocStatus::Simulating:
      id_select->setChecked(false);
      id_pan->setChecked(false);
      id_newstate->setChecked(false);
      id_newtrans->setChecked(false);
      id_zoom->setChecked(false);
      id_machinesim->setChecked(true);
      tbselect->setChecked(false);
      tbpan->setChecked(false);
      tbzoom->setChecked(false);
      tbstatenew->setChecked(false);
      tbtransnew->setChecked(false);
      tbmachinesim->setChecked(true);
      break;
  }
  switch (m) {
    case DocStatus::Pan:
      m_mainView->viewport()->setCursor(Qt::SizeAllCursor);
      break;
    case DocStatus::NewState:
    case DocStatus::NewTransition:
      m_mainView->viewport()->setCursor(Qt::CrossCursor);
      break;
    case DocStatus::Zooming:
      m_mainView->viewport()->setCursor(*zoomCursor);
      break;
    default:
      m_mainView->viewport()->setCursor(Qt::ArrowCursor);
      break;
  }
}

/// Repaints the scroll view
void MainWindow::repaintViewport()
{
  m_mainView->widget()->repaint();
}

/// Updates all menus.
void MainWindow::updateMenuBar()
{
  int numstates, numtrans;

  id_import->setEnabled(true);
  // id_import_graphviz->setEnabled(true);

  if (m_project) {
    id_save->setEnabled(true);
    id_saveas->setEnabled(true);
    id_print->setEnabled(true);
    id_export->setEnabled(true);
    id_close->setEnabled(true);
    id_selectall->setEnabled(true);
    id_deselectall->setEnabled(true);
    id_newstate->setEnabled(true);
    id_newtrans->setEnabled(true);
    // id_newtrans->setEnabled(true);
    if (m_project->machine && m_project->machine->getType() == Ascii)
      id_export_ragel->setEnabled(true);
    else
      id_export_ragel->setEnabled(false);
    if (m_project->machine && m_project->machine->getType() == Text) {
      id_export_ahdl->setEnabled(false);
      id_export_vhdl->setEnabled(false);
      id_export_verilog->setEnabled(false);
      id_export_kiss->setEnabled(false);
      id_export_vvvv->setEnabled(true);
      id_export_scxml->setEnabled(true);
      id_export_smc->setEnabled(true);
      id_viewstateenc->setEnabled(false);
      id_viewmoore->setEnabled(false);
      tbmachinesim->setEnabled(false);
    } else {
      id_export_ahdl->setEnabled(true);
      id_export_vhdl->setEnabled(true);
      id_export_verilog->setEnabled(true);
      id_export_kiss->setEnabled(true);
      id_export_vvvv->setEnabled(false);
      id_export_scxml->setEnabled(false);
      id_export_smc->setEnabled(false);
      id_viewstateenc->setEnabled(true);
      id_viewmoore->setEnabled(true);
      tbmachinesim->setEnabled(true);
    }
    id_viewmealyin->setEnabled(true);
    id_viewmealyout->setEnabled(true);
    id_viewgrid->setEnabled(true);
    id_ioview->setEnabled(true);
    id_viewshadows->setEnabled(true);
    id_zoom->setEnabled(true);
    id_zoomin->setEnabled(true);
    id_zoomout->setEnabled(true);
    id_zoom100->setEnabled(true);
    id_select->setEnabled(true);
    id_pan->setEnabled(true);
    id_machineedit->setEnabled(true);
    id_correctcodes->setEnabled(true);
    id_machineicheck->setEnabled(true);
    tbsave->setEnabled(true);
    tbprint->setEnabled(true);
    tbselect->setEnabled(true);
    tbpan->setEnabled(true);
    tbzoom->setEnabled(true);
    tbzoomin->setEnabled(true);
    tbzoomout->setEnabled(true);
    tbstatenew->setEnabled(true);
    tbtransnew->setEnabled(true);
  } else {
    id_save->setEnabled(false);
    id_saveas->setEnabled(false);
    id_print->setEnabled(false);
    id_export->setEnabled(false);
    id_close->setEnabled(false);
    id_selectall->setEnabled(false);
    id_deselectall->setEnabled(false);
    id_newstate->setEnabled(false);
    id_newtrans->setEnabled(false);
    id_viewstateenc->setEnabled(false);
    id_viewmoore->setEnabled(false);
    id_viewmealyin->setEnabled(false);
    id_viewmealyout->setEnabled(false);
    id_viewgrid->setEnabled(false);
    id_viewshadows->setEnabled(false);
    id_ioview->setEnabled(false);
    id_zoom->setEnabled(false);
    id_zoomin->setEnabled(false);
    id_zoomout->setEnabled(false);
    id_zoom100->setEnabled(false);
    id_select->setEnabled(false);
    id_pan->setEnabled(false);
    id_machineedit->setEnabled(false);
    id_correctcodes->setEnabled(false);
    id_machineicheck->setEnabled(false);
    tbsave->setEnabled(false);
    tbprint->setEnabled(false);
    tbselect->setEnabled(false);
    tbpan->setEnabled(false);
    tbzoom->setEnabled(false);
    tbzoomin->setEnabled(false);
    tbzoomout->setEnabled(false);
    tbstatenew->setEnabled(false);
    tbtransnew->setEnabled(false);
    tbmachinesim->setEnabled(false);
  }

  numtrans = m_mainView->getDrawArea()->getSelection()->countTransitions();
  numstates = m_mainView->getDrawArea()->getSelection()->countStates();

  if (m_project && m_project->machine && m_project->machine->getType() != Text &&
      m_project->machine->getNumStates() > 0) {
    id_machinesim->setEnabled(true);
    tbmachinesim->setEnabled(true);
  } else {
    id_machinesim->setEnabled(false);
    tbmachinesim->setEnabled(false);
  }

  if (numtrans) {
    id_trans_straight->setEnabled(true);
    tbtransstraighten->setEnabled(true);
    id_cedittrans->setEnabled(true);
    id_ctrans_straight->setEnabled(true);
  } else {
    id_trans_straight->setEnabled(false);
    tbtransstraighten->setEnabled(false);
    id_cedittrans->setEnabled(false);
    id_ctrans_straight->setEnabled(false);
  }

  if (numstates > 0) {
    id_setend->setEnabled(true);
    id_csetend->setEnabled(true);
  } else {
    id_setend->setEnabled(false);
    id_csetend->setEnabled(false);
  }
  if (numstates == 1) {
    id_setinitial->setEnabled(true);
    id_editstate->setEnabled(true);
  } else {
    id_setinitial->setEnabled(false);
    id_editstate->setEnabled(false);
  }

  if (numtrans == 1) {
    id_edittrans->setEnabled(true);
  } else {
    id_edittrans->setEnabled(false);
  }

  if (numstates + numtrans > 0) {
    id_delete->setEnabled(true);
    id_cut->setEnabled(true);
    id_copy->setEnabled(true);
    id_csdelete->setEnabled(true);
    id_cscut->setEnabled(true);
    id_cscopy->setEnabled(true);
    id_ctdelete->setEnabled(true);
    id_ctcut->setEnabled(true);
    id_ctcopy->setEnabled(true);
    tbcut->setEnabled(true);
    tbcopy->setEnabled(true);
  } else {
    id_delete->setEnabled(false);
    id_cut->setEnabled(false);
    id_copy->setEnabled(false);
    id_csdelete->setEnabled(false);
    id_cscut->setEnabled(false);
    id_cscopy->setEnabled(false);
    id_ctdelete->setEnabled(false);
    id_ctcut->setEnabled(false);
    id_ctcopy->setEnabled(false);
    tbcut->setEnabled(false);
    tbcopy->setEnabled(false);
  }

  //  updatePaste();

  if (doc_options.getViewStateEncoding())
    id_viewstateenc->setChecked(true);
  else
    id_viewstateenc->setChecked(false);

  if (doc_options.getViewMoore())
    id_viewmoore->setChecked(true);
  else
    id_viewmoore->setChecked(false);

  if (doc_options.getViewMealyIn())
    id_viewmealyin->setChecked(true);
  else
    id_viewmealyin->setChecked(false);

  if (doc_options.getViewMealyOut())
    id_viewmealyout->setChecked(true);
  else
    id_viewmealyout->setChecked(false);

  if (doc_options.getViewGrid())
    id_viewgrid->setChecked(true);
  else
    id_viewgrid->setChecked(false);

  if (doc_options.getViewIOView())
    id_ioview->setChecked(true);
  else
    id_ioview->setChecked(false);

  if (doc_options.getStateShadows())
    id_viewshadows->setChecked(true);
  else
    id_viewshadows->setChecked(false);

  if (m_project && !m_project->getUndoBuffer()->isEmpty()) {
    id_undo->setEnabled(true);
    id_csundo->setEnabled(true);
    id_ctundo->setEnabled(true);
    tbundo->setEnabled(true);
  } else {
    id_undo->setEnabled(false);
    id_csundo->setEnabled(false);
    id_ctundo->setEnabled(false);
    tbundo->setEnabled(false);
  }

  if (doc_status.getMode() == DocStatus::Simulating) {
    id_undo->setEnabled(false);
    id_csundo->setEnabled(false);
    id_ctundo->setEnabled(false);
    id_select->setEnabled(false);
    id_pan->setEnabled(false);
    id_cut->setEnabled(false);
    id_copy->setEnabled(false);
    id_paste->setEnabled(false);
    id_delete->setEnabled(false);
    id_cscut->setEnabled(false);
    id_cscopy->setEnabled(false);
    id_csdelete->setEnabled(false);
    id_ctcut->setEnabled(false);
    id_ctcopy->setEnabled(false);
    id_ctdelete->setEnabled(false);
    id_selectall->setEnabled(false);
    id_deselectall->setEnabled(false);
    id_zoom->setEnabled(false);
    id_machineedit->setEnabled(false);
    id_editstate->setEnabled(false);
    id_setinitial->setEnabled(false);
    id_ceditstate->setEnabled(false);
    id_csetinitial->setEnabled(false);
    id_setend->setEnabled(false);
    id_newstate->setEnabled(false);
    id_newtrans->setEnabled(false);
    id_edittrans->setEnabled(false);
    id_trans_straight->setEnabled(false);
    id_cedittrans->setEnabled(false);
    id_ctrans_straight->setEnabled(false);
    tbselect->setEnabled(false);
    tbpan->setEnabled(false);
    tbzoom->setEnabled(false);
    tbundo->setEnabled(false);
    tbcut->setEnabled(false);
    tbcopy->setEnabled(false);
    tbstatenew->setEnabled(false);
    tbtransnew->setEnabled(false);
    tbtransstraighten->setEnabled(false);
  }
}

/// Updates the paste tool button and menu item
void MainWindow::updatePaste()
{
  if (m_project && qApp->clipboard()->mimeData()->hasFormat("text/qfsm-objects")) {
    id_paste->setEnabled(true);
    tbpaste->setEnabled(true);
  } else {
    id_paste->setEnabled(false);
    tbpaste->setEnabled(false);
  }
}

/// Updates the title bar.
void MainWindow::updateTitleBar()
{
  QString s;
  s = "Qfsm";
  if (m_project) {
    QString f;
    f = fileio->getActFilePath();
    if (!f.isNull()) {
      QFileInfo fi(f);
      f = fi.fileName();
      s = "Qfsm - " + f;
      if (m_project->hasChanged())
        s += " " + tr("(modified)");
    }
  }
  setWindowTitle(s);
}

/// Updates the status bar
void MainWindow::updateStatusBar()
{
  int selected;
  int scale;

  if (m_project) {
    selected = m_mainView->getDrawArea()->getSelection()->count();
    scale = int(m_mainView->getDrawArea()->getScale() * 100 + 0.5);
  } else {
    selected = -1;
    scale = -1;
  }

  statusbar->setSelected(selected);
  emit updateStatusZoom(scale);
}

/// Updates menu, title bar and status bar
void MainWindow::updateAll()
{
  //  updatePaste();
  updateMenuBar();
  updateTitleBar();
  updateStatusBar();
  if (vvvv_export->isVisible())
    updateVVVV();
}

/// Refreshes the MRU file list
void MainWindow::refreshMRU()
{
  int id, index = 0;
  menu_mru->clear();
  QStringList list = m_control->getMRUList();

  fileio->loadMRU(list);
  QStringList::Iterator it;

  for (it = list.begin(); it != list.end(); ++it) {
    // id = menu_mru->insertItem(*it);
    switch (index) {
      case 0:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent0()));
        break;
      case 1:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent1()));
        break;
      case 2:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent2()));
        break;
      case 3:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent3()));
        break;
      case 4:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent4()));
        break;
      case 5:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent5()));
        break;
      case 6:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent6()));
        break;
      case 7:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent7()));
        break;
      case 8:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent8()));
        break;
      case 9:
        menu_mru->addAction(*it, this, SLOT(fileOpenRecent9()));
        break;
    }
    index++;
  }
}

/// Shows the context menu for a state
void MainWindow::showContextState()
{
  cmenu_state->popup(QCursor::pos());
}

/// Shows the context menu for a transition
void MainWindow::showContextTrans()
{
  cmenu_trans->popup(QCursor::pos());
}

/// Shows the context menu for the scrollview
void MainWindow::showContext()
{
  cmenu_sview->popup(QCursor::pos());
}

/// Sends a message @a s to the status bar
void MainWindow::sbMessage(QString s)
{
  statusbar->showMessage(s);
}

/// Sends a message @a s for time @a t to the status bar
void MainWindow::sbMessage(QString s, int t)
{
  statusbar->showMessage(s, t);
}

/// Creates a new file
void MainWindow::fileNew()
{
  int result;
  bool sim = false;

  if (doc_status.getMode() == DocStatus::Simulating)
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
  Project* p = new Project(this);

  result = machinemanager->addMachine(p);
  if (result) {
    if (sim)
      simulator->closeDlg();

    if (m_project) {
      delete m_project;
      m_project = NULL;
    }
    m_project = p;
    fileio->setActFilePath(QString{});

    statusbar->showMessage(m_project->machine->getName() + " " + tr("created."), 2000);
  } else {
    return;
  }

  setMode(DocStatus::Select);
  m_mainView->getDrawArea()->reset();
  m_mainView->widget()->repaint();

  updateAll();
}

/// Opens an existing file.
void MainWindow::fileOpen()
{
  Project* p;

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

  /*
  QCursor oldcursor1 = cursor();
  QCursor oldcursor2 = m_mainView->viewport()->cursor();
  setCursor(waitCursor);
  m_mainView->viewport()->setCursor(waitCursor);
  */
  // qApp->setOverrideCursor(waitCursor);

  p = fileio->openFileXML();
  if (p) {
    if (m_project) {
      delete m_project;
      m_project = NULL;
    }
    statusbar->showMessage(tr("File") + " " + fileio->getActFileName() + " " + tr("loaded."), 2000);
    m_project = p;
    p->getUndoBuffer()->clear();

    updateAll();
    m_mainView->updateBackground();
    m_mainView->getDrawArea()->resetState();
    // m_mainView->getDrawArea()->updateCanvasSize();
    m_mainView->updateSize();
    if (doc_status.getMode() == DocStatus::Simulating) {
      if (!simulator->startSimulation(m_project->machine))
        setMode(DocStatus::Select);
    } else
      m_mainView->widget()->repaint();

    //    menu_mru->insertItem(fileio->getActFile(), -1, 0);
    m_control->addMRUEntry(fileio->getActFilePath());
    fileio->saveMRU(m_control->getMRUList());

    //      statusbar->showMessage(tr("File %1 opened").arg(fileio->getActFile()),
    //      3000);
  } else if (!fileio->getActFilePath().isNull()) {
    Error::info(tr("File %1 could not be opened").arg(fileio->getActFilePath()));
    statusbar->clearMessage();
  }
  /*
  setCursor(oldcursor1);
  m_mainView->viewport()->setCursor(oldcursor2);
  */
  // qApp->restoreOverrideCursor();
}

/// Opens a file from the MRU file list with the name @a fileName
void MainWindow::fileOpenRecent(QString fileName)
{
  Project* p;

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

  /*
  QCursor oldcursor1 = cursor();
  QCursor oldcursor2 = m_mainView->viewport()->cursor();
  setCursor(waitCursor);
  m_mainView->viewport()->setCursor(waitCursor);
  */

  p = fileio->openFileXML(fileName);
  if (p) {
    if (m_project) {
      delete m_project;
      m_project = NULL;
    }
    statusbar->showMessage(tr("File") + " " + fileio->getActFileName() + " " + tr("loaded."), 2000);
    m_project = p;
    p->getUndoBuffer()->clear();

    updateAll();
    m_mainView->updateBackground();
    m_mainView->getDrawArea()->resetState();
    // m_mainView->getDrawArea()->updateCanvasSize();
    m_mainView->updateSize();
    if (doc_status.getMode() == DocStatus::Simulating) {
      if (!simulator->startSimulation(m_project->machine))
        setMode(DocStatus::Select);
    } else
      m_mainView->widget()->repaint();

    //    menu_mru->insertItem(fileio->getActFile(), -1, 0);
    m_control->addMRUEntry(fileio->getActFilePath());
    fileio->saveMRU(m_control->getMRUList());
  } else {
    Error::info(tr("File %1 could not be opened").arg(fileName));
    statusbar->clearMessage();
    m_control->removeMRUEntry(fileName);
    fileio->saveMRU(m_control->getMRUList());
  }
  /*
  setCursor(oldcursor1);
  m_mainView->viewport()->setCursor(oldcursor2);
  */
}

/// Opens the file in the MRU list entry 0
void MainWindow::fileOpenRecent0()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 0)
    fileOpenRecent(list[0]);
}

/// Opens the file in the MRU list entry 1
void MainWindow::fileOpenRecent1()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 1)
    fileOpenRecent(list[1]);
}

/// Opens the file in the MRU list entry 2
void MainWindow::fileOpenRecent2()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 2)
    fileOpenRecent(list[2]);
}

/// Opens the file in the MRU list entry 3
void MainWindow::fileOpenRecent3()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 3)
    fileOpenRecent(list[3]);
}

/// Opens the file in the MRU list entry 4
void MainWindow::fileOpenRecent4()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 4)
    fileOpenRecent(list[4]);
}

/// Opens the file in the MRU list entry 5
void MainWindow::fileOpenRecent5()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 5)
    fileOpenRecent(list[5]);
}

/// Opens the file in the MRU list entry 6
void MainWindow::fileOpenRecent6()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 6)
    fileOpenRecent(list[6]);
}

/// Opens the file in the MRU list entry 7
void MainWindow::fileOpenRecent7()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 7)
    fileOpenRecent(list[7]);
}

/// Opens the file in the MRU list entry 8
void MainWindow::fileOpenRecent8()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 8)
    fileOpenRecent(list[8]);
}

/// Opens the file in the MRU list entry 9
void MainWindow::fileOpenRecent9()
{
  QStringList list = m_control->getMRUList();
  if (list.count() > 9)
    fileOpenRecent(list[9]);
}

/// Saves the current file.
bool MainWindow::fileSave()
{
  if (m_project) {
    bool result;
    bool saveas = (fileio->getActFileName().isEmpty());
    QCursor oldcursor1 = cursor();
    QCursor oldcursor2 = m_mainView->viewport()->cursor();
    setCursor(Qt::WaitCursor);
    m_mainView->viewport()->setCursor(Qt::WaitCursor);

    result = fileio->saveFile(m_project);

    if (result) {
      statusbar->showMessage(tr("File") + " " + fileio->getActFileName() + " " + tr("saved."), 2000);
      m_project->getUndoBuffer()->clear();
      if (saveas) {
        m_control->addMRUEntry(fileio->getActFilePath());
        fileio->saveMRU(m_control->getMRUList());
      }
    }

    updateAll();

    setCursor(oldcursor1);
    m_mainView->viewport()->setCursor(oldcursor2);

    return result;
  }

  return false;
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
      statusbar->showMessage(tr("File") + " " + fileio->getActFileName() + " " + tr("saved."), 2000);
      m_project->getUndoBuffer()->clear();
      m_control->addMRUEntry(fileio->getActFilePath());
      fileio->saveMRU(m_control->getMRUList());
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
  Project* p;

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
    statusbar->showMessage(tr("File") + " " + fileio->getActFileName() + " " + tr("imported."), 2000);
    m_project = p;
    p->getUndoBuffer()->clear();

    updateAll();
    m_mainView->updateBackground();
    m_mainView->getDrawArea()->resetState();
    // m_mainView->getDrawArea()->updateCanvasSize();
    if (doc_status.getMode() == DocStatus::Simulating) {
      if (!simulator->startSimulation(m_project->machine))
        setMode(DocStatus::Select);
    } else {
      // m_mainView->widget()->repaint();
      // m_mainView->getDrawArea()->zoomReset();
      m_mainView->updateSize();
    }
  } else if (!fileio->getActImportFilePath().isNull()) {
    Error::info(tr("File %1 could not be opened").arg(fileio->getActFileName()));
    statusbar->clearMessage();
  }
}

/// Exports the current diagram to an EPS file
bool MainWindow::fileExportEPS()
{
  if (m_project) {
    bool result;

    m_project->machine->updateDefaultTransitions();

    ExportEPS* exp = new ExportEPS(&doc_options);
    result = fileio->exportFile(m_project, exp, m_mainView);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

    updateAll();
    return result;
  }
  return false;
}

//
/// Exports the current diagram to a SVG file
bool MainWindow::fileExportSVG()
{
  if (m_project) {
    bool result;

    m_project->machine->updateDefaultTransitions();

    ExportSVG* exp = new ExportSVG(&doc_options);
    result = fileio->exportFile(m_project, exp, m_mainView);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    m_project->machine->updateDefaultTransitions();

    ExportPNG* exp = new ExportPNG(&doc_options);
    result = fileio->exportFile(m_project, exp, m_mainView);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    m_project->machine->updateDefaultTransitions();
    ExportAHDL* exp = new ExportAHDL(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    vhdl_export->init(&doc_options, m_project->machine);
    switch (vhdl_export->exec()) {
      case QDialog::Accepted:
        doc_options.applyOptions(this);
        break;
      case QDialog::Rejected:
        return true;
        break;
    }

    m_project->machine->updateDefaultTransitions();
    ExportVHDL* exp = new ExportVHDL(&doc_options);

    result = exp->checkMachineNames(m_project->machine, &doc_options, &invalidNames);

    if (!result) {
      errorMessage = tr("Export of file %1 failed!").arg(fileio->getActExportFileName()) + "\n\n" +
                     tr("The following identifiers do not match the VHDL syntax:") + "\n";

      errorMessage += invalidNames.join("\n");

      Error::warningOk(errorMessage);

      statusbar->showMessage(tr("Export of file") + " " + fileio->getActExportFileName() + " " + tr("failed."), 2000);
      delete exp;
      return false;
    }

    if (!m_project || !exp)
      return false;

    if (!exp->validateMachine(m_project->machine))
      return false;

    QString path_entity = vhdl_export->getEntityPath();
    QString path_arch = vhdl_export->getArchitecturePath();

    QFile ftmp(path_entity);
    if (ftmp.exists()) {
      if (Error::warningOkCancel(tr("File %1 exists. Do you want to overwrite it?").arg(path_entity)) !=
          QMessageBox::Ok) {
        delete exp;
        return false;
      }
    }

    std::ofstream fout_entity(path_entity.toStdString());

    if (!fout_entity) {
      Error::warningOk(tr("Unable to write file %1!").arg(path_entity));
      delete exp;
      return false;
    }

    if (doc_options.getVHDLSepFiles()) {
      ftmp.setFileName(path_arch);
      if (ftmp.exists()) {
        if (Error::warningOkCancel(tr("File %1 exists. Do you want to overwrite it?").arg(path_arch)) !=
            QMessageBox::Ok) {
          delete exp;
          return false;
        }
      }

      std::ofstream fout_architecture(path_arch.toStdString());
      if (!fout_architecture) {
        Error::warningOk(tr("Unable to write file %1!").arg(path_arch));
        delete exp;
        return false;
      }
      exp->init(&fout_entity, &fout_architecture, m_project->machine, path_entity, NULL);
      exp->doExport();
    } else {
      exp->init(&fout_entity, &fout_entity, m_project->machine, path_entity, NULL);
      exp->doExport();
    }

    //  result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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
    m_project->machine->updateDefaultTransitions();
    ExportIODescription* exp = new ExportIODescription(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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
    Error err;
    QString base_dir_name, testbench_dir_name, testvector_dir_name, package_dir_name, logfile_dir_name;
    QDir testbenchDir, testvectorDir, packageDir, logfileDir;
    std::ofstream *testbench_out, *testvector_out, *package_out;

    testbench_export->init(&doc_options, m_project->machine);

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

        if (m_project->machine->getNumOutputs() > 0) {
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
          if (Error::warningOkCancel(
                  tr("File %1 exists. Do you want to overwrite it?").arg(doc_options.getTestbenchVHDLPath())) !=
              QMessageBox::Ok)
            return false;
        }
        ftmp.setFileName(base_dir_name + doc_options.getTestvectorASCIIPath());
        if (ftmp.exists()) {
          if (Error::warningOkCancel(
                  tr("File %1 exists. Do you want to overwrite it?").arg(doc_options.getTestvectorASCIIPath())) !=
              QMessageBox::Ok)
            return false;
        }
        ftmp.setFileName(base_dir_name + doc_options.getTestpackageVHDLPath());
        if (ftmp.exists()) {
          if (Error::warningOkCancel(
                  tr("File %1 exists. Do you want to overwrite it?").arg(doc_options.getTestpackageVHDLPath())) !=
              QMessageBox::Ok)
            return false;
        }

        testbench_out = new std::ofstream((base_dir_name + doc_options.getTestbenchVHDLPath()).toLatin1().data());
        testvector_out = new std::ofstream((base_dir_name + doc_options.getTestvectorASCIIPath()).toLatin1().data());
        package_out = new std::ofstream((base_dir_name + doc_options.getTestpackageVHDLPath()).toLatin1().data());

        if (!testbench_out) {
          Error::warningOk(tr("Unable to open file %1!").arg(doc_options.getTestbenchVHDLPath()));
          return false;
        }
        if (!testvector_out) {
          Error::warningOk(tr("Unable to open file %1!").arg(doc_options.getTestvectorASCIIPath()));
          return false;
        }
        if (!package_out) {
          Error::warningOk(tr("Unable to open file %1!").arg(doc_options.getTestpackageVHDLPath()));
          return false;
        }
        break;
      } else
        return true;
    }

    result = ExportVHDL::checkMachineNames(m_project->machine, &doc_options, &invalidNames);
    if (!result) {
      errorMessage = tr("Export of file %1 failed!").arg(fileio->getActExportFileName()) + "\n\n" +
                     tr("The following identifiers do not match the VHDL syntax:") + "\n";
      errorMessage += invalidNames.join("\n");
      Error::warningOk(errorMessage);

      statusbar->showMessage(tr("Export of file") + " " + fileio->getActExportFileName() + " " + tr("failed."), 2000);
      delete testvector_out;
      delete testbench_out;
      delete package_out;
      return false;
    }

    m_project->machine->updateDefaultTransitions();

    ExportTestbenchVHDL* exportTestbench = new ExportTestbenchVHDL(&doc_options);
    ExportTestvectorASCII* exportTestvector = new ExportTestvectorASCII(&doc_options);

    exportTestbench->init(testbench_out, package_out, m_project->machine, doc_options.getTestbenchVHDLPath(), NULL);
    exportTestvector->init(testvector_out, m_project->machine, doc_options.getTestvectorASCIIPath(), NULL);

    exportTestbench->doExport();
    exportTestvector->doExport();

    delete exportTestbench;
    delete exportTestvector;
    delete testvector_out;
    delete testbench_out;
    delete package_out;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    m_project->machine->updateDefaultTransitions();
    ExportVerilog* exp = new ExportVerilog(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    m_project->machine->updateDefaultTransitions();
    ExportKISS* exp = new ExportKISS(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    m_project->machine->updateDefaultTransitions();
    updateVVVV();
    vvvv_export->show();
    /*
    if (result)
      statusbar->showMessage(tr("File")+" "+ fileio->getActExportFileName() + " " +
          tr("exported."), 2000);

          */
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

    m_project->machine->updateDefaultTransitions();
    ExportSCXML* exp = new ExportSCXML(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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
  exp->init(&ostr, m_project->machine);
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

    TableBuilderASCII* tb = new TableBuilderASCII(this, m_project->machine, &doc_options);
    ExportStateTable* exp = new ExportStateTable(&doc_options, tb);
    result = fileio->exportFile(m_project, exp);
    delete exp;
    delete tb;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    TableBuilderLatex* tb = new TableBuilderLatex(this, m_project->machine, &doc_options);
    ExportStateTable* exp = new ExportStateTable(&doc_options, tb);
    result = fileio->exportFile(m_project, exp);
    delete exp;
    delete tb;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    TableBuilderHTML* tb = new TableBuilderHTML(this, m_project->machine, &doc_options);
    ExportStateTable* exp = new ExportStateTable(&doc_options, tb);
    result = fileio->exportFile(m_project, exp);
    delete exp;
    delete tb;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    m_project->machine->updateDefaultTransitions();
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
        if (Error::warningOkCancel(tr("File %1 exists. Do you want to overwrite it?").arg(act_file)) != QMessageBox::Ok)
          create_action_file = false;
      }
    }

    if (create_action_file)
      exp->writeActionFile(act_file.toStdString().c_str(), fileio->getActExportFileName().toStdString().c_str());

    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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

    m_project->machine->updateDefaultTransitions();
    ExportSMC* exp = new ExportSMC(&doc_options);
    result = fileio->exportFile(m_project, exp);
    delete exp;

    if (result)
      statusbar->showMessage(tr("File") + " " + fileio->getActExportFileName() + " " + tr("exported."), 2000);

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
  /*
  if (m_project && m_project->hasChanged())
  {
    switch(mb_changed->exec())
    {
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
  */
  close();
  //  emit quitWindow(this);
}

/*
/// Opens a new window
void MainWindow::fileNewWindow()
{
//  MainWindow* wmain;

//  wmain = new MainWindow();
//  wmain->setLanguage(getLanguage());
//  wmain->resize(600, 500);
//  wmain->show();
  m_control->newWindow(getLanguage());
}
*/

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

    delete m_project;
    m_project = NULL;

    m_mainView->getDrawArea()->resetState();
    m_mainView->updateBackground();
    m_mainView->getDrawArea()->getSelection()->clear();
    m_mainView->getDrawArea()->repaint();

    setMode(DocStatus::Select);
    simulator->closeDlg();

    statusbar->showMessage(tr("File") + " " + fileio->getActFileName() + " " + tr("closed."), 2000);

    updateAll();

    return true;
  }
  return false;
}

/// Undo the last action.
void MainWindow::editUndo()
{
  if (m_project) {
    m_project->getUndoBuffer()->undo();
    updateAll();
    m_mainView->widget()->repaint();
  }
}

/// Cuts the selected objects from the current machine and puts it on the
/// clipboard
void MainWindow::editCut()
{
  int count = m_mainView->getDrawArea()->getSelection()->count();

  bcut = true;
  editCopy();
  editDelete();
  bcut = false;

  if (count == 1)
    statusbar->showMessage(QString::number(count) + " " + tr("object cut."), 2000);
  else
    statusbar->showMessage(QString::number(count) + " " + tr("objects cut."), 2000);

  updatePaste();
}

/// Copies the selected objects to the clipboard
void MainWindow::editCopy()
{
  QString data;

  if (!m_project)
    return;

  if (!edit->copy(m_mainView->getDrawArea()->getSelection(), m_project, m_project->machine, data))
    return;
  //  qDebug(data);

  QClipboard* cb = QApplication::clipboard();

  MimeMachine* mm = new MimeMachine(data);
  //  QTextDrag* td = new QTextDrag(data);
  cb->setMimeData(mm);

  if (!bcut) {
    int count = m_mainView->getDrawArea()->getSelection()->count();
    if (count == 1)
      statusbar->showMessage(QString::number(count) + " " + tr("object copied."), 2000);
    else
      statusbar->showMessage(QString::number(count) + " " + tr("objects copied."), 2000);
  }

  //  qDebug(cb->data()->format());
  updatePaste();
}

/// Pastes the objects on the clipboard into the current machine
void MainWindow::editPaste()
{
  if (!m_project)
    return;

  QString format;
  //  QByteArray cbdata;
  QString data;
  MimeMachine* mm;

  QClipboard* cb = QApplication::clipboard();

  format = cb->mimeData()->formats().first();

  //  qDebug(format);
  if (format != "text/qfsm-objects")
    return;

  mm = (MimeMachine*)cb->mimeData();
  //  data = cb->text();

  data = QString(mm->data("text/qfsm-objects"));

  //  QString data = QString(cbdata);
  //  qDebug(data);
  //  data = cb->text();

  if (data.isEmpty())
    return;

  m_mainView->getDrawArea()->getSelection()->deselectAll(m_project->machine);

  if (edit->paste(m_mainView->getDrawArea()->getSelection(), m_project, m_project->machine, data)) {
    emit objectsPasted();
    m_project->setChanged();
  }

  int count = m_mainView->getDrawArea()->getSelection()->count();
  if (count == 1)
    statusbar->showMessage(QString::number(count) + " " + tr("object pasted."), 2000);
  else
    statusbar->showMessage(QString::number(count) + " " + tr("objects pasted."), 2000);

  m_mainView->widget()->repaint();
  updateAll();
}

/// Delete the selected objects.
void MainWindow::editDelete()
{
  int count = m_mainView->getDrawArea()->getSelection()->count();

  edit->deleteSelection(m_mainView->getDrawArea()->getSelection(), m_project->machine);

  if (!bcut) {
    if (count == 1)
      statusbar->showMessage(QString::number(count) + " " + tr("object deleted."), 2000);
    else
      statusbar->showMessage(QString::number(count) + " " + tr("objects deleted."), 2000);
  }

  m_project->setChanged();
  updateAll();
  m_mainView->widget()->repaint();
}

/// Set select mode.
void MainWindow::editSelect()
{
  setMode(DocStatus::Select);
}

/// Called when 'Edit->Select all' is clicked
void MainWindow::editSelectAll()
{
  DRect bound;
  if (m_mainView->getDrawArea()->getSelection()->selectAll(m_project->machine, bound))
    emit allSelected();
  m_mainView->getDrawArea()->setSelectionRect(bound);
  updateAll();
}

/// Called when 'Edit->Deselect all' is clicked
void MainWindow::editDeselectAll()
{
  m_mainView->getDrawArea()->getSelection()->deselectAll(m_project->machine);
  m_mainView->widget()->repaint();
  updateAll();
}

/// Edit options.
void MainWindow::editOptions()
{
  if (tabwidgetdialog->exec()) {
    doc_options.applyOptions(this);
    fileio->saveOptions(&doc_options);
    m_mainView->widget()->repaint();
  }
}

/// Initiate a drag operation and process the drop result
bool MainWindow::runDragOperation(bool force_copy)
{
  bool ret = false;
  QString data;

  if (edit->copy(this->m_mainView->getDrawArea()->getSelection(), m_project, m_project->machine, data)) {
    MimeMachine* mm = new MimeMachine(data);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mm);

    Qt::DropAction dropAction;
    if (force_copy)
      dropAction = drag->exec(Qt::CopyAction);
    else
      dropAction = drag->exec(Qt::CopyAction);

    switch (dropAction) {
      case Qt::IgnoreAction:
        qDebug("Drag action ignored");
        break;
      case Qt::CopyAction:
      case Qt::LinkAction:
        qDebug("Drag action finished");
        ret = true;
        break;
      case Qt::MoveAction:
      case Qt::TargetMoveAction:
        qDebug("Drag action finished, deleting data");
        ret = true;
        this->editDelete();
        break;
    }
  }
  return ret;
}

/// Toggle view state encoding
void MainWindow::viewStateEncoding()
{
  doc_options.setViewStateEncoding(!doc_options.getViewStateEncoding());
  updateAll();
  m_mainView->widget()->repaint();
}

/// Toggle view moore outputs.
void MainWindow::viewMooreOutputs()
{
  doc_options.setViewMoore(!doc_options.getViewMoore());
  updateAll();
  m_mainView->widget()->repaint();
}

/// Toggle view mealy outputs.
void MainWindow::viewMealyOutputs()
{
  doc_options.setViewMealyOut(!doc_options.getViewMealyOut());
  updateAll();
  m_mainView->widget()->repaint();
}

/// Toggle view mealy inputs.
void MainWindow::viewMealyInputs()
{
  doc_options.setViewMealyIn(!doc_options.getViewMealyIn());
  updateAll();
  m_mainView->widget()->repaint();
}

/// Toggle view grid.
void MainWindow::viewGrid()
{
  QString str;
  doc_options.setViewGrid(!doc_options.getViewGrid());
  if (doc_options.getViewGrid())
    str = tr("on");
  else
    str = tr("off");
  statusbar->showMessage(tr("Grid is %1.").arg(str), 2000);

  updateAll();
  m_mainView->widget()->repaint();
}

/// Toggle view shadows.
void MainWindow::viewShadows()
{
  doc_options.setStateShadows(!doc_options.getStateShadows());
  updateAll();
  m_mainView->widget()->repaint();
}

/// Set panning mode
void MainWindow::viewPan()
{
  //  QDragEnterEvent*ev=new QDragEnterEvent(QPoint(0,0),Qt::CopyAction,new
  //  QMimeData(),Qt::LeftButton,Qt::NoModifier); QPaintEvent*ev=new
  //  QPaintEvent(QRect(0,0,100,100)); QApplication::postEvent(m_mainView,ev);
  m_mainView->widget()->repaint();

  setMode(DocStatus::Pan);
}

/// Set zooming mode
void MainWindow::viewZoom()
{
  setMode(DocStatus::Zooming);
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
  doc_options.setViewIOView(!doc_options.getViewIOView());

  if (doc_options.getViewIOView() && m_project)
    view_io->show();
  else
    view_io->hide();

  updateMenuBar();
}

/// Update IOView text
void MainWindow::updateIOView(Machine* m)
{
  if (m != NULL) {
    view_io->updateIOList(m);

    if (doc_options.getViewIOView())
      view_io->show();
    else
      view_io->hide();

    updateMenuBar();
  }
}

/// Edit the current machine.
void MainWindow::machineEdit()
{
  machinemanager->editMachine(m_project);
  updateAll();
  m_mainView->widget()->repaint();
}

/// Automatically correct the state codes of the machine.
void MainWindow::machineCorrectCodes()
{
  m_project->machine->correctCodes();
  updateAll();
  m_mainView->widget()->repaint();
}

/// Simulate the current machine.
void MainWindow::machineSimulate()
{
  if (getMode() != DocStatus::Simulating) {
    if (simulator->startSimulation(m_project->machine)) {
      setMode(DocStatus::Simulating);
      m_mainView->widget()->repaint();
    }
  } else {
    simulator->stopSimulation();
    setMode(DocStatus::Select);
  }
  updateAll();
}

/// Called when 'Machine->Check Integrity' is clicked
void MainWindow::machineICheck()
{
  if (m_project && m_project->machine) {
    QCursor oldcursor1 = cursor();
    QCursor oldcursor2 = m_mainView->viewport()->cursor();
    setCursor(Qt::WaitCursor);
    m_mainView->viewport()->setCursor(Qt::WaitCursor);

    statusbar->showMessage(tr("Checking machine..."));
    m_project->machine->checkIntegrity(ichecker);
    statusbar->showMessage(tr("Check finished."), 2000);

    setCursor(oldcursor1);
    m_mainView->viewport()->setCursor(oldcursor2);
  }
}

/// Add new state to current machine.
void MainWindow::stateNew()
{
  setMode(DocStatus::NewState);
}

/// Edit selected state.
void MainWindow::stateEdit()
{
  GState* s;
  int otype;
  s = (GState*)m_mainView->getDrawArea()->getContextObject(otype);

  if (!s)
    s = m_mainView->getDrawArea()->getSelection()->getSList().front();

  if (s && m_project)
    statemanager->editState(s);
}

/// Set selected state as initial state.
void MainWindow::stateSetInitial()
{
  GState* s;
  int otype;
  s = (GState*)m_mainView->getDrawArea()->getContextObject(otype);

  if (!s)
    s = m_mainView->getDrawArea()->getSelection()->getSList().front();

  if (s && m_project) {
    statemanager->setInitialState(m_project->machine, s);
    updateAll();
    m_mainView->widget()->repaint();
  }
}

/// Set selected state as final state.
void MainWindow::stateSetFinal()
{
  // GState* s;
  //  int otype;
  // s = NULL; //(GState*)m_mainView->getContextObject(otype);
  Machine* m;

  if (!m_project)
    return;
  m = m_project->machine;
  if (!m)
    return;

  statemanager->setFinalStates(m, m_mainView->getDrawArea()->getSelection()->getSList());

  updateAll();
  m_mainView->widget()->repaint();

  /*
  if (!s)
    s = m_mainView->getSelection()->getSList().getFirst();

  if (s && m_project)
  {
    statemanager->setEndState(m_project->machine, s);
    updateAll();
    m_mainView->viewport()->repaint();
  }
  */
}

/// Add new transition.
void MainWindow::transNew()
{
  setMode(DocStatus::NewTransition);
}

/// Edit selected transition.
void MainWindow::transEdit()
{
  GTransition* t;
  int otype;
  t = (GTransition*)m_mainView->getDrawArea()->getContextObject(otype);

  if (!t)
    t = m_mainView->getDrawArea()->getSelection()->getTList().front();

  if (t && m_project)
    transmanager->editTransition(m_project->machine, t);
}

/// Straighten selected transitions.
void MainWindow::transStraighten()
{
  GTransition* t;
  //  int otype;
  t = NULL; //(GTransition*)m_mainView->getContextObject(otype);
  if (t) {
    m_project->getUndoBuffer()->changeTransition(t);
    t->straighten();
  } else
    transmanager->straightenSelection(&m_mainView->getDrawArea()->getSelection()->getTList());

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
  qfsm::AppInfo::about(this);
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
