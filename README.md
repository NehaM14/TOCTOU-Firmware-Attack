# TOCTOU-Firmware-Attack
Compilation:
We used the gcc compiler to compile the C codes. Please follow the instructions below and run them in the following order only.

gcc <file-name> -o <executable-name> -lpthread

For example: gcc crypto_engine.c -o crypto -lpthread

Once the compilation for all the files are done, run them in the following order and make sure to run these in separate terminals.
1. Run the crypto executable
2. Run the device now
3. Run the driver. To run the driver, please pass the firmware as the command line arguments as shown below for each protocol.

To see the recreated attack:
F_basic:
1. Go to the /1 - F_basic folder and compile the three files using the information provided above.
2. Run the executable of crypto engine in one terminal
3. Open another terminal and run the executable of device. By this time both of these terminals are up and running and waiting for a request from the driver
4. Open two terminals for the driver. Run the executable of the driver and pass the firmware string along with it.
	For eg. ./driver abcd
	Note: In our implementation we have considered string "abcd" as the authentic firmware
5. In the first driver terminal send a request with the correct firmware - ./driver abcd
6. While the request is being processed, in the second driver terminal send a request with the malicious firmware - ./driver corrupt for example.
7. See the console window for useful information on what is happening in the background.

F_active:
1. Go to the /2 - F_active folder and compile the three files using the information provided above.
2. Run the executable of crypto engine in one terminal
3. Open another terminal and run the executable of device. By this time both of these terminals are up and running and waiting for a request from the driver
4. Open three terminals for the driver. Run the executable of the driver and pass the firmware string along with it. Here, along with the firmware pass the reset status as the third argument as well.
	For eg. ./driver abcd false
	Note: In our implementation we have considered string "abcd" as the authentic firmware and false as the default value for reset
5. In the first driver terminal send a request with the correct firmware - ./driver abcd false
	Note: You would be able to see that the authentication is being performed at the Crypto server
6. While the request is being processed, in the second driver terminal send a request with the malicious firmware but without resetting the device - ./driver corrupt true
	Note: You would be able to observe that the firmware cannot be copied. This is intended. The reset status makes sure that the memory is updated only once.
7. Now again send a request in the third driver terminal with the malicious firmware and with resetting the device - ./driver corrupt false
	Note: Since the device is reset, the firmware is copied in this case and allows for the firmware attack.
8. See the console window for useful information on what is happening in the background.

F_lock_unlock:
1. Go to the /3 - F_lock_unlock/WithAttack folder and compile the file using the information provided above.
2. Here, you would be using a single terminal. Run the executable and see the console window for useful information on what is happening in the background.

F_unlock_lock:
1. Go to the /4 - F_unlock_lock/WithAttack folder and compile the file using the information provided above.
2. Here, you would be using a single terminal. Run the executable and see the console window for useful information on what is happening in the background.



    