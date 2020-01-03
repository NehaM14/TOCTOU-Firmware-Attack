#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<netinet/in.h>
#include<sys/shm.h>
#define SHSIZE 16

int main()
{
	int device_socket, driver_socket, bind_status, address_size, listen_status,ack, shmid;
	char isolated_memory[128], auth_status[16], *lock, *shm;
	key_t key;
	key = 1111;
	shmid = shmget(key,SHSIZE, IPC_CREAT | 0666);
	//printf("SHMID: %d\n", shmid);
	if(shmid < 0)
	{
		printf("Error with Shared memory\n");
		exit(0);	
	}
	lock = shmat(shmid, NULL, 0);
	if(lock == (char *) -1)
	{
		printf("Error with Shared pointer\n");
		exit(0);	
	}
	memcpy(lock,"F", 2);
	
	// Create a socket and check for errors
	device_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(device_socket < 0)
	{
		printf("Error while creating socket\n\n");
		return -1;
	}
	
	// Populate the address related data structures
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8000);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	// Bind the device socket 
	bind_status = bind(device_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(bind_status == -1)
	{
		printf("Bind failed\n\n");
		return -1;
	}
	
	// Do a listen
	listen_status = listen(device_socket, 3);
	if(listen_status == -1)
	{
		printf("Listen failed\n\n");
		return -1;
	}
	printf("Device is waiting for update, if any!\n\n");
	
	// Accept a connection from the driver
	address_size = sizeof(device_socket);
	driver_socket = accept(device_socket, (struct sockaddr*)&server_addr, (socklen_t*)&address_size);
	if(driver_socket == -1)
	{
		printf("Accept failed\n");
		return -1;
	}
	
	// Copy the firmware to the isolated memory of the device
		
	if(strcmp("F", lock) == 0)
	{	
	printf("Isolated Memory is not locked...Copying image to isolated memory\n\n");	
	sleep(2);	
	read(driver_socket, &isolated_memory, sizeof(isolated_memory));
	}
	else
	{
	printf("Isolated Memory is locked...Cannot copy image to memory\n\n");
	exit(0);
	}
	
	// ---------------------------------------------------
	// -TO DO- 
	/* Create another socket here and establish a connection
	 * to the CE - Cryptographic engine */
	 
	// Create a socket and check for errors
	
	int device_CE_socket, con_status_CE;
	
	device_CE_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(device_CE_socket < 0)
	{
		printf("Error while creating socket\n\n");
		return -1;
	}
	
	// Populate the address related data structures
	struct sockaddr_in server_addr_CE;
	server_addr_CE.sin_family = AF_INET;
	server_addr_CE.sin_port = htons(8000);
	server_addr_CE.sin_addr.s_addr = inet_addr("127.0.0.2");
	
	// Connect to the crypto engine
	con_status_CE = connect(device_CE_socket, (struct sockaddr*)&server_addr_CE, sizeof(server_addr_CE));
	if(con_status_CE == -1)
	{
		printf("Connection was not established\n\n");
		return -1;
	}
	if(con_status_CE == 0)
	{
		printf("Connection to CE successfull\n\n");
	}
	// Send an authorization request to the crypto engine
	//printf("Number of arguments: %d\n", number_of_args);
	printf("Sending image to CE for verification...\n");	
	sleep(1);
	write(device_CE_socket, (char*)isolated_memory, strlen(isolated_memory));
		
	// Wait here for the status from the crypto engine
	read(device_CE_socket, &auth_status, sizeof(auth_status));
	
	if (strcmp("PASSED", auth_status) == 0)
		{
			printf("Authorization succedded, Activating F/W: %s on isolated memory\n\n",isolated_memory);
			//printf("Status of lock: %s\n", lock);
			write(device_CE_socket, (char*)ack, sizeof(ack));
		}
	else
		{
			printf("Authorization failed!!!\n\n");			
			write(device_CE_socket, (char*)ack, sizeof(ack));
		}
	// ---------------------------------------------------
	
	sleep(2);
	printf("Sending status to driver...\n\n");
	write(driver_socket, (char *)auth_status, strlen(auth_status));
	sleep(2);
	close(device_socket);
	//while(1){}
	
	return 0;
}

