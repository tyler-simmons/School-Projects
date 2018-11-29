#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include "serverLib.h"


#define DEFAULT_DICTIONARY "test.txt"
#define DEFAULT_PORT 8888
#define MAX_WORDSIZE 64
#define NUM_WORKERS 3





/* Globals */
int numWords;

char *server_dictionary;
int server_port;
int default_dictionary_selection;
int default_port_selection;

char **dictionaryWords;




int getLines(char *dictionaryName);
void populateDictionary(char **dictionary, char *dictionaryName);
int checkSpelling(char **dictionaryWords, char *word, int numWords);




int main(int argc, char **argv){
    
    
    /******************************/
        /*Handle the arg cases*/
    /******************************/
    
    if (argc > 3){ /* too many args */
        printf("Error: too many arguments\n");
        exit(-1);
    } else if (argc == 1){ /* use defualts */
        /******************************/
        default_dictionary_selection = 1;
        default_port_selection = 1;
        /******************************/
        
        server_port = DEFAULT_PORT;
        server_dictionary = DEFAULT_DICTIONARY;
    
    
    } else if (argc == 2){ /*user dictionary | default port */
        /******************************/
        default_port_selection = 1;
        default_dictionary_selection = 0;
        /******************************/
        
        server_port = DEFAULT_PORT;
        FILE *temp = fopen(*(argv + 1), "r");
        if(temp == NULL){
            printf("Error: dictionary failed to open\n");
            exit(-1);
        }else{
            fclose(temp);
            server_dictionary = malloc((sizeof(char) * strlen(*(argv + 1))) + 1);
            server_dictionary = *(argv + 1);
        }
    
    } else if (argc == 3){ /* user dictionary | user port */
        
        /******************************/
        default_port_selection = 0;
        default_dictionary_selection = 0;
        /******************************/
        
        int n;
        n = atoi(*(argv + 2));
        if (n == 0){
            printf("Error: bad port number\n");
            exit(-1);
        } else {
           server_port = n;  
        } 
        FILE *temp1 = fopen(*(argv + 1), "r");
        if(temp1 == NULL){
            printf("Error: dictionary failed to open\n");
            exit(-1);
        }else{
            fclose(temp1);
            server_dictionary = malloc((sizeof(char) * strlen(*(argv + 1))) + 1);
            server_dictionary = *(argv + 1);
        } 
    }
    
    
    
    
    
    /******************************/
        /*Initialize Globals*/
    /******************************/
    
    numWords = getLines(DEFAULT_DICTIONARY);
    dictionaryWords = malloc(numWords * sizeof(char*));
    populateDictionary(dictionaryWords, DEFAULT_DICTIONARY);

    



    /******************************/
        /*Initialize the server*/
    /******************************/
    int server_socket;
    struct sockaddr_in server;

    printf("\n***SERVER ATTEMPTING TO OPEN CONNECTION***\n");
    server_socket = socket(AF_INET , SOCK_STREAM , 0);
    
    if (server_socket == -1){
        printf("Server: Error creating socket\n");
        exit(-1);
    } else {
        printf("Server: Socket creation successful\n");
    }
    
    
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    
    printf("\n***BINDING SERVER'S ADDRESS***\n");
    if (bind(server_socket,(struct sockaddr *)&server , sizeof(server)) < 0){
        printf("Server: Bind failed\n");
    }
    

    
    listen(server_socket, 64);
    
    
    

    
    
    /******************************/
        /*Socket*/
    /******************************/
    
	
    int new_socket;
    struct sockaddr_in client;
    int c;
    c = sizeof(struct sockaddr_in);
    new_socket = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&c);
    if (new_socket < 0){
        printf("Error accepting socket\n");
    }
	char *message = "\n***CONNECTED TO SPELLCHECK SERVER***\n";
    write(new_socket , message , strlen(message));
    
    
    
    
    
    
    int read_size;
    char *clientInput = malloc(sizeof(char) * MAX_WORDSIZE);
    
    

    while(read_size = recv(new_socket, clientInput, sizeof(clientInput) - 1, 0) > 0){
        
        int correct;
        correct = checkSpelling(dictionaryWords, clientInput, numWords);
        
        
        
        if (correct == 1){
            char *notify = malloc((sizeof(char) * MAX_WORDSIZE) + 3);
            strcpy(notify, clientInput);
            int i = 0;
            strcat(notify, "\n***OK***\n\n");
            write(new_socket, notify, strlen(notify));
            
            free(notify); 
        } else {
            char *badnotify = malloc((sizeof(char) * MAX_WORDSIZE) + 3);
            strcpy(badnotify, clientInput);
            int i;
            strcat(badnotify, "***MISSPELLED***\n\n");
            write(new_socket, badnotify, strlen(badnotify));
            
            free(badnotify);
        }

    }
    
    

    
    
    
    
    

    
    shutdown(server_socket, 2);
    
    
    return 0;

}



/* given a file name, opens the file and returns the number of newline characters */
int getLines(char *dictionaryName){
    
    FILE *counterFP = fopen(dictionaryName, "r");
    int count = 0;
    int c;
    
    /* successful file opening check */
    if(counterFP == NULL){
        printf("Error: getLines failed to open file\n");
    }
    
    /* loop through file incrementing line counter upon each newline */
    while ((c = fgetc(counterFP)) != EOF){
        /* increment count for each new line */
        if (c == '\n'){
            count ++;
        }
    }
    
    fclose(counterFP);
    return count;
}



/* 
 * Given a preallocated space of memory for char**, uses the filename of the
 * supplied dictionary to populate the elements of the dictionary in memory
 */
void populateDictionary(char **dictionary, char *dictionaryName){
    
    FILE *populateFP = fopen(dictionaryName, "r");
    /* Allocate for the working line to be populated by fgets */
    char *currentLine = malloc(MAX_WORDSIZE * sizeof(char));
    int offset = 0;
    int wordLength;

    /*
     * Go through the file -> for each line allocate memory in the char **
     * that was passed in as an argument -> copy that char* to the given
     * location with supplied offset
     */ 
    while(fgets(currentLine, MAX_WORDSIZE, populateFP) != NULL){
        wordLength = strlen(currentLine);
        *(dictionary + offset) = malloc((wordLength * sizeof(char)) + 1);
        strcpy(*(dictionary + offset), currentLine);
        offset++;
    }

    fclose(populateFP);
    free(currentLine);
}

int checkSpelling(char **dictionaryWords, char *word, int numWords){
    int offset = 0;
    int stopPos;
    int checker;
    int tracker;
    int wordMatch = 0;
    
    /* while there are still words in dictionary */
    while (offset < numWords){
        
        /* how long is that dictionary term */
        //printf("debug current word: %s", *(dictionaryWords + offset));
        stopPos = (strlen(*(dictionaryWords + offset)) - 2);
        
        /* reset the checker */
        checker = 0;
        
        //printf("(OUTER)stop position: %d\n", stopPos);
        
        
        /* check if the word matches that particular dictionary entry */
        for (tracker = 0; tracker <= stopPos; tracker++){
            
            
            /**********************************************/
            //printf("\n(INNER)track and off, %d %d\n", tracker, offset);
            //printf("(INNER)dictionary word: %s", *(dictionaryWords + offset));
            //printf("(INNER)supplied word: %s", word);
            //printf("(INNER)%c and %c\n\n\n", *(*(dictionaryWords + offset) + tracker), *(word + tracker));
            /**********************************************/
            
            
            
            /* word doesn't match ->  break this word loop and let outer loop know that word was wrong(checker)*/
            if (*(*(dictionaryWords + offset) + tracker) != *(word + tracker)){
                checker = -1;
                //printf("Moving on...\n\n");
                break;
            } 

        }
        
        /* word didnt match -> onto next one */
        if (checker == -1){
            offset = offset + 1;
        } else {    /* none of the characters were a mismatch -> correct word */
            /* set return value to indicate match found */
            wordMatch = 1;
            /*no more outer looping through dictionary needed */
            break;
        }
    
    }
    
    return wordMatch;
}


