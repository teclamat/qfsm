cmake_minimum_required(VERSION 3.18)

project(qfsm
  VERSION 0.60
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets Xml Svg PrintSupport)

set(QFSM_HEADERS
	src/Const.h
	src/Convert.h
	src/DocStatus.h
	src/Draw.h
	src/DrawArea.h
	src/DRect.h
	src/Edit.h
	src/Error.h
	src/Export.h
	src/ExportAHDL.h
	src/ExportEPS.h
	src/ExportIODescription.h
	src/ExportKISS.h
	src/ExportPNG.h
	src/ExportRagel.h
	src/ExportRagelDlg.h
	src/ExportSCXML.h
	src/ExportSMC.h
	src/ExportStateTable.h
	src/ExportStateTableDlg.h
	src/ExportSVG.h
	src/ExportVerilog.h
	src/ExportVHDL.h
	src/ExportVVVV.h
	src/FileIO.h
	src/GITransition.h
	src/GObject.h
	src/Grid.h
	src/GState.h
	src/GTransition.h
	src/ICheck.h
	src/ImportGraphviz.h
	src/IOInfo.h
	src/IOInfoASCII.h
	src/IOInfoBin.h
	src/IOInfoList.h
	src/IOInfoText.h
	# src/ListBoxItems.h
	src/Machine.h
	src/MachineManager.h
	src/MainControl.h
	src/MainWindow.h
	src/MimeMachine.h
	src/OptGeneral.h
	src/Options.h
	src/PrintManager.h
	src/Project.h
	src/ScrollView.h
	src/Selection.h
	src/Simulator.h
	src/State.h
	src/StateManager.h
	src/StatusBar.h
	src/TableBuilder.h
	src/TableBuilderASCII.h
	src/TableBuilderHTML.h
	src/TableBuilderLatex.h
	src/ToolTipSV.h
	src/Transition.h
	src/TransitionInfo.h
	src/TransitionInfoASCII.h
	src/TransitionInfoBin.h
	src/TransitionInfoText.h
	src/TransitionManager.h
	src/Undo.h
	src/UndoBuffer.h
	src/Utils.h
	src/WSpinBox.h
	src/XMLHandler.h
	src/Zoom.h
  src/AppInfo.h
  src/ExportAHDLDlg.h
  src/ExportTestbenchDlg.h
  src/ExportTestbenchVHDL.h
  src/ExportTestvectorASCII.h
  src/ExportVerilogDlg.h
  src/ExportVHDLDlg.h
  src/ExportVVVVDlg.h
  src/ICheckDlg.h
  src/Import.h
  src/IOViewDlg.h
  src/MachinePropertiesDlg.h
  src/OptDisplayDlg.h
  src/OptGeneralDlg.h
  src/OptPrintingDlg.h
  src/SimulatorDlg.h
  src/StatePropertiesDlg.h
  src/TestvectorGenerator.h
  src/TransitionPropertiesDlg.h
)

set(QFSM_SOURCES
	src/Convert.cpp
	src/DRect.cpp
	src/DocStatus.cpp
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
	src/ExportVHDL.cpp
	src/ExportVVVV.cpp
	src/ExportVVVVDlg.cpp
	src/ExportVerilog.cpp
	src/FileIO.cpp
	src/GITransition.cpp
	src/GObject.cpp
	src/GState.cpp
	src/GTransition.cpp
	src/Grid.cpp
	src/ICheck.cpp
	src/IOInfo.cpp
	src/IOInfoASCII.cpp
	src/IOInfoBin.cpp
	src/IOInfoList.cpp
	src/IOInfoText.cpp
	src/ImportGraphviz.cpp
	# src/ListBoxItems.cpp
	src/Machine.cpp
	src/MachineManager.cpp
	src/MainControl.cpp
	src/MainWindow.cpp
	src/MimeMachine.cpp
	src/OptGeneral.cpp
	src/Options.cpp
	src/PrintManager.cpp
	src/Project.cpp
	src/ScrollView.cpp
	src/Selection.cpp
	src/Simulator.cpp
	src/State.cpp
	src/StateManager.cpp
	src/StatusBar.cpp
	src/TableBuilder.cpp
	src/TableBuilderASCII.cpp
	src/TableBuilderHTML.cpp
	src/TableBuilderLatex.cpp
	src/ToolTipSV.cpp
	src/Transition.cpp
	src/TransitionInfo.cpp
	src/TransitionInfoASCII.cpp
	src/TransitionInfoBin.cpp
	src/TransitionInfoText.cpp
	src/TransitionManager.cpp
	src/Undo.cpp
	src/UndoBuffer.cpp
	src/Utils.cpp
	src/WSpinBox.cpp
	src/XMLHandler.cpp
	src/Zoom.cpp
	src/main.cpp
  src/AppInfo.cpp
  src/ExportTestbenchDlg.cpp
  src/ExportTestbenchVHDL.cpp
  src/ExportTestvectorASCII.cpp
  src/ExportVHDLDlg.cpp
  src/ExportVerilogDlg.cpp
  src/ICheckDlg.cpp
  src/IOViewDlg.cpp
  src/Import.cpp
  src/MachinePropertiesDlg.cpp
  src/OptDisplayDlg.cpp
  src/OptGeneralDlg.cpp
  src/OptPrintingDlg.cpp
  src/SimulatorDlg.cpp
  src/StatePropertiesDlg.cpp
  src/TestvectorGenerator.cpp
  src/TransitionPropertiesDlg.cpp
)

add_executable(qfsm ${QFSM_HEADERS} ${QFSM_SOURCES})

target_link_libraries(qfsm PRIVATE Qt::Core Qt::Widgets Qt::Xml Qt::Svg Qt::PrintSupport)

string(TIMESTAMP QFSM_BUILD_DATE "%Y-%m-%d" UTC)

target_compile_definitions(qfsm PRIVATE
	QFSM_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
	QFSM_VERSION_MINOR=${PROJECT_VERSION_MINOR}
	QFSM_BUILD_DATE="${QFSM_BUILD_DATE}"
)

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
