/*
 Copyright 2003, 2016 Jeff Verkoeyen. All Rights Reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>
#include <math.h.>

typedef struct ColorDef
{
	float r;	//Red
	float g;	//Green
	float b;	//Blue
	float a;	//Alpha
} sColor;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class OpenGL
{
private:
	AUX_RGBImageRec* LoadBMP(char* Filename);
	GLuint	base;
	int NUMTEXTS;
public:
	HDC			hDC;
	HGLRC		hRC;
	HWND		hWnd;
	HINSTANCE	hInstance;

	bool	active;
	bool	keys[256],lkeys[256];
	bool	gfullscreen;

	GLuint	tFont;
	int fmode,bmode;

	GLuint*	texture;

	OpenGL()
	{
		active=true;
		gfullscreen=false;
		hDC=NULL;
		hRC=NULL;
		hWnd=NULL;
		NUMTEXTS=3;
		fmode=GL_FILL;
		bmode=GL_FILL;
	};

	~OpenGL()
	{
		if(tFont)
			KillFont();
		if(texture)
			delete texture;
	};
	bool CreateGLWindow(char* title, int width=800, int height=600, int bits=16, bool fullscreen=false, bool cursor=false);
	bool InitGL();					// Sets up the GL Window

	bool LoadTexture(char* filename, int space);	// Filename, slot
	void Cursor(bool cursor);		// TRUE or FALSE
	void ClearColor(sColor color);	// Takes an sColor variable
	void Culling(bool cull);		// TRUE or FALSE
	void CullFace(int face);		// GL_BACK or GL_FRONT
	void Texture2D(bool bTexture);	// TRUE or FALSE
	void SetNumTexts(int nTexts);	// Number of Textures
	void DrawCube(float size);		// Size
	void DrawCubet(float size);		// Size
	void Mode(int face, int mode);	// GL_FRONT or GL_BACK, GL_FILL or GL_LINE or GL_POINT
	void UpdateKeys();				// Updates the keypresses

	bool LoadFont(char* filename);	// Loads a font texture
	void BuildFont();		//Builds the font
	void KillFont();
	void glPrint(GLint x, GLint y, int set, const char *fmt, ...);

	void ResizeGLScene(GLsizei width, GLsizei height);
	void KillGLWindow();
};

void OpenGL::UpdateKeys()
{
	memcpy(lkeys,keys,sizeof(bool)*256);
}

void OpenGL::BuildFont()									// Build Our Font Display List
{
	base=glGenLists(256);									// Creating 256 Display Lists
	glBindTexture(GL_TEXTURE_2D, tFont);		// Select Our Font Texture

	for(int loop1=0;loop1<256;loop1++)					// Loop Through All 256 Lists
	{
		float cx=float(loop1%16)/16.0f;						// X Position Of Current Character
		float cy=float(loop1/16)/16.0f;						// Y Position Of Current Character

		glNewList(base+loop1,GL_COMPILE);					// Start Building A List
			glBegin(GL_QUADS);								// Use A Quad For Each Character
				glTexCoord2f(cx,1-cy-0.0625f);			// Texture Coord(Bottom Left)
				glVertex2i(0,16);						// Vertex Coord(Bottom Left)
				glTexCoord2f(cx+0.0625f,1-cy-0.0625f);	// Texture Coord(Bottom Right)
				glVertex2i(16,16);						// Vertex Coord(Bottom Right)
				glTexCoord2f(cx+0.0625f,1-cy);			// Texture Coord(Top Right)
				glVertex2i(16,0);						// Vertex Coord(Top Right)
				glTexCoord2f(cx,1-cy);					// Texture Coord(Top Left)
				glVertex2i(0,0);							// Vertex Coord(Top Left)
			glEnd();										// Done Building Our Quad(Character)
			glTranslated(14,0,0);							// Move To The Right Of The Character
		glEndList();										// Done Building The Display List
	}														// Loop Until All 256 Are Built
}

void OpenGL::KillFont()									// Delete The Font From Memory
{
	glDeleteLists(base,256);							// Delete All 256 Display Lists
}

void OpenGL::glPrint(GLint x, GLint y, int set, const char *fmt, ...)	// Where The Printing Happens
{
	glEnable(GL_BLEND);
	int reset=0;
	if(fmode!=GL_FILL)
	{
		reset=fmode;
		Mode(GL_FRONT,GL_FILL);
	}
	glBindTexture(GL_TEXTURE_2D, tFont);
	char		text[1024];							// Holds Our String
	va_list		ap;									// Pointer To List Of Arguments

	if(fmt == NULL)								// If There's No Text
		return;										// Do Nothing

	va_start(ap, fmt);								// Parses The String For Variables
	    vsprintf(text, fmt, ap);					// And Converts Symbols To Actual Numbers
	va_end(ap);										// Results Are Stored In Text

	if(set>1)										// Did User Choose An Invalid Character Set?
		set=1;										// If So, Select Set 1(Italic)

	glMatrixMode(GL_PROJECTION);					// Select The Projection Matrix

	glPushMatrix();
	
	glLoadIdentity();								// Reset The Projection Matrix
	glOrtho(0.0f,800,600,0.0f,-1.0f,1.0f);

	glMatrixMode(GL_MODELVIEW);						// Select The Modelview Matrix

	glLoadIdentity();								// Reset The Modelview Matrix
	glTranslated(x,y,0);							// Position The Text(0,0 - Top Left)
	glListBase(base-32+(128*set));					// Choose The Font Set(0 or 1)

	glCallLists(strlen(text),GL_UNSIGNED_BYTE, text);	// Write The Text To The Screen

	glMatrixMode(GL_PROJECTION);					// Select The Projection Matrix
	glLoadIdentity();								// Reset The Projection Matrix

	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);						// Select The Modelview Matrix
	glLoadIdentity();
	glDisable(GL_BLEND);
	if(reset)
		Mode(GL_FRONT,reset);
}

void OpenGL::Mode(int face, int mode)
{
	glPolygonMode(face,mode);
	if(face==GL_FRONT)
		fmode=mode;
	else if(face==GL_BACK)
		bmode=mode;
}

void OpenGL::DrawCubet(float size)
{
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);glVertex3f(size, size, size);		// Front
		glTexCoord2f(1.0f, 1.0f);glVertex3f(-size, size, size);
		glTexCoord2f(1.0f, 0.0f);glVertex3f(-size,-size, size);
		glTexCoord2f(0.0f, 0.0f);glVertex3f(size,-size, size);

		glTexCoord2f(0.0f, 1.0f);glVertex3f(-size, size, size);		// Left
		glTexCoord2f(1.0f, 1.0f);glVertex3f(-size, size,-size);
		glTexCoord2f(1.0f, 0.0f);glVertex3f(-size,-size,-size);
		glTexCoord2f(0.0f, 0.0f);glVertex3f(-size,-size, size);

		glTexCoord2f(0.0f, 1.0f);glVertex3f(size, size,-size);		// Right
		glTexCoord2f(1.0f, 1.0f);glVertex3f(size, size, size);
		glTexCoord2f(1.0f, 0.0f);glVertex3f(size,-size, size);
		glTexCoord2f(0.0f, 0.0f);glVertex3f(size,-size,-size);

		glTexCoord2f(0.0f, 1.0f);glVertex3f(-size, size,-size);		// Back
		glTexCoord2f(1.0f, 1.0f);glVertex3f(size, size,-size);
		glTexCoord2f(1.0f, 0.0f);glVertex3f(size,-size,-size);
		glTexCoord2f(0.0f, 0.0f);glVertex3f(-size,-size,-size);

		glTexCoord2f(0.0f, 1.0f);glVertex3f(size, size,-size);		// Top
		glTexCoord2f(1.0f, 1.0f);glVertex3f(-size, size,-size);
		glTexCoord2f(1.0f, 0.0f);glVertex3f(-size, size, size);
		glTexCoord2f(0.0f, 0.0f);glVertex3f(size, size, size);

		glTexCoord2f(0.0f, 1.0f);glVertex3f(-size,-size,-size);		// Bottom
		glTexCoord2f(1.0f, 1.0f);glVertex3f(size,-size,-size);
		glTexCoord2f(1.0f, 0.0f);glVertex3f(size,-size, size);
		glTexCoord2f(0.0f, 0.0f);glVertex3f(-size,-size, size);
	glEnd();
}

void OpenGL::DrawCube(float size)
{
	unsigned char on=glIsEnabled(GL_TEXTURE_2D);

	glDisable(GL_TEXTURE_2D);	

	glBegin(GL_QUADS);
		glVertex3f(size, size, size);		// Front
		glVertex3f(-size, size, size);
		glVertex3f(-size,-size, size);
		glVertex3f(size,-size, size);

		glVertex3f(-size, size, size);		// Left
		glVertex3f(-size, size,-size);
		glVertex3f(-size,-size,-size);
		glVertex3f(-size,-size, size);

		glVertex3f(size, size,-size);		// Right
		glVertex3f(size, size, size);
		glVertex3f(size,-size, size);
		glVertex3f(size,-size,-size);

		glVertex3f(-size, size,-size);		// Back
		glVertex3f(size, size,-size);
		glVertex3f(size,-size,-size);
		glVertex3f(-size,-size,-size);

		glVertex3f(size, size,-size);		// Top
		glVertex3f(-size, size,-size);
		glVertex3f(-size, size, size);
		glVertex3f(size, size, size);

		glVertex3f(-size,-size,-size);		// Bottom
		glVertex3f(size,-size,-size);
		glVertex3f(size,-size, size);
		glVertex3f(-size,-size, size);
	glEnd();

	if(on)
		glEnable(GL_TEXTURE_2D);
}

void OpenGL::SetNumTexts(int nTexts)
{
	texture=new GLuint[nTexts];
	NUMTEXTS=nTexts;
}

AUX_RGBImageRec* OpenGL::LoadBMP(char *Filename)					// Loads A Bitmap Image
{
	FILE *File=NULL;							// File Handle

	if(!Filename)
		return NULL;

	File=fopen(Filename,"r");

	if(File)								// Does The File Exist?
	{
		fclose(File);							// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;								// If Load Failed Return NULL
}

bool OpenGL::LoadFont(char* filename)
{
	bool Status=FALSE;
	
	AUX_RGBImageRec *TextureImage[1];

	memset(TextureImage,0,sizeof(void *)*1);

	if(TextureImage[0]=LoadBMP(filename))
	{
		Status=TRUE;
		glGenTextures(1, &tFont);

		glBindTexture(GL_TEXTURE_2D, tFont);
		
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		
		if(TextureImage[0]->data)
			free(TextureImage[0]->data);
		
		free(TextureImage[0]);						// Free The Image Structure
	}
	else
	{
		char temp[100];
		sprintf(temp,"Can't find %s",filename);
		MessageBox(NULL,temp,"ERROR",MB_OK|MB_ICONEXCLAMATION);
	}

	return Status;								// Return The Status
}

bool OpenGL::LoadTexture(char* filename, int space)
{
	bool Status=FALSE;
	
	AUX_RGBImageRec *TextureImage[1];

	memset(TextureImage,0,sizeof(void *)*1);

	if(TextureImage[0]=LoadBMP(filename))
	{
		Status=TRUE;
		glGenTextures(1, &texture[space]);

		glBindTexture(GL_TEXTURE_2D, texture[space]);
		
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		
		if(TextureImage[0]->data)
			free(TextureImage[0]->data);
		
		free(TextureImage[0]);						// Free The Image Structure
	}
	else
	{
		char temp[100];
		sprintf(temp,"Can't find %s",filename);
		MessageBox(NULL,temp,"ERROR",MB_OK|MB_ICONEXCLAMATION);
	}

	return Status;								// Return The Status
}

void OpenGL::Texture2D(bool bTexture)
{
	if(bTexture)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
}

void OpenGL::CullFace(int face)
{
	glCullFace(face);
}

void OpenGL::Culling(bool cull)
{
	if(cull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void OpenGL::ClearColor(sColor color)
{
	glClearColor(color.r,color.g,color.b,color.a);
}

void OpenGL::Cursor(bool cursor)
{
	ShowCursor(cursor);
}

void OpenGL::KillGLWindow()										// Properly Kill The Window
{
	if(gfullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if(hRC)											// Do We Have A Rendering Context?
	{
		if(!wglMakeCurrent(NULL,NULL))
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);

		if(!wglDeleteContext(hRC))
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hRC=NULL;										// Set RC To NULL
	}

	if(hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if(hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if(!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

void OpenGL::ResizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if(!height)
		height=1;

	glViewport(0,0,width,height);

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,3000.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

bool OpenGL::CreateGLWindow(char *title, int width, int height, int bits, bool fullscreen, bool cursor)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	gfullscreen=fullscreen;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		=(WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if(!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if(fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if(ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if(MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","ERROR",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen=FALSE
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	ShowCursor(cursor);

	if(fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if(!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer(Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if(!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ResizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if(!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

bool OpenGL::InitGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	//glEnable(GL_BLEND);
	return TRUE;
}