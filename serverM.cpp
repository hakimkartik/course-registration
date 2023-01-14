// g++ -std=c++11  -shared-libgcc  serverM.cpp -o  serverM
#include "codec.h"
#include <sstream>
#include <array>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>		//It contains the data structures required for socket
#include <netinet/in.h>		//It has the constants and structures required for Internet domain address., contains sockaddr_in
#include <sys/types.h>		//It has definitions of number of data types used for system calls.
using namespace std;

// constant decleartions
#define TCP_PORT 25928
#define UDP_CLIENT_PORT 24928
#define CRED_SERVER_PORT 21928
#define EE_SERVER_PORT 23928
#define CS_SERVER_PORT 22928
#define localhost "127.0.0.1"

// constant decleartions end

// global variables start
string gloabl_username(""); //global variable to store name of verified the user/client

vector<string> multiple_courses_result; //global variable to list of course-ids received from client

bool multiple_course_flag = false; // 
// global variables end

// function declaration start
array<string, 2> extract_username_password(char buffer[])
{	
	// reference: TutorialsPoint
	array<string, 2> test;
	string tmp(buffer);
	string delimiter = ",";

	int index = tmp.find(delimiter);
	string token = tmp.substr(0, index);
	string um = token.substr(9, token.length());

	string token2 = tmp.substr(index + 1, tmp.length());
	string passw = token2.substr(9, token2.length());

	test[0] = um;
	test[1] = passw;
	return test;
}

array<string, 2> extract_course_category(char buffer[])
{
	/*
	function to check the received string template
	look if all is present as substring, will set category to everything
	otherwise, will return an array of course id and category requested
	*/

	array<string, 2> test;
	string tmp(buffer);
	string delimiter = ",";
	int index = tmp.find(delimiter);
	string token = tmp.substr(0, index);
	string course;

	if (token.find("all") != std::string::npos)
	{
		course = token.substr(6, token.length());
	}
	else
	{
		course = token.substr(7, token.length());
	}

	string token2 = tmp.substr(index + 1, tmp.length());
	string category = token2.substr(9, token2.length());

	test[0] = course;
	test[1] = category;
	return test;
}

vector<string> extract_multiple_courses(string tmp)
{
	/*
	function to extarct mutiple ; seperated course-ids 
	and store them in global multiple_courses_result vector variable
	*/
	stringstream s_stream(tmp);	//create string stream from the string
	while (s_stream.good())
	{
		string substr;
		getline(s_stream, substr, ';');	//get first string delimited by comma
		multiple_courses_result.push_back(substr);
	}

	return multiple_courses_result;
}

int create_tcp_socket()
{
	// reference : Beej
	int tcp_server_fd;
	int opt = 1;

	// Creating socket file descriptor
	if ((tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("tcp socket creation failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port PORT
	if (setsockopt(tcp_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("tcp setsockopt failed");
		exit(EXIT_FAILURE);
	}

	return tcp_server_fd;

}

struct sockaddr_in setup_tcp_socket_params()
{
	// reference : Beej + GeeksforGeeks
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	address.sin_family = AF_INET;
	if (inet_pton(AF_INET, localhost, &address.sin_addr) <= 0)
    { 
      printf( "Failed, Address not supported \n");
      // exit(EXIT_FAILURE);
    }
	else{
		address.sin_addr.s_addr = INADDR_ANY;
	}
	
	address.sin_port = htons(TCP_PORT);
	return address;
}

string extract_username_password_from_buffer(int new_socket, char buffer[])
{
	// function to read course id and cateogry and encrypt them and return as a comma seperated string
	EncDec obj = EncDec();

	array<string, 2> ret_array;
	ret_array = extract_username_password(buffer);
	string um = ret_array[0];
	string passw = ret_array[1];

	gloabl_username = um;

	printf
		("The main server received the authentication for %s using TCP over port %d.\n",
			um.c_str(), TCP_PORT);

	um = obj.encrypt(um);
	passw = obj.encrypt(passw);

	string response(um);
	response.append(",");
	response.append(passw);
	return response;

}

int accept_tcp_connections(int tcp_server_fd, struct sockaddr_in address)
{
	// reference : Beej + GeeksforGeeks
	int addrlen = sizeof(address);
	int new_socket = accept(tcp_server_fd, (struct sockaddr *) &address,
			(socklen_t*) &addrlen);
	if (new_socket < 0)
	{
		perror("tcp accept failed");
		exit(EXIT_FAILURE);
	}

	return new_socket;
}

string extract_info(int new_socket, char buffer[])
{
	string tmp(buffer);
	string result;
	if (tmp.find("username") != string::npos)
	{
		result = extract_username_password_from_buffer(new_socket, buffer);
	}
	else
	{
		printf("Received msg without username \n");
	}

	return result;
}

array<string, 2> extract_course_info(char buffer[])
{
	array<string, 2> tmp;
	tmp = extract_course_category(buffer);
	return tmp;
}

string receive_cred_msg_via_tcp(int tcp_server_fd, int new_socket,
	struct sockaddr_in address)
{
	// reference: GeeksforGeeks

	int addrlen = sizeof(address);
	int valread;
	char buffer[1024] = { 0 };
	valread = read(new_socket, buffer, 1024);
	return extract_info(new_socket, buffer);

}

bool check_for_syntax(char buffer[])
{
	// function to check if substring all is present in the message received from client, meaning multiple course-ids have been provided
	string tmp(buffer);
	if (tmp.find("all") != std::string::npos)
	{
		return true;
	}

	return false;
}

vector<string> extract_multiple_courses_from_buffer(char buffer[])
{
	array<string, 2> x = extract_course_category(buffer);
	return extract_multiple_courses(x[0]);
}

array<string, 2> receive_course_msg_via_tcp(int tcp_server_fd,
	int new_socket,
	struct sockaddr_in address)
{
	// reference: GeeksforGeeks + LinuxHowTo

	int addrlen = sizeof(address);
	int valread;
	char buffer[1024] = { 0 };

	valread = read(new_socket, buffer, 1024);
	multiple_course_flag = check_for_syntax(buffer);
	if (multiple_course_flag)
	{
		multiple_courses_result.clear();
		extract_multiple_courses_from_buffer(buffer);
	}

	array<string, 2> tmp = extract_course_info(buffer);
	return tmp;

}

void send_msg_via_tcp(int new_socket, string response)
{
	// reference: Beej + GeeksforGeeks
	send(new_socket, response.c_str(), response.length(), 0);
}

void send_msg_via_udp(int sockfd, string msg,
	struct sockaddr_in destn_server_addr)
{
	// reference: Beej
	socklen_t tmp = sizeof(destn_server_addr);
	sendto(sockfd, msg.c_str(), 1024, 0, (struct sockaddr *) &destn_server_addr, tmp);
}

string process_validation_from_cred(char buffer[])
{
	// based on integer replied from serverC, form a string response to send back to client
	string tmp(buffer);
	string result;

	if (tmp.compare("-1") == 0)
	{
		result.append("Username not found");
	}
	else if (tmp.compare("0") == 0)
	{
		result.append("Username found, Password didnt match");
	}
	else if (tmp.compare("1") == 0)
	{
		result.append("Username verified");
	}
	else
	{
		result.append("Not sure");
	}

	return result;
}

string receive_msg_via_udp_from_cred_server(int sockfd,
	struct sockaddr_in serverAddr)
{
	// reference: Beej
	int buffer_len = 2048;
	string msg;

	char buffer[2048] = { 0 };

	socklen_t tmp = sizeof(serverAddr);

	if (recvfrom(sockfd, buffer, buffer_len, 0, (struct sockaddr *) &serverAddr, &tmp) > 0)
	{
		msg.append(process_validation_from_cred(buffer));
	}
	else
	{
		msg.append("NO_DATA");
	}

	return msg;

}

string receive_msg_via_udp_from_backend_server(int sockfd,
	struct sockaddr_in serverAddr)
{
	// reference: Beej
	int buffer_len = 2048;
	string msg;
	char buffer[2048] = { 0 };

	socklen_t tmp = sizeof(serverAddr);

	if (recvfrom(sockfd, buffer, buffer_len, 0, (struct sockaddr *) &serverAddr, &tmp) > 0)
	{
		msg.append(buffer);
	}
	else
	{
		msg.append("NO_DATA");
	}

	return msg;

}

string send_and_receive_from_backend(int tcp_server_fd,
	int new_tcp_socket,
	array<string, 2> arr,
	int udp_client_fd,
	struct sockaddr_in ee_server_addr,
	struct sockaddr_in cs_server_addr)
{
	// function to send and receive msg from backend servers
	string course = arr[0];
	string category = arr[1];
	string tmpx("course:");
	string tcp_response;

	cout << "The main server received from " << gloabl_username <<
		" to query course " << course << " about " << category <<
		" using TCP over port " << TCP_PORT << " ." << "\n";
	string department = course.substr(0, 2);

	if (department.compare("EE") == 0)
	{
		cout << "The main server sent a request to serverEE \n";
		tmpx.append(course);
		tmpx.append(",category:");
		tmpx.append(category);

		send_msg_via_udp(udp_client_fd, tmpx, ee_server_addr);

		tcp_response =
			receive_msg_via_udp_from_backend_server(udp_client_fd,
				ee_server_addr);

		cout <<
			"The main server received the response from serverEE using UDP over port " <<
			UDP_CLIENT_PORT << ".\n";
	}
	else
	{
		tmpx.append(arr[0]);
		tmpx.append(",category:");
		tmpx.append(arr[1]);

		cout << "The main server sent a request to serverCS \n";

		send_msg_via_udp(udp_client_fd, tmpx, cs_server_addr);

		tcp_response =
			receive_msg_via_udp_from_backend_server(udp_client_fd,
				cs_server_addr);

		cout <<
			"The main server received the response from serverCS using UDP over port " <<
			UDP_CLIENT_PORT << ".\n";
	}

	return tcp_response;

}
// function definations end

int main(int argc, char	const *argv[])
{
	// reference: Beej + GeeksforGeeks
	int tcp_server_fd = create_tcp_socket();
	struct sockaddr_in address = setup_tcp_socket_params();
	int addrlen = sizeof(address);
	int cred_sock_fd, ee_sock_fd, cs_sock_fd, udp_client_fd, new_tcp_socket;
	struct sockaddr_in cred_server_addr, ee_server_addr, cs_server_addr,
	my_udp_addr;

	int cred_server_port = CRED_SERVER_PORT;
	int ee_server_port = EE_SERVER_PORT;
	int cs_server_port = CS_SERVER_PORT;
	int my_udp_port = UDP_CLIENT_PORT;

	udp_client_fd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&my_udp_addr, '\0', sizeof(my_udp_addr));
	my_udp_addr.sin_family = AF_INET;
	my_udp_addr.sin_port = htons(my_udp_port);
	if (inet_pton(AF_INET, localhost, &my_udp_addr.sin_addr) <= 0)
    { 
       printf( "Failed, Address not supported \n");
       //exit(EXIT_FAILURE);
    }
	else
	{
		my_udp_addr.sin_addr.s_addr = INADDR_ANY;
	}
	

	cred_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&cred_server_addr, '\0', sizeof(cred_server_addr));
	cred_server_addr.sin_family = AF_INET;
	cred_server_addr.sin_port = htons(cred_server_port);
	//cred_server_addr.sin_addr.s_addr = INADDR_ANY;
	if (inet_pton(AF_INET, localhost, &cred_server_addr.sin_addr) <= 0)
    { 
       printf( "Failed, Address not supported \n");
       //exit(EXIT_FAILURE);
    }
	else
	{
		cred_server_addr.sin_addr.s_addr = INADDR_ANY;
	}

	ee_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&ee_server_addr, '\0', sizeof(ee_server_addr));
	ee_server_addr.sin_family = AF_INET;
	ee_server_addr.sin_port = htons(ee_server_port);
	ee_server_addr.sin_addr.s_addr = INADDR_ANY;
	if (inet_pton(AF_INET, localhost, &ee_server_addr.sin_addr) <= 0)
    { 
       printf( "Failed, Address not supported \n");
       //exit(EXIT_FAILURE);
    }
	else
	{
		ee_server_addr.sin_addr.s_addr = INADDR_ANY;
	}
	
	cs_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&cs_server_addr, '\0', sizeof(cs_server_addr));
	cs_server_addr.sin_family = AF_INET;
	cs_server_addr.sin_port = htons(cs_server_port);
	if (inet_pton(AF_INET, localhost, &cs_server_addr.sin_addr) <= 0)
    { 
       printf( "Failed, Address not supported \n");
       //exit(EXIT_FAILURE);
    }
	else
	{
		cs_server_addr.sin_addr.s_addr = INADDR_ANY;
	}

	int res = ::bind(udp_client_fd, (sockaddr*) &my_udp_addr,
		sizeof(my_udp_addr));

	if (res < 0)
	{
		cerr << "UDP binding has failed\n";
	}

	if (::bind(tcp_server_fd, (struct sockaddr *) &address, sizeof(address))< 0)
	{
		perror("serverM: tcp bind failed \n");
		exit(EXIT_FAILURE);
	}

	if (listen(tcp_server_fd, 5) < 0)
	{
		perror("serverM: listening failed \n");
		exit(EXIT_FAILURE);
	}

	socklen_t tmp = sizeof(cred_server_addr);

	::bind(cred_sock_fd, (struct sockaddr *) &cred_server_addr, sizeof(tmp));

	socklen_t tmp2 = sizeof(ee_server_addr);

	::bind(ee_sock_fd, (struct sockaddr *) &ee_server_addr, sizeof(tmp2));

	socklen_t tmp3 = sizeof(cs_server_addr);

	::bind(cs_sock_fd, (struct sockaddr *) &cs_server_addr, sizeof(tmp3));

	int user_verified_flag = 0;
	cout << "The main server is up and running. \n";

	while (1)
	{
		new_tcp_socket = accept_tcp_connections(tcp_server_fd, address);
		string response, msg, course, category;
		if (!user_verified_flag)
		{
			response =
				receive_cred_msg_via_tcp(tcp_server_fd, new_tcp_socket, address);

			cout <<
				"The main server sent an authentication request to serverC. \n";

			send_msg_via_udp(udp_client_fd, response, cred_server_addr);

			msg =
				receive_msg_via_udp_from_cred_server(udp_client_fd,
					cred_server_addr);

			cout <<
				"The main server received the result of the authentication request from ServerC using UDP over port " <<
				UDP_CLIENT_PORT << ". " << "\n";

			send_msg_via_tcp(new_tcp_socket, msg);

			cout <<
				"The main server sent the authentication result to the client. \n";
		}

		if (user_verified_flag || msg.compare("Username verified") == 0)
		{
			user_verified_flag = 1;

			array<string, 2> response_array =
				receive_course_msg_via_tcp(tcp_server_fd, new_tcp_socket,
					address);

			string tcp_response("");
			string tmp;

			if (multiple_course_flag)
			{
				for (int i = 0; i < multiple_courses_result.size(); i++)
				{
					course = multiple_courses_result.at(i);
					//cout<<"In the for loop for" <<course<<"\n";
					response_array[0] = course;
					response_array[1] = "everything";
					tmp =
						send_and_receive_from_backend(tcp_server_fd,
							new_tcp_socket, response_array,
							udp_client_fd,
							ee_server_addr,
							cs_server_addr);
					if (tmp.length() >= 1 && tmp.compare(",,,") != 0)
					{
						tcp_response.append(course);
						tcp_response.append(": ");
						tcp_response.append(tmp);
					}
					else
					{
						tcp_response.append(course);
						tcp_response.append(": No data found");
						
					}

					if (i < multiple_courses_result.size() - 1)
					{
						tcp_response.append("\n");
					}
				}

				//unset the flag
				multiple_course_flag = false;
			}
			else
			{
				tcp_response =
					send_and_receive_from_backend(tcp_server_fd, new_tcp_socket,
						response_array, udp_client_fd,
						ee_server_addr,
						cs_server_addr);
			}

			send_msg_via_tcp(new_tcp_socket, tcp_response);
			cout <<
				"The main server sent the query information to the client.\n";
		}
		else
		{
			user_verified_flag = 0;
		}

		// closing the connected socket
		close(new_tcp_socket);
		cout << "\n";
	}

	// closing the listening socket
	shutdown(tcp_server_fd, SHUT_RDWR);
	return 0;
}
