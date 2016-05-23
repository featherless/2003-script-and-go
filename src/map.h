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

#define BOOLEAN		1
#define CHARACTER	2
#define FLOAT		3
#define INTEGER		4

class map
{
private:
	int type;
	bool used;
public:
	char name[100];
	bool bvalue;
	char cvalue[1024];
	float fvalue;
	int ivalue;

	map();
	bool create(char* Name,char* Type);
	bool isOn();
	int Type();
};

map::map()
{
	used=false;
}

bool map::isOn()
{
	return used;
}

int map::Type()
{
	return type;
}

bool map::create(char* Name,char* Type)
{
	strcpy(name,Name);

	if(!strcmp(Type,"bool"))
		type=BOOLEAN;
	else if(!strcmp(Type,"char"))
		type=CHARACTER;
	else if(!strcmp(Type,"float"))
		type=FLOAT;
	else if(!strcmp(Type,"int"))
		type=INTEGER;
	else
		return false;

	used=true;

	return true;
}