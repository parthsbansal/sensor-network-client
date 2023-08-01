/**
 * client.c
 *
 * @author Lucas Walbeck
 *
 * USD COMP 375: Computer Networks
 * Project 1
 *
 * Description: The following program is a client which receives data from the USD Sensor 
 * Network in order to determine the temperature, humidity, and temperature
 * around USD's campus by connecting to the sensor ports to receive the data
 */

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFF_SIZE 1024

long prompt();
int connectToHost(char *hostname, char *port);
void mainLoop();
void connectToSensorNetwork(char *request);

int main() {
	printf("WELCOME TO THE COMP375 SENSOR NETWORK\n\n");
	mainLoop();
	return 0;
}

/**
 * Loop to keep asking user what they want to do and calling the appropriate
 * function to handle the selection.
 *
 * @param server_fd Socket file descriptor for communicating with the server
 */
void mainLoop() {

	while (1) {
		
		long selection = prompt();

		switch (selection) {
			case 1:
				// TODO: Handle case one by calling a function you write
				connectToSensorNetwork("AIR TEMPERATURE");
				break;
			case 2:
				connectToSensorNetwork("RELATIVE HUMIDITY");
				break;
			case 3:
				connectToSensorNetwork("WIND SPEED");
				break;
			case 4:
				printf("GOODBYE\n");
				exit(0);
				break;
			// TODO: add cases for other menu options
			default:
				fprintf(stderr, "ERROR: Invalid selection\n");
				break;
		}
	}

}

/** 
 * Print command prompt to user and obtain user input.
 *
 * @return The user's desired selection, or -1 if invalid selection.
 */
long prompt() {
	// TODO: add printfs to print out the options
	printf("Which sensor would you like to read:\n\n");
	printf("\t(1) Air temperature\n");
	printf("\t(2) Relative humidity\n");
	printf("\t(3) Wind speed\n");
	printf("\t(4) Quit Program\n");
	printf("Selection: ");
	// Read in a value from standard input
	char input[10];
	memset(input, 0, 10); // set all characters in input to '\0' (i.e. nul)
	char *read_str = fgets(input, 10, stdin);

	// Check if EOF or an error, exiting the program in both cases.
	if (read_str == NULL) {
		if (feof(stdin)) {
			exit(0);
		}
		else if (ferror(stdin)) {
			perror("fgets");
			exit(1);
		}
	}
	// get rid of newline, if there is one
	char *new_line = strchr(input, '\n');
	if (new_line != NULL) new_line[0] = '\0';

	// convert string to a long int
	char *end;
	long selection = strtol(input, &end, 10);

	if (end == input || *end != '\0') {
		selection = -1;
	}

	return selection;
}

/**
 * Socket implementation of connecting to a host at a specific port.
 *
 * @param hostname The name of the host to connect to (e.g. "foo.sandiego.edu")
 * @param port The port number to connect to
 * @return File descriptor of new socket to use.
 */
int connectToHost(char *hostname, char *port) {
	// Step 1: fill in the address info in preparation for setting 
	//   up the socket
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;       // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// get ready to connect
	if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Step 2: Make a call to socket
	int fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (fd == -1) {
		perror("socket");
		exit(1);
	}

	// Step 3: connect!
	if (connect(fd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
		perror("connect");
		exit(1);
	}

	freeaddrinfo(servinfo); // free's the memory allocated by getaddrinfo

	return fd;
}
/**
 * Connects to the comp375 network and sensor network with a given request
 * in order to print out the following specified data in the request
 * @param request is the string given by the prompt function (e.g. AIR TEMPERATURE)
 * @return nothing, but instead prints out the following data in a legible format
 */
void connectToSensorNetwork(char *request) {

	/*
	Connection to USD Main Server
	*/
	int server_fd = connectToHost("comp375.sandiego.edu", "47789");
	
	/*
	Connection to USD Network
	*/
	char buff[BUFF_SIZE];
	memset(buff, 0, BUFF_SIZE);
	strcpy(buff, "AUTH password123\n");
	if(send(server_fd, buff, BUFF_SIZE, 0) == -1) {
		printf("Something went wrong connecting\n");
	}
	memset(buff, 0, BUFF_SIZE);
	if(recv(server_fd, buff, BUFF_SIZE, 0) == -1) {
		printf("Something went wrong receiving\n");
	}

	/*
	Connection to Sensor Network
	*/
	char sensorPort[6];
	memset(sensorPort, 0, 6);
	strncpy(sensorPort, &buff[28], 5); //grabbing port # from substring of buff
	sensorPort[5] = '\0';
	int sensor_fd = connectToHost("sensor.sandiego.edu", sensorPort);
	memset(buff, 0, BUFF_SIZE);
	strcpy(buff, "AUTH sensorpass321\n");
	if(send(sensor_fd, buff, BUFF_SIZE, 0) == -1) {
		printf("Something went wrong connecting to sensor\n");
	}
	memset(buff, 0, BUFF_SIZE);
	if(recv(sensor_fd, buff, BUFF_SIZE, 0) == -1) {
		printf("Something went wrong receiving\n");
	}

	/*
	Selection of Sensor Data
	*/
	memset(buff, 0, BUFF_SIZE);
	strcpy(buff, request);
	if(send(sensor_fd, buff, BUFF_SIZE, 0) == -1) {
		printf("Something went wrong connecting to sensor\n");
	}
	memset(buff, 0, BUFF_SIZE);
	if(recv(sensor_fd, buff, BUFF_SIZE, 0) == -1) {
		printf("Something went wrong receiving\n");
	}

	/*
	Formatting of Time
	*/
	char timeBuff[11];
	memset(timeBuff, 0, 11);
	strncpy(timeBuff, &buff[0], 10);
	timeBuff[10] = '\0';
	time_t timeSnap = atol(timeBuff);

	/*
	Formatting of Read In Data
	*/
	int len = strlen(buff) - 11;
	char conditionBuff[len];
	memset(conditionBuff, 0, len);
	strncpy(conditionBuff, &buff[11], len);
	conditionBuff[len-1] = '\0';

	/*
	Printing of Data
	*/
	printf("The last %s reading was %s, taken at %s\n", request, conditionBuff, ctime(&timeSnap));

	/*
	Closing of Sensor Data
	*/
	memset(buff, 0, BUFF_SIZE);
	strcpy(buff, "CLOSE\n");
	if(send(sensor_fd, buff, BUFF_SIZE, 0) == -1) {
		printf("Something went wrong sending\n");
	}
	memset(buff, 0, BUFF_SIZE);
	if(recv(sensor_fd, buff, BUFF_SIZE, 0) == -1) {
		printf("Something went wrong receiving\n");
	}

	//calling close for both functions
	close(sensor_fd);
	close(server_fd);
}
