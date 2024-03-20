// Colton Melhase
// melhasec
// CS374 W2024
// Project 5 - One-Time Pads

// PSEUDO CODE - dec_client
// Set up socket
// Send/recieve verification
// Send cipher
// Send key
// Recieve message

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

// Size of 1000 is big enough to complete the ~70000 long file within 10 seconds
// while also showing off the server's ability to handle multiple packets in
// one connection
const int BUFFER_SIZE = 1000;

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber, fileCharsRead, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  // char buffer[256];
  // Check usage & args
  if (argc < 3) { 
    fprintf(stderr,"USAGE: messageFile keyFile port\n", argv[0]); 
    exit(0); 
  } 

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }
  // Send message to server
  // Write to the server

  // Allocate buffers
  char* messageBuffer = calloc(BUFFER_SIZE, sizeof(char));
  memset(messageBuffer, '\0', BUFFER_SIZE);
  char* keyBuffer = calloc(BUFFER_SIZE, sizeof(char));
  memset(keyBuffer, '\0', BUFFER_SIZE);
  char* cipherBuffer = calloc(BUFFER_SIZE, sizeof(char));
  memset(cipherBuffer, '\0', BUFFER_SIZE);

  // ###################################################################################
  // VERIFY AUTHORIZED CONNECTION
  // ###################################################################################

  // Send DECRYPT message to verify connection is to dec_server (not enc_server)
  char* verify = "DECRYPT";
  char serverRes[10];
  memset(serverRes, '\0', 10);
  charsWritten = send(socketFD, verify, strlen(verify), 0);
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(verify)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }
  // RECEIVE GO-AHEAD TO SEND DATA
  charsRead = recv(socketFD, serverRes, 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  if(!(strcmp(serverRes, "1") == 0)) {
    error("CLIENT: ERROR: CANNOT CONNECT TO NON-DECRYPT SERVER");
  }

  // ###################################################################################
  // OPEN CIPHER TEXT AND KEY FILES
  // ################################################################################### 

  // Open the ciphertext
  FILE *cipherText = fopen(argv[1], "r");
  if (cipherText == NULL) {
        error("CLIENT: fopen: cipherText: Could not open file\n");
  }
  // Open the key
  FILE *key = fopen(argv[2], "r");
  if (key == NULL) {
        error("CLIENT: fopen: key: Could not open file\n");
  }

  do {

    // ###################################################################################
    // SEND NEXT CHUNK OF CIPHER TEXT
    // ###################################################################################

    // SEND DATA
    // Send next chunk of cipherText
    fileCharsRead = fread(cipherBuffer, sizeof(char), BUFFER_SIZE-1, cipherText);
    // printf("\nAttempting to send: %s\n", cipherBuffer); fflush(stdout); fflush(stdout);
    charsWritten = send(socketFD, cipherBuffer, strlen(cipherBuffer), 0); 
    if (charsWritten < 0){
      error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(cipherBuffer) - 1){
      printf("CLIENT: WARNING: Not all data written to socket!\n");
    }
    memset(cipherBuffer, '\0', BUFFER_SIZE);

    // ###################################################################################
    // SEND NEXT CHUNK OF KEY TEXT
    // ###################################################################################

    // Send next chunk of key
    fread(keyBuffer, sizeof(char), charsWritten, key);
    if(strlen(keyBuffer) < charsWritten) {
      error("CLIENT: WARNING: Key length insufficient\n");
      close(socketFD); 
      return 0;
    }
    // printf("\nAttempting to send: %s\n", keyBuffer); fflush(stdout); fflush(stdout);
    charsWritten = send(socketFD, keyBuffer, strlen(keyBuffer), 0); 
    if (charsWritten < 0){
      error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(keyBuffer) - 1){
      printf("CLIENT: WARNING: Not all data written to socket!\n");
    }
    memset(keyBuffer, '\0', BUFFER_SIZE);
    
    // ###################################################################################
    // RECEIVE NEXT CHUNK OF MESSAGE TEXT
    // ###################################################################################

    // RECEIVE DATA
    // Get return message from server
    // Clear out the buffer again for reuse
    memset(messageBuffer, '\0', BUFFER_SIZE);
    
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, messageBuffer, BUFFER_SIZE - 1, 0); 
    if (charsRead < 0){
      error("CLIENT: ERROR reading from socket");
    }
    printf("%s", messageBuffer); fflush(stdout); // print message
  } while (fileCharsRead == BUFFER_SIZE-1); // cipher buffer maxed out = more to send
  // Close the socket
  close(socketFD); 
  return 0;
}