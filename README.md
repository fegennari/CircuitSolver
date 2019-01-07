# CircuitSolver
Digital/Analog Circuit Design and Simulation System. CircuitSolver contains a variety of features for design and analysis of simple analog and digital circuits, as well as a built-in graphing calculator. It's great for solving those homework problems from EE course textbooks. The UI is text-based with some simple graphics with clickable buttons.

This project was originally written in C++ (mostly C) in CodeWarrior during my time at Carnegie Mellon University from 1996-2000. I've since ported this to Windows, linux, and cygwin. I provide the source of CircuitSolver for educational purposes but with no support. Use it how you would like.

Only the Windows build is currently supported. The WinCircuit Solver.vcxproj file can be opened with any recent version of MS Visual Studio (2010, 2015, etc.) and should build with no external dependencies and run on any modern version of MS Windows.

That's right, there are no dependencies. Everything is self-contained. All of the graphics, window management, mouse/keyboard input, file I/O, etc. are done in software using native OS system calls. This project was written before most of the utility libraries in use today even existed. 
