#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<netinet/in.h>
#include<sys/shm.h>
#define SHSIZE 4
int main()
{
	int crypto_socket, device_socket, bind_status, address_size, listen_status, ack, ack_status, lock_id, flag_id;
	char auth_signal[32];
	char auth_status[16], *lock, *flag;
	key_t lock_key, flag_key;
	lock_key = 1111;
	flag_key = 3333;	
	lock_id = shmget(lock_key,SHSIZE, 0666);
	flag_id = shmget(flag_key,SHSIZE, 0666);
	//printf("SHMID: %d\n", shmid);
	
	if((lock_id < 0) || (flag_id < 0))
	{
		printf("Error with Shared memory\n");
		exit(0);	
	}
	lock = shmat(lock_id, NULL, 0);
	flag = shmat(flag_id, NULL, 0);
	if((lock == (char *) -1) || (flag == (char *) -1))
	{
		printf("Error with Shared pointer\n");
		exit(0);	
	}
	
	// Create a socket and check for errors
	crypto_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(crypto_socket < 0)
	{
		printf("Error while creating socket\n\n");
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
	printf("Crypto Engine is UP!\n\n");
	
	// Accept a connection from the device
	address_size = sizeof(crypto_socket);
	device_socket = accept(crypto_socket, (struct sockaddr*)&server_addr_CE, (socklen_t*)&address_size);
	if(device_socket == -1)
	{
		printf("Accept failed\n");
		return -1;
	}
	// Read the request from the device for authorization
		while(1){
		if(strcmp("T", flag) == 0){
		printf("Authorization request received\n\n");
		sleep(1);		
		printf("Authorization granted!!! Unlocking Isolated Memory!!!\n\n");
		sleep(1);			
		memcpy(lock, "F", 1);
		//sleep(1);
		break;
		}		
		}
		while(1){		
			
		sleep(1);	
		if (read(device_socket, &auth_signal, sizeof(auth_signal)) > 0)
		{	
			printf("Locking Isolated Memory\n\n");
			sleep(2);			
			memcpy(lock, "T", 1);
			printf("Verifying image!\n\n");
			sleep(2);
			if (strcmp("abcd", auth_signal) == 0)
			{	sleep(2);
				strcpy(auth_status, "PASSED");
				printf("Signature verification succeeded for: %s\n\n",auth_signal);
				break;
			}
			else
			{	sleep(2);
				strcpy(auth_status, "FAILED");
				printf("Signature verification failed!\n\n");
				break;			
			}
		}
		}
	
	
	sleep(2);
	
	// Send the authorization status to the device
	//printf("Sending authorization status to device...\n\n");
	sleep(2);	
	write(device_socket, (char *)auth_status, strlen(auth_status));
	sleep(2);
	close(crypto_socket);
	//while(1){}	
	return 0;
}

