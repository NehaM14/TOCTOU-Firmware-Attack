#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<netinet/in.h>
#include<sys/shm.h>
#define SHSIZE 4


int main(int argc, char** argv)
{
	int driver_socket, con_status, rec_data, reset_id;
	char buffer[128], *reset;
	key_t reset_key;
	reset_key = 1010;
	reset_id = shmget(reset_key,SHSIZE, IPC_CREAT | 0666);
	//number_of_args = argc;
	if(reset_id < 0)
	{
		printf("Error with Shared memory\n");
		exit(0);	
	}
	reset = shmat(reset_id, NULL, 0);
	if(reset == (char *) -1)
	{
		printf("Error with Shared pointer\n");
		exit(0);	
	}	
	if(argc < 2)
	{
		printf("Firmware missing! Pass the firmware as an argument\n\n");
		return -1;
	}
	
	printf("Driver is listening for new image...\n\n");	
	// Reset the driver
	if (argc >= 2)
	{
		memcpy(reset,"T", 1);
		printf("New image detected! Resetting the device...!\n\n");
		sleep(1);
	}
	
	// Create a socket and check for errors
	driver_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(driver_socket < 0)
	{
		printf("Error while creating socket\n\n");
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
		printf("Connection was not established!!!\n\n");
		return -1;
	}
	else if(con_status == 0)
	{
		printf("Connection established to device!...\n\n");
	}
	
	// Send firmware to the device for copying
	printf("Sending image to device...\n\n");
	sleep(2);
	write(driver_socket, argv[1], strlen(argv[1]));
	if(argc > 2)
	{
		sleep(6);		
		write(driver_socket, argv[2], strlen(argv[2]));
	}		
	
	// Wait here for the status from the device
	read(driver_socket, &buffer, sizeof(buffer));
	
	printf("Status: %s\n", buffer);
	sleep(3);
	close(driver_socket);
	//while(1){}
	return 0;
}


