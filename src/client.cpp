#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>

using namespace std;
mutex mtx;
const int backLog = 3;
const int maxDataSize = 1460;

void server_recv(int socketFd)
{
  char rcvDataBuf[maxDataSize];
  int status, dataRecvd;
  string rcvDataStr;
  memset(&rcvDataBuf, 0, maxDataSize);
  while ((dataRecvd = recv(socketFd, rcvDataBuf, maxDataSize, 0)) > 0)
  {
    rcvDataStr = rcvDataBuf;
    cout << rcvDataStr.c_str();
    memset(&rcvDataBuf, 0, maxDataSize);
  }
}

int main(void)
{

  uint16_t serverPort = 3002;
  string serverIpAddr = "127.0.0.1";
  cout << "Enter the ip address and port number of server" << endl;
  cin >> serverIpAddr;
  cin >> serverPort;

  int clientSocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!clientSocketFd)
  {
    cout << "Error creating socket" << endl;
    exit(1);
  }

  struct sockaddr_in serverSockAddressInfo;
  serverSockAddressInfo.sin_family = AF_INET;
  serverSockAddressInfo.sin_port = htons(serverPort);
  inet_pton(AF_INET, serverIpAddr.c_str(), &(serverSockAddressInfo.sin_addr));
  memset(&(serverSockAddressInfo.sin_zero), '\0', 8);

  int flags = 0;
  int dataRecvd = 0, dataSent = 0;
  struct sockaddr_in clientAddressInfo;
  char sendDataBuf[maxDataSize];
  string sendDataStr;

  int ret = connect(clientSocketFd, (struct sockaddr *)&serverSockAddressInfo, sizeof(struct sockaddr));
  if (ret < 0)
  {
    cout << "Error with server connection " << endl;
    close(clientSocketFd);
    exit(1);
  }
  cin.ignore();
  thread t1(server_recv, clientSocketFd);
  cout << "Start Conversation.." << endl;

  while (1)
  {
    memset(&sendDataBuf, 0, maxDataSize);
    getline(cin, sendDataStr);
    cin.clear();
    cout << "Me :" << sendDataStr.c_str() << endl;
    dataSent = send(clientSocketFd, sendDataStr.c_str(), sendDataStr.length(), flags);
    if (!strcmp(sendDataStr.c_str(), "bye"))
    {
      break;
    }
  }
  t1.join();
  cout << "All done closing socket now" << endl;
  close(clientSocketFd);
  return 0;
}