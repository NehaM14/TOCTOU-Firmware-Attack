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
	int crypto_socket, device_socket, bind_status, address_size, listen_status, lock = 1;
	char temp_buff[128];
	char auth_signal[128];
	char auth_status[16];
    char auth_granted[16];

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

    // IM is locked by default
	strcpy(shm_lock, "lock");

	// Create a socket and check for errors
	crypto_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (crypto_socket < 0)
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
	bind_status = bind(crypto_socket, (struct sockaddr*) & server_addr_CE, sizeof(server_addr_CE));
	if (bind_status == -1)
	{
		printf("Bind failed\n");
		return -1;
	}

	// Do a listen
	listen_status = listen(crypto_socket, 3);
	if (listen_status == -1)
	{
		printf("Listen failed\n");
		return -1;
	}
	printf("Crypto Engine UP!\n");

	// Accept a connection from the device
	address_size = sizeof(crypto_socket);
	device_socket = accept(crypto_socket, (struct sockaddr*) & server_addr_CE, (socklen_t*)&address_size);
	if (device_socket == -1)
	{
		printf("Accept failed\n");
		return -1;
	}

	// Unlock IM to copy data
	if (read(device_socket, &temp_buff, sizeof(temp_buff)) > 0)
	{
		printf("Unlock IM\n");
		strcpy(shm_lock, "unlock");
        strcpy(auth_granted, "GRANTED");
        printf("Authentication granted to copy to IM!\n");
	}

    // Sent auth_granted status to the device
	write(device_socket, (char*)auth_granted, strlen(auth_granted));

	sleep(5);

	// Authenticate the firmware
    if (read(device_socket, &auth_signal, sizeof(auth_signal)) > 0)
    {
        strcpy(shm_lock, "lock");
        if (strcmp("abcd", auth_signal) == 0)
        {
            strcpy(auth_status, "PASSED");
            printf("Signature verification succeeded!\n");
        }
        else
        {
            strcpy(auth_status, "FAILED");
            printf("Signature verification failed!\n");
        }
    }


	sleep(5);

	// Send the authorization status to the device
	write(device_socket, (char*)auth_status, strlen(auth_status));

	close(crypto_socket);

	return 0;
}
