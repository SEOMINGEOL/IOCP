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
        // 입출력 완료 대기시작
        if (GetQueuedCompletionStatus(threadHandler, &receiveBytes, &completionKey, (LPOVERLAPPED*)&eventSocket, INFINITE) == FALSE)
        {
            if (receiveBytes == 0)
            {
                //비정상 접속 종료
                Log(Error, Log_Form::Format("Client Abnormal DisConnected(" + (std::string)")", WSAGetLastError()));
            }
            else
            {
                //접속 종료가 아닌 다른 에러일 경우
                Log(Error, Log_Form::Format("GetQueuedCompletionStatus Failure(" + (std::string)")", WSAGetLastError()));
            }
            closesocket(eventSocket->socket);
            free(eventSocket);
            return 1;
        }

        eventSocket->dataBuffer.len = receiveBytes;

        if (receiveBytes == 0)
        {
            //정상적인 접속종료
            Log(Normal, Log_Form::Format("Client Normal DisConnected(" + (std::string)")", WSAGetLastError()));
            closesocket(eventSocket->socket);
            free(eventSocket);
            continue;
        }
        else
        {
            //데이터 처리
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
    // 입출력결과를 처리하는 핸들러
    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    //스레드 생성 CPU * 2
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    int threadCount = systemInfo.dwNumberOfProcessors * 2;

    //unsigned long threadId;

    // thread Handler 선언
    HANDLE* hThread = (HANDLE*)malloc(threadCount * sizeof(HANDLE));

    // thread 생성
    for (int i = 0; i < threadCount; i++)
    {
        //threadId로 따로 제어하는것은 없기 때문에 공백
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

            // 중첩 소캣을 지정하고 완료시 실행될 함수를 넘겨준다.
            hIOCP = CreateIoCompletionPort((HANDLE)user->GetUserSocket(), hIOCP, (DWORD)socketInfo, 0);

            Log(Normal, "Client Connected(" + std::string(user->GetUserIp()) + (std::string)")");
            //최초의 CP 등록 WSARecv, WSASend와 같은 명령어가 주어져야 입력이나 출력이 완료된 후 메인쓰레드에서 처리할 수 있게 됨.
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
        //생성, bind, Listen시 발생되는에러 캐치
        expt.ShowException();
    }
    return 1;
}