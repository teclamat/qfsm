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
/*
Qt 4 Port by Rainer Strobel

added Qt:: to all constants
changed QList.getFirst calls to QList.front

modified method ExportVHDL to switch between
ExportVHDL and ExportVHDLFHA class

in constructor:
disabled all dialogs for testing
added menu item for Export VHDL FH Augsburg and Export IO description
the menu bar is requested from QMainWindow by calling menuBar()


in method updateMenuBar:
the setItemChecked and setItemEnabled calls for the menus
were now called for the sub-menus directly, instead of menubar,
because this did not work under qt 4

int method editCopy:
replaced cb->setData  by cb->setMimeData call, which is compatible
with Qt 4

in method editPaste:
replaced old code to access clipboard data by code, which uses
the Qt 4 class QMimeData

in method createToolBar:
The toolbar can be requested by addToolBar from QMainWindow.
All buttons must be added to the toolbar by calling addWidget.

*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QCursor>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QTabWidget>
#include <QToolBar>

#include "ExportAHDLDlg.h"
#include "ExportRagelDlg.h"
#include "ExportStateTableDlg.h"
#include "ExportTestbenchDlg.h"
#include "ExportTestbenchVHDL.h"
#include "ExportTestvectorASCII.h"
#include "ExportVHDLDlg.h"
#include "ExportVVVVDlg.h"
#include "ExportVerilogDlg.h"
#include "IOViewDlg.h"
#include "MachineManager.h"
#include "OptDisplayDlg.h"
#include "OptGeneralDlg.h"
#include "OptPrintingDlg.h"
#include "Options.h"
#include "OptionsDlg.h"
#include "ScrollView.h"
#include "StateManager.h"
#include "TransitionManager.h"

#include "gui/scene.hpp"
#include "gui/view.hpp"

// class QTranslator;
class QToolBar;
class FileIO;
class PrintManager;
class Simulator;
class ICheck;
class QAction;

namespace qfsm {
class MainControl;
class Project;
class OptionsManager;
namespace gui {
class ActionsManager;
class StatusBar;
} // namespace gui
} // namespace qfsm

enum class DocumentMode {
  Select,        ///< Select
  Pan,           ///< Pan view
  NewState,      ///< Add new states
  NewTransition, ///< Add new transitions
  Zooming,       ///< Zoom in/out
  Simulating     ///< Simultate the machine
};

/**
 * @class MainWindow
 * @brief The main window of the application.
 *
 * Stores all the menus, the status bar, the project, the file-io object, the
 * options and the control classes.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  MainWindow(QObject* a_parent = nullptr);
  ~MainWindow();

  /// Returns the options.
  Options* getOptions() { return &doc_options; }
  /// Returns the scroll view.
  ScrollView* getScrollView() { return m_mainView; }
  /// Returns the status bar.
  qfsm::MainControl* control() { return m_control; }
  qfsm::Project* project() { return m_project; }
  qfsm::OptionsManager* options() { return m_optionsManager; }
  qfsm::gui::View* view() { return m_view; }
  qfsm::gui::Scene* scene() { return m_view->scene(); }
  qfsm::gui::ActionsManager* actionsManager() { return m_actionsManager; }
  DocumentMode mode() const { return m_mode; }
  QMenu* contextCommon();
  /// Returns the tab dialog for the general options
  OptGeneralDlgImpl* getOptGeneral() { return opt_general; }
  /// Returns the tab dialog for the display options
  OptDisplayDlgImpl* getOptDisplay() { return opt_display; }
  /// Returns the tab dialog for the printing options
  OptPrintingDlgImpl* getOptPrinting() { return opt_printing; }
  /// Returns the AHDL export dialog
  ExportAHDLDlgImpl* getExportAHDL() { return ahdl_export; }
  /// Returns the VHDL export dialog
  ExportVHDLDlgImpl* getExportVHDL() { return vhdl_export; }
  /// Returns the VHDL export dialog
  ExportTestbenchDlgImpl* getExportTestbench() { return testbench_export; }
  /// Returns the Verilog export dialog
  ExportVerilogDlgImpl* getExportVerilog() { return ver_export; }
  /// Returns the State table export dialog
  ExportStateTableDlgImpl* getExportStateTable() { return statetable_export; }
  /// Returns the Ragel export dialog
  ExportRagelDlgImpl* getExportRagel() { return ragel_export; }
  /// Returns the VVVV export dialog
  ExportVVVVDlgImpl* getExportVVVV() { return vvvv_export; }
  /// Sets the string with the language
  void setLanguage(QString s)
  {
    language = s;
    opt_general->setLanguage(language);
  }
  /// Gets the string with the language
  QString getLanguage() { return language; }
  void updateIOView(Machine*);
  bool runDragOperation(bool force_copy);

  //    void repaintView() { wscroll->viewport()->repaint(); };
  /// Returns true if the shift key is pressed otherwise false
  bool shiftPressed() { return shift_pressed; }
  /// Returns true if the control key is pressed otherwise false
  bool controlPressed() { return control_pressed; }

  /// State manager
  StateManager* m_stateManager{};
  /// Machine manager
  MachineManager* m_machineManager{};
  /// Transition manager
  TransitionManager* m_transitionManager{};
  /// File I/O
  FileIO* fileio{};
  /// Print manager
  PrintManager* printmanager{};

  void setMode(DocumentMode a_mode);

  /// true if this window is about to close
  bool aboutToClose{};

 protected:
  virtual void keyPressEvent(QKeyEvent*);
  virtual void keyReleaseEvent(QKeyEvent*);
  virtual void closeEvent(QCloseEvent*);
  virtual void focusInEvent(QFocusEvent*);
  virtual void dragEnterEvent(QDragEnterEvent*);
  virtual void dropEvent(QDropEvent*);

 private:
  void createToolBar();

 private:
  qfsm::MainControl* m_control;
  qfsm::OptionsManager* m_optionsManager;
  qfsm::gui::StatusBar* m_statusBar;
  qfsm::gui::ActionsManager* m_actionsManager;
  qfsm::gui::View* m_view;
  ScrollView* m_mainView;

  qfsm::Project* m_project{ nullptr };

 private:
  /// Toolbar
  QToolBar* toolbar;
  /// Messagebox that is opend when the user wants to close a changed file
  QMessageBox* mb_changed;
  /// Options
  Options doc_options;

  DocumentMode m_mode{ DocumentMode::Select };

  OptionsDlg* tabwidgetdialog;
  /// General options dialog
  OptGeneralDlgImpl* opt_general;
  /// Display options dialog
  OptDisplayDlgImpl* opt_display;
  /// Printing options dialog
  OptPrintingDlgImpl* opt_printing;
  /// VHDL export options dialog
  ExportVHDLDlgImpl* vhdl_export;
  /// Verilog export options dialog
  ExportVerilogDlgImpl* ver_export;
  /// AHDL export options dialog
  ExportAHDLDlgImpl* ahdl_export;
  /// State table export options dialog
  ExportStateTableDlgImpl* statetable_export;
  /// Ragel export options dialog
  ExportRagelDlgImpl* ragel_export;
  /// Testbench export options dialog
  ExportTestbenchDlgImpl* testbench_export;
  /// Testbench export options dialog
  ExportVVVVDlgImpl* vvvv_export;

  /// IO view dialog
  IOViewDlgImpl* view_io;

  /// Simulator
  Simulator* simulator;
  /// Integrity checker
  ICheck* ichecker;

  /// If true the shift key was pressed
  bool shift_pressed;
  /// If true the control key was pressed
  bool control_pressed;

  /// true if a cut operation, false if a copy operation is performed (used in paste)
  bool m_isCutOperation;
  /// Language
  QString language;
  /// Previous view cursor (used when wait cursor is set)
  QCursor previous_viewcursor;

  QIcon appIcon{};

 signals:
  /// Emited when a set of objects has been pasted into this main window
  void objectsPasted();
  /// Emited when this main window is about to close
  void quitWindow(MainWindow*);
  /// Emited when the zoom factor changes
  void updateStatusZoom(int);
  /// Emited when the escape key has been pressed
  void escapePressed();

  void modeChanged(DocumentMode);

 public slots:
  void refreshMRU();
  void repaintViewport();
  void updateAll();
  void updateMenuBar();
  void updateTitleBar();
  void updateStatusBar();
  void updateVVVV();

  void showContextState();
  void showContextTrans();
  void showContext();

  void statusMessage(const QString& a_message);
  void statusMessage(const QString& a_message, int a_timeout);
  void setWaitCursor();
  void setPreviousCursor();

  void fileNew();
  void fileOpen();
  void fileOpen(const QString& a_fileName);
  bool fileSave();
  bool fileSaveAs();

  void fileImportGraphviz();

  bool fileExportEPS();
  bool fileExportSVG();
  bool fileExportPNG();
  bool fileExportAHDL();
  bool fileExportVHDL();
  bool fileExportIODescription();
  bool fileExportVerilog();
  bool fileExportKISS();
  bool fileExportSCXML();
  bool fileExportVVVV();
  bool fileExportTestbench();
  bool fileExportSTASCII();
  bool fileExportSTLatex();
  bool fileExportSTHTML();
  bool fileExportRagel();
  bool fileExportSMC();
  void filePrint();
  bool fileClose();
  void fileQuit();

  void editUndo();
  void editCut();
  void editCopy();
  void editPaste();
  void editDelete();
  void editSelect();
  void editSelectAll();
  void editDeselectAll();
  void editOptions();

  void viewStateEncoding();
  void viewMooreOutputs();
  void viewMealyInputs();
  void viewMealyOutputs();
  void viewGrid();
  void viewShadows();
  void viewPan();
  void viewZoomIn();
  void viewZoomOut();
  void viewZoom100();
  void viewIOView();

  void machineEdit();
  void machineSimulate();
  void machineICheck();
  void machineCorrectCodes();

  void stateNew();
  void stateEdit();
  void stateSetInitial();
  void stateSetFinal();

  void transNew();
  void transEdit();
  void transStraighten();

  void helpManual();
  void helpAbout();
  void helpAboutQt();
};

#endif
