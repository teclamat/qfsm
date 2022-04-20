cmake_minimum_required(VERSION 3.16)

project(qfsm
  VERSION 0.60
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core QUIET)
if(Qt6_FOUND)
  find_package(Qt6 COMPONENTS Core Widgets OpenGLWidgets Svg PrintSupport LinguistTools)
  set(QFSM_QT_LIBS Qt6::Core Qt6::Widgets Qt6::OpenGLWidgets Qt6::Svg Qt6::PrintSupport)
else()
  find_package(Qt5 COMPONENTS Core Widgets Svg PrintSupport LinguistTools)
  set(QFSM_QT_LIBS Qt5::Core Qt5::Widgets Qt5::Svg Qt5::PrintSupport)
endif()

set(QFSM_HEADERS
  src/gui/scene.hpp
  src/gui/stateitem.hpp
  src/gui/view.hpp
  src/gui/actionsmanager.hpp
  src/gui/transitionitem.hpp
  src/gui/controlhandleitem.hpp

  src/optionsmanager.hpp
  src/maincontrol.hpp
  src/info.hpp

  # src/ListBoxItems.h
  src/Const.h
  src/Convert.h
  src/DRect.h
  src/Draw.h
  src/DrawArea.h
  src/Edit.h
  src/Error.h
  src/Export.h
  src/ExportAHDL.h
  src/ExportAHDLDlg.h
  src/ExportEPS.h
  src/ExportIODescription.h
  src/ExportKISS.h
  src/ExportPNG.h
  src/ExportRagel.h
  src/ExportRagelDlg.h
  src/ExportSCXML.h
  src/ExportSMC.h
  src/ExportSVG.h
  src/ExportStateTable.h
  src/ExportStateTableDlg.h
  src/ExportTestbenchDlg.h
  src/ExportTestbenchVHDL.h
  src/ExportTestvectorASCII.h
  src/ExportVHDL.h
  src/ExportVHDLDlg.h
  src/ExportVVVV.h
  src/ExportVVVVDlg.h
  src/ExportVerilog.h
  src/ExportVerilogDlg.h
  src/FileIO.h
  src/GITransition.h
  src/GObject.h
  src/GState.h
  src/GTransition.h
  src/Grid.h
  src/ICheck.h
  src/ICheckDlg.h
  src/IOInfo.h
  src/IOInfoASCII.h
  src/IOInfoBin.h
  src/IOInfoList.h
  src/IOInfoText.h
  src/IOViewDlg.h
  src/Import.h
  src/ImportGraphviz.h
  src/Machine.h
  src/MachineManager.h
  src/MachinePropertiesDlg.h
  src/MainWindow.h
  src/OptDisplayDlg.h
  src/OptGeneral.h
  src/OptGeneralDlg.h
  src/OptPrintingDlg.h
  src/Options.h
  src/OptionsDlg.h
  src/PrintManager.h
  src/Project.h
  src/ScrollView.h
  src/Selection.h
  src/Simulator.h
  src/SimulatorDlg.h
  src/State.h
  src/StateManager.h
  src/StatePropertiesDlg.h
  src/StatusBar.h
  src/TableBuilder.h
  src/TableBuilderASCII.h
  src/TableBuilderHTML.h
  src/TableBuilderLatex.h
  src/TestvectorGenerator.h
  src/Transition.h
  src/TransitionInfo.h
  src/TransitionInfoASCII.h
  src/TransitionInfoBin.h
  src/TransitionInfoText.h
  src/TransitionManager.h
  src/TransitionPropertiesDlg.h
  src/Undo.h
  src/UndoBuffer.h
  src/Utils.h
  src/WSpinBox.h
  src/XMLHandler.h
  src/Zoom.h
)

set(QFSM_SOURCES
  # src/ListBoxItems.cpp
  src/gui/scene.cpp
  src/gui/stateitem.cpp
  src/gui/transitionitem.cpp
  src/gui/view.cpp
  src/gui/actionsmanager.cpp
  src/gui/controlhandleitem.cpp

  src/optionsmanager.cpp
  src/info.cpp
  src/maincontrol.cpp

  src/Convert.cpp
  src/DRect.cpp
  src/Draw.cpp
  src/DrawArea.cpp
  src/Edit.cpp
  src/Error.cpp
  src/Export.cpp
  src/ExportAHDL.cpp
  src/ExportAHDLDlg.cpp
  src/ExportEPS.cpp
  src/ExportIODescription.cpp
  src/ExportKISS.cpp
  src/ExportPNG.cpp
  src/ExportRagel.cpp
  src/ExportRagelDlg.cpp
  src/ExportSCXML.cpp
  src/ExportSMC.cpp
  src/ExportSVG.cpp
  src/ExportStateTable.cpp
  src/ExportStateTableDlg.cpp
  src/ExportTestbenchDlg.cpp
  src/ExportTestbenchVHDL.cpp
  src/ExportTestvectorASCII.cpp
  src/ExportVHDL.cpp
  src/ExportVHDLDlg.cpp
  src/ExportVVVV.cpp
  src/ExportVVVVDlg.cpp
  src/ExportVerilog.cpp
  src/ExportVerilogDlg.cpp
  src/FileIO.cpp
  src/GITransition.cpp
  src/GObject.cpp
  src/GState.cpp
  src/GTransition.cpp
  src/Grid.cpp
  src/ICheck.cpp
  src/ICheckDlg.cpp
  src/IOInfo.cpp
  src/IOInfoASCII.cpp
  src/IOInfoBin.cpp
  src/IOInfoList.cpp
  src/IOInfoText.cpp
  src/IOViewDlg.cpp
  src/Import.cpp
  src/ImportGraphviz.cpp
  src/Machine.cpp
  src/MachineManager.cpp
  src/MachinePropertiesDlg.cpp
  src/MainWindow.cpp
  src/OptDisplayDlg.cpp
  src/OptGeneral.cpp
  src/OptGeneralDlg.cpp
  src/OptPrintingDlg.cpp
  src/Options.cpp
  src/OptionsDlg.cpp
  src/PrintManager.cpp
  src/Project.cpp
  src/ScrollView.cpp
  src/Selection.cpp
  src/Simulator.cpp
  src/SimulatorDlg.cpp
  src/State.cpp
  src/StateManager.cpp
  src/StatePropertiesDlg.cpp
  src/StatusBar.cpp
  src/TableBuilder.cpp
  src/TableBuilderASCII.cpp
  src/TableBuilderHTML.cpp
  src/TableBuilderLatex.cpp
  src/TestvectorGenerator.cpp
  src/Transition.cpp
  src/TransitionInfo.cpp
  src/TransitionInfoASCII.cpp
  src/TransitionInfoBin.cpp
  src/TransitionInfoText.cpp
  src/TransitionManager.cpp
  src/TransitionPropertiesDlg.cpp
  src/Undo.cpp
  src/UndoBuffer.cpp
  src/Utils.cpp
  src/WSpinBox.cpp
  src/XMLHandler.cpp
  src/Zoom.cpp
  src/main.cpp
)


add_executable(qfsm ${QFSM_HEADERS} ${QFSM_SOURCES} res/resources.qrc)

target_link_libraries(qfsm PRIVATE ${QFSM_QT_LIBS})

set(TS_FILES
  i18n/qfsm_en_US.ts
  i18n/qfsm_pl_PL.ts
)

if(Qt6_FOUND)
  qt6_add_translations(qfsm TS_FILES ${TS_FILES})
else()
  qt5_add_translation(QFSM_TRANSLATIONS ${TS_FILES})
endif()

string(TIMESTAMP QFSM_BUILD_DATE "%Y-%m-%d" UTC)

target_compile_definitions(qfsm PRIVATE
  QFSM_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  QFSM_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  QFSM_BUILD_DATE="${QFSM_BUILD_DATE}"
)

target_include_directories(qfsm PRIVATE src)

if(MSVC)
  target_compile_definitions(qfsm PRIVATE
    QFSM_LANGUAGE_DIR="po"
    QFSM_HELP_DIR="doc/user"
  )
else()
  target_compile_definitions(qfsm PRIVATE
    QFSM_LANGUAGE_DIR="${CMAKE_INSTALL_PREFIX}/share/qfsm/"
    QFSM_HELP_DIR="${CMAKE_INSTALL_PREFIX}/share/doc/qfsm/"
  )
endif()
