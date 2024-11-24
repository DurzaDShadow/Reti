#ifdef WIN32
#include <winsock.h>
#else
	#include <string.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#define closesocket close
#endif
#include "protocol.h"	// Application protocol file header.h
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// 	Function prototype
int Check(char text[]);

void errorhandler(char *error_message) {
	printf("%s", error_message);
}

void clearwinsock() {
#ifdef WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {
// Inizializing winsock with error prompt
#ifdef WIN32
	// If there is error in WSA, print it
	WSADATA wsa_data;
	int warning = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (warning != 0) {
		printf("Error at WSASturtup\n");
		return -1;
	}
#endif

//	Creating welcome socket
	int my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
		// If socket shows an error, print error msg
		errorhandler("Socket creation failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

//	Setting address
	struct sockaddr_in sad;
// 	Extra bytes must contain 0
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(PROTOPORT);

	printf("The server ip and port are: %s : %d\n", inet_ntoa(sad.sin_addr),
			htons(sad.sin_port));
	printf("Connection request...\n");

	//	Connecting to the server
	if (connect(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		//	Error msg text
		errorhandler("Failed to connect.\n");
		closesocket(my_socket);
		clearwinsock();
		system("pause");
		return -1;
	} else {	//	Print if it is connected.
		printf("Successfully connected to the server\n");
	}

	char receivedString[BUFFERSIZE];
	char sendChoice[BUFFERSIZE];


	while (Check(sendChoice) != 1) {
		//	Commands for user
		printf(
				"\nInsert the type of password you want to generate:\n"
						"- n for numerical\n- a for alphabetical\n- m for mixed (letters+numbers)\n"
						"- s for secure (letters+numbers+symbols). \n"
						"After choosing the type of password select the length (between 6 and 32)"
						"[Enter 'q' to stop the connection].\n");
		printf("(password type) space (password length))\n");
		fgets(sendChoice, sizeof(sendChoice), stdin);
		sendChoice[strlen(sendChoice) - 1] = '\0';



		//	Send to server
		if (send(my_socket, sendChoice, strlen(sendChoice), 0)
				!= strlen(sendChoice)) {
			errorhandler("Error when client sending Buffer to server.\n");
			break;
		}

		// 	Receive from server
		if (recv(my_socket, receivedString, sizeof(receivedString), 0) < 0) {
			errorhandler("Error when client receiving Buffer.\n");
			break;
		}
		//	Print string
		if (Check(sendChoice) != 1) {
			printf("%s\n", receivedString);
		}

	}

	//	Closing connection
	closesocket(my_socket);
	clearwinsock();
	system("pause");
	return (0);
}

int Check(char text[]) {
	for (int increase = 0; increase < strlen(text); ++increase) {
		if (*(text + increase) == 'q') {
			return 1;
		}
	}

	return 0;
}

