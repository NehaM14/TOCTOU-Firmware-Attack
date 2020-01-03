#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/types.h>

#include<netinet/in.h>

#include <pthread.h>

typedef struct
{
	int driver_socket;
	struct sockaddr address;
	int addr_len;
} connection_t;

char isolated_memory[16];

void* copy_firmware(void* ptr)
{
	connection_t * conn;
	
	char auth_status[16];
	
	if (!ptr) 
	{
		pthread_exit(0);
	}	
	conn = (connection_t *)ptr;
	
	// Copy the firmware to the isolated memory of the device
	read(conn->driver_socket, &isolated_memory, sizeof(isolated_memory));
		
	printf("Isolated Memory contains: %s\n", isolated_memory);
		
	// Create a socket for CE and check for errors
	int device_CE_socket, con_status_CE;
	
	device_CE_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(device_CE_socket < 0)
	{
		printf("Error while creating socket\n");
		return -1;
	}
		
	// Populate the address related data structures
	struct sockaddr_in server_addr_CE;
	server_addr_CE.sin_family = AF_INET;
	server_addr_CE.sin_port = htons(8000);
	server_addr_CE.sin_addr.s_addr = inet_addr("127.0.0.2");
		
	printf("Isolated Memory is copied. Waiting for an authentication request from the Cryptographic Engine\n");
		
	// Connect to the crypto engine
	con_status_CE = connect(device_CE_socket, (struct sockaddr*)&server_addr_CE, sizeof(server_addr_CE));
	if(con_status_CE == -1)
	{
		printf("Connection was not established\n");
		return -1;
	}
	
	// Send an authorization request to the crypto engine
	write(device_CE_socket, (char*)isolated_memory, strlen(isolated_memory));
	
	// Wait here for the status from the crypto engine
	read(device_CE_socket, &auth_status, sizeof(auth_status));
		
	if (strcmp("PASSED", auth_status) == 0)
	{
		printf("Authentication PASSED: Isolated Memory is running firmware: %s\n\n\n", isolated_memory);
	}
	else
	{
		printf("Authentication FAILED: Did not jump to Isolated Memory\n\n\n");
	}
	
	write(conn->driver_socket, (char *)auth_status, strlen(auth_status));
		
	close(device_CE_socket);
	free(conn);
	pthread_exit(0);
	
}

int main()
{
	int device_socket, driver_socket, bind_status, address_size, listen_status;
	connection_t * connection;
	
	pthread_t device_thread;
	
	// Create a socket and check for errors
	device_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(device_socket < 0)
	{
		printf("Error while creating socket\n");
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
		printf("Bind failed\n");
		return -1;
	}
	
	// Do a listen
	listen_status = listen(device_socket, 3);
	if(listen_status == -1)
	{
		printf("Listen failed\n");
		return -1;
	}
	
	printf("Device is running. Waiting for a request from the Driver\n");
	
	while(1)
	{	
		// Accept a connection from the driver
		connection = (connection_t *)malloc(sizeof(connection_t));
		connection->driver_socket = accept(device_socket, &connection->address, &connection->addr_len);
		if(connection->driver_socket == -1)
		{
			printf("Accept failed\n");
			free(connection);
			return -1;
		}
		else
		{
			pthread_create(&device_thread, 0, copy_firmware, (void *)connection);
			pthread_detach(device_thread);
		}		
	}
	
	return 0;
}

