#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../base32.h"
#include "../dns.h"
#include <sys/stat.h>
#include "dns_receiver_events.h"

#define PORT      20000    //TODO set to 53 when sending the project
#define BLOCKSIZE 120

struct __attribute__((__packed__)) dns_payload {
  uint32_t sequence;
  uint8_t length;
  char data[BLOCKSIZE];
};


/****function declarations****/
void argvs(int argc, char *argv[]);

/****end of function declarations****/


/*****main function*****/
int main(int argc, char *argv[]){
    argvs(argc, argv);

    int sockfd;
    unsigned char buffer[MAX_BUFFER_SIZE];
    char *dir = argv[2];
    char *fileTO;
    char *base;
    char *filePath;
    
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "Socket creation failed.\n");
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "Bind failed.\n");
        exit(0);
    }
    int cou = 1;
    socklen_t len = sizeof(cliaddr);
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        int num_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE,
                                    MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        char client_addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cliaddr.sin_addr), client_addr_str, INET_ADDRSTRLEN);
        //printf("---------------------------\nReceived %d bytes from %s\n",num_received, client_addr_str);
        struct dns_header *header = (struct dns_header *)buffer;
        struct dns_query name_query;
        uint8_t payloadBuf[300];
        
        
        extract_dns_query(buffer, &name_query);

        if (ntohs(header->id) == 5555) {
            uint8_t base32_buf[300] = {0};
            for (int i = 0; i < name_query.num_segments - 2; ++i) {
                strncat((char *)base32_buf, name_query.segment[i], 299);
            }
            
            
            dns_receiver__on_transfer_init(&(cliaddr.sin_addr));
            base32_decode(base32_buf, payloadBuf, 300);
            //struct dns_payload *payload = (struct dns_payload *)payloadBuf;
            base = malloc(strlen((char *)payloadBuf));
            memcpy(base,payloadBuf, strlen((char *)payloadBuf));
            
            strcat((char *)base32_buf, ".");
            strcat((char *)base32_buf, base);
            //dns_receiver__on_chunk_received(&(cliaddr.sin_addr),NULL,header->id,payload->length/8);
            dns_receiver__on_query_parsed(NULL,(char *)base32_buf);

            if (strcmp(argv[1],base) != 0){
                fprintf(stderr, "BASE not same.\n"); 
                exit(0);
            }
            
        }

        if (ntohs(header->id) == 2323) {
            uint8_t base32_buf[300] = {0};
            for (int i = 0; i < name_query.num_segments - 2; ++i) {
                strncat((char *)base32_buf, name_query.segment[i], 299);
            }

    

            base32_decode(base32_buf, payloadBuf, 300);
            //struct dns_payload *payload = (struct dns_payload *)payloadBuf;
            filePath = malloc(strlen(argv[2])+1);
            memcpy(filePath,argv[2], strlen(argv[2]));
            strcat(filePath, "/");
            fileTO = (char*)payloadBuf;  
            strcat(filePath, fileTO);
            strcat((char *)base32_buf, ".");
            strcat((char *)base32_buf, base);
            struct stat stat_info = {0};
            if(stat(dir, &stat_info) == -1) {
                mkdir(dir, 0777);
            }
            //dns_receiver__on_chunk_received(&(cliaddr.sin_addr),filePath,header->id,payload->length/8);
            dns_receiver__on_query_parsed(NULL,(char *)base32_buf);
            
        }
        
        //save incoming data
        if (ntohs(header->id) == 2222) {
            uint8_t base32_buf[300] = {0};
            for (int i = 0; i < name_query.num_segments - 2; ++i) {
                strncat((char *)base32_buf, name_query.segment[i], 299);
            }
            
            uint8_t payloadBuf[300];
            base32_decode(base32_buf, payloadBuf, 300);
            
            
            strcat((char *)base32_buf, ".");
            strcat((char *)base32_buf, base);

            //struct dns_payload *payload = (struct dns_payload *)payloadBuf;
            //printf("Payload: %d\n", payload->length);
            //printf("sequence %d, length %d\n", payload->sequence,payload->length);
            char filename[100];         //destination dir parsing
            strcpy (filename, dir);
            char fileHelp[100] = "/\0";
            strcat (filename, fileHelp);
            strcat(filename, fileTO);           //file name 
            FILE *fout = fopen(filename, "a+b");
            fseek(fout, strlen((const char *)payloadBuf), 0);
            fwrite(payloadBuf, 1, strlen((const char *)payloadBuf), fout);
            fclose(fout);
            
            dns_receiver__on_chunk_received(&(cliaddr.sin_addr),filePath,cou,strlen(((char *)base32_buf)+1)/8*5-5);
            dns_receiver__on_query_parsed(filePath,(char *)base32_buf);
            cou++;
            //printf("Wrote %d bytes to %s at offset %d\n", payload->length, filename,payload->sequence * 120);
        }
        
        //responce to incoming packets
        int response_length = prepare_response(&name_query, buffer, num_received,300, "16.32.64.128");

        if (sendto(sockfd, buffer, response_length, 0, (struct sockaddr *)&cliaddr,sizeof(cliaddr)) == -1) {
            fprintf(stderr, "sendto failed.\n");
        }
        
        //end program
        if (ntohs(header->id) == 5252) {
            uint8_t base32_buf[300] = {0};
            for (int i = 0; i < name_query.num_segments - 2; ++i) {
                strncat((char *)base32_buf, name_query.segment[i], 299);
            }
            base32_decode(base32_buf, payloadBuf, 300);
            //struct dns_payload *payload = (struct dns_payload *)payloadBuf;
            strcat((char *)base32_buf, ".");
            strcat((char *)base32_buf, base);
            //dns_receiver__on_chunk_received(&(cliaddr.sin_addr),filePath,header->id,payload->length/8);
            dns_receiver__on_query_parsed(NULL,(char *)base32_buf);
            
            FILE* fp = fopen(filePath, "r");
            // checking if the file exist or not
            if (fp == NULL) {
                printf("File Not Found!\n");
                return -1;
            } 
            fseek(fp, 0L, SEEK_END);
        
            // calculating the size of the file
            long int res = ftell(fp);
        
            // closing the file
            fclose(fp);

            dns_receiver__on_transfer_completed(filePath, (res)/8);
            break;
        }
        
    }
}
/*****end of main function*****/

/****functions****/
//argv check
void argvs(int argc, char *argv[]){
    if(argc == 3){
        return;
    }else{
        fprintf(stderr, "Invalid number of arguments.\n");
        fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
        exit(0);  //error
    }
}