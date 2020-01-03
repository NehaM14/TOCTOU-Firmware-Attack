#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMSZ 6


int main(int argc, char** argv)
{
	int driver_socket, con_status, rec_data;
	char buffer[128];

	char c;
    int shmid;
    key_t key;
    char *shm_active;

    key = 5678;

    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    if ((shm_active = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
	
	if(argc < 3)
	{
		printf("Firmware or reset status missing! Pass the firmware as an argument\n");
		return -1;
	}
	
	strcpy(shm_active, argv[2]);
	
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
	
	// Send firmware to the device for copying
	write(driver_socket, argv[1], strlen(argv[1]));
	
	
	// Wait here for the status from the device
	read(driver_socket, &buffer, sizeof(buffer));
	
	printf("%s\n", buffer);
	return 0;
}

