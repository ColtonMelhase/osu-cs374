// Colton Melhase
// melhasec
// CS374 W2024
// Project 5 - One-Time Pads

// PSEUDO CODE - dec_client
// Set up listening port
// While forever
  // accept new connection
    // fork off new process
      // PARENT
        // close current connection as child now owns connection
      // CHILD
        // Receive/Send verifcation
        // Receive cipher
        // Receive key
        // message = cipher - (key % 27)
        // Send message

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
      if(strcmp(messageBuffer, "DECRYPT") == 0) {
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
        close(connectionSocket); // reject non-dec client
      }
      // printf("SERVER: sent verification msg to client\n"); fflush(stdout);

      do {
        // Get the message from the client and display it
        memset(cipherBuffer, '\0', BUFFER_SIZE);
        // Read the client's cipher from the socket
        charsRead = recv(connectionSocket, cipherBuffer, BUFFER_SIZE-1, 0); 
        if (charsRead < 0){
          error("ERROR reading from socket");
        }
        // printf("SERVER: I received this from the client: \"%s\"\n", cipherBuffer); fflush(stdout);
        
        memset(keyBuffer, '\0', BUFFER_SIZE);
        charsRead = recv(connectionSocket, keyBuffer, BUFFER_SIZE-1, 0); 
        if (charsRead < 0){
          error("ERROR reading from socket");
        }
        // printf("SERVER: I received this from the client: \"%s\"\n", keyBuffer); fflush(stdout);
        // printf("\nCHARS READ: %d\n\n", charsRead); fflush(stdout);

        // DECRYPT MESSAGE
        memset(messageBuffer, '\0', BUFFER_SIZE);
        int m = 0;
        int k = 0;
        int c = 0;
        for(int i = 0; i < strlen(cipherBuffer); i++) {
          if(cipherBuffer[i] == ' ') { // cast char to int
            c = 26;
          } else {
            c = (int)cipherBuffer[i]-65;
          }
          if(keyBuffer[i] == ' ') { // cast char to int
            k = 26;
          } else {
            k = (int)keyBuffer[i]-65;
          }
          // printf("%d: subbing chars %d(%c) - %d(%c) = %d : Mod 27 = %d(%c)\n", i, c, (char)(65+c), k, (char)(65+k), 
                    // c-k, c-k < 0 ? (c - k + 27) % 27 : c - k % 27, c-k < 0 ? (char)(((c - k + 27) % 27) + 65) : (char)((c - k % 27)+65)); fflush(stdout);
          if(c-k < 0) {
            m = (c - k + 27) % 27;
          } else {
            m = c - k % 27;
          }
          if(m == 26) { // cast int to char
            messageBuffer[i] = ' ';
          } else {
            messageBuffer[i] = (char)(m+65);
          }
        }
        if(!(charsRead == BUFFER_SIZE-1)) { // If charsRead is < BUFFER_SIZE-1, then at end of file, attach newline
          messageBuffer[strlen(cipherBuffer)-1] = '\n';
        }
        // Send message back to the client
        // printf("SERVER: Sending message %s\n", messageBuffer); fflush(stdout);
        charsSent = send(connectionSocket, 
                        messageBuffer, strlen(messageBuffer), 0); 
        if (charsSent < 0){
          error("ERROR writing to socket");
        }
        // printf("SERVER: charsRead == %d\n", charsRead); fflush(stdout);
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
