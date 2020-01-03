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

int main()
{
	int crypto_socket, device_socket, bind_status, address_size, listen_status;
	char auth_signal[32];
	char auth_status[16];
	char ack_signal[8];

	/* Shared Memory Implementation for Lock variable*/
	int shm_id_lock;
	key_t key_lock = 5678;
	char *shm_lock;
	
	shm_id_lock = shmget(key_lock, BUFFER_SIZE, IPC_CREAT | 0666);
	if(shm_id_lock < 0)
	{
		printf("Shared memory not created\n");
		return -1;
	}
	
	shm_lock = shmat(shm_id_lock, NULL, 0);
	if(shm_lock == (char*)-1)
	{
		printf("Shared memory not allocated\n");
		return -1;
	}
	
	// Initially the IM is set to unlocked state
	strcpy(shm_lock, "Unlock");
	
	// Create a socket and check for errors
	crypto_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(crypto_socket < 0)
	{
		printf("Error while creating socket\n");
		return -1;
	}
	
	// Populate the address related data structures
	struct sockaddr_in server_addr_CE;
	server_addr_CE.sin_family = AF_INET;
	server_addr_CE.sin_port = htons(8000);
	server_addr_CE.sin_addr.s_addr = inet_addr("127.0.0.2");
	
	// Bind the crypto socket 
	bind_status = bind(crypto_socket, (struct sockaddr*)&server_addr_CE, sizeof(server_addr_CE));
	if(bind_status == -1)
	{
		printf("Bind failed\n");
		return -1;
	}
	
	// Do a listen
	listen_status = listen(crypto_socket, 3);
	if(listen_status == -1)
	{
		printf("Listen failed\n");
		return -1;
	}
	
	printf("Cryptographic Engine is up and running. Waiting for a request from the Device\n");
	
	// Accept a connection from the device
	address_size = sizeof(crypto_socket);
	device_socket = accept(crypto_socket, (struct sockaddr*)&server_addr_CE, (socklen_t*)&address_size);
	if(device_socket == -1)
	{
		printf("Accept failed\n");
		return -1;
	}
	
	// Read the request from the device for authorization
	if (read(device_socket, &auth_signal, sizeof(auth_signal)) > 0)
	{
		// Lock the IM
		strcpy(shm_lock, "Lock");
		
		if (strcmp("abcd", auth_signal) == 0)
		{
			strcpy(auth_status, "PASSED");
		}
		else
		{
			strcpy(auth_status, "FAILED");
		}
	}
	
	sleep(5);
	
	// Send the authorization status to the device
	write(device_socket, (char *)auth_status, strlen(auth_status));
	
	// Receive an acknowledgement from the Device about the status
	read(device_socket, &ack_signal, sizeof(ack_signal));
	
	// Unlock the IM based on the acknowledgement received
	if (strcmp("ACK_OK", ack_signal) == 0)
	{
		strcpy(shm_lock, "Unlock");
	}
	
	close(crypto_socket);
	
	
	return 0;
}

