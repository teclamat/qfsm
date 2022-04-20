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

#include "FileIO.h"

#include "gui/error.hpp"

#include "Convert.h"
#include "Export.h"
#include "GState.h"
#include "IOInfoASCII.h"
#include "IOInfoText.h"
#include "Import.h"
#include "Machine.h"
#include "MainWindow.h"
#include "Project.h"
#include "TransitionInfoASCII.h"
#include "TransitionInfoBin.h"
#include "TransitionInfoText.h"
#include "XMLHandler.h"

#include <QColor>
#include <QDebug>
#include <QFileDialog>
#include <QMap>
#include <QTextStream>
#include <QWidget>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QtGlobal>
namespace Qt {
static auto endl = ::endl;
static auto bin = ::bin;
static auto dec = ::dec;
} // namespace Qt
#endif

/**
 * Constructor.
 * Initialises the FileIO object with the parent @a parent.
 */
FileIO::FileIO(QWidget* parent)
  : QObject(parent)
{
  filedlg = new QFileDialog(parent, "", act_dir.dirName(), "Finite State Machine (*.fsm)");
  filedlg->setFileMode(QFileDialog::AnyFile);
  filedlg->setAcceptMode(QFileDialog::AcceptSave);
  filedlg->setOption(QFileDialog::DontConfirmOverwrite);

  importdlg = new QFileDialog(parent, "", act_dir.dirName(), "");
  importdlg->setFileMode(QFileDialog::AnyFile);
  importdlg->setAcceptMode(QFileDialog::AcceptSave);
  importdlg->setOption(QFileDialog::DontConfirmOverwrite);

  //  exportdlg = new Q3FileDialog(act_dir.dirName(), QString::null, parent,
  //  "exportdlg", true); exportdlg->setMode(Q3FileDialog::AnyFile);
  exportdlg = new QFileDialog(parent, "", act_dir.dirName(), "");
  exportdlg->setFileMode(QFileDialog::AnyFile);
  exportdlg->setAcceptMode(QFileDialog::AcceptSave);
  exportdlg->setOption(QFileDialog::DontConfirmOverwrite);

  mb_statecode = new QMessageBox("qfsm",
                                 tr("The file cannot be saved because of incorrect state codes. Do you "
                                    "want to correct that?"),
                                 QMessageBox::Critical, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                                 QMessageBox::Cancel | QMessageBox::Escape, parent);
  mb_statecode->setButtonText(QMessageBox::Yes, tr("Yes"));
  mb_statecode->setButtonText(QMessageBox::No, tr("No"));
  mb_statecode->setButtonText(QMessageBox::Cancel, tr("Cancel"));

  act_file = QString{};
  act_exportfile = QString{};
  act_export_dir = QString{};
  main = (MainWindow*)parent;
}

/// Destructor
FileIO::~FileIO() {}

/**
 * Opens a 'fsm'-file.
 * Opens the '.fsm' file @a mrufile.
 * If @a mrufile is null a file dialog is opened first.
 */
qfsm::Project* FileIO::openFileXML(QString mrufile /*=QString::null*/)
{
  qfsm::Project* p = NULL;
  //  filedlg->setMode(Q3FileDialog::ExistingFile);
  filedlg->setAcceptMode(QFileDialog::AcceptOpen);
  filedlg->setFileMode(QFileDialog::ExistingFile);

  if (mrufile.isNull()) {
    if (!filedlg->exec()) {
      act_file = QString{};
      return p;
    }
    act_file = filedlg->selectedFiles().first();
  } else {
    if (!QFile::exists(mrufile))
      return NULL;

    act_file = mrufile;
  }

  emit sbMessage(tr("File loading..."));
  QFile file(act_file);
  if (!file.open(QFile::ReadOnly))
    return NULL;

  p = new qfsm::Project(main);
  XMLHandler handler(p);
  handler.setDevice(&file);
  // QXmlInputSource source(file);
  // QXmlSimpleReader reader;

  // reader.setContentHandler(&handler);
  emit setWaitCursor();
  if (handler.parse()) {
    file.close();
    emit setPreviousCursor();
    return p;
  }
  file.close();

  p->machine()->updateDefaultTransitions();

  emit setPreviousCursor();
  return NULL;
}

/**
 * Save project as 'fsm'-file with a new name.
 * Asks for a file name and saves the project @a p as a 'fsm'-file.
 */
bool FileIO::saveFileAs(qfsm::Project* p)
{
  if (!p->machine()->checkStateCodes()) {
    switch (mb_statecode->exec()) {
      case QMessageBox::Yes:
        break;
      case QMessageBox::No:
        return false;
        break;
      case QMessageBox::Cancel:
        return false;
        break;
    }
  }
  //  filedlg->setMode(Q3FileDialog::AnyFile);
  filedlg->setAcceptMode(QFileDialog::AcceptSave);
  filedlg->setFileMode(QFileDialog::AnyFile);
  if (!act_file.isEmpty())
    //    filedlg->setSelection(act_file);
    filedlg->selectFile(act_file);

  if (filedlg->exec()) {
    act_file = filedlg->selectedFiles().first();

    QString name = act_file.right(act_file.length() - act_file.lastIndexOf("/"));
    if (name.right(4) != ".fsm")
      act_file.append(".fsm");

    if (QFile::exists(act_file)) {
      if (qfsm::gui::error::warn(tr("File exists. Do you want to overwrite it?"), qfsm::gui::error::Button::Cancel) !=
          QMessageBox::Ok)
        return false;
    }

    return doSaveXML(p);
  }
  return false;
}

/**
 * Saves project as 'fsm'-file.
 * If no file name is given, it first asks for the file name.
 */
bool FileIO::saveFile(qfsm::Project* p)
{
  if (!p->machine()->checkStateCodes()) {
    switch (mb_statecode->exec()) {
      case QMessageBox::Yes:
        break;
      case QMessageBox::No:
        return false;
      case QMessageBox::Cancel:
        return false;
    }
  }
  if (act_file.isEmpty())
    return saveFileAs(p);
  else
    return doSaveXML(p);
}

/**
 * Performs the actual saving of project @a p in XML format.
 */
bool FileIO::doSaveXML(qfsm::Project* p)
{
  Machine* m = p->machine();
  if (!m)
    return false;

  QFile file(act_file);
  if (!file.open(QIODevice::WriteOnly)) {
    qfsm::gui::error::info(tr("File cannot be written."));
    qDebug("file cannot be opened for writing");
    return false;
  }

  p->saveTo(&file);

  file.close();

  p->setChanged(false);

  return true;
}

/**
 * Saves the application options.
 * Saves options @a opt in the qfsmrc-file which is in $HOME/.qfsm/.
 */
int FileIO::saveOptions(Options* opt)
{
  int result = 0;
  QString stmp;
  QDir dir = QDir::home();

  QDir qfsmdir = createQfsmDir();

  QFile file(qfsmdir.absolutePath() + "/qfsmrc");
  if (!file.open(QIODevice::WriteOnly)) {
    qDebug("options not saved");
    return 1;
  }

  QTextStream fout(&file);

  fout << "view_stateenc " << (int)opt->getViewStateEncoding() << Qt::endl;
  fout << "view_moore " << (int)opt->getViewMoore() << Qt::endl;
  fout << "view_mealyin " << (int)opt->getViewMealyIn() << Qt::endl;
  fout << "view_mealyout " << (int)opt->getViewMealyOut() << Qt::endl;
  fout << "view_grid " << (int)opt->getViewGrid() << Qt::endl;
  fout << "view_ioview " << (int)opt->getViewIOView() << Qt::endl;

  fout << "state_shadows " << (int)opt->getStateShadows() << Qt::endl;
  fout << "state_shadow_color " << (unsigned int)opt->getStateShadowColor().rgb() << Qt::endl;

  fout << "grid_size " << opt->getGridSize() << Qt::endl;
  fout << "grid_color " << (unsigned int)opt->getGridColor().rgb() << Qt::endl;

  fout << "tooltips " << (int)opt->getToolTips() << Qt::endl;
  fout << "iomark " << (int)opt->getIOMark() << Qt::endl;
  fout << "ionames " << (int)opt->getDisplayIONames() << Qt::endl;
  fout << "drawbox " << (int)opt->getDrawBox() << Qt::endl;

  stmp = opt->getInitialDescriptor().trimmed();
  if (stmp.isEmpty())
    stmp = getEmptyFieldString();
  fout << "initial_descriptor " << stmp << Qt::endl;
  stmp = opt->getInversionDescriptor().trimmed();
  if (stmp.isEmpty())
    stmp = getEmptyFieldString();
  fout << "inversion_descriptor " << stmp << Qt::endl;
  stmp = opt->getAnyInputDescriptor().trimmed();
  if (stmp.isEmpty())
    stmp = getEmptyFieldString();
  fout << "any_input_descriptor " << stmp << Qt::endl;
  stmp = opt->getDefaultTransitionDescriptor().trimmed();
  if (stmp.isEmpty())
    stmp = getEmptyFieldString();
  fout << "default_descriptor " << stmp << Qt::endl;

  fout << "ahdl_sync_reset " << (int)opt->getAHDLSyncReset() << Qt::endl;
  fout << "ahdl_use_moore " << (int)opt->getAHDLUseMoore() << Qt::endl;

  fout << "vhdl_symbolic_states " << (int)opt->getVHDLSymbolicStates() << Qt::endl;
  fout << "vhdl_debug_state " << (int)opt->getVHDLDebugState() << Qt::endl;
  fout << "vhdl_sync_reset " << (int)opt->getVHDLSyncReset() << Qt::endl;
  fout << "vhdl_sync_enable " << (int)opt->getVHDLSyncEnable() << Qt::endl;
  fout << "vhdl_stdlogic " << (int)opt->getVHDLStdLogic() << Qt::endl;
  fout << "vhdl_io_names " << (int)opt->getVHDLInOutNames() << Qt::endl;
  fout << "vhdl_neg_reset " << (int)opt->getVHDLNegReset() << Qt::endl;
  fout << "vhdl_io_header " << (int)opt->getVHDLIOheader() << Qt::endl;
  fout << "vhdl_alliance " << (int)opt->getVHDLAlliance() << Qt::endl;
  fout << "vhdl_cond_notation " << (int)opt->getVHDLCondNotation() << Qt::endl;
  fout << "vhdl_state_code " << (int)opt->getVHDLStateCode() << Qt::endl;
  fout << "vhdl_sync_look_ahead " << (int)opt->getVHDLSyncLookAhead() << Qt::endl;
  fout << "vhdl_sep_files " << (int)opt->getVHDLSepFiles() << Qt::endl;
  stmp = opt->getVHDLArchitectureName().trimmed();
  if (stmp.isEmpty())
    stmp = "behave";
  fout << "vhdl_architecture_name " << stmp << Qt::endl;

  fout << "testbench_stdlogic " << (int)opt->getTestbenchStdLogic() << Qt::endl;
  fout << "testbench_sync_reset " << (int)opt->getTestbenchSynchronousReset() << Qt::endl;
  fout << "testbench_sync_enable " << (int)opt->getTestbenchSynchronousEnable() << Qt::endl;
  fout << "testbench_negated_reset " << (int)opt->getTestbenchNegatedReset() << Qt::endl;
  fout << "testbench_io_header " << (int)opt->getTestbenchIOHeader() << Qt::endl;
  fout << "testbench_io_names " << (int)opt->getTestbenchIONames() << Qt::endl;
  stmp = opt->getTestbenchVHDLPath().trimmed();
  if (stmp.isEmpty())
    stmp = getEmptyFieldString();
  fout << "testbench_vhdl_path " << stmp << Qt::endl;
  stmp = opt->getTestvectorASCIIPath().trimmed();
  if (stmp.isEmpty())
    stmp = getEmptyFieldString();
  fout << "testvector_ascii_path " << stmp << Qt::endl;
  stmp = opt->getTestpackageVHDLPath().trimmed();
  if (stmp.isEmpty())
    stmp = getEmptyFieldString();
  fout << "testpackage_vhdl_path " << stmp << Qt::endl;
  stmp = opt->getTestbenchLogfilePath().trimmed();
  if (stmp.isEmpty())
    stmp = getEmptyFieldString();
  fout << "testbench_logfile_path " << stmp << Qt::endl;
  stmp = opt->getTestbenchBaseDirectory().trimmed();
  if (stmp.isEmpty())
    stmp = getEmptyFieldString();
  fout << "testbench_base_directory " << stmp << Qt::endl;

  fout << "ver_sync_reset " << (int)opt->getVerilogSyncReset() << Qt::endl;
  // fout << "ver_register_out " << (int)opt->getVerilogRegisterOut() << endl;

  fout << "statetable_includeout " << (int)opt->getStateTableIncludeOut() << Qt::endl;
  fout << "statetable_resolve_inverted " << (int)opt->getStateTableResolveInverted() << Qt::endl;
  fout << "statetable_orientation " << (int)opt->getStateTableOrientation() << Qt::endl;

  fout << "ragel_create_action " << (int)opt->getRagelCreateAction() << Qt::endl;
  fout << "ragel_lang_action " << (int)opt->getRagelLangAction() << Qt::endl;
  fout << "ragel_default_transitions " << (int)opt->getRagelDefaultTransitions() << Qt::endl;
  fout << "vvvv_reset " << (int)opt->getVVVVReset() << Qt::endl;
  fout << "vvvv_reset_event " << opt->getVVVVResetEvent() << Qt::endl;
  fout << "vvvv_reset_action " << opt->getVVVVResetAction() << Qt::endl;
  fout << "print_header " << (int)opt->getPrintHeader() << Qt::endl;

  fout << Qt::endl;

  file.close();

  return result;
}

/**
 * Loads the application options.
 * Loads the options from $HOME/.qfsm/qfsmrc into @a opt.
 */
int FileIO::loadOptions(Options* opt)
{
  int result = 0;
  QDir dir = QDir::home();
  QMap<QString, QString> _map;
  QString key, value;

#ifdef Q_OS_WIN
  QFile file(dir.absolutePath() + "/Application Data/qfsm/qfsmrc");
#else
  QFile file(dir.absolutePath() + "/.qfsm/qfsmrc");
#endif

  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "options not loaded";
    return 1;
  }

  QTextStream fin(&file);

  fin >> key >> value;

  while (!fin.atEnd()) {
    _map.insert(key, value);
    fin >> key >> value;
    if (value == getEmptyFieldString())
      value = "";
  }

  setOptions(&_map, opt);

  return result;
}

/**
 * Sets the loaded application options.
 * @param map pairs of strings which contain the options
 * @param opt options object where the options will be saved
 */
void FileIO::setOptions(QMap<QString, QString>* _map, Options* opt)
{
  QMap<QString, QString>::Iterator it;
  QString key, data;
  int idata;
  unsigned int uidata;

  for (it = _map->begin(); it != _map->end(); ++it) {
    key = it.key();
    data = it.value();
    if (data == getEmptyFieldString())
      data = "";
    idata = data.toInt();
    uidata = data.toUInt();

    if (key == "view_stateenc")
      opt->setViewStateEncoding(idata);
    else if (key == "view_moore")
      opt->setViewMoore(idata);
    else if (key == "view_mealyin")
      opt->setViewMealyIn(idata);
    else if (key == "view_mealyout")
      opt->setViewMealyOut(idata);
    else if (key == "view_grid")
      opt->setViewGrid(idata);
    else if (key == "view_ioview")
      opt->setViewIOView(idata);
    else if (key == "state_shadows")
      opt->setStateShadows(idata);
    else if (key == "state_shadow_color") {
      QColor c;
      c.setRgb((QRgb)uidata);
      opt->setStateShadowColor(c);
    } else if (key == "grid_size")
      opt->setGridSize(idata);
    else if (key == "grid_color") {
      QColor c;
      c.setRgb((QRgb)uidata);
      opt->setGridColor(c);
    } else if (key == "tooltips")
      opt->setToolTips(idata);
    else if (key == "iomark")
      opt->setIOMark(idata);
    else if (key == "ionames")
      opt->setDisplayIONames(idata);
    else if (key == "drawbox")
      opt->setDrawBox(idata);
    else if (key == "initial_descriptor" || key == "start_descriptor")
      opt->setInitialDescriptor(data);
    else if (key == "inversion_descriptor")
      opt->setInversionDescriptor(data);
    else if (key == "any_input_descriptor")
      opt->setAnyInputDescriptor(data);
    else if (key == "default_descriptor")
      opt->setDefaultTransitionDescriptor(data);
    else if (key == "ahdl_sync_reset")
      opt->setAHDLSyncReset(idata);
    else if (key == "ahdl_use_moore")
      opt->setAHDLUseMoore(idata);
    else if (key == "vhdl_symbolic_states")
      opt->setVHDLSymbolicStates(idata);
    else if (key == "vhdl_debug_state")
      opt->setVHDLDebugState(idata);
    else if (key == "vhdl_sync_reset")
      opt->setVHDLSyncReset(idata);
    else if (key == "vhdl_stdlogic")
      opt->setVHDLStdLogic(idata);
    else if (key == "vhdl_sync_reset")
      opt->setVHDLSyncReset(idata);
    else if (key == "vhdl_stdlogic")
      opt->setVHDLStdLogic(idata);
    else if (key == "vhdl_sync_look_ahead")
      opt->setVHDLSyncLookAhead(idata);
    else if (key == "vhdl_sep_files")
      opt->setVHDLSepFiles(idata);
    else if (key == "default_architecture_name")
      opt->setVHDLArchitectureName(data);
    else if (key == "vhdl_fha") // deprecated
    {
      if (idata == 1) {
        opt->setVHDLInOutNames(true);
        opt->setVHDLNegReset(true);
        opt->setVHDLIOheader(true);
        opt->setVHDLAlliance(false);
        opt->setVHDLCondNotation(1);
      }
    } else if (key == "vhdl_io_names")
      opt->setVHDLInOutNames(idata);
    else if (key == "ver_sync_reset")
      opt->setVerilogSyncReset(idata);
    else if (key == "vhdl_sync_enable")
      opt->setVHDLSyncEnable(idata);
    else if (key == "vhdl_neg_reset")
      opt->setVHDLNegReset(idata);
    else if (key == "vhdl_io_header")
      opt->setVHDLIOheader(idata);
    else if (key == "vhdl_alliance")
      opt->setVHDLAlliance(idata);
    else if (key == "vhdl_state_code")
      opt->setVHDLStateCode(idata);
    else if (key == "vhdl_cond_notation")
      opt->setVHDLCondNotation(idata);
    /*
    else if (key=="ver_register_out")
      opt->setVerilogRegisterOut(idata);
      */
    else if (key == "testbench_stdlogic")
      opt->setTestbenchStdLogic(idata);
    else if (key == "testbench_sync_reset")
      opt->setTestbenchSynchronousReset(idata);
    else if (key == "testbench_sync_enable")
      opt->setTestbenchSynchronousEnable(idata);
    else if (key == "testbench_negated_reset")
      opt->setTestbenchNegatedReset(idata);
    else if (key == "testbench_io_header")
      opt->setTestbenchIOHeader(idata);
    else if (key == "testbench_io_names")
      opt->setTestbenchIONames(idata);
    else if (key == "testbench_vhdl_path")
      opt->setTestbenchVHDLPath(data);
    else if (key == "testvector_ascii_path")
      opt->setTestvectorASCIIPath(data);
    else if (key == "testpackage_vhdl_path")
      opt->setTestpackageVHDLPath(data);
    else if (key == "testbench_logfile_path")
      opt->setTestbenchLogfilePath(data);
    else if (key == "testbench_base_directory")
      opt->setTestbenchBaseDirectory(data);
    else if (key == "statetable_includeout")
      opt->setStateTableIncludeOut(idata);
    else if (key == "statetable_resolve_inverted")
      opt->setStateTableResolveInverted(idata);
    else if (key == "statetable_orientation")
      opt->setStateTableOrientation(idata);
    else if (key == "ragel_create_action")
      opt->setRagelCreateAction(idata);
    else if (key == "ragel_lang_action")
      opt->setRagelLangAction(idata);
    else if (key == "ragel_default_transitions")
      opt->setRagelDefaultTransitions(idata);
    else if (key == "vvvv_reset")
      opt->setVVVVReset(idata);
    else if (key == "vvvv_reset_event")
      opt->setVVVVResetEvent(data);
    else if (key == "vvvv_reset_action")
      opt->setVVVVResetAction(data);
    else if (key == "print_header")
      opt->setPrintHeader(idata);
  }
}

/**
 * Imports a file
 * Opens a file dialog, imports a file, and creates a new project
 */
qfsm::Project* FileIO::importFile(Import* imp, ScrollView* sv /*=NULL*/)
{
  using namespace std;

  QString ext;

  if (!imp)
    return NULL;

  qfsm::Project* p = NULL;
  importdlg->setAcceptMode(QFileDialog::AcceptOpen);
  importdlg->setFileMode(QFileDialog::ExistingFile);
  importdlg->setNameFilters({ imp->fileFilter(), "All Files (*)" });

  if (!importdlg->exec()) {
    act_importfile = QString{};
    return p;
  }
  act_importfile = importdlg->selectedFiles().first();
  act_import_dir = importdlg->directory().absolutePath();

  QString name = act_importfile.right(act_importfile.length() - act_importfile.lastIndexOf("/"));
  if (name.right(1 + imp->defaultExtension().length()) != QString("." + imp->defaultExtension()))
    act_importfile.append("." + imp->defaultExtension());

  // p = new Project(main);

  ifstream fin(act_importfile.toStdString());

  if (!fin)
    return NULL;

  emit setWaitCursor();

  imp->init(&fin, main, act_importfile, sv);
  p = imp->doImport();

  emit setPreviousCursor();

  return p;
}

/**
 * Exports the actual file.
 * Opens a file dialog and exports the actual machine into the specified format.
 */
bool FileIO::exportFile(qfsm::Project* p, Export* exp, ScrollView* sv /*=NULL*/)
{
  using namespace std;

  QString ext;

  if (!p || !exp)
    return false;

  if (!exp->validateMachine(p->machine()))
    return false;

  //  exportdlg->setMode(Q3FileDialog::AnyFile);
  exportdlg->setFileMode(QFileDialog::AnyFile);
  if (!act_exportfile.isEmpty())
    //    exportdlg->setSelection(act_exportfile);
    exportdlg->selectFile(act_exportfile);
  else
    exportdlg->selectFile(p->machine()->getName());

  exportdlg->setNameFilters({ exp->fileFilter(), "All Files (*)" });

  if (exportdlg->exec()) {
    act_exportfile = exportdlg->selectedFiles().first();
    //    act_export_dir = exportdlg->dirPath();
    act_export_dir = exportdlg->directory().absolutePath();

    QString name = act_exportfile.right(act_exportfile.length() - act_exportfile.lastIndexOf("/"));
    if (name.right(1 + exp->defaultExtension().length()) != QString("." + exp->defaultExtension()))
      act_exportfile.append("." + exp->defaultExtension());

    QFile ftmp(act_exportfile);
    if (ftmp.exists()) {
      if (qfsm::gui::error::warn(tr("File exists. Do you want to overwrite it?"), qfsm::gui::error::Button::Cancel) !=
          QMessageBox::Ok)
        return false;
    }

    ofstream fout(act_exportfile.toStdString());

    if (!fout)
      return false;

    emit setWaitCursor();

    exp->init(&fout, p->machine(), act_exportfile, sv);
    exp->doExport();

    emit setPreviousCursor();

    return true;
  }
  return false;
}

QDir FileIO::createQfsmDir()
{
  QDir dir = QDir::home();
#ifdef Q_OS_WIN
  QDir qfsmdir(dir.absolutePath() + "/Application Data/qfsm");
  if (!qfsmdir.exists()) {
    QDir appdir(dir.absolutePath() + "/Application Data");
    if (!appdir.exists()) {
      if (!dir.mkdir("Application Data"))
        qDebug("Application Data not created");
    }
    if (!dir.mkdir("Application Data/qfsm"))
      qDebug("Application Data/qfsm not created");
  }
  return qfsmdir;
#else
  QDir qfsmdir(dir.absolutePath() + "/.qfsm");
  if (!qfsmdir.exists()) {
    if (!dir.mkdir(".qfsm"))
      qDebug(".qfsm not created");
  }
  return qfsmdir;
#endif
}
