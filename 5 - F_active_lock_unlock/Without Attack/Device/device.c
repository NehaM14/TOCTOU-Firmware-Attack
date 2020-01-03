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
	int device_socket, driver_socket, bind_status, address_size, listen_status, auth_read;
	char isolated_memory[128], auth_status[16], ack_signal[8];
	
	/* Shared Memory Implementation for Active variable*/
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
	
	// Accept a connection from the driver
	address_size = sizeof(device_socket);
	driver_socket = accept(device_socket, (struct sockaddr*)&server_addr, (socklen_t*)&address_size);
	if(driver_socket == -1)
	{
		printf("Accept failed\n");
		return -1;
	}
	
	// If Active is true, exit the process and do not copy the firmware to IM
	if(strcmp("False", shm_active) != 0)
	{
		printf("Active is True. Cannot copy firmware to Isolated Memory\n");
		return -1;
	}	
		
	strcpy(shm_active, "True");
		
	// If IM is locked by Crypto_Engine, exit the process and do not copy the firmware to IM
	while(strcmp("Unlock", shm_lock) != 0)
	{
		printf("IM is locked. Cannot copy firmware to Isolated Memory\n");
	}
	
	// Copy the firmware to the isolated memory of the device
	read(driver_socket, &isolated_memory, sizeof(isolated_memory));
	
	printf("Isolated Memory contains: %s\n", isolated_memory);
	
	// ---------------------------------------------------
	// -TO DO- 
	/* Create another socket here and establish a connection
	 * to the CE - Cryptographic engine */
	 
	// Create a socket and check for errors
	
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
	auth_read = read(device_CE_socket, &auth_status, sizeof(auth_status));
	
	// If active is True, then check the authorization status
	if((auth_read > 0) && (strcmp(shm_active, "True") == 0))
	{
		strcpy(ack_signal, "ACK_OK");
		// Send an acknowledgement signal to Crypto for unlocking IM
		write(device_CE_socket, (char*)ack_signal, strlen(ack_signal));
		
		if (strcmp("PASSED", auth_status) == 0)
		{
			printf("Authentication PASSED: Isolated Memory is running firmware: %s\n", isolated_memory);
		}
		else
		{
			printf("Authentication FAILED. Malicious firmware\n");
		}
	}
	

	// ---------------------------------------------------
	
	sleep(3);
	write(driver_socket, (char *)auth_status, strlen(auth_status));
	
	close(device_CE_socket);
	close(device_socket);
	
	return 0;
}

