// Colton Melhase
// melhasec
// CS374 W2024
// Project 5 - One-Time Pads

// PSEUDO CODE - enc_client
// Set up socket
// Send/receive verification
// Send message
// Send key
// Recieve cipher

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
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
    // printf("CLIENT: ERROR connecting: %s\n", strerror(errno)); fflush(stdout);
    error("CLIENT: ERROR connecting:");
  }

  // Allocate buffers for the plain text, the key, and the cipher text.
  char* messageBuffer = calloc(BUFFER_SIZE, sizeof(char));
  memset(messageBuffer, '\0', BUFFER_SIZE);
  char* keyBuffer = calloc(BUFFER_SIZE, sizeof(char));
  memset(keyBuffer, '\0', BUFFER_SIZE);
  char* cipherBuffer = calloc(BUFFER_SIZE, sizeof(char));
  memset(cipherBuffer, '\0', BUFFER_SIZE);

  // ###################################################################################
  // VERIFY AUTHORIZED CONNECTION
  // ###################################################################################

  // Send ENCRYPT message to verify connection is to enc_server (not dec_server)
  char* verify = "ENCRYPT";
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
    error("CLIENT: ERROR: CANNOT CONNECT TO NON-ENCYRPT SERVER");
  }
  // ###################################################################################
  // OPEN PLAIN TEXT AND KEY FILES
  // ###################################################################################

  // Open the plaintext
  FILE *plainText = fopen(argv[1], "r");
  if (plainText == NULL) {
        error("CLIENT: fopen: plainText: Could not open file\n");
  }
  // Open the key
  FILE *key = fopen(argv[2], "r");
  if (key == NULL) {
        error("CLIENT: fopen: key: Could not open file\n");
  }
  
  do {

    // ###################################################################################
    // SEND NEXT CHUNK OF PLAIN TEXT
    // ###################################################################################

    // SEND DATA
    // Send next chunk of plainText
    fileCharsRead = fread(messageBuffer, sizeof(char), BUFFER_SIZE-1, plainText);
    // printf("\nAttempting to send: %s\n\n", messageBuffer); fflush(stdout); fflush(stdout);

    // Check for bad chars
    for(int i = 0; i < fileCharsRead-1; i++) {
      // printf("Checking char %c (%d)\n", messageBuffer[i], (int)messageBuffer[i]); fflush(stdout);
      if(!((int)messageBuffer[i] == 32 || (int)messageBuffer[i] >= 65 && (int)messageBuffer[i] <=90)) {
        error("CLIENT: Invalid character(s) to encrypt");
      }
    }
    charsWritten = send(socketFD, messageBuffer, strlen(messageBuffer), 0); 
    if (charsWritten < 0){
      error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(messageBuffer) - 1){
      printf("CLIENT: WARNING: Not all data written to socket!\n");
    }
    memset(messageBuffer, '\0', BUFFER_SIZE);

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
    // printf("\nAttempting to send KEY: %s\n\n", keyBuffer); fflush(stdout); fflush(stdout);
    charsWritten = send(socketFD, keyBuffer, strlen(keyBuffer), 0); 
    if (charsWritten < 0){
      error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(keyBuffer) - 1){
      printf("CLIENT: WARNING: Not all data written to socket!\n");
    }
    memset(keyBuffer, '\0', BUFFER_SIZE);
    
    // ###################################################################################
    // RECEIVE NEXT CHUNK OF CIPHER TEXT
    // ###################################################################################

    // RECEIVE DATA 
    // Get return message from server
    // Clear out the buffer again for reuse
    memset(cipherBuffer, '\0', BUFFER_SIZE);
    
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, cipherBuffer, BUFFER_SIZE - 1, 0); 
    if (charsRead < 0){
      error("CLIENT: ERROR reading from socket");
    }
    printf("%s", cipherBuffer); fflush(stdout); // print cipher
  } while (fileCharsRead == BUFFER_SIZE-1); // message buffer maxed out = more to send
  // Close the socket
  close(socketFD); 
  return 0;
}