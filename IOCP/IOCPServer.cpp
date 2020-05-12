#include "IOCPServer.h"
#include "Socket.h"
#include "Exception.h"
#include <vector>

static std::vector<User*> users = std::vector<User*>();

void IOCPServer::AddUser(User* user)
{
    user_mutex.lock();
    users.push_back(user);
    user_mutex.unlock();
    Log_printf(Normal, "유저가 추가되었습니다.(%s) 현재 유저 수 : %d", user->GetUserIp().c_str(), users.size());
}

void IOCPServer::DeleteUser(SOCKET userSocket)
{
    
    user_mutex.lock();
    int length = users.size();
    for (int i = 0 ; i < length ; i++)
    {
        if (userSocket == users[i]->GetUserSocket())
        {
            Log_printf(Normal, "유저가 나갔습니다.(%s)", users[i]->GetUserIp().c_str());
            users.erase(users.begin() + i);
            Log_printf(Normal, "현재 유저 수 : %d", users.size());
            break;
        }
    }
    user_mutex.unlock();
}

void IOCPServer::Send_Data(SOCKET socket, WSABUF* buf)
{
    DWORD sendBytes;
    if (WSASend(socket, buf, 1, &sendBytes, 0, NULL, NULL) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            Log_printf(Error, "Fail WSASend(error_code : %d)", WSAGetLastError());
        }
    }
}

void IOCPServer::SendAllClient(WSABUF* buf)
{
    int len = users.size();
    //Log_printf(Normal, "Send message : %s (%d bytes)", buf->buf, buf->len);
    for (int i = 0; i < len; i++)
    {
        Send_Data(users[i]->GetUserSocket(), buf);
    }
}

void IOCPServer::Read_Data(SOCKET socket, WSABUF* buf, WSAOVERLAPPED* overlapped)
{
    DWORD receiveBytes;
    DWORD flags = 0;

    
    if (WSARecv(socket, buf, 1, &receiveBytes, &flags, overlapped, NULL) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            Log_printf(Error, "Fail WSARecv(error_code : %d)", WSAGetLastError());
        }
    }
}

DWORD WINAPI IOCPServer::workerThread(LPVOID hIOCP)
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
                Log_printf(Error, "Client Abnormal DisConnected(%d)", WSAGetLastError());
                DeleteUser(eventSocket->socket);
            }
            else
            {
                //접속 종료가 아닌 다른 에러일 경우
                Log_printf(Error, "CGetQueuedCompletionStatus Failure(%d)", WSAGetLastError());
                DeleteUser(eventSocket->socket);
            }

            closesocket(eventSocket->socket);
            free(eventSocket);
            return 1;
        }

        eventSocket->dataBuffer.len = receiveBytes;

        if (receiveBytes == 0)
        {
            //정상적인 접속종료
            Log_printf(Error, "Client Normal DisConnected(%d)", WSAGetLastError());
            DeleteUser(eventSocket->socket);
            closesocket(eventSocket->socket);
            free(eventSocket);
            continue;
        }
        else
        {
            //데이터 처리
            SendAllClient(&(eventSocket->dataBuffer));

            /*
            //미사용으로 주석
            if (WSASend(eventSocket->socket, &(eventSocket->dataBuffer), 1, &sendBytes, 0, NULL, NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSA_IO_PENDING)
                {
                    Log_printf(Error, "Fail WSASend(error_code : %d)", WSAGetLastError());
                }
            }
            */

            Log_printf(Normal, "Receive message : %s (%d bytes)", eventSocket->dataBuffer.buf, eventSocket->dataBuffer.len);
            memset(eventSocket->messageBuffer, 0x00, MAX_BUF_SIZE);
            eventSocket->receiveBytes = 0;
            eventSocket->sendBytes = 0;
            eventSocket->dataBuffer.len = MAX_BUF_SIZE;
            eventSocket->dataBuffer.buf = eventSocket->messageBuffer;
            flags = 0;

            Read_Data(eventSocket->socket, &(eventSocket->dataBuffer), &eventSocket->overlapped);
            /*
            미사용으로 주석
            if (WSARecv(eventSocket->socket, &(eventSocket->dataBuffer), 1, &receiveBytes, &flags, &eventSocket->overlapped, NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSA_IO_PENDING)
                {
                    Log_printf(Error, "Fail WSARecv(error_code : %d)", WSAGetLastError());
                }
            }
            */
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

        SOCKETINFO* socketInfo;
        DWORD receiveBytes;
        DWORD flag;

        Log_printf(Normal, "Server Start");
        while (1)
        {
            //Accept에서는 Hold 됨.
            User* user = socket.Accept();

            
            socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
            memset((void*)socketInfo, 0x00, sizeof(struct SOCKETINFO));
            socketInfo->socket = user->GetUserSocket();
            socketInfo->receiveBytes = 0;
            socketInfo->sendBytes = 0;
            socketInfo->dataBuffer.len = MAX_BUF_SIZE;
            socketInfo->dataBuffer.buf = socketInfo->messageBuffer;
            
            /*
            user->socketInfo->socket = user->GetUserSocket();
            */
            flag = 0;
            

            // 중첩 소캣을 지정하고 완료시 실행될 함수를 넘겨준다.
            hIOCP = CreateIoCompletionPort((HANDLE)user->GetUserSocket(), hIOCP, (DWORD)socketInfo, 0);

            //유저 추가
            AddUser(user);

            Log_printf(Normal, "Client Connected(%s)", user->GetUserIp().c_str());
            //최초의 CP 등록 WSARecv, WSASend와 같은 명령어가 주어져야 입력이나 출력이 완료된 후 메인쓰레드에서 처리할 수 있게 됨.
            
            Read_Data(socketInfo->socket, &socketInfo->dataBuffer, &(socketInfo->overlapped));
            /*
            미사용으로 주석
            if (WSARecv(socketInfo->socket, &socketInfo->dataBuffer, 1, &receiveBytes, &flag, &(socketInfo->overlapped), NULL))
            {
                if (WSAGetLastError() != WSA_IO_PENDING)
                {
                    Log_printf(Error, "IO pending Failure");
                    return 1;
                
                }
            }
           */
        }
    }
    catch (SocketException& expt)
    {
        //생성, bind, Listen시 발생되는에러 캐치
        expt.ShowException();
    }
    catch (...)
    {
        Log_printf(Error, "Some Error...");
    }

    return 1;
}