#include "framework.h"

int main(int argc, char** argv)
{
    WSADATA wsaData;

    int i = 0;
    i = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (i != NO_ERROR)
    {
        std::cerr << "Error at WSAStartup: " << i << std::endl;
        return 0;
    }

    HANDLE hThread;
    DWORD dwThreadId;
    hThread = CreateThread(NULL, 0, Server::ServerThread, NULL, 0, &dwThreadId);

    MSG msg;
    BOOL bRet;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}