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

#define PORT      20000
#define BLOCKSIZE 120

struct __attribute__((__packed__)) dns_payload {
  uint32_t sequence;
  uint8_t length;
  char data[BLOCKSIZE];
};


            /****function declarations****/
void argvs(int argc, char *argv[]);
void save_data(struct dns_query *dns_query, char *fileTO, char *dir);

        /****end of function declarations****/


/*****main function*****/
int main(int argc, char *argv[]){
    argvs(argc, argv);

    int sockfd;
    unsigned char buffer[MAX_BUFFER_SIZE];
    char *dir = argv[2];
    char *fileTO;
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "Socket creation failed.\n");
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "Bind failed.\n");
        return 0;
    }

    socklen_t len = sizeof(cliaddr);
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        int num_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE,
                                    MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        char client_addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cliaddr.sin_addr), client_addr_str, INET_ADDRSTRLEN);
        printf("---------------------------\nReceived %d bytes from %s\n",num_received, client_addr_str);
        struct dns_header *header = (struct dns_header *)buffer;
        struct dns_query name_query;
        uint8_t payload_buf[300];
        
        
        extract_dns_query(buffer, &name_query);
        if (ntohs(header->id) == 2323) {
            uint8_t base32_buf[300] = {0};
            for (int i = 0; i < name_query.num_segments - 2; ++i) {
                strncat((char *)base32_buf, name_query.segment[i], 299);
            }
            base32_decode(base32_buf, payload_buf, 300);
            
            struct stat stat_info = {0};
            if(stat(dir, &stat_info) == -1) {
                mkdir(dir, 0777);
            }
            fileTO = (char*)payload_buf;
        }
        
        //save incoming data
        if (ntohs(header->id) == 2222) {
            save_data(&name_query , fileTO, dir);
        }
        
        //responce to incoming packets
        int response_length = prepare_response(&name_query, buffer, num_received,300, "16.32.64.128");

        if (sendto(sockfd, buffer, response_length, 0, (struct sockaddr *)&cliaddr,sizeof(cliaddr)) == -1) {
            fprintf(stderr, "sendto failed.\n"); //TODO make diff err
        }
        //end program
        if (ntohs(header->id) == 5252) {
            break;
        }
    }
}
/*****end of main function*****/

/****functions****/
void argvs(int argc, char *argv[]){
    if(argc == 3){
        return;
    }else{
        fprintf(stderr, "Invalid number of arguments.\n");
        fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
        return;  //error
    }
}


void save_data(struct dns_query *dns_query, char *fileTO, char *dir) {
    uint8_t base32_buf[300] = {0};
    for (int i = 0; i < dns_query->num_segments - 2; ++i) {
        strncat((char *)base32_buf, dns_query->segment[i], 299);
    }
    uint8_t payload_buf[300];
    base32_decode(base32_buf, payload_buf, 300);

    struct dns_payload *payload = (struct dns_payload *)payload_buf;
    printf("Payload: %d\n", payload->length);
    printf("sequence %d, length %d\n", payload->sequence,payload->length);
    char filename[100];         //destination dir parsing
    strcpy (filename, dir);
    char fileHelp[100] = "/\0";
    strcat (filename, fileHelp);
    strcat(filename, fileTO);           //file name 
    FILE *fout = fopen(filename, "a+b");
    fseek(fout, strlen((const char *)payload_buf), 0);

    fwrite(payload_buf, 1, strlen((const char *)payload_buf), fout);
    fclose(fout);
    printf("Wrote %d bytes to %s at offset %d\n", payload->length, filename,payload->sequence * 120);

    return;
}

