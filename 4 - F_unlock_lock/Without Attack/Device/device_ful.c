#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/types.h>

#include<netinet/in.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define shmsize 8

int main()
{
	int device_socket, driver_socket, bind_status, address_size, listen_status;
	char isolated_memory[128], temp_buff[128], auth_status[16], auth_granted[16];

	// Shared Memory
	int shm_id;
	key_t key = 1234;
    char *shm_address;

	shm_id = shmget(key, shmsize, IPC_CREAT | 0666);
    shm_address = shmat(shm_id, NULL, 0);

    // Shared Lock
	int shm_id_lock;
	key_t key_lock = 5678;
	char *shm_lock;

	shm_id_lock = shmget(key_lock, shmsize, IPC_CREAT | 0666);
	shm_lock = shmat(shm_id_lock, NULL, 0);

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

	// Accept a connection from the driver
	address_size = sizeof(device_socket);
	driver_socket = accept(device_socket, (struct sockaddr*)&server_addr, (socklen_t*)&address_size);
	if(driver_socket == -1)
	{
		printf("Accept failed\n");
		return -1;
	}

	// Copy the firmware to the temporary buffer of the device
	if(strcmp("lock", shm_lock) == 0)
	{
		read(driver_socket, &temp_buff, sizeof(temp_buff));
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
		printf("Error while creating socket\n");
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
		printf("Connection was not established\n");
		return -1;
	}

    // Send data of temporary buffer to the crypto engine
	write(device_CE_socket, (char*)temp_buff, strlen(temp_buff));

    // Copy data to IM
	read(device_CE_socket, &auth_granted, sizeof(auth_granted));
	if (strcmp("GRANTED", auth_granted) == 0)
	{
        strcpy((char*)isolated_memory, (char*)temp_buff);
        printf("Isolated Memory contains: %s\n", isolated_memory);
	}

	// Send an authorization request to the crypto engine
	write(device_CE_socket, (char*)isolated_memory, strlen(isolated_memory));

	// Wait here for the status from the crypto engine
	read(device_CE_socket, &auth_status, sizeof(auth_status));


	// ---------------------------------------------------

	sleep(3);
	write(driver_socket, (char *)auth_status, strlen(auth_status));

	return 0;
}

