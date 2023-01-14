// g++ -std=c++11  -shared-libgcc  serverCS.cpp -o  output-files/serverCS
#include <sstream>
#include <iostream>
#include <array>
#include <vector>
#include <fstream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include "stringUtil.h"
using namespace std;

// global varibale start
#define MAXBUFLEN 2042
#define MY_UDP_PORT "22928"
#define localhost "127.0.0.1"
map<string, string> course_id_and_credit;
map<string, string> course_id_and_professor;
map<string, string> course_id_and_days;
map<string, string> course_id_and_title;
// global varibale end

// function defination start
int create_udp_socket()
{
	// reference: Beej + GeeksforGeeks
	int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	int
	optval = 1;
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
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
			sizeof(int)))
	{
		perror("udp setsockopt failed");
		exit(EXIT_FAILURE);
	}

	//cout << "socketfd:  " << socketfd << "\n";
	return socketfd;
}

array<string, 2> extract_username_password(char buffer[])
{
	StringUtils obj = StringUtils();
	array<string, 2> test;
	string tmp(buffer);
	string delimiter = ",";
	int index = tmp.find(delimiter);
	string course = tmp.substr(0, index);
	string credit = tmp.substr(index + 1, tmp.length());
	obj.trim(course);
	obj.trim(credit);
	return test;
}

void insert_info_in_map(string my_str)
{
	// reference: TutorialsPoint

	vector<string> result;
	stringstream s_stream(my_str);	//create string stream from the string
	while (s_stream.good())
	{
		string substr;
		getline(s_stream, substr, ',');	//get first string delimited by comma
		result.push_back(substr);
	}

	string course_id = result[0];
	string credits = result[1];
	string prof = result[2];
	string days = result[3];
	string title = result[4];
	course_id_and_credit[course_id] = credits;
	course_id_and_professor[course_id] = prof;
	course_id_and_days[course_id] = days;
	course_id_and_title[course_id] = title;
}

void load_cs_txt_file()
{
	// function to read cs.txt file and store in the global variable 
	ifstream myfile;
	string filepath("./cs.txt");
	myfile.open(filepath);
	string myline;
	if (myfile.is_open())
	{
		while (myfile)
		{
			getline(myfile, myline);
			insert_info_in_map(myline);
		}
	}
	else
	{
		printf("Couldn't open file: %s \n", filepath.c_str());
		exit(EXIT_FAILURE);
		
	}
}

array<string, 2> extract_course_category(char buffer[])
{
	// reference: TutorialsPoint
	array<string, 2> test;
	string tmp(buffer);
	string delimiter = ",";
	int index = tmp.find(delimiter);
	string token = tmp.substr(0, index);
	string course = token.substr(7, token.length());
	string token2 = tmp.substr(index + 1, tmp.length());
	string category = token2.substr(9, token2.length());
	test[0] = course;
	test[1] = category;
	return test;
}

array<string, 2> extract_course_info(char buffer[])
{
	array<string, 2> tmp;
	tmp = extract_course_category(buffer);
	return tmp;
}

string set_the_response(string category, string course)
{
	/*
	function to check set the response string to be printed on client side
	if category is everything, will append all information
	else it will append only the requested category info
	*/
	string result;

	if (category.compare("everything") == 0)
	{
		result.append(course_id_and_credit[course]);
		result.append(",");
		result.append(course_id_and_professor[course]);
		result.append(",");
		result.append(course_id_and_days[course]);
		result.append(",");
		result.append(course_id_and_title[course]);
		//cout << "everything result : " << result << "\n";
	}
	else
	{
		if (category.compare("Days") == 0)
		{
			result.append(course_id_and_days[course]);
		}
		else if (category.compare("CourseName") == 0)
		{
			result.append(course_id_and_title[course]);
		}
		else if (category.compare("Professor") == 0)
		{
			result.append(course_id_and_professor[course]);
		}
		else if (category.compare("Credit") == 0)
		{
			result.append(course_id_and_credit[course]);
		}
		else
		{
			result.append("Not found");
		}
	}

	if (result.compare("Not found") == 0 || result.length() < 1 || result.compare(",,,") == 0)
	{
		cout<<"Didnt find the course: "<<course<<"\n";
	}
	else
	{
		cout<<"The course information has been found: The "<<category<<" of "<<course<<" is "<<result<<". \n";	
	}

	return result;
}


void new_flow()
{
	// reference : Beej
	load_cs_txt_file();
	int sockfd, rv, numbytes;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	//char s[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;	// set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	//hints.ai_flags = AI_PASSIVE;	// use my IP
	hints.ai_flags = INADDR_ANY;	// using localhost as IP
	
	if ((rv = getaddrinfo(NULL, MY_UDP_PORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "serverCS: getaddrinfo: %s\n", gai_strerror(rv));
		exit(2);
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("serverCS: socket creation error \n");
			continue;
		}

		//bind(sockfd, p->ai_addr, p->ai_addrlen) ;
		if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("serverCS: bind failed .. will try other sockets\n");
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "serverCS: bind failed\n");
		exit(2);
	}

	freeaddrinfo(servinfo);
	printf("The ServerCS is up and running using UDP on port %s.\n", MY_UDP_PORT);
	while (1)
	{
		char buf[MAXBUFLEN] = { '0' };

		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1)
		{
			perror("serverCS: recvfrom\n");
			exit(1);
		}

		buf[numbytes] = '\0';
		array<string, 2> tmp = extract_course_info(buf);
		printf("The ServerCS received a request from the Main Server about the %s of %s.\n", tmp[1].c_str(), tmp[0].c_str());
		string response = set_the_response(tmp[1], tmp[0]);
		sendto(sockfd, response.c_str(), response.length(), 0, (struct sockaddr *) &their_addr, addr_len);
		printf("The ServerCS finished sending the response to the Main Server.\n");
		printf("\n");
	}

	close(sockfd);
}

// function defination end
int main(int argc, char	const *argv[])
{
	new_flow();
	return 0;
}
