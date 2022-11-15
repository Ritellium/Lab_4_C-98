#include "../Header.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#pragma warning (disable:4996)

int main(int argc, char* argv[])
{
    FILE* file_bin;
    errno_t error_file;
    char* file_name = argv[1];
    printf("I'm Sender number %d, type below to sent messages\n", atoi(argv[2]));

    wchar_t* name11 = new wchar_t[30];
    wcscpy(name11, L"sender_ready_");
    wchar_t* name33 = new wchar_t[30];
    wcscpy(name33, L"someone_reads");

    std::wstring s = std::to_wstring(atoi(argv[2]));
    const wchar_t* senderNum = s.c_str();

    HANDLE this_ready = OpenEvent(EVENT_ALL_ACCESS, EVENT_MODIFY_STATE, wcscat(name11, senderNum));
    HANDLE critsec_access = OpenEvent(EVENT_ALL_ACCESS, EVENT_MODIFY_STATE, name33);

    if (this_ready == nullptr || critsec_access == nullptr)
    {
        printf("Wrong HANDLEs of events");
        return 0;
    }

    printf_s("%ls\n", name11);
    int action = 0;
    do
    {
        WaitForSingleObject(critsec_access, INFINITE);

        try_again:
        printf(" <1> to input message to send (maximum 20 symbols), <0> to end work : \n");
        scanf_s("%d", &action);

        if (action == 1)
        {
            error_file = fopen_s(&file_bin, file_name, "ab");
            if (error_file != 0)
            {
                printf("Error opening file");
                return 0;
            }
            auto message = new char[21];
            printf("Enter the message\n");
            scanf_s("%s", message, 20);
            MyFunctions::sendMessage(file_bin, message);

            fclose(file_bin);
            SetEvent(this_ready);
            SetEvent(critsec_access);
            break;
        }
        else if (action == 0)
        {
            printf("Stopping process...\n");
            SetEvent(this_ready);
            SetEvent(critsec_access);
            break;
        }     
        else
        {
            printf("wrong input, try again: \n");
            goto try_again;
        }
    } while (true);

    Sleep(500);
    return 0;
}
