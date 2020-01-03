#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/types.h>

#include<netinet/in.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BUFFER_SIZE 8

int main(int argc, char** argv)
{
	int driver_socket, con_status, rec_data, reset = 1;
	char buffer[128];
	
	/* Shared Memory Implementation */
	int shm_id;
	key_t key = 1234;
	char *shm_active;
	
	shm_id = shmget(key, BUFFER_SIZE, IPC_CREAT | 0666);
	if(shm_id < 0)
	{
		printf("Shared memory not created\n");
		return -1;
	}
	
	shm_active = shmat(shm_id, NULL, 0);
	if(shm_active == (char*)-1)
	{
		printf("Shared memory not allocated\n");
		return -1;
	}
	
	if(reset == 1)
	{
		strcpy(shm_active, "False");
		reset = 0;
	}
	
	
	if(argc < 2)
	{
		printf("Firmware missing! Pass the firmware as an argument\n");
		return -1;
	}

	
	// Create a socket and check for errors
	driver_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(driver_socket < 0)
	{
		printf("Error while creating socket\n");
		return -1;
	}
	
	// Populate the address related data structures
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8000);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	
	// Connect to the device
	con_status = connect(driver_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(con_status == -1)
	{
		printf("Connection was not established\n");
		return -1;
	}
	
	printf("Loading the firmware to the Device. Waiting for the authentication result\n");
	
	// Send firmware to the device for copying
	write(driver_socket, argv[1], strlen(argv[1]));
	
	
	// Wait here for the status from the device
	read(driver_socket, &buffer, sizeof(buffer));
	
	printf("%s\n", buffer);
	
	close(driver_socket);
	
	return 0;
}

