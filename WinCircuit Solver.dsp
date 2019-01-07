# Microsoft Developer Studio Project File - Name="WinCircuit Solver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=WinCircuit Solver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinCircuit Solver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinCircuit Solver.mak" CFG="WinCircuit Solver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinCircuit Solver - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "WinCircuit Solver - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WinCircuit Solver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "INCLUDE_WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "CS_SET_NAMESPACE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "WinCircuit Solver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /Ob2 /I "header" /I "../header" /I "source/Win32" /I "../source/Win32" /D "WIN32" /D "INCLUDE_WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "CS_SET_NAMESPACE" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /map /debug /machine:I386 /nodefaultlib:"libc.lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "WinCircuit Solver - Win32 Release"
# Name "WinCircuit Solver - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "RLC Opt Source"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\Source\RLC_opt\C.S.Optimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\RLC_opt\C.S.Oracle.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\RLC_opt\C.S.QueryPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\RLC_opt\C.S.RLC.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\RLC_opt\C.S.RLC_Tree.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\C.S.AnalogAnalysis.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.ComplexNumbers.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Draw.cpp

!IF  "$(CFG)" == "WinCircuit Solver - Win32 Release"

!ELSEIF  "$(CFG)" == "WinCircuit Solver - Win32 Debug"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Equation.cpp

!IF  "$(CFG)" == "WinCircuit Solver - Win32 Release"

!ELSEIF  "$(CFG)" == "WinCircuit Solver - Win32 Debug"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\C.S.FSMDesign.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Graph.cpp

!IF  "$(CFG)" == "WinCircuit Solver - Win32 Release"

!ELSEIF  "$(CFG)" == "WinCircuit Solver - Win32 Debug"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\Win32\C.S.GraphicsWin32.cpp

!IF  "$(CFG)" == "WinCircuit Solver - Win32 Release"

!ELSEIF  "$(CFG)" == "WinCircuit Solver - Win32 Debug"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\C.S.GridArray.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.InputVector.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.IntegratedCircuit.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Win32\C.S.InterfaceWin32.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.IOVector.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.LinkCircuit.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Logic.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.LogicComponents.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.LogicDelay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.LogicEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.LogicFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.LogicPropagation.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.MatrixUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.MOSFET.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Netlist.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.NetPointerSet.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Nodes.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.PlaceAndRoute.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Scheduling.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.SchematicEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.SimulationSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.StringTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.TextGraphics.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.TopoSort.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Transforms.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Transistor.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Undo.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Utilities.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.Window.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C.S.WindowOps.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\CircuitSolver.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Borrowed_Code\FFT.cpp

!IF  "$(CFG)" == "WinCircuit Solver - Win32 Release"

!ELSEIF  "$(CFG)" == "WinCircuit Solver - Win32 Debug"

# ADD CPP /W3

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "RLC Opt Headers"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\Source\RLC_opt\Optimizer.h
# End Source File
# Begin Source File

SOURCE=.\Source\RLC_opt\Oracle.h
# End Source File
# Begin Source File

SOURCE=.\Source\RLC_opt\RLC_Tree.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Header\CircuitSolver.h
# End Source File
# Begin Source File

SOURCE=.\Header\CompArray.h
# End Source File
# Begin Source File

SOURCE=.\Header\Complex.h
# End Source File
# Begin Source File

SOURCE=.\Header\ComponentID.h
# End Source File
# Begin Source File

SOURCE=.\Header\Delay.h
# End Source File
# Begin Source File

SOURCE=.\Header\Draw.h
# End Source File
# Begin Source File

SOURCE=.\Header\Equation.h
# End Source File
# Begin Source File

SOURCE=.\Header\FSMDesign.h
# End Source File
# Begin Source File

SOURCE=.\Header\Graphics.h
# End Source File
# Begin Source File

SOURCE=.\Header\GridArray.h
# End Source File
# Begin Source File

SOURCE=.\Header\IOVector.h
# End Source File
# Begin Source File

SOURCE=.\Header\Logic.h
# End Source File
# Begin Source File

SOURCE=.\Header\Matrix.h
# End Source File
# Begin Source File

SOURCE=.\Header\Netlist.h
# End Source File
# Begin Source File

SOURCE=.\Header\NetPointerSet.h
# End Source File
# Begin Source File

SOURCE=.\Header\PropagationFunctions.h
# End Source File
# Begin Source File

SOURCE=.\Header\Propagations.h
# End Source File
# Begin Source File

SOURCE=.\Header\resource.h
# End Source File
# Begin Source File

SOURCE=.\Header\Scheduling.h
# End Source File
# Begin Source File

SOURCE=.\Header\STL_Support.h
# End Source File
# Begin Source File

SOURCE=.\Header\StringTable.h
# End Source File
# Begin Source File

SOURCE=.\Header\TextGraphics.h
# End Source File
# Begin Source File

SOURCE=.\Header\Undo.h
# End Source File
# Begin Source File

SOURCE=.\Source\Win32\Win32Interface.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Blank.cur
# End Source File
# Begin Source File

SOURCE=.\CircuitSolver.ico
# End Source File
# Begin Source File

SOURCE=.\CircuitSolver.rc
# End Source File
# End Group
# End Target
# End Project
