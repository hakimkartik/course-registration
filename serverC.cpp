// g++ -std=c++11  -shared-libgcc  serverC.cpp -o  serverC
#include <sstream>
#include <iostream>
#include <array>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <arpa/inet.h>
#include <sys/socket.h>		//It contains the data structures required for socket
#include <netinet/in.h>		//It has the constants and structures required for Internet domain address.
#include <sys/types.h>		//It has definitions of number of data types used for system calls.
#include <map>
#include "stringUtil.h"

using namespace std;

// global varibale start
#define UDP_PORT 21928
#define localhost "127.0.0.1"
map<string, string> uname_password_map;
// global varibale end

// function defination start

int create_udp_socket()
{
  // reference: Beej 
  int socketfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (socketfd < 0)
  {
    perror("udp socket creation failed");
    exit(EXIT_FAILURE);
  }

  /*
  setsockopt: Handy debugging trick that lets us rerun the server immediately after we kill it;
  otherwise we have to wait about 20 secs.
  helps us avoid "ERROR on binding: Address already in use" error.
  */
  int optval = 1;

  if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int)))
  {
    perror("udp setsockopt failed");
    exit(EXIT_FAILURE);
  }

  return socketfd;
}

array<string, 2> extract_username_password(char buffer[])
{
  // reference: TutorialsPoint 
  // function to split given buffer string using comma as delimiter
  StringUtils obj = StringUtils();

  array<string, 2> test;
  string tmp(buffer);
  string delimiter = ",";

  int index = tmp.find(delimiter);
  string um = tmp.substr(0, index);
  string passw = tmp.substr(index + 1, tmp.length());
  obj.trim(um);
  obj.trim(passw);
  test[0] = um;
  test[1] = passw;
  return test;
}

void load_cred_txt_file()
{
  // function to read cred.txt and store in a global map
  // reference: TutorialsPoint
  ifstream myfile;
  string filepath("./cred.txt");
  myfile.open(filepath);
  string myline;
  if (myfile.is_open())
  {
    while (myfile)
    {
      getline(myfile, myline);
      char char_array[myline.length() + 1];
      const char *ptr = myline.c_str();
      strcpy(char_array, ptr);
      array<string, 2> ret_array;
      ret_array = extract_username_password(char_array);

      uname_password_map[ret_array[0]] = ret_array[1];
    }
  }
  else
  {
    printf("Couldn't open file: %s \n", filepath.c_str());
  }
}

bool exists_in_hashmap(string uname)
{
  if (uname_password_map.find(uname) == uname_password_map.end())
  {
    return false;
  }

  return true;
}

int verfiy_credentials(string uname, string passw)
{
  int verified = -1;

  if (exists_in_hashmap(uname))
  {
    verified = 0;
    if (passw.compare(uname_password_map[uname]) == 0)
    {
      verified = 1;
    }
  }

  return verified;
}

string check_received_data(char buffer[])
{
  array<string, 2> ret_array;
  ret_array = extract_username_password(buffer);
  string um = ret_array[0];
  string passw = ret_array[1];

  return to_string(verfiy_credentials(um, passw));
}

// function defination end

int main(int argc, char const *argv[])
{
  // reference: Beej
  load_cred_txt_file();

  int sockfd;
  struct sockaddr_in udp_server, udp_client;

  char buffer[1024];
  socklen_t addr_size;

  sockfd = create_udp_socket();

  memset(&udp_client, 0, sizeof(udp_client));
  memset(&udp_server, '\0', sizeof(udp_server));

  udp_server.sin_family = AF_INET;
  udp_server.sin_port = htons(UDP_PORT);
  if (inet_pton(AF_INET, localhost, &udp_server.sin_addr) <= 0)
  { 
       printf( "Failed, Address not supported \n");
       //exit(EXIT_FAILURE);
  }
  else
  {
    udp_server.sin_addr.s_addr = INADDR_ANY;
  }
  ::bind(sockfd, (struct sockaddr *) &udp_server, sizeof(udp_server));

  printf("The ServerC is up and running using UDP on port %d. \n", UDP_PORT);

  addr_size = sizeof(udp_client);
  while (1)
  {
    recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *) &udp_client, &addr_size);
    printf("The ServerC received an authentication request from the Main Server.\n");
    string response = check_received_data(buffer);
    sendto(sockfd, response.c_str(), response.length(), 0, (struct sockaddr *) &udp_client, addr_size);
    printf("The ServerC finished sending the response to the Main Server. \n");
    printf("\n");
  }

  return 0;
}
