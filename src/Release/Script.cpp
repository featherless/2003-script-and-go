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

#include <fstream.h>
#include <string.h>
#include <windows.h>
#include <ctype.h>
#include <map>

#define NUMCOMMANDS		12
#define NUMPARAMS		20

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

char commands[NUMCOMMANDS] [20]=
{
	"DrawLine",
	"EnableTextures",
	"DisableTextures",
	"Reset",
	"SetColor",
	"Move",
	"Box",
	"EnableCulling",
	"DisableCulling",
	"CullFace",
	"Rotate",
	"DrawMode"
};

unsigned int commandparams[NUMCOMMANDS] [2]=
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

float variables[1024];

void Error(char* title, UINT uType, const char* fmt, ...);

bool openedonce=false;

bool scripting()
{
	ifstream script;

	if(!openedonce)
	{
		for(int a=0;a<1024;a++)
			variables[a]=0;
	}

	openedonce=true;

	script.open("script.spt",ios::nocreate | ios::in);

	if(script)
	{
		script.seekg(ios::beg);
		
		char buff[1024]=" ";	//buffer to hold our whole string
		char buff2=' ';			//buffer to hold each character

		bool comment=false;		//Which type of comment we're in
		bool comment2=false;

		int pos=0;				//Position in the buffer

		bool begin=false;		//Program is started
		bool end=false;			//Program is done

		int line=1;
		
		while(!script.eof() && !end)	//Test if at the end of the file or END
		{
			script.get(buff2);	//Get the current character

			if(buff2==-1)	//Double check if we are at the end of the file
				break;		//break out of the loop
			if(buff2=='/')	//Test for comments
			{
				char buff3;
				script.get(buff3);
				if(buff3=='*')
					comment=true;
				else if(buff3=='/')
					comment2=true;
				else
					script.seekg(-1,ios::cur);
			}
			if(!comment && !comment2 && buff2!=';')	//If we are NOT at the end of a command
			{
				buff[pos]=buff2;	//copy the character in to the buffer
				pos++;				//increase the position
			}

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
				if(!strcmp(buff,"BEGIN") && !begin)	//if BEGIN and not already begun...
					begin=true;						//Then we've begun
				else if(!strcmp(buff,"BEGIN") && begin)	//If there are two begins....
				{
					Error("Multiple BEGINs",MB_OK | MB_ICONINFORMATION,"Two BEGIN commands found.");
					return false;
				}

				if(!strcmp(buff,"END") && !begin)
				{
					Error("End in wrong spot",MB_OK | MB_ICONINFORMATION,"Error, found END, but not BEGIN. Make sure BEGIN is before END.");
					return false;
				}
				else if(!strcmp(buff,"END") && begin)
					end=true;

				if(begin && strcmp(buff,"BEGIN") && strcmp(buff,"END"))	//if we've completely started and
				{	//the commands aren't BEGIN or END

					char command[100]=" ";	//Fill the command buffer with stuff....
					unsigned int lparen=-1;	//Set the left parentheses position to -1, an impossible position
					
					for(unsigned int a=0;a<strlen(buff);a++)
					{
						if(buff[a]!='(')		//Try to find the left parenthesis
							command[a]=buff[a];	//If we didn't find it, just keep copying the command to the buffer
						else
						{
							lparen=a;			//If we do find it, set the position and break from the loop, to save time
							break;
						}
					}
					command[a]='\0';			//Close up our command buffer

					if(command[0]=='@')
					{
						int type=-1;
						int stop=-1;
						for(int a=1;a<(int)strlen(command);a++)
						{
							if(command[a]=='=')
							{
								type=EQUALING;
								stop=a;
							}

							if(command[a]=='+')
							{
								type=ADDITION;
								stop=a;
							}

							if(command[a-1]=='+' && command[a]=='+')
							{
								type=INCREMENTING;
								stop=a;
							}

							if(command[a]=='-')
							{
								type=SUBTRACTION;
								stop=a;
							}

							if(command[a-1]=='-' && command[a]=='-')
							{
								type=DECREMENTING;
								stop=a;
							}

							if(command[a]=='*')
							{
								type=MULTIPLICATION;
								stop=a;
							}

							if(command[a]=='/')
							{
								type=DIVISION;
								stop=a;
							}
							command[a-1]=command[a];

						}
						command[a-1]='\0';
						if(type==-1)
						{
							Error("Couldn't find operand",MB_OK | MB_ICONINFORMATION,"Couldn't find operand.");
							return false;
						}
						char pointer[40];
						int point;
						float value=0.0f;
						for(a=0;a<stop-1;a++)
							pointer[a]=command[a];
						pointer[a]='\0';
						point=atoi(pointer);

						if(type!=INCREMENTING && type!=DECREMENTING)
						{
							for(a=stop;a<(int)strlen(command);a++)
								pointer[a-stop]=command[a];
							pointer[a-stop+1]='\0';
							value=(float)atof(pointer);
						}

						if(type==EQUALING)
							variables[point]=value;

						if(type==ADDITION)
							variables[point]+=value;

						if(type==INCREMENTING)
							variables[point]++;

						if(type==SUBTRACTION)
							variables[point]-=value;

						if(type==DECREMENTING)
							variables[point]--;

						if(type==MULTIPLICATION)
							variables[point]*=value;

						if(type==DIVISION)
							variables[point]/=value;
					}
					else
					{
						bool foundcommand=false;	//Bool for if the command even exists
						int numofcommand=0;			//The number of the command

						for(a=0;a<NUMCOMMANDS;a++)				//Loop through all of the commands
						{
							if(!strcmp(command,commands[a]))	//If we found it....
							{
								foundcommand=true;				//SAY that we found it
								numofcommand=a;					//Set the position
								break;							//Break from the loop
							}
						}
						if(!foundcommand)	//If we didn't find the command
						{
							Error("Command doesn't exist",MB_OK | MB_ICONINFORMATION,"Error, %s, does not exist\nLine: %d",command,line);
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
									
									if(value[0]==34 || value[0]==39)
									{
										for(unsigned int b=1;b<strlen(value)-1;b++)
											value[b-1]=value[b];
										value[b-1]='\0';
										
										strcpy(cparams[numcparams],value);
										numcparams++;				//Increase our number of cparams
									}
									else
									{
										if(value[0]=='@')
										{
											for(unsigned int b=1;b<strlen(value);b++)
												value[b-1]=value[b];
											value[b-1]='\0';
											int point=atoi(value);
											params[numparams]=variables[point];
										}
										else
											params[numparams]=atof(value);	//Convert the value to a float
										numparams++;				//Increase our number of params
									}
									
									character=0;				//Reset the current character
									strcpy(value," ");			//Fill the buffer with junk
								}
							}
						}
						
						bool ok=true;
						
						if(commandparams[numofcommand] [0]>numparams)
						{
							Error("Too few numerical parameters",MB_OK | MB_ICONINFORMATION,"Too few numerical parameters for %s\nRequires %d\nLine: %d",command,commandparams[numofcommand] [0],line);
							if(ok)
								Error("Contains",MB_OK | MB_ICONINFORMATION,"Contains:\n%d character parameters\n%d numerical parameters",numcparams,numparams);
							ok=false;
						}
						if(commandparams[numofcommand] [0]<numparams || (lparen!=-1 && commandparams[numofcommand] [0]==0 && commandparams[numofcommand] [1]==0))
						{
							if(commandparams[numofcommand] [0]>0)
								Error("Too many numerical parameters",MB_OK | MB_ICONINFORMATION,"Too many numerical parameters for %s\nRequires only %d\nLine: %d",command,commandparams[numofcommand] [0],line);
							else
								Error("Too many numerical parameters",MB_OK | MB_ICONINFORMATION,"Too many numerical parameters for %s\nRequires none.\nLine: %d",command,line);
							if(ok)
								Error("Contains",MB_OK | MB_ICONINFORMATION,"Contains:\n%d character parameters\n%d numerical parameters",numcparams,numparams);
							ok=false;
						}
						
						if(commandparams[numofcommand] [1]>numcparams)
						{
							Error("Too few character parameters",MB_OK | MB_ICONINFORMATION,"Too few character parameters for %s\nRequires %d\nLine: %d",command,commandparams[numofcommand] [1],line);
							if(ok)
								Error("Contains",MB_OK | MB_ICONINFORMATION,"Contains:\n%d character parameters\n%d numerical parameters",numcparams,numparams);
							ok=false;
						}
						if(commandparams[numofcommand] [1]<numcparams || (lparen!=-1 && commandparams[numofcommand] [0]==0 && commandparams[numofcommand] [1]==0))
						{
							if(commandparams[numofcommand] [1]>0)
								Error("Too many character parameters",MB_OK | MB_ICONINFORMATION,"Too many character parameters for %s\nRequires only %d\nLine: %d",command,commandparams[numofcommand] [1],line);
							else
								Error("Too many character parameters",MB_OK | MB_ICONINFORMATION,"Too many character parameters for %s\nRequires none.\nLine: %d",command,line);
							if(ok)
								Error("Contains",MB_OK | MB_ICONINFORMATION,"Contains:\n%d character parameters\n%d numerical parameters",numcparams,numparams);
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