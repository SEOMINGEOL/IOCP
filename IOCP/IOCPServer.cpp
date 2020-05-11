#include "IOCPServer.h"
#include "Socket.h"
#include "User.h"
#include "Exception.h"
#include <vector>

DWORD WINAPI workerThread(LPVOID hIOCP)
{
    HANDLE threadHandler = *((HANDLE*)hIOCP);
    DWORD receiveBytes;
    DWORD sendBytes;
    DWORD completionKey;
    DWORD flags;

    SOCKETINFO* eventSocket;

    while (1)
    {
        // ����� �Ϸ� ������
        if (GetQueuedCompletionStatus(threadHandler, &receiveBytes, &completionKey, (LPOVERLAPPED*)&eventSocket, INFINITE) == FALSE)
        {
            if (receiveBytes == 0)
            {
                //������ ���� ����
                Log(Error, Log_Form::Format("Client Abnormal DisConnected(" + (std::string)")", WSAGetLastError()));
            }
            else
            {
                //���� ���ᰡ �ƴ� �ٸ� ������ ���
                Log(Error, Log_Form::Format("GetQueuedCompletionStatus Failure(" + (std::string)")", WSAGetLastError()));
            }
            closesocket(eventSocket->socket);
            free(eventSocket);
            return 1;
        }

        eventSocket->dataBuffer.len = receiveBytes;

        if (receiveBytes == 0)
        {
            //�������� ��������
            Log(Normal, Log_Form::Format("Client Normal DisConnected(" + (std::string)")", WSAGetLastError()));
            closesocket(eventSocket->socket);
            free(eventSocket);
            continue;
        }
        else
        {
            //������ ó��
            Log_printf(Normal, "Receive message : %s (%d bytes", eventSocket->dataBuffer.buf, eventSocket->dataBuffer.len);
            if (WSASend(eventSocket->socket, &(eventSocket->dataBuffer), 1, &sendBytes, 0, NULL, NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSA_IO_PENDING)
                {
                    Log(Error, Log_Form::Format("Fail WSASend(error_code : %d)\n", WSAGetLastError()));
                }
            }

            Log_printf(Normal, "Send message : %s (%d bytes)", eventSocket->dataBuffer.buf, eventSocket->dataBuffer.len);
            memset(eventSocket->messageBuffer, 0x00, MAX_BUF_SIZE);
            eventSocket->receiveBytes = 0;
            eventSocket->sendBytes = 0;
            eventSocket->dataBuffer.len = MAX_BUF_SIZE;
            eventSocket->dataBuffer.buf = eventSocket->messageBuffer;
            flags = 0;

            if (WSARecv(eventSocket->socket, &(eventSocket->dataBuffer), 1, &receiveBytes, &flags, &eventSocket->overlapped, NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSA_IO_PENDING)
                {
                    Log_printf(Error, "Fail WSARecv(error_code : %d)\n", WSAGetLastError());
                }
            }
        }
    }
}

IOCPServer::IOCPServer()
{
    // ����°���� ó���ϴ� �ڵ鷯
    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    //������ ���� CPU * 2
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    int threadCount = systemInfo.dwNumberOfProcessors * 2;

    //unsigned long threadId;

    // thread Handler ����
    HANDLE* hThread = (HANDLE*)malloc(threadCount * sizeof(HANDLE));

    // thread ����
    for (int i = 0; i < threadCount; i++)
    {
        //threadId�� ���� �����ϴ°��� ���� ������ ����
        //hThread[i] = CreateThread(NULL, 0, makeThread, &hIOCP, 0, &threadId);
        hThread[i] = CreateThread(NULL, 0, workerThread, &hIOCP, 0, NULL);
    }
}

BOOL IOCPServer::Start()
{
    try
    {
        Socket socket(SERVER_PORT);

        socket.Bind();
        socket.Listen();

        SOCKADDR_IN clientAddr;
        int addrLen = sizeof(SOCKADDR_IN);
        memset(&clientAddr, 0, addrLen);

        SOCKETINFO* socketInfo;
        DWORD receiveBytes;
        DWORD flags;

        std::vector<User*> users = std::vector<User*>();

        Log_printf(Normal, "Server Start");
        while (1)
        {
            User* user = socket.Accept();

            socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
            memset((void*)socketInfo, 0x00, sizeof(struct SOCKETINFO));
            socketInfo->socket = user->GetUserSocket();
            socketInfo->receiveBytes = 0;
            socketInfo->sendBytes = 0;
            socketInfo->dataBuffer.len = MAX_BUF_SIZE;
            socketInfo->dataBuffer.buf = socketInfo->messageBuffer;
            flags = 0;

            // ��ø ��Ĺ�� �����ϰ� �Ϸ�� ����� �Լ��� �Ѱ��ش�.
            hIOCP = CreateIoCompletionPort((HANDLE)user->GetUserSocket(), hIOCP, (DWORD)socketInfo, 0);

            Log(Normal, "Client Connected(" + std::string(user->GetUserIp()) + (std::string)")");
            //������ CP ��� WSARecv, WSASend�� ���� ��ɾ �־����� �Է��̳� ����� �Ϸ�� �� ���ξ����忡�� ó���� �� �ְ� ��.
            if (WSARecv(socketInfo->socket, &socketInfo->dataBuffer, 1, &receiveBytes, &flags, &(socketInfo->overlapped), NULL))
            {
                if (WSAGetLastError() != WSA_IO_PENDING)
                {
                    printf("Error - IO pending Failure\n");
                    return 1;
                }
            }
            
        }
    }
    catch (SocketException& expt)
    {
        //����, bind, Listen�� �߻��Ǵ¿��� ĳġ
        expt.ShowException();
    }
    return 1;
}