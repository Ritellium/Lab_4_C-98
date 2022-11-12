#include "Header.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <charconv>

using namespace MyFunctions;

char* MyFunctions::wchar_to_char(const wchar_t* pwchar)
{
	// get the number of characters in the string.
	int currentCharIndex = 0;
	char currentChar = pwchar[currentCharIndex];

	while (currentChar != '\0')
	{
		currentCharIndex++;
		currentChar = pwchar[currentCharIndex];
	}

	const int charCount = currentCharIndex + 1;

	// allocate a new block of memory size char (1 byte) instead of wide char (2 bytes)
	char* filePathC = (char*)malloc(sizeof(char) * charCount);

	for (int i = 0; i < charCount; i++)
	{
		// convert to char (1 byte)
		char character = pwchar[i];

		*filePathC = character;

		filePathC += sizeof(char);

	}
	filePathC += '\0';

	filePathC -= (sizeof(char) * charCount);

	return filePathC;
}

void MyFunctions::sendMessage(FILE* file, const char* message)
{
	int length = strlen(message) + 1;
	fwrite(&length, sizeof(length), 1, file);
	for (int i = 0; i < length; i++)
	{
		fwrite(&message[i], sizeof(char), 1, file);
	}
}

void MyFunctions::readMessage(FILE* file, char* readHere)
{
	if (readHere != nullptr)
	{
		delete[] readHere;
	}
	int length = 0;
	fread(&length, sizeof(length), 1, file);
	readHere = new char[length];
	for (int i = 0; i < length; i++)
	{
		fread(&readHere[i], sizeof(char), 1, file);
	}
}