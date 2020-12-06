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

#define LENGTH 2048
#define MAX_CLIENTS 100
#define NAME_LEN 32

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];
long int encKey = 1020;
char msg[2080];
void encrypt(char str[]);
void decrypt(char str[]);
int checkUser(char str[]);


void str_overwrite_stdout() {
	printf("%s", "> ");
	fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
	int i;
	for (i = 0; i < length; i++) { // trim \n
		if (arr[i] == '\n') {
			arr[i] = '\0';
			break;
		}
	}
}
//Signal Function
void catch_ctrl_c_and_exit(int sig) {
	flag = 1;
}

void send_msg_handler() {
	char message[LENGTH] = {};
	char buffer[LENGTH + NAME_LEN + 2] = {};

	while(1) {
		str_overwrite_stdout();
		fgets(message, LENGTH, stdin);
		str_trim_lf(message, LENGTH);

		if (strcmp(message, "exit") == 0) {
				break;
		}
		else{
			sprintf(buffer, "%s: %s\n", name, message);
			strcpy(msg, buffer);
			encrypt(msg);
			strcpy(buffer,msg);
			send(sockfd, buffer, strlen(buffer), 0);
		}

		bzero(message, LENGTH);
		bzero(buffer, LENGTH + NAME_LEN);
	}
	catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
	char message[LENGTH] = {};
	while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
		if (receive > 0) {
			int x = checkUser(message);
			if(x == 1){
				printf("%s",message);
			}
			else{
				strcpy(msg,message);
				decrypt(msg);
				printf("%s", msg);
			}
			
			str_overwrite_stdout();
		}
		else if (receive == 0) {
			break;
		}
		memset(message, 0, sizeof(message));
	}
}

int main(int argc, char **argv){
	//Initial arg Check
	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}
	//localHost IP
	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	signal(SIGINT, catch_ctrl_c_and_exit);

	printf("Please enter your name: ");
	fgets(name, NAME_LEN, stdin);
	str_trim_lf(name, strlen(name));

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

	pthread_t send_msg_thread;
	if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
	if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){
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

int checkUser(char str[]){
    int search[2];
	search[0] =10;
	search[1] = 10;
    int count1 = 0, count2 = 0, i, j, flag;
    while (str[count1] != '\0')
        count1++;
    while (search[count2] != '\0')
        count2++;
    for (i = 0; i <= count1 - count2; i++)
    {
        for (j = i; j < i + count2; j++)
        {
            flag = 1;
            if (str[j] != search[j - i])
            {
                flag = 0;
                break;
            }
        }
        if (flag == 1)
            break;
    }
    if (flag == 1)
        return 1;
    else
        return 0;
 
    return 0;
}