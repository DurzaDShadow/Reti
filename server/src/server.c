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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "protocol.h"		// Application protocol file header.h

//Function prototype
void generatePassword(char *pw, int passwordLength, int b, int e);
void generateNumeric(char *pw, int passwordLengt);
void generateAlpha(char *pw, int passwordLengt);
void generateMixed(char *pw, int passwordLengt);
void generateSecure(char *pw, int passwordLengt);
int Check(char string[]);

void clearwinsock() {
#ifdef WIN32
	WSACleanup();
#endif
}

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

int main(int argc, char *argv[]) {
	// Initializing winsock with error prompt
#ifdef WIN32
	WSADATA wsa_data;
	// If there is error in WSA, print it.
	int warning = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (warning != 0) {
		errorhandler("Error at WSAStartup()\n");	//	Error msg
		return 0;
	}
#endif

	//	Creating welcome socket
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (my_socket < 0) {
		// If socket shows an error, print error msg
		errorhandler("Socket creation failed.\n");
		clearwinsock();
		return -1;
	}

//	Setting address
	struct sockaddr_in sad;
	// Extra bytes must contain 0
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;

	//	Set address
	int IP_Address;

	if (argc > 1) {
		IP_Address = atoi(argv[1]); // if argument specified convert argument to binary
	} else {
		IP_Address = PROTOPORT;
	}        // use default port number

	if (IP_Address < 0) {
		printf("bad port number %s \n", argv[1]);
		return 0;
	}
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");

	sad.sin_port = htons(IP_Address);
	// This happens if server is active and it isn't shutdown
	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("bind() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

//	Listen
	if (listen(my_socket, QLEN) < 0) {
		//	Error msg
		errorhandler("Listen() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

//	Accept a new connection
	// Creating structure for the client address
	struct sockaddr_in cad;
	// New socket descriptor for the client
	int clientSocket;
	// Creating the size of the client address
	int client_len;
	printf("\nWaiting for a client to connect...\n\n");

	char Send[SIZE];
	char Receive[SIZE];
	int passwordLength;
	char choice;
	//char password [SIZE];
	char pw[SIZE];

//	Accepting connection
	while (1) {
		//	Client length of the address
		client_len = sizeof(cad);
		if ((clientSocket = accept(my_socket, (struct sockaddr*) &cad,
				&client_len)) < 0) {
			// Print error msg then close connection
			errorhandler("accept() failed.\n");
			//Close connection
			closesocket(clientSocket);
			clearwinsock();
			return 0;
		}

		printf("\nConnection established with: %s:%d\n",
				inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));

		while (1) {

			memset(Send, 0, sizeof(Send));
			memset(Receive, 0, sizeof(Receive));
			printf("Waiting for the choice:\n");

			// received string
			if (recv(clientSocket, Receive, sizeof(Receive), 0) < 0) {
				errorhandler("Accept() failed.\n");
				break;
			} else {
				printf("Received String: %s\n\n", Receive);
			}

// 	Checking connection
			if (Check(Receive) == 1) {
				printf("Closing connection\n"
						"Waiting for a next client to connect...\n");
				closesocket(clientSocket);
				break;
			}

// 	Verifying expression is valid
			char *ArrayToken[2];
			char *token;
			const char white_space[2] = " ";

			token = strtok(Receive, white_space);

			// Sentinel Value
			int increase = 0;

			// Continue until token is not empty
			while (token != NULL) {
				ArrayToken[increase] = token;
				token = strtok(NULL, white_space);
				increase++;
			}
			// Renaming choice
			choice = ArrayToken[0][0];
			passwordLength = strtol(ArrayToken[1], NULL, 10);

			printf("%c %d\n\n", choice, passwordLength);
			// System message for check the expression is equal.
			printf("\n" "Client's request: %c %d"
					"\n", choice, passwordLength);
			if ((choice == 'n' || choice == 'a' || choice == 'm' || choice == 's') && (passwordLength > 5 && passwordLength < 33)){
			switch (choice) {
			case 'n':
				generateNumeric(pw, passwordLength);
				break;
			case 'a':
				generateAlpha(pw, passwordLength);
				break;
			case 'm':
				generateMixed(pw, passwordLength);
				break;
			case 's':
				generateSecure(pw, passwordLength);
				break;
			default:
				break;
			}


			printf("Password: %s\n", pw);
			sprintf(Send, "%s", pw);
			}else{
				printf("Enter a valid choice!");
				sprintf(Send, "Enter a valid choice!");
			}

			//	Print error msg if it can not send data to the client
			if (send(clientSocket, Send, sizeof(Send), 0) < 0) {
				errorhandler("Error when server sending data to the client.\n");
				break;
			}
			puts("\n");
		}

		closesocket(clientSocket);
	}
	return 0;
}

void generateNumeric(char *pw, int passwordLength) {
	int b = 53, e = 61;
	generatePassword(pw, passwordLength, b, e);
}

void generateAlpha(char *pw, int passwordLength) {
	int b = 27, e = 51;
	generatePassword(pw, passwordLength, b, e);
}

void generateMixed(char *pw, int passwordLength) {
	int b = 0, e = 62;
	generatePassword(pw, passwordLength, b, e);
}

void generateSecure(char *pw, int passwordLength) {
	int b = 0, e = 88;
	generatePassword(pw, passwordLength, b, e);
}

void generatePassword(char *pw, int passwordLength, int b, int e) {
	srand(time(NULL));
	char *charset =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+[]{}|;:,.<>?/~";
	for (int i = 0; i < passwordLength; i++) {
		pw[i] = charset[(rand() % (e - b + 1)) + b];
	}
	pw[passwordLength] = '\0';
}

int Check(char string[]) {

	for (int i = 0; i < strlen(string); ++i) {
		if (*(string + i) == 'q') {
			return 1;
		}
	}

	return 0;
}

