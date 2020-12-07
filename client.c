#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <math.h>

#define LENGTH 2048		//Max size of message can only be 2KB
#define MAX_CLIENTS 100		//Max No of clients that can join are 100
#define NAME_LEN 32		//Size of Name can only be 32 bytes

// Global variables
volatile sig_atomic_t flag = 0;	//to check for any signals to quit
int sockfd = 0;
char name[32];
long int encKey = 1020;
char msg[2080];
void encrypt(char str[]);	//Function to encrypt the message
void decrypt(char str[]);	//Function to decrypt the message

//Function to Clear out stdout
void str_overwrite_stdout() {
	printf("%s", "> ");
	fflush(stdout);
}

//Function to replace linefeed with null terminator
void str_trim_lf (char* arr, int length) {
	int i;
	for (i = 0; i < length; i++) { // trim \n
		if (arr[i] == '\n') {
			arr[i] = '\0';
			break;
		}
	}
}
//Signal Function to catch exit like Ctrl+C or manual exit 
void catch_ctrl_c_and_exit(int sig) {
	flag = 1;
}

// Function to Send Messages from client to server
void send_msg_handler() {
	char message[LENGTH] = {};				//var to store message
	char buffer[LENGTH + NAME_LEN + 2] = {};		//var that stores message along with name 

	while(1) {
		str_overwrite_stdout();				//function call
		fgets(message, LENGTH, stdin);			//store message in message
		str_trim_lf(message, LENGTH);			//function call

		if (strcmp(message, "exit") == 0) {		//check if user wants to quit
				break;
		}
		else{						//else continue
			sprintf(buffer, "%s: %s\n", name, message);		//store message and name in buffer
			strcpy(msg, buffer);					//copy buffer to msg
			encrypt(msg);						//encrypt message function call
			strcpy(buffer,msg);					//copy encrypted message back to buffer
			send(sockfd, buffer, strlen(buffer), 0);		//send buffer to server
		}

		bzero(message, LENGTH);				// clear message var
		bzero(buffer, LENGTH + NAME_LEN);		// clear buffer var
	}
	catch_ctrl_c_and_exit(2);				// function call
}

//Function to recieve messages from server
void recv_msg_handler() {
	char message[LENGTH] = {};				//var to store message
	while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
		if (receive > 0) {
			
			if(message[LENGTH] == 10){	//if message is someone joined then display encryption key on every client
				printf("%s",message);
			}
			else{				//else continue with message
				strcpy(msg,message);	//copy message to msg var
				decrypt(msg);		//decrypt msg function call
				printf("%s", msg);	//display message
			}
			
			str_overwrite_stdout();		//clear stdout
		}
		else if (receive == 0) {
			break;
		}
		memset(message, 0, sizeof(message));	//clear message var
	}
}

//Driver Function
int main(int argc, char **argv){
	//Initial arg Check
	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}
	//localHost IP
	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);
	//signal initialization
	signal(SIGINT, catch_ctrl_c_and_exit);

	printf("Please enter your name: ");
	fgets(name, NAME_LEN, stdin);				//store name of user
	str_trim_lf(name, strlen(name));			//function call

	if (strlen(name) > NAME_LEN-1 || strlen(name) < 2){
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	// Socket settings
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);


	// Connect to Server
	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(sockfd, name, NAME_LEN , 0);

	printf("=== WELCOME TO THE CHATROOM ===\n");

	pthread_t send_msg_thread;						//Creating thread to send messages
	if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;						//Creating thread to receive messages
	if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){							//if signal return true then terminate threads and exit
			printf("\nBye\n");
			break;
		}
	}

	close(sockfd);

	return EXIT_SUCCESS;
}

//function to encrypt the message
void encrypt(char str[]){
    for(int i = 0; ( str[i] != '\0'); i++)
    	str[i] = str[i] + encKey;
}

//function to decrypt the message
void decrypt(char str[]){
    for(int i = 0; (str[i] != '\0'); i++)
    	str[i] = str[i] - encKey;
}
