#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <mutex>

using namespace std;
int clients[100];
int no_of_clients = 0;
int maxDataSize = 500;
mutex mtx;
void removeClient(int clientFd)
{
  mtx.lock();
  int i, j = 0;
  for (i = 0; i < no_of_clients; i++)
    if (clients[i] == clientFd)
      break;

  while (i < no_of_clients - 1)
  {
    clients[i] = clients[i + 1];
    i++;
  }

  no_of_clients--;
  mtx.unlock();
}
void ToAllClients(char *msg, int sender)
{
  int clientFd = sender;
  mtx.lock();

  // Print the message to the server for message logs
  printf("[INCOMING_MSGS] %s", msg);
  int client;
  for (client = 0; client < no_of_clients; client++)
  {
    if (clients[client] != clientFd)
    {
      send(clients[client], msg, strlen(msg), 0);
    }
  }
  mtx.unlock();
}

void FromClient(int client)
{
  char rcvDataBuf[maxDataSize], sendDataBuf[maxDataSize];
  string rcvDataStr;
  int clientFd = client;
  memset(&sendDataBuf, 0, maxDataSize);
  int status, databytes;
  while ((databytes = recv(clientFd, rcvDataBuf, maxDataSize, 0)) > 0)
  {
    stringstream ss;
    ss << "Client " << clientFd << ": " << rcvDataBuf << endl;
    string formatted_string = ss.str();
    memset(&sendDataBuf, 0, maxDataSize);
    strcpy(sendDataBuf, formatted_string.c_str());
    ToAllClients(sendDataBuf, client);

    rcvDataStr = rcvDataBuf;
    if (!strcmp(rcvDataStr.c_str(), "bye"))
    {
      cout << "Closing Client:" << clientFd;
      close(clientFd);
      removeClient(clientFd);
    }
    memset(&rcvDataBuf, 0, maxDataSize);
  }
}

int main(void)
{
  uint16_t serverPort = 3002;
  string serverIpAddr = "127.0.0.1";
  cout << "Enter the ip address and port number to listen the connections for" << endl;
  cin >> serverIpAddr;
  cin >> serverPort;
  socklen_t client_addr_size;
  thread t1;

  int serverSocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!serverSocketFd)
  {
    cout << "Error creating socket" << endl;
    exit(1);
  }

  struct sockaddr_in serverSockAddressInfo;
  serverSockAddressInfo.sin_family = AF_INET;
  serverSockAddressInfo.sin_port = htons(serverPort);
  inet_pton(AF_INET, serverIpAddr.c_str(), &(serverSockAddressInfo.sin_addr));
  memset(&(serverSockAddressInfo.sin_zero), '\0', 8);
  printf("Server listening on IP %x:PORT %d \n", serverSockAddressInfo.sin_addr.s_addr, serverPort);

  int ret = bind(serverSocketFd, (struct sockaddr *)&serverSockAddressInfo, sizeof(struct sockaddr));
  if (ret < 0)
  {
    cout << "Error binding socket" << endl;
    close(serverSocketFd);
    exit(1);
  }

  ret = listen(serverSocketFd, 5);
  if (!serverSocketFd)
  {
    cout << "Error listening socket" << endl;
    close(serverSocketFd);
    exit(1);
  }

  socklen_t sinSize = sizeof(struct sockaddr_in);
  int flags = 0;
  int dataRecvd = 0, dataSent = 0;
  struct sockaddr_in clientAddressInfo;
  char rcvDataBuf[maxDataSize], sendDataBuf[maxDataSize];
  string sendDataStr;

  memset(&clientAddressInfo, 0, sizeof(struct sockaddr_in));
  memset(&rcvDataBuf, 0, maxDataSize);

  while (true)
  {
    int newClientFd = accept(serverSocketFd, (struct sockaddr *)&clientAddressInfo, &sinSize);
    if (!newClientFd)
    {
      cout << "Error with new client connection " << endl;
      close(serverSocketFd);
      exit(1);
    }
    mtx.lock();
    printf("New Client[%d] added ! \n", newClientFd);
    clients[no_of_clients] = newClientFd;
    no_of_clients++;
    t1 = thread(FromClient, newClientFd);
    t1.detach();
    mtx.unlock();
  }
  return 0;
}