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

// Script-and-Go Engine v1.0b by Jeff Verkoeyen
//
// Purpose:
//
//		S-a-G is built to allow virtually anyone to program in OpenGL,
//		whether or not they have a compiler or any prior knowledge of
//		programming.
//
// Aimed at:
//
//		I am hoping to get this program out to everyone who does not have
//		a compiler, or just wants an easier way to use OpenGL.
//
// Production Dates:
//
//		March 14: Started production of S-a-G
//		March 15: Added about 5 new commands to the engine
//		March 16: Finished the variable system.
//
// Versions:
//
//		v1.0b: First released March 15
//			Includes about 10 commands and basic variable system support.
//
// S-a-G was created March of 2003 and is a production of IVGDA
// S-a-G will NOT be edited in any way without consent of Jeff Verkoeyen


/////////////////////////////HEADER FILES//////////////////////////
////////
////////	All the header files needed for the scripting engine
////////
///////////////////////////////////////////////////////////////////

#include <fstream.h>	// For file i/o
#include <string.h>		// For string operations
#include <windows.h>	// Error boxes
#include "map.h"		// For the map variable

////////////////////////PRE-DEFINED VARIABLES//////////////////////
////////
////////	All the pre-defined variables for the engine are made
////////	here.
////////
///////////////////////////////////////////////////////////////////

#define NUMCOMMANDS		12	// Number of commands
#define NUMPARAMS		20	// Max number of parameters to read in

#define ADDITION		1
#define MULTIPLICATION	2
#define DIVISION		3
#define SUBTRACTION		4
#define EQUALING		5
#define INCREMENTING	6
#define DECREMENTING	7

#define DRAWLINE			0x00
#define ENABLETEXTURES		0x01
#define DISABLETEXTURES		0x02
#define RESET				0x03
#define SETCOLOR			0x04
#define MOVE				0x05
#define BOX					0x06
#define ENABLECULLING		0x07
#define	DISABLECULLING		0x08
#define CULLFACE			0x09
#define ROTATE				0x0A
#define DRAWMODE			0x0B

/////////////////////////////COMMANDS//////////////////////////////
////////
////////	Each of the commands must be defined in this array for
////////	them to be classified as valid commands.
////////
///////////////////////////////////////////////////////////////////

const char commands[NUMCOMMANDS] [20]=
{
	"DrawLine",			// DrawLine(x1,y1,z1,x2,y2,z2);
	"EnableTextures",	// EnableTextures;
	"DisableTextures",	// DisableTextures;
	"Reset",			// Reset;
	"SetColor",			// SetColor(red,green,blue,alpha);
	"Move",				// Move(x,y,z);
	"Box",				// Box(x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4);
	"EnableCulling",	// EnableCulling;
	"DisableCulling",	// DisableCulling
	"CullFace",			// CullFace(<face>);
	"Rotate",			// Rotate(rotation,x,y,z);
	"DrawMode"			// DrawMode(<face>,<mode);
};

////////////////////////COMMAND PARAMETERS/////////////////////////
////////
////////	Number of each type of parameter for each command.
////////	{<numerical>,<character>}
////////
///////////////////////////////////////////////////////////////////

const unsigned int commandparams[NUMCOMMANDS] [2]=
{
	{6,0},
	{0,0},
	{0,0},
	{0,0},
	{4,0},
	{3,0},
	{12,0},
	{0,0},
	{0,0},
	{0,1},
	{4,0},
	{0,2}
};

////////////////////////////PROTOTYPES/////////////////////////////
////////
////////	Prototypes for all of the functions go here.
////////
///////////////////////////////////////////////////////////////////

void Error(char* title, UINT uType, const char* fmt, ...);
void Cleanup();
bool testvariables(char* buff,int line);

/////////////////////////GLOBAL VARIABLES//////////////////////////
////////
////////	Global variables that will remain in the program the
////////	entire time.
////////
///////////////////////////////////////////////////////////////////

bool openedonce=false;

unsigned int NUMVARIABLES=20;

map *Variables=NULL;

/////////////////////////////FUNTIONS//////////////////////////////
////////
////////	Everything below here is a funtion and can be used
////////	outside of this file.
////////
///////////////////////////////////////////////////////////////////

bool scripting()
{
	ifstream script;			//Our file variable

	script.open("script.spt",ios::nocreate | ios::in);	//Open the file

	if(script)	//If it opened successfully
	{
		script.seekg(ios::beg);		//Get to the beginning of the file

//////////////////////TEMPORARY VARIABLES//////////////////////////
////////
////////	Temporary variables that don't need to be kept at the
////////	end of each loop
////////
///////////////////////////////////////////////////////////////////

		char buff[1024]=" ";		//buffer to hold our whole string
		char buff2=' ';				//buffer to hold each character

		bool comment=false;			//Which type of comment we're in, Long
		bool comment2=false;		//Short comment

		int pos=0;					//Position in the buffer

		bool begin=false;			//Program is started
		bool end=false;				//Program is done
		bool variablebegin=false;	//Variable section

		int line=1;					//Which line we're looking at

		if(!openedonce)				//If this is the first time we've opened it
			Variables=new map[NUMVARIABLES];	//Create the variable array

/////////////////////////MAIN TEST LOOP////////////////////////////
////////
////////	Main test loop that runs through the whole file
////////
///////////////////////////////////////////////////////////////////
		
		while(!script.eof() && !end)	//Test if at the end of the file or END
		{
			script.get(buff2);		//Get the current character

			if(buff2==-1)			//Double check if we are at the end of the file
				break;				//break out of the loop

////////////////////////COMMENT TESTING////////////////////////////
////////
////////	All the testing for beginning the comments goes here
////////
///////////////////////////////////////////////////////////////////

			if(buff2=='/')			//Test for comments
			{
				char buff3;			//Create a temporary buffer
				script.get(buff3);	//Read in the next character
				if(buff3=='*')		//If it is a long comment /*
					comment=true;	//set the variable
				else if(buff3=='/')	//otherwise if it's a short comment //
					comment2=true;	//set the other variable
				else				//If it's not a comment
					script.seekg(-1,ios::cur);	//Just move back a spot
			}
			if(!comment && !comment2 && buff2!=';')	//If we are NOT at the end of a command
			{
				buff[pos]=buff2;	//copy the character in to the buffer
				pos++;				//increase the position
			}

////////////////////////FOUND A COMMAND////////////////////////////
////////
////////	If we found a semicolon, then we patch the string up
////////	and test the command, if there is one
////////
///////////////////////////////////////////////////////////////////

			if(buff2==';' && !comment && !comment2)	//If it's the end of a command, but not in a comment
			{
				buff[pos]='\0';	//Close up the buffer

				bool started=false;	//Have we started the actual code? (not whitespace)

				for(unsigned int a=0;a<strlen(buff);a++)
				{
					if(isalpha(buff[a]))	//if the character is not whitespace, we've started
						started=true;		//Set the started variable

					if(buff[a]=='\n' || buff[a]=='	' || (buff[a]==' ' && !started))
					{	//if the character is a space, newline, or tab

						for(unsigned int b=a+1;b<=strlen(buff);b++)
							buff[b-1]=buff[b];	//cut it out

						a--;	//move back a character so we can retest it...just in case
					}
				}

////////////////////TESTING BEGIN/END/VARIABLES////////////////////
////////
////////	Testing for setting the states of each of the main
////////	loops.
////////
///////////////////////////////////////////////////////////////////

				if(!strcmp(buff,"VARIABLES") && !begin && !variablebegin)	//If the buffer holds "VARIABLES", haven't begun, and the variable section hasn't started,
					variablebegin=true;			//Set the variable state on
				else if(variablebegin && !strcmp(buff,"VARIABLES"))
				{	//Test if there are more than one VARIABLES
					Error("Multiple VARIABLESs",MB_OK | MB_ICONINFORMATION,"Two VARIABLES commands found.");
					return false;
				}
				else if(!strcmp(buff,"VARIABLES") && begin)
				{	//Test if there is a VARIABLES inside the main loop
					Error("Cannot declare variables inside main loop",MB_OK | MB_ICONINFORMATION,"Cannot declare variables inside the BEGIN-END code.");
					return false;
				}

				if(!strcmp(buff,"BEGIN") && !begin)	//if BEGIN and not already begun...
				{
					begin=true;						//Then we've begun
					variablebegin=false;
				}
				else if(!strcmp(buff,"BEGIN") && begin)	//If there are two begins....
				{
					Error("Multiple BEGINs",MB_OK | MB_ICONINFORMATION,"Two BEGIN commands found.");
					return false;
				}

				if(!strcmp(buff,"END") && !begin)
				{
					Error("End in wrong spot",MB_OK | MB_ICONINFORMATION,"Error, found END, but not BEGIN. Make sure BEGIN is before END.");
					return false;	//END is before begin
				}
				else if(!strcmp(buff,"END") && begin)
				{
					end=true;		//End only if we've already started
					return true;
				}

				if(!variablebegin && !begin && strcmp(buff,"VARIABLES") && strcmp(buff,"BEGIN"))
				{
					Error("Command is floating",MB_OK | MB_ICONINFORMATION,"%s is a floating command.  Not sure what do to with it.\nLine %d",buff,line);
					return false;
				}

///////////////////////VARIABLE TESTING////////////////////////////
////////
////////	Testing in the VARIABLES section of code goes here
////////
///////////////////////////////////////////////////////////////////

				if(variablebegin && !openedonce && strcmp(buff,"VARIABLES"))
				{	// Only runs through this block of code once.
					if(buff[0]=='@')
					{	//Are we declaring a variable?
						for(unsigned int a=1;a<=strlen(buff);a++)	// Cut out the @
							buff[a-1]=buff[a];
						buff[a]='\0';				// Patch it up again
						
						char type[10],name[100];	// Holds our variable type, and its name
						
						int space=-1;					// Position of the space

						for(a=0;a<strlen(buff);a++)
						{
							if(buff[a]==' ')
							{
								space=a;
								break;
							}
							type[a]=buff[a];
						}
						type[a]='\0';

						if(space==-1)
						{
							Error("Nothing to Declare",MB_OK | MB_ICONINFORMATION,"%s  Nothing to declare.\nLine: %d",buff,line);
							return false;
						}
						
						for(a=space+1;a<strlen(buff);a++)
							name[a-space-1]=buff[a];
						name[a-space-1]='\0';

						for(a=0;a<strlen(name);a++)
						{
							if(name[a]=='/' ||
							   name[a]=='*' ||
							   name[a]=='&' ||
							   name[a]=='%' ||
							   name[a]=='#' ||
							   name[a]=='@' ||
							   name[a]=='!' ||
							   name[a]=='(' ||
							   name[a]==')' ||
							   name[a]=='\\' ||
							   name[a]==',')
							{
								Error("Illegal Character",MB_OK | MB_ICONINFORMATION,"Illegal character found in variable  %s\nIllegal characters: ! @ # $ % ^ & * ( ) / \\ ,\nLine: %d",name,line);
								return false;
							}
						}
						
						if(!strcmp(name,"char") || !strcmp(name,"bool") || !strcmp(name,"int") || !strcmp(name,"float") || !strcmp(name,"true") || !strcmp(name,"false"))
						{
							Error("Invalid Name",MB_OK | MB_ICONINFORMATION,"%s is a variable type, you may not declare a variable with this name.\nLine: %d",name,line);
							return false;
						}

						if(!strcmp(name,"BEGIN") || !strcmp(name,"VARIABLES") || !strcmp(name,"END"))
						{
							Error("Invalid Name",MB_OK | MB_ICONINFORMATION,"%s is not a valid variable name, don't mock the program.\nLine: %d",name,line);
							return false;
						}
						
						for(a=0;a<NUMCOMMANDS;a++)
						{
							if(!strcmp(name,commands[a]))
							{
								Error("Invalid Name",MB_OK | MB_ICONINFORMATION,"%s is a command, you may not declare a variable with this name.\nLine: %d",name,line);
								return false;
							}
						}

						for(a=0;a<(unsigned int)NUMVARIABLES;a++)
						{
							if(!strcmp(name,Variables[a].name))
							{
								Error("Variable already exists",MB_OK | MB_ICONINFORMATION,"%s already exists. Please use a different name.\nLine: %d",name,line);
								return false;
							}
						}
						
						int use=-1;
						
						for(a=0;a<(unsigned int)NUMVARIABLES;a++)
						{
							if(!Variables[a].isOn())
							{
								use=a;
								break;
							}
						}
						
						if(use==-1)
						{
							NUMVARIABLES++;
							Variables=(map*)realloc(Variables,sizeof(map)*NUMVARIABLES);
							use=NUMVARIABLES-1;
						}
						
						if(!Variables[use].create(name,type))
						{
							Error("Invalid Variable Type",MB_OK | MB_ICONINFORMATION,"%s is an invalid variable type.\nLine: %d",type,line);
							return false;
						}
					}
					else
					{
						if(!testvariables(buff,line))
							return false;
					}
				}

///////////////////////MAIN LOOP TESTING///////////////////////////
////////
////////	Testing the BEGIN-END loop goes here
////////
///////////////////////////////////////////////////////////////////

				if(begin && strcmp(buff,"BEGIN") && strcmp(buff,"END"))	//if we've completely started and
				{	//the commands aren't BEGIN or END

					unsigned int a;

					char variable[100];

					bool oktogo=false;

					for(a=0;a<strlen(buff);a++)
					{
						if(buff[a]=='*' || buff[a]=='/' || buff[a]=='+' || buff[a]=='-' || buff[a]=='=')
						{
							oktogo=true;
							break;
						}
						else if(buff[a]==';' || buff[a]=='(')
							break;
						else
							variable[a]=buff[a];
					}
					variable[a]='\0';

					if(oktogo)
					{
						if(!testvariables(buff,line))
							return false;
					}
					else
					{
						char command[100]=" ";	//Fill the command buffer with empty stuff....
						unsigned int lparen=-1;	//Set the left parentheses position to -1, an impossible position
						
						for(a=0;a<strlen(buff);a++)	//Run through the whole string
						{
							if(buff[a]!='(')		//Try to find the left parenthesis
								command[a]=buff[a];	//If we didn't find it, just keep copying the command to the buffer
							else
							{
								lparen=a;			//If we do find it, set the position and break from the loop, to save time
								break;				//Breaks from the loop
							}
						}
						command[a]='\0';			//Close up our command buffer
						
						int numofcommand=-1;			//The number of the command
						
						for(a=0;a<NUMCOMMANDS;a++)				//Loop through all of the commands
						{
							if(!strcmp(command,commands[a]))	//If we found it....
							{
								numofcommand=a;					//Set the position
								break;							//Break from the loop
							}
						}
						if(numofcommand==-1)	//If we didn't find the command
						{
							Error("Command doesn't exist",MB_OK | MB_ICONINFORMATION,"Error, %s does not exist as a command.\nLine: %d",command,line);
							return false;
						}
						
						//If we made it this far, we know two things:
						//1) The command exists
						//2) We know whether or not there is a left parenthesis
						
						double params[NUMPARAMS];	//Holds up to # parameters
						char cparams[5] [30];		//Holds up to # character parameters
						
						unsigned int numparams=0;	//The number of parameters that are found
						unsigned int numcparams=0;	//The number of character parameters that are found
						
						if(lparen>=0)				//If there IS a left parenthesis
						{
							int character=0;		//The character we are editing
							
							char value[20];			//Holds the parameter value
							
							for(a=lparen+1;a<strlen(buff);a++)	//Run through the ( stuff )
							{
								if(buff[a]!=',' && buff[a]!=')')	//If there's a not , or a right parenthesis
								{
									value[character]=buff[a];	//Keep adding on to the buffer
									character++;				//Increase the character value
								}
								else
								{
									value[character]='\0';		//Close up our buffer

									int found=-1;
									for(unsigned int b=0;b<NUMVARIABLES;b++)
									{
										if(!strcmp(value,Variables[b].name))
										{
											found=b;
											break;
										}
									}

									if(found!=-1)
									{
										switch(Variables[found].Type())
										{
										case INTEGER:
											params[numparams]=(float)Variables[found].ivalue;
											numparams++;
											break;
										case FLOAT:
											params[numparams]=Variables[found].fvalue;
											numparams++;
											break;
										case BOOLEAN:
											Error("Can't pass BOOL",MB_OK | MB_ICONINFORMATION,"You can't pass a bool to a function.\nLine: %d",line);
											break;
										case CHARACTER:
											strcpy(cparams[numcparams],Variables[found].cvalue);
											numcparams++;
											break;
										}
									}
									else if(value[0]==34 && value[strlen(value)-1]==34)
									{
										for(unsigned int b=1;b<strlen(value)-1;b++)
											value[b-1]=value[b];
										value[b-1]='\0';
										
										strcpy(cparams[numcparams],value);
										numcparams++;				//Increase our number of cparams
									}
									else if(value[0]==34 && value[strlen(value)-1]!=34)
									{
										Error("Missing Quotation Marks",MB_OK | MB_ICONINFORMATION,"%s  Missing quotation mark at the end.\nLine: %d",value,line);
										return false;
									}
									else if(value[0]!=34 && value[strlen(value)-1]==34)
									{
										Error("Missing Quotation Marks",MB_OK | MB_ICONINFORMATION,"%s  Missing quotation mark at the beginning.\nLine: %d",value,line);
										return false;
									}
									else if(!isalpha(value[0]))
									{
										params[numparams]=atof(value);	//Convert the value to a float
										numparams++;				//Increase our number of params
									}
									else
									{
										Error("Invalid Value",MB_OK | MB_ICONINFORMATION,"%s  Is not a proper value.\nLine: %d",value,line);
										return false;
									}

									
									character=0;				//Reset the current character
									strcpy(value," ");			//Fill the buffer with junk
								}
							}
						}
						
						bool ok=true;
						
						if(commandparams[numofcommand] [0]>numparams)
						{
							Error("Too few numerical parameters",MB_OK | MB_ICONINFORMATION,"Too few numerical parameters for %s\nRequires %d.\nLine: %d",command,commandparams[numofcommand] [0],line);
							if(ok)
								Error("Contains",MB_OK | MB_ICONINFORMATION,"Contains:\n%d character parameters\n%d numerical parameters.",numcparams,numparams);
							ok=false;
						}
						if(commandparams[numofcommand] [0]<numparams || (lparen!=-1 && commandparams[numofcommand] [0]==0 && commandparams[numofcommand] [1]==0))
						{
							if(commandparams[numofcommand] [0]>0)
								Error("Too many numerical parameters",MB_OK | MB_ICONINFORMATION,"Too many numerical parameters for %s\nRequires only %d.\nLine: %d",command,commandparams[numofcommand] [0],line);
							else
								Error("Too many numerical parameters",MB_OK | MB_ICONINFORMATION,"Too many numerical parameters for %s\nRequires none.\nLine: %d",command,line);
							if(ok)
								Error("Contains",MB_OK | MB_ICONINFORMATION,"Contains:\n%d character parameters\n%d numerical parameters.",numcparams,numparams);
							ok=false;
						}
						
						if(commandparams[numofcommand] [1]>numcparams)
						{
							Error("Too few character parameters",MB_OK | MB_ICONINFORMATION,"Too few character parameters for %s\nRequires %d.\nLine: %d",command,commandparams[numofcommand] [1],line);
							if(ok)
								Error("Contains",MB_OK | MB_ICONINFORMATION,"Contains:\n%d character parameters\n%d numerical parameters.",numcparams,numparams);
							ok=false;
						}
						if(commandparams[numofcommand] [1]<numcparams || (lparen!=-1 && commandparams[numofcommand] [0]==0 && commandparams[numofcommand] [1]==0))
						{
							if(commandparams[numofcommand] [1]>0)
								Error("Too many character parameters",MB_OK | MB_ICONINFORMATION,"Too many character parameters for %s\nRequires only %d.\nLine: %d",command,commandparams[numofcommand] [1],line);
							else
								Error("Too many character parameters",MB_OK | MB_ICONINFORMATION,"Too many character parameters for %s\nRequires none.\nLine: %d",command,line);
							if(ok)
								Error("Contains",MB_OK | MB_ICONINFORMATION,"Contains:\n%d character parameters\n%d numerical parameters.",numcparams,numparams);
							ok=false;
						}
						
						if(!ok)
							return false;
						
						switch(numofcommand)
						{
						case DRAWLINE:
							glBegin(GL_LINES);
							glVertex3d(params[0],params[1],params[2]);
							glVertex3d(params[3],params[4],params[5]);
							glEnd();
							break;
						case ENABLETEXTURES:
							glEnable(GL_TEXTURE_2D);
							break;
						case DISABLETEXTURES:
							glDisable(GL_TEXTURE_2D);
							break;
						case RESET:
							glLoadIdentity();
							break;
						case SETCOLOR:
							glColor4d(params[0],params[1],params[2],params[3]);
							break;
						case MOVE:
							glTranslated(params[0],params[1],params[2]);
							break;
						case BOX:
							glBegin(GL_QUADS);
							glVertex3d(params[3],params[4],params[5]);
							glVertex3d(params[0],params[1],params[2]);
							glVertex3d(params[9],params[10],params[11]);
							glVertex3d(params[6],params[7],params[8]);
							glEnd();
							break;
						case ENABLECULLING:
							glEnable(GL_CULL_FACE);
							break;
						case DISABLECULLING:
							glDisable(GL_CULL_FACE);
							break;
						case CULLFACE:
							if(!strcmp(cparams[0],"front"))
								glCullFace(GL_FRONT);
							else if(!strcmp(cparams[0],"back"))
								glCullFace(GL_BACK);
							else
							{
								Error("Illegal character",MB_OK | MB_ICONINFORMATION,"Command %s only takes either %cback%c or %cfront%c, %s will not work.\nLine: %d",command,34,34,34,34,cparams[0],line);
								return false;
							}
							break;
						case ROTATE:
							glRotated(params[0],params[1],params[2],params[3]);
							break;
						case DRAWMODE:
							unsigned int face,mode;
							if(!strcmp(cparams[0],"front"))
								face=GL_FRONT;
							if(!strcmp(cparams[0],"back"))
								face=GL_BACK;
							if(!strcmp(cparams[1],"fill"))
								mode=GL_FILL;
							if(!strcmp(cparams[1],"lines"))
								mode=GL_LINE;
							if(!strcmp(cparams[1],"points"))
								mode=GL_POINT;
							glPolygonMode(face,mode);
							break;
						}
					}
				}

				pos=0;
				strcpy(buff," ");
			}

			if(buff2=='*')
			{
				char buff3;
				script.get(buff3);
				if(buff3=='/')
					comment=false;
				else
					script.seekg(-1,ios::cur);
			}
			if(buff2=='\n')
			{
				comment2=false;
				line++;
			}
		}

		if(!end)
		{
			Error("Missing END",MB_OK | MB_ICONINFORMATION,"Cannot find END command!");
			return false;
		}
	}
	else
	{
		Error("Missing File",MB_OK | MB_ICONINFORMATION,"Couldn't Find script.spt");
		return false;
	}

	openedonce=true;

	return true;
}

void Error(char* title, UINT uType, const char* fmt, ...)
{
	char buff[1024];
	
	va_list		ap;

	if(fmt == NULL)
		return;

	va_start(ap, fmt);
	    vsprintf(buff, fmt, ap);
	va_end(ap);

	MessageBox(NULL,buff,title,uType);
}

void Cleanup()
{
	if(Variables)
		delete [] Variables;
}

bool testvariables(char* buff, int line)
{
	int operand=-1;
	for(unsigned int a=0;a<strlen(buff);a++)
	{
		if(buff[a]=='+' || buff[a]=='-' || buff[a]=='*' || buff[a]=='/' || buff[a]=='=')
		{
			operand=a;
			break;
		}
	}
	if(operand==-1)
	{
		Error("No Operator Found",MB_OK | MB_ICONINFORMATION,"No operator found. + - * / =\nLine: %d",line);
		return false;
	}
	
	char variable[100],value[100];
	
	for(a=0;a<(unsigned int)operand;a++)
		variable[a]=buff[a];
	variable[a]='\0';
	
	int position=-1;
	
	for(a=0;a<NUMVARIABLES;a++)
	{
		if(!strcmp(variable,Variables[a].name))
			position=a;
	}
	
	if(position==-1)
	{
		Error("Variable Does Not Exist",MB_OK | MB_ICONINFORMATION,"%s has not been declared.\nLine: %d",variable,line);
		return false;
	}
	
	for(a=operand+1;a<(int)strlen(buff);a++)
		value[a-operand-1]=buff[a];
	value[a-operand-1]='\0';
	
	char cvalue[100];
	int ivalue=0;
	float fvalue=0;
	bool bvalue;
	
	bool rightvalue=false;
	int rvalpos;
	
	for(a=0;a<NUMVARIABLES;a++)
	{
		if(!strcmp(value,Variables[a].name))
		{
			rightvalue=true;
			rvalpos=a;
			break;
		}
	}
	
	if(rightvalue)
	{
		strcpy(cvalue,Variables[a].cvalue);
		ivalue=Variables[a].ivalue;
		fvalue=Variables[a].fvalue;
		bvalue=Variables[a].bvalue;
	}
	else if(value[0]==34 && value[strlen(value)-1]!=34)
	{
		Error("Open Quote",MB_OK | MB_ICONINFORMATION,"%s  Missing right quote.\nLine: %d",value,line);
		return false;
	}
	else if(value[0]!=34 && value[strlen(value)-1]==34)
	{
		Error("Open Quote",MB_OK | MB_ICONINFORMATION,"%s  Missing left quote.\nLine: %d",value,line);
		return false;
	}
	else if(value[0]!=34 && value[strlen(value)-1]!=34)
	{
		for(a=0;a<(int)strlen(value);a++)
		{
			if(!isdigit(value[a]) && value[a]!='.')
			{
				Error("Right Variable Does Not Exist",MB_OK | MB_ICONINFORMATION,"%s does not exist as a variable.\nLine: %d",value,line);
				return false;
			}
		}
	}
	
	int lType=Variables[position].Type();
	
	if(buff[operand]=='=')
	{
		if(lType==INTEGER)
			Variables[position].ivalue=atoi(value)+ivalue+(int)fvalue;
		
		if(lType==CHARACTER)
		{
			if(value[0]!=34 || value[strlen(value)-1]!=34)
			{
				Error("Incorrect Format",MB_OK | MB_ICONINFORMATION,"%s must have quotation marks surrounding the text.\nLine: %d",value,line);
				return false;
			}
			for(a=1;a<strlen(value)-1;a++)
				value[a-1]=value[a];
			value[a-1]='\0';
			if(!rightvalue)
				strcpy(Variables[position].cvalue,value);
			else
				strcpy(Variables[position].cvalue,cvalue);
		}
		
		if(lType==BOOLEAN)
		{
			if(!strcmp(value,"true"))
				Variables[position].bvalue=true;
			else if(!strcmp(value,"false"))
				Variables[position].bvalue=false;
			else if(rightvalue)
				Variables[position].bvalue=bvalue;
			else
			{
				Error("Invalid Bool operator",MB_OK | MB_ICONINFORMATION,"%s -must be either true or false.\nLine: %d",value,line);
				return false;
			}
		}
		
		if(lType==FLOAT)
			Variables[position].fvalue=(float)atof(value)+fvalue+(float)ivalue;
	}
	
	if(buff[operand]=='+')
	{
		if(lType==INTEGER)
			Variables[position].ivalue+=atoi(value)+ivalue+(int)fvalue;
		
		if(lType==CHARACTER)
		{
			Error("Invalid Operator",MB_OK | MB_ICONINFORMATION,"Cannot add chars\nLine: %d",line);
			return false;
		}
		
		if(lType==BOOLEAN)
		{
			Error("Invalid Operator",MB_OK | MB_ICONINFORMATION,"Cannot add bools\nLine: %d",line);
			return false;
		}
		
		if(lType==FLOAT)
			Variables[position].fvalue+=(float)atof(value)+fvalue+(float)ivalue;
	}
	
	if(buff[operand]=='-')
	{
		if(lType==INTEGER)
			Variables[position].ivalue-=atoi(value)+ivalue+(int)fvalue;
		
		if(lType==CHARACTER)
		{
			Error("Invalid Operator",MB_OK | MB_ICONINFORMATION,"Cannot subtract chars\nLine: %d",line);
			return false;
		}
		
		if(lType==BOOLEAN)
		{
			Error("Invalid Operator",MB_OK | MB_ICONINFORMATION,"Cannot subtract bools\nLine: %d",line);
			return false;
		}
		
		if(lType==FLOAT)
			Variables[position].fvalue-=(float)atof(value)+fvalue+(float)ivalue;
	}
	
	if(buff[operand]=='*')
	{
		if(lType==INTEGER)
			Variables[position].ivalue=Variables[position].ivalue*atoi(value)+Variables[position].ivalue*ivalue+Variables[position].ivalue*(int)fvalue;
		
		if(lType==CHARACTER)
		{
			Error("Invalid Operator",MB_OK | MB_ICONINFORMATION,"Cannot multiply chars\nLine: %d",line);
			return false;
		}
		
		if(lType==BOOLEAN)
		{
			Error("Invalid Operator",MB_OK | MB_ICONINFORMATION,"Cannot multiply bools\nLine: %d",line);
			return false;
		}
		
		if(lType==FLOAT)
			Variables[position].fvalue=Variables[position].fvalue*(float)atof(value)+Variables[position].fvalue*fvalue+Variables[position].fvalue*(float)ivalue;
	}
	
	if(buff[operand]=='/')
	{
		if(lType==INTEGER)
		{
			if(atoi(value))
				Variables[position].ivalue=Variables[position].ivalue/atoi(value);
			else if(ivalue)
				Variables[position].ivalue=Variables[position].ivalue/ivalue;
			else if((int)fvalue)
				Variables[position].ivalue=Variables[position].ivalue/(int)fvalue;
			else
			{
				Error("Divide by 0",MB_OK | MB_ICONINFORMATION,"Cannot divide by 0.\nLine: %d",line);
				return false;
			}
		}
		
		if(lType==CHARACTER)
		{
			Error("Invalid Operator",MB_OK | MB_ICONINFORMATION,"Cannot multiply chars\nLine: %d",line);
			return false;
		}
		
		if(lType==BOOLEAN)
		{
			Error("Invalid Operator",MB_OK | MB_ICONINFORMATION,"Cannot multiply bools\nLine: %d",line);
			return false;
		}
		
		if(lType==FLOAT)
		{
			if((float)atof(value))
				Variables[position].fvalue=Variables[position].fvalue/(float)atof(value);
			else if(fvalue)
				Variables[position].fvalue=Variables[position].fvalue/fvalue;
			else if((float)ivalue)
				Variables[position].fvalue=Variables[position].fvalue/(float)ivalue;
			else
			{
				Error("Divide by 0",MB_OK | MB_ICONINFORMATION,"Cannot divide by 0.\nLine: %d",line);
				return false;
			}
		}
	}
	if(!strcmp(Variables[position].name,"integer"))
	{
		Error("fasdfads",MB_OK | MB_ICONINFORMATION,"what??? %d    %d",Variables[position].ivalue,ivalue);
	}
	return true;
}