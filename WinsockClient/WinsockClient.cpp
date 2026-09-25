#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib");

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

int main(int argc, char* argv[])
{
    // Winsock 초기화
    WSADATA wsaData;

    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    struct addrinfo *result = NULL,
                *ptr = NULL,
                hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // 서버 주소와 포트 번호 조회
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;

    // getaddrinfo가 반환한 첫 번째 주소로 연결 시도
    ptr = result;

    // 서버에 연결할 소켓 생성
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // 서버에 연결
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    // getaddrinfo는 서버 주소에 해당하는 연결 후보들을 연결 리스트로 반환한다.
    // 첫 번째 주소로 connect에 실패하면 다음 주소도 시도할 수 있다.
    // 이 예제에서는 재시도하지 않고 결과를 해제한 뒤 오류 메시지를 출력한다.

    freeaddrinfo(result);
    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    int recvbuflen = DEFAULT_BUFLEN;;

    const char* sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];

    // 버퍼에 담긴 문자열을 서버에 전송
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // 더 이상 전송할 데이터가 없으므로 연결의 송신을 종료
    // 송신을 종료한 후에도 ConnectSocket을 사용하여 데이터 수신이 가능하다
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // 서버가 연결을 종료할 때까지 데이터를 수신
    do
    {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

        if (iResult > 0)
        {
            printf("Bytes received: %d\n", iResult);
        }
        else if (iResult == 0)
        {
            printf("Connection closed\n");
        }
        else
        {
            printf("recv failed: %d\n", WSAGetLastError());
        }
    }
    while (iResult > 0);

    return 0;
}
