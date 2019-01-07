#include <windows.h>
#include <windowsx.h>
#include <stdio.h>


// Acquisition.cpp - Written by Frank Gennari 3/5/01 for EE145M Lab
void do_acquisition();
void read_data(int iteration, HDC Context);



void do_acquisition() {

	// create window
	WNDCLASS graphicsWindow;
	HWND DrawingWindow;
	HDC DWContext;
	HCURSOR arrowCursor = LoadCursor(NULL, IDC_ARROW); // Default cursor
  	HBRUSH bkBrush = HBRUSH(COLOR_WINDOW + 1);         // Brush for painting window background	
  	HINSTANCE thisInstance = GetModuleHandle(0);
	
	graphicsWindow.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	graphicsWindow.lpfnWndProc = DefWindowProc;
	graphicsWindow.cbClsExtra = 0;
	graphicsWindow.cbWndExtra = 0;
	graphicsWindow.hInstance = thisInstance;
	graphicsWindow.hIcon = NULL;
	graphicsWindow.hCursor = arrowCursor;
	graphicsWindow.hbrBackground = bkBrush;
	graphicsWindow.lpszMenuName = NULL;
	graphicsWindow.lpszClassName = "EE145M Lab";
  
    RegisterClass (&graphicsWindow);
    
	DrawingWindow = CreateWindow ("EE145M Lab",
		"EE145M LAb Data Acquisition Terminal",     // Set window title
		WS_OVERLAPPEDWINDOW, // Use standard window style
		0,                   // Initial x
		0, // and y position
		320, // Initial width
		160, // and height
		NULL,                // No parent window
		NULL,                // Use menu from window class
		thisInstance,        // Use current program instance
		NULL);               // No special creation parameters
		
	if (DrawingWindow == NULL)
		exit(1);
     
	ShowWindow(DrawingWindow, SW_SHOWNORMAL);
	UpdateWindow(DrawingWindow);
	
	// create graphic context
	DWContext = GetDC(DrawingWindow); // Get device context for client area	
	if (DWContext == NULL)
		exit(1);
	
	// create font
	LOGFONT FontStruct;
	HFONT DWFont;
	
	FontStruct.lfUnderline = FALSE;
    FontStruct.lfStrikeOut = FALSE;
    FontStruct.lfItalic    = FALSE;
    FontStruct.lfEscapement = FALSE;
    FontStruct.lfOrientation = FALSE;
    FontStruct.lfOutPrecision = OUT_DEFAULT_PRECIS;
    FontStruct.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    FontStruct.lfCharSet      = ANSI_CHARSET;
    FontStruct.lfQuality      = DRAFT_QUALITY;
    FontStruct.lfWeight      = FW_NORMAL;
    FontStruct.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
    
    FontStruct.lfHeight = 16;
    FontStruct.lfWidth = 8;
    
    DWFont = CreateFontIndirect(&FontStruct);
    if (DWFont == NULL)
    	exit(1);
    	
    SelectObject(DWContext, DWFont);
    
    // draw text
    //SetBkMode(DWContext, TRANSPARENT);
    //SetBkMode(DWContext, OPAQUE);
	TextOut(DWContext, 20, 20, ((const char *)"Acquiring Data. Click to stop."), 30);
    
    // acquisition loop
    BOOL ContinueFlag = TRUE;
    BOOL SeeMessage;
	MSG theMessage;
	int i = 0;
    
    while (ContinueFlag) {
    	read_data(i, DWContext);
    	++i;
	
		SeeMessage = PeekMessage(&theMessage, NULL, 0, 0, 1); // Get next message	
		if (SeeMessage) {
			if (theMessage.message == WM_LBUTTONDOWN) {
				ContinueFlag = FALSE;
			}
			TranslateMessage(&theMessage); // Convert virtual keys to characters
			DispatchMessage(&theMessage); // Send message to window procedure
		}
    }
    
    // cleanup
    DeleteObject(DWFont);
    ReleaseDC(DrawingWindow, DWContext);
    DestroyWindow(DrawingWindow); // Destroy the window
}




void read_data(int iteration, HDC Context) {

	char text[32] = {0};
	double value;
	
	//value = 1234.5;
	value = iteration/1000;
	
	if (iteration%1000 == 0) {
		sprintf(text, "%f", value);
		TextOut(Context, 100, 60, ((const char *)text), 8);
	}
}









