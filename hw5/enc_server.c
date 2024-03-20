// Colton Melhase
// melhasec
// CS374 W2024
// Project 5 - One-Time Pads

// PSEUDO CODE - enc_client
// Set up listening port
// While forever
  // accept new connection
    // fork off new process
      // PARENT
        // close current connection as child now owns connection
      // CHILD
        // Receive/Send verifcation
        // Receive message
        // Receive key
        // cipher = (Message + Key) % 27
        // Send cipher

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Size of 1000 is big enough to complete the ~70000 long file within 10 seconds
// while also showing off the server's ability to handle multiple packets in
// one connection
const int BUFFER_SIZE = 1000;

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead, charsSent;
  char messageBuffer[BUFFER_SIZE];
  char keyBuffer[BUFFER_SIZE];
  char cipherBuffer[BUFFER_SIZE];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 1) { 
    fprintf(stderr,"USAGE: port\n"); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  int childPid = 0;
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    // printf("SERVER: Connected to client running at host %d port %d\n", 
    //                       ntohs(clientAddress.sin_addr.s_addr),
    //                       ntohs(clientAddress.sin_port));

    if((childPid = fork()) == -1) {
      close(connectionSocket);
      error("SERVER: fork() failed");
    } else if(childPid > 0) { // parent
      close(connectionSocket);
      // printf("Server MAIN forked off new child PID %d\n", childPid); fflush(stdout);
    } else if(childPid == 0) { // child
      // RECEIVE/SEND VERIFICATION MSGS
      memset(messageBuffer, '\0', BUFFER_SIZE);
      charsRead = recv(connectionSocket, messageBuffer, 255, 0);
      if (charsRead < 0){
        error("ERROR reading from socket");
      }
      if(strcmp(messageBuffer, "ENCRYPT") == 0) {
        // printf("\nSERVER: Received %s TAG, sending 1\n", messageBuffer); fflush(stdout);
        charsRead = send(connectionSocket, 
                      "1", 1, 0); 
        if (charsRead < 0){
          error("ERROR writing to socket");
        }
      } else {
        charsRead = send(connectionSocket, 
                      "0", 1, 0); 
        if (charsRead < 0){
          error("ERROR writing to socket");
        }
        close(connectionSocket); // reject non-enc client
      }
      // printf("SERVER: sent verification msg to client\n"); fflush(stdout);
      do {
        // Get the message from the client and display it
        memset(messageBuffer, '\0', BUFFER_SIZE);
        // Read the client's message from the socket
        charsRead = recv(connectionSocket, messageBuffer, BUFFER_SIZE-1, 0); 
        if (charsRead < 0){
          error("ERROR reading from socket");
        }
        // printf("SERVER: I received this from the client: \"%s\"\n", messageBuffer); fflush(stdout);
        memset(keyBuffer, '\0', BUFFER_SIZE);
        charsRead = recv(connectionSocket, keyBuffer, BUFFER_SIZE-1, 0); 
        if (charsRead < 0){
          error("ERROR reading from socket");
        }
        // printf("SERVER: I received this from the client: \"%s\"\n", keyBuffer); fflush(stdout);

        // ENCRYPT MESSAGE
        memset(cipherBuffer, '\0', BUFFER_SIZE);
        int m = 0;
        int k = 0;
        int c = 0;
        for(int i = 0; i < strlen(messageBuffer); i++) {
          if(messageBuffer[i] == ' ') { // cast char to int
            m = 26;
          } else {
            m = (int)messageBuffer[i]-65;
          }
          if(keyBuffer[i] == ' ') { // cast char to int
            k = 26;
          } else {
            k = (int)keyBuffer[i]-65;
          }
          // printf("%d: Adding chars %d + %d = %d : Mod 27 = %d(%c)\n", i, m, k, 
                    // m+k, (m+k)%27, (char)(65+(m+k)%27)); fflush(stdout);
          c = (m + k) % 27;
          if(c == 26) {
            cipherBuffer[i] = ' '; // cast int to char
          } else {
            cipherBuffer[i] = (char)(c+65);
          }
        }
        if(!(charsRead == BUFFER_SIZE-1)) { // If charsRead is < BUFFER_SIZE-1, then at end of file, attach newline
          cipherBuffer[strlen(messageBuffer)-1] = '\n';
        }
        // Send cipher back to the client
        // printf("SERVER: Sending: %s\n", cipherBuffer); fflush(stdout);
        charsSent = send(connectionSocket, 
                        cipherBuffer, strlen(cipherBuffer), 0); 
        if (charsSent < 0){
          error("ERROR writing to socket");
        }
      } while(charsRead == BUFFER_SIZE-1);
      // Close the connection socket for this client
      // printf("SERVER: JOB COMPLETE\n");
      close(connectionSocket);
      return 0;
      }
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
