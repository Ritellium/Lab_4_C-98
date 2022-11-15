#include <Windows.h>
#include <iostream>
#include <string>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <charconv>
#include "../Header.h"

#pragma warning (disable:4996)

/*
Написать программу для передачи сообщений между процессами через общий файл. 
Программа включает один процесс Receiver и несколько процессов Sender. 
Процессы Sender посылают сообщения процессу Receiver

remember, no STD...
*/

int main(int argc, char* argv[])
{
	errno_t error_file;
	FILE* file_bin;
	// Input for Creator

	auto file_name = new wchar_t[40];
	printf("input name of binary file (it will be created): ");
	scanf_s("%ls", file_name, 41);

	error_file = fopen_s(&file_bin, MyFunctions::wchar_to_char(file_name), "wb");
	if (error_file != 0)
	{
		printf("Error opening file");
		return 0;
	}
	fclose(file_bin);

	int stringsEmount = 0;
	printf("input number of strings in .bin file: ");
	scanf_s("%d", &stringsEmount);

	int sendersEmount = 0;
	printf("input number of Sender processes: ");
	scanf_s("%d", &sendersEmount);

	HANDLE* senders_ready = new HANDLE[sendersEmount];
	HANDLE someone_reads;

	LPWSTR* name1 = new LPWSTR[sendersEmount];
	for (int i = 0; i < sendersEmount; i++)
	{
		std::wstring s = std::to_wstring(i + 1);
		const wchar_t* senderNum = s.c_str();

		name1[i] = new wchar_t[40];
		wcscpy(name1[i], L"sender_ready_");
		senders_ready[i] = CreateEvent(nullptr, FALSE, FALSE, wcscat(name1[i], senderNum));
		printf_s("%ls\n", name1);
	} 
	wchar_t name2[] = L"someone_reads";
	someone_reads = CreateEvent(nullptr, FALSE, FALSE, name2);
	
	// Events initialization 

	// Senders work
	auto SenderAllocation = new wchar_t[40];
	wcscpy(SenderAllocation, L"Sender.exe");
	wcscat(SenderAllocation, L" ");
	wcscat(SenderAllocation, file_name);
	wcscat(SenderAllocation, L" ");

	auto* data = new LPWSTR[sendersEmount];
	for (int i = 0; i < sendersEmount; i++)
	{
		data[i] = new wchar_t[40];
		wcscpy(data[i], SenderAllocation);
		std::wstring s = std::to_wstring(i + 1);
		const wchar_t* senderNum = s.c_str();
		wcscat(data[i], senderNum);
	}

	auto* senders_StartInf = new _STARTUPINFOW[sendersEmount];
	auto* senders_PrInf = new _PROCESS_INFORMATION[sendersEmount];

	for(int i=0; i<sendersEmount; i++)
	{
		ZeroMemory(&senders_StartInf[i], sizeof(_STARTUPINFOW));
		senders_StartInf[i].cb = sizeof(_STARTUPINFOW);
	}	
	
	// создаем Senders
	for (int i = 0; i < sendersEmount; i++)
	{
		if (!CreateProcessW(nullptr, data[i], nullptr, nullptr, FALSE,
			CREATE_NEW_CONSOLE, nullptr, nullptr, &senders_StartInf[i], &senders_PrInf[i]))
		{
			printf("The Sender %d is not created. Process stopping...\n", (i + 1));
			return 0;
		}
		else
		{
			printf("The Sender %d is runned.\n", (i + 1));
		}
	}
	SetEvent(someone_reads);

	WaitForMultipleObjects(sendersEmount, senders_ready, true, INFINITE);
	// Вывод бинарника на консоль

	error_file = fopen_s(&file_bin, MyFunctions::wchar_to_char(file_name), "rb");
	if (error_file != 0)
	{
		printf("Error opening file");
		return 0;
	}

	char* readed_message = new char[30];
	printf("Messages: \n");
	for (int i = 0; i < sendersEmount; i++)
	{
		MyFunctions::readMessage(file_bin, readed_message);
		printf("%s\n", readed_message);
	}

	fclose(file_bin);

	// Конец вывода бинарника

	for (int i = 0; i < sendersEmount; i++)
	{
		CloseHandle(senders_PrInf[i].hThread);
		CloseHandle(senders_PrInf[i].hProcess);
	}

	// Чистка

	delete[] senders_ready;
	for (int i = 0; i < sendersEmount; i++)
	{
		delete[] name1[i];
	}
	delete[] name1;
	delete[] SenderAllocation;
	for (int i = 0; i < sendersEmount; i++)
	{
		delete[] data[i];
	}
	delete[] data;
	delete[] senders_StartInf;
	delete[] senders_PrInf;

	return 0;
}