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
    char* file_name = (argv[1]);
    printf("I'm Sender number %s, type below to sent messages\n", argv[7]);

    LPCWSTR name_senders_ready = MyFunctions::GetWC(argv[2]);
    LPCWSTR name_continue_work = MyFunctions::GetWC(argv[3]);
    LPCWSTR name_end_work = MyFunctions::GetWC(argv[4]);
    LPCWSTR string_counter = MyFunctions::GetWC(argv[5]);
    LPCWSTR senders_counter = MyFunctions::GetWC(argv[6]);

    HANDLE this_ready = OpenEventW(EVENT_ALL_ACCESS, EVENT_MODIFY_STATE, name_senders_ready);
    HANDLE continue_work = OpenEventW(EVENT_ALL_ACCESS, EVENT_MODIFY_STATE, name_continue_work);
    HANDLE end_work = OpenEventW(EVENT_ALL_ACCESS, EVENT_MODIFY_STATE, name_end_work);
    HANDLE semaphore_string = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, SEMAPHORE_MODIFY_STATE, string_counter);
    HANDLE semaphore_senders = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, SEMAPHORE_MODIFY_STATE, senders_counter);

    if (this_ready == nullptr || semaphore_string == nullptr || semaphore_senders == nullptr || continue_work == nullptr || end_work == nullptr)
    {
        printf("Wrong HANDLEs of sync objects");

        delete[] file_name;

        return 0;
    }

    int action = 0;
    bool stopped = false;

    do
    {
        if (!stopped)
        {
            printf("<1> to input message, <other> to end work : \n");
            scanf_s("%d", &action);
        }
        if (action == 1 && !stopped)
        {
            error_file = fopen_s(&file_bin, file_name, "ab");
            if (error_file != 0)
            {
                printf("Error opening file");
                return 0;
            }
            auto message = new char[21];
            printf("Enter the message (maximum 20 symbols)\n");
            scanf_s("%s", message, 20);

            if (WaitForSingleObject(semaphore_string, 50) == WAIT_TIMEOUT)
            {
                printf("The file is full\n");
                delete[] message;
                SetEvent(this_ready);
                fclose(file_bin);
                WaitForSingleObject(continue_work, INFINITE);
                if (WaitForSingleObject(end_work, 50) == WAIT_TIMEOUT)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
            else
            {
                ReleaseSemaphore(semaphore_senders, 1, NULL);
                MyFunctions::sendMessage(file_bin, message);
                delete[] message;
                fclose(file_bin);
                continue;
            }
        }
        else if (action != 1 || stopped)
        {
            if (!stopped)
            {
                printf("Process stopped.\n");
            }
            stopped = true;
            SetEvent(this_ready);
            WaitForSingleObject(continue_work, INFINITE);
            if (WaitForSingleObject(end_work, 50) == WAIT_TIMEOUT)
            {
                continue;
            }
            else
            {
                break;
            }
        }     
    } while (true);

    Sleep(500);
    return 0;
}
