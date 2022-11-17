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
	fclose(file_bin); // just file creation (and check of input)

	int stringsEmount = 0;
	printf("input number of strings in .bin file: ");
	scanf_s("%d", &stringsEmount);

	int sendersEmount = 0;
	printf("input number of Sender processes: ");
	scanf_s("%d", &sendersEmount);

	HANDLE* senders_ready = new HANDLE[sendersEmount];
	HANDLE* continue_work = new HANDLE[sendersEmount];
	HANDLE* end_work = new HANDLE[sendersEmount];
	HANDLE string_counter;
	HANDLE senders_counter;

	LPWSTR* names_senders_ready = new LPWSTR[sendersEmount];
	LPWSTR* names_continue_work = new LPWSTR[sendersEmount];
	LPWSTR* names_end_work = new LPWSTR[sendersEmount];

	// Events initialization 
	for (int i = 0; i < sendersEmount; i++)
	{
		std::wstring s = std::to_wstring(i + 1);
		const wchar_t* senderNum = s.c_str();

		names_senders_ready[i] = new wchar_t[40];
		wcscpy(names_senders_ready[i], L"sender_ready_");
		wcscat(names_senders_ready[i], senderNum);
		names_continue_work[i] = new wchar_t[40];
		wcscpy(names_continue_work[i], L"continue_work_");
		wcscat(names_continue_work[i], senderNum);
		names_end_work[i] = new wchar_t[40];
		wcscpy(names_end_work[i], L"end_work_");
		wcscat(names_end_work[i], senderNum);

		senders_ready[i] = CreateEventW(nullptr, FALSE, FALSE, names_senders_ready[i]);
		continue_work[i] = CreateEventW(nullptr, FALSE, FALSE, names_continue_work[i]);
		end_work[i] = CreateEventW(nullptr, FALSE, FALSE, names_end_work[i]);
	} 
	wchar_t name_str_counter[] = L"string_counter";
	wchar_t name_sender_counter[] = L"senders_counter";
	string_counter = CreateSemaphoreW(nullptr, 0, stringsEmount, name_str_counter);
	ReleaseSemaphore(string_counter, stringsEmount, NULL);
	senders_counter = CreateSemaphoreW(nullptr, 0, sendersEmount, name_sender_counter);

	// Senders work
	auto SenderAllocation = new wchar_t[40];
	wcscpy(SenderAllocation, L"Sender.exe"); // argv[0]
	wcscat(SenderAllocation, L" ");
	wcscat(SenderAllocation, file_name); // argv[1]

	auto* data = new LPWSTR[sendersEmount];
	for (int i = 0; i < sendersEmount; i++)
	{
		data[i] = new wchar_t[150];
		wcscpy(data[i], SenderAllocation);
		std::wstring s = std::to_wstring(i + 1);
		const wchar_t* senderNum = s.c_str();
		wcscat(data[i], L" ");
		wcscat(data[i], names_senders_ready[i]); // argv[2]
		wcscat(data[i], L" ");
		wcscat(data[i], names_continue_work[i]); // argv[3]
		wcscat(data[i], L" ");
		wcscat(data[i], names_end_work[i]); // argv[4]
		wcscat(data[i], L" ");
		wcscat(data[i], name_str_counter); // argv[5]
		wcscat(data[i], L" ");
		wcscat(data[i], name_sender_counter); // argv[6]
		wcscat(data[i], L" ");
		wcscat(data[i], senderNum); // argv[7]
	}

	_STARTUPINFOW* senders_StartInf = new _STARTUPINFOW[sendersEmount];
	_PROCESS_INFORMATION* senders_PrInf = new _PROCESS_INFORMATION[sendersEmount];

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
			// Чистка
			{
				delete[] SenderAllocation;

				delete[] names_end_work;
				delete[] names_continue_work;
				delete[] names_senders_ready;

				delete[] end_work;
				delete[] continue_work;
				delete[] senders_ready;

				delete[] data;

				delete[] senders_StartInf;
				delete[] senders_PrInf;
			}// Чистка
			return 0;
		}
		else
		{
			printf("The Sender %d is runned.\n", (i + 1));
		}
	}

	int action = 0;
	do
	{
		WaitForMultipleObjects(sendersEmount, senders_ready, true, INFINITE);

		printf("<1> to read messages, <other> to end work : \n");
		scanf_s("%d", &action);

		int semaphores_on = 0;

		if (action == 1)
		{
			fopen_s(&file_bin, MyFunctions::wchar_to_char(file_name), "rb");

			char* readed_message = new char[30];
			printf("Messages: \n");

			while(WaitForSingleObject(senders_counter, 20) != WAIT_TIMEOUT)
			{
				MyFunctions::readMessage(file_bin, readed_message);
				printf("%s\n", readed_message);
				semaphores_on++;
			}

			delete[] readed_message;
			fclose(file_bin);

			ReleaseSemaphore(string_counter, stringsEmount, NULL);
			for (int i = 0; i < sendersEmount; i++)
			{
				SetEvent(continue_work[i]);
			}
		}

		if (action != 1 || semaphores_on < stringsEmount)
		{
			if (semaphores_on < stringsEmount)
			{
				printf("There are no more working senders, process stopping...");
			}
			for (int i = 0; i < sendersEmount; i++)
			{
				SetEvent(continue_work[i]);
				Sleep(5);
				SetEvent(end_work[i]);
				Sleep(25);
				CloseHandle(senders_PrInf[i].hThread);
				CloseHandle(senders_PrInf[i].hProcess);
			}			
			break;
		}

		fopen_s(&file_bin, MyFunctions::wchar_to_char(file_name), "wb");
		fclose(file_bin);
		printf("Now waiting for messages\n");
		Sleep(200);

	} while (true);
	// Чистка
	{
		delete[] SenderAllocation;

		delete[] names_end_work;
		delete[] names_continue_work;
		delete[] names_senders_ready;

		delete[] end_work;
		delete[] continue_work;
		delete[] senders_ready;

		delete[] data;

		delete[] senders_StartInf;
		delete[] senders_PrInf;
	}
	return 0;
}