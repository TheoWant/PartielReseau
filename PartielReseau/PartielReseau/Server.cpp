#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "framework.h"

using namespace std;

#define WM_SOCKET WM_USER + 1

std::vector<SOCKET> clients;
int result;

std::string GetRequest(std::string data)
{
    std::string file_path = "";

    size_t pos = data.find("GET ");
    if (pos != -1)
    {
        size_t end_pos = data.find(" HTTP/1", pos + 4);
        if (end_pos != -1)
        {
            file_path = data.substr(pos + 4, end_pos - (pos + 4));
        }
    }

    return file_path;
}

Server::Server()
{

}

extern "C" LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


DWORD WINAPI Server::ServerThread(LPVOID lpParam) {
    SOCKET clientSocket;
    SOCKADDR_IN clientSocketInfo;

    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = NULL;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        NULL,       // Instance handle
        NULL        // Additional application data
    );

    //    ShowWindow(hwnd,1);

    unsigned short port = 1000;

    Server* server = new Server();

    int i = 0;

    server->listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server->listenSocket == INVALID_SOCKET)
    {
        std::cerr << "Error at listenSocket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }

    server->serverSocketInfo.sin_family = AF_INET;
    server->serverSocketInfo.sin_addr.s_addr = INADDR_ANY;
    server->serverSocketInfo.sin_port = htons(port);

    i = bind(server->listenSocket, (SOCKADDR*)&server->serverSocketInfo, sizeof(serverSocketInfo));
    if (i == SOCKET_ERROR)
    {
        std::cerr << "bind function failed with error: " << WSAGetLastError() << std::endl;
        i = closesocket(server->listenSocket);
        if (i == SOCKET_ERROR)
            std::cerr << "closesocket function failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }

    if (listen(server->listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "listen function failed with error: " << WSAGetLastError() << std::endl;
    }

    std::cout << "\033[30m\033[42mServer started.\033[0m" << std::endl;

    WSAAsyncSelect(server->listenSocket, hwnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE | FD_READ);

    MSG msg;
    BOOL bRet;

    while (GetMessage(&msg, hwnd, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    //PostMessage();
    return 0;
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SOCKET Accept;

    switch (uMsg)
    {

    case WM_SOCKET:
        // Determine whether an error occurred on the
        // socket by using the WSAGETSELECTERROR() macro
        if (WSAGETSELECTERROR(lParam))
        {
            // Display the error and close the socket
            closesocket((SOCKET)wParam);
            break;
        }
        // Determine what event occurred on the socket
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_ACCEPT:
            // Accept an incoming connection
            clients.push_back(accept(wParam, NULL, NULL));
            // Prepare accepted socket for read, write, and close notification
            WSAAsyncSelect(clients[clients.size()-1], hwnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
            std::cout << "Client connected !" << std::endl;
            break;
        case FD_READ:
            // Receive data from the socket in wParam
        {
            char buffer[1024];
            std::string msg = "";
            for (int i = 0; i < clients.size(); i++)
            {
                result = recv(clients[i], buffer, sizeof(buffer), 0);
                memmove(buffer, buffer, result);
                buffer[result] = '\0';
                printf("Bytes received: %d\n", result);
                msg += buffer;
                std::cout << msg;

                std::string file_path = GetRequest(msg);


                if (file_path == "/")
                    file_path = "index.html";

                std::cout << "\nREQUEST : " << file_path << "\n";

                std::string path = "../www/" + file_path;
                ifstream file(path);

                // Check if file is found and open
                if (!file.is_open()) {
                    std::cout << "\nImpossible d'ouvrir le fichier\n" << std::endl;
                }

                std::string content;
                
                std::string line;

                while (getline(file, line)) {
                    content += line;
                }

                file.close();

                std::string contentType;

                if (file_path.find(".html") != std::string::npos)
                {
                    contentType = "text/html";
                }
                else if (file_path.find(".css") != std::string::npos)
                {
                    contentType = "text/css";
                }
                else
                {
                    contentType = "text/plain";
                }

                std::string response =
                    "HTTP/1.1 200 OK \r\n"
                    "Content-Type: " + contentType + "\r\n"
                    "Content-Length: " + std::to_string(content.length()) + "\r\n"
                    "\r\n"
                    + content;

                cout << response << endl;

                const char* msg = response.data();
                char* chr = const_cast<char*>(msg);


                result = send(clients[i], msg, (int)strlen(msg), 0);
                if (result == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    return false;
                }
                printf("Bytes Sent: %ld\n", result);
                return true;
            }
            
            
        }
        break;
        case FD_WRITE:
            // The socket in wParam is ready for sending data
            break;

        case FD_CLOSE:
            // The connection is now closed
            closesocket((SOCKET)wParam);
            break;
        } 
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}