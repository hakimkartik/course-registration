// g++ -std=c++11 -shared-libgcc  client.cpp -o client
#include <iostream>
#include <sstream>
#include <array>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>		//It contains the data structures required for socket
#include <netinet/in.h>		//It has the constants and structures required for Internet domain address.
#include <sys/types.h>		//It has definitions of number of data types used for system calls.
#include "stringUtil.h"

using namespace std;
// global variables start
#define TCP_PORT 25928
#define localhost "127.0.0.1"

string gloabl_username("");
unsigned int MY_TCP_PORT;
// global variables end

//function defination start
void send_msg_via_tcp(int sock, string message)
{
	// reference: Beej + GeeksforGeeks
	int len = send(sock, message.c_str(), message.length(), 0) != message.length();
	if (len)
	{
		printf("Send Error: Could not send message: %s !!", message.c_str());
		exit(EXIT_FAILURE);
	}
}

string read_msg_via_tcp(int sock)
{
	// reference: GeeksforGeeks
	const int bsize = 6000;
	char buffer[bsize] = { 0 };
	int valread = read(sock, buffer, bsize);
	return string(buffer);
}

int setup_tcp_params(struct sockaddr_in serv_addr)
{
	// reference: Beej + GeeksforGeeks 
	int sock = 0;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		exit(EXIT_FAILURE);
	}

	return sock;
}

array<int, 2> connect_via_tcp()
{
	// reference: Beej
	struct sockaddr_in serv_addr;
	array<int, 2> tmp;

	int sock = setup_tcp_params(serv_addr);
	int client_fd;

	memset(&serv_addr, '\0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(TCP_PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, localhost, &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		exit(EXIT_FAILURE);
	}

	if ((client_fd = connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0)
	{
		printf("\nConnection to socket failed \n");
		printf("Please check if serverM is up and running \n");
		exit(EXIT_FAILURE);
	}

	tmp[0] = sock;
	tmp[1] = client_fd;

	socklen_t len = sizeof(serv_addr);
	getsockname(sock, (struct sockaddr *) &serv_addr, &len);

	MY_TCP_PORT = ntohs(serv_addr.sin_port);

	return tmp;
}

int process_crendential_result(string resp)
{
	// function to check the credential response from serverM and based on it, set the flag
	string uname_not_found("Username not found");
	string uname_found_passwd_not_found("Username found, Password didnt match");
	string uname_verified("Username verified");
	string not_sure("Not sure");

	int result = 0;

	if (resp.compare(uname_not_found) == 0)
	{
		result = -1;
	}
	else if (resp.compare(uname_found_passwd_not_found) == 0)
	{
		result = 0;
	}
	else if (resp.compare(uname_verified) == 0)
	{
		result = 1;
	}
	else
	{
		result = -2;
	}

	return result;
}

int process_username_password(int sock)
{
	// function to read username and password, form the string template, and send and receive its result from serverM
	int tmp = 0;
	string username("");
	string password("");

	printf("Please enter the username: ");
	cin >> username;

	printf("Please enter the password: ");
	cin >> password;

	string message("username:");
	message.append(username);
	message.append(",password:");
	message.append(password);

	send_msg_via_tcp(sock, message);

	cout << username << " sent an authentication request to the main server. \n";

	gloabl_username = username;

	string resp = read_msg_via_tcp(sock);

	tmp = process_crendential_result(resp);

	return tmp;
}

void print_result(string course, string category, string result)
{
	// function to print appropriate message on console, based on result received from serverM
	if (category.compare("everything") != 0)
	{
		if (result.compare("NO_DATA") == 0 || result.compare("Not found") == 0 || result.length() < 1)
		{
			//printf("Didn’t find the course: %s.", course.c_str());
			cout<<"Didn’t find the course: "<<course<<".\n";
		}
		else
		{
			//printf("The course information has been founded: The %s of %s is %s \n", category.c_str(), course.c_str(), result.c_str());
			cout<<"The "<<category<<" of "<<course<<" is "<<result<<".\n";
		}
	}
	else
	{
		printf("%s", result.c_str());
		printf("\n");
	}
}

void send_and_receive_course_info(int sock, string course, string category)
{
	// function to read course id and category, form the string template, and send and receive its result from serverM
	string resp("");
	string msg("course:");
	msg.append(course);
	msg.append(",category:");
	msg.append(category);
	send_msg_via_tcp(sock, msg);
	printf("%s sent a request to the main server. \n", gloabl_username.c_str());
	resp = read_msg_via_tcp(sock);
	printf("The client received the response from the Main server using TCP over port %d. \n", MY_TCP_PORT);
	print_result(course, category, resp);
}

int main()
{
	// reference: Beej +  GeeksforGeeks
	array<int, 2> tmp;
	int sock, client_fd;

	int user_verified_flag = 0;
	int incorrect_verification_counter = 3;

	printf("The client is up and running.\n");

	while (1)
	{
		string course_array[200];

		tmp = connect_via_tcp();
		sock = tmp[0];
		client_fd = tmp[1];

		int credential_check_result = 1;
		if (!user_verified_flag)
		{
			printf("\n");
			credential_check_result = process_username_password(sock);
			// printf("Credential Result : %d \n----------------\n",credential_check_result);
			if (credential_check_result == 1)
			{
				printf("%s received the result of authentication using TCP over port %d. Authentication is successful \n", gloabl_username.c_str(), MY_TCP_PORT);
				user_verified_flag = 1;
				incorrect_verification_counter = 3;
			}
		}

		if (credential_check_result == 1 || user_verified_flag == 1)
		{
			user_verified_flag = 1;
			//incorrect_verification_counter = 0;

			printf("\n");
			printf("Please enter the course code to query: ");
			// cin >> course;
			int size = 0;
			string z;
			StringUtils obj = StringUtils();
			while (true)
			{
				cin >> z;
				obj.trim(z);
				course_array[size++] = z;
				if (cin.peek() == '\n') break;
			}

			if (size <= 1)
			{
				string category("");
				string course = course_array[0];
				printf("Please enter the category(Credit/Professor/Days/CourseName): ");
				cin >> category;
				string resp("");
				string msg("course:");
				msg.append(course);
				msg.append(",category:");
				msg.append(category);
				send_msg_via_tcp(sock, msg);
				printf("%s sent a request to the main server. \n", gloabl_username.c_str());
				resp = read_msg_via_tcp(sock);
				printf("The client received the response from the Main server using TCP over port %d. \n", MY_TCP_PORT);
				print_result(course, category, resp);
			}
			else
			{
				int i = 0;
				string all_string("#all#:");
				for (int j = 0; j < size; j++)
				{
					all_string.append(course_array[j]);
					if (j != size - 1)
					{
						all_string.append(";");
					}
				}
				string category("everything");
				string course = all_string;
				string resp("");
				string msg("");
				msg.append(course);
				msg.append(",category:everything");
				//msg.append(category);
				//printf("Given data %s, %s \n", course.c_str(), category.c_str());
				send_msg_via_tcp(sock, msg);
				printf("%s sent a request to the main server. \n", gloabl_username.c_str());
				resp = read_msg_via_tcp(sock);
				printf("The client received the response from the Main server using TCP over port %d. \n", MY_TCP_PORT);
				print_result(course, category, resp);
			}

			printf("\n-----Start a new request-----\n");
		}
		else
		{
			incorrect_verification_counter--;
			user_verified_flag = 0;

			if (credential_check_result == -1)
			{
				printf("%s received the result of authentication using TCP over port %d. Authentication failed: Username Does not exist. \n", gloabl_username.c_str(), MY_TCP_PORT);
			}
			else if (credential_check_result == 0)
			{
				printf("%s received the result of authentication using TCP over port %d. Authentication failed: Password does not match. \n", gloabl_username.c_str(), MY_TCP_PORT);
			}
			else
			{
				credential_check_result = 1;
			}

			printf("Attempts remaining: %d\n", incorrect_verification_counter);
		}

		//close(client_fd);

		if (incorrect_verification_counter <= 0)
		{
			printf("Authentication Failed for 3 attempts. Client will shut down. \n");
			break;
		}
	}

	close(client_fd);
	shutdown(sock, SHUT_RDWR);
    return 0;
}