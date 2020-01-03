#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/types.h>

#include<netinet/in.h>

#include <pthread.h>

typedef struct
{
	int device_socket;
	struct sockaddr address;
	int addr_len;
} connection_t;

void* auth_firmware(void* ptr)
{
	char auth_firmware[32], auth_status[16];
	connection_t * conn;
	
	if (!ptr) 
	{
		pthread_exit(0);
	}	
	conn = (connection_t *)ptr;

	// Read the request from the device for authorization
	read(conn->device_socket, &auth_firmware, sizeof(auth_firmware));

	// Authorize the firmware
	if (strcmp("abcd", auth_firmware) == 0)
	{
		strcpy(auth_status, "PASSED");
	}
	else
	{
		strcpy(auth_status, "FAILED");
	}
	
	sleep(20);	

	// Send the authorization status to the device
	write(conn->device_socket, (char *)auth_status, strlen(auth_status));
	
	free(conn);
	pthread_exit(0);
}

int main()
{
	int crypto_socket, device_socket, bind_status, address_size, listen_status;

	connection_t * connection;
	
	pthread_t crypto_thread;
	
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

	while(1)
	{	
		// Accept a connection from the device
		connection = (connection_t *)malloc(sizeof(connection_t));
		connection->device_socket = accept(crypto_socket, &connection->address, &connection->addr_len);
		if(connection->device_socket == -1)
		{
			printf("Accept failed\n");
			free(connection);
			return -1;
		}
		else
		{
			pthread_create(&crypto_thread, 0, auth_firmware, (void *)connection);
			pthread_detach(crypto_thread);
		}		
	}	
	
	return 0;
}

