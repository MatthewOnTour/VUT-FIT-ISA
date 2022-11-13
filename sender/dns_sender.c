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
#include "sys/time.h"

#define PORT    53     //port used for DNS comunication 
#define BLOCK   50



            /****function declarations****/
void argvs(int argc, char *argv[] ,bool *ipShow, bool *srcPath, int *ipNum);
int isValidIp4 (char *str);
void positioning (int argc, bool *ipShow, bool *srcPath, int *ipNum, int *baseNum, int *dstNum, int *srcNum);
bool file_exists(const char *filename);
        /****end of function declarations****/
        

/*****main function*****/
int main(int argc, char *argv[]){
    FILE *fp;
    FILE *fip;
    char fileIP[255];
    unsigned char c[101];
        /****flags and intgrs****/
    bool ipShow = false;    //flag for "-u"
    bool srcPath = false;   //flag for [SRC_FILEPATH]
    int ipNum = 0;          // position of "-u", position of ip is ipNum + 1 
    int baseNum = 0;        //position of {BASE_HOST}
    int dstNum = 0;         //position of {DST_FILEPATH}
    int srcNum = 0;         //position of [SRC_FILEPATH]
        /****end of flags and intgrs****/

    argvs(argc, argv, &ipShow, &srcPath, &ipNum);
    positioning(argc, &ipShow, &srcPath, &ipNum, &baseNum, &dstNum, &srcNum);
    
     /**file opening**/              //TODO make it in array or smth???
    if (srcPath == false){
        if (srcNum == 99){
            fp = stdin; /* read from standard input if no argument */
        }else{
            fp = fopen(argv[srcNum], "r");
            if (fp == NULL) {
                fprintf(stderr, "cannot open %s\n", argv[srcNum]);
                exit(0);
            }
        }
    }else{
        char *filename = argv[srcNum];
        if (file_exists(filename)){
            //This is good no action needed
            //:D
        }else{
            fprintf(stderr, "File %s doesn't exist or path is invalid \n", filename);
            exit(0);
        }
    }
    char *filename = "/etc/resolv.conf";
    if (file_exists(filename)){
            //This is good no action needed
            //:D
    }else{
        fprintf(stderr, "File %s doesn't exist or path is invalid \n", filename);
        exit(0);
    }

    fip = fopen("/etc/resolv.conf" , "r");
    char *ipTmp;
    if(fip == NULL) {
      fprintf(stderr, "File /etc/resolv.conf doesn't exist or path is invalid \n");
      exit(0);
   }

    while (fgets (fileIP, 255, fip) != NULL){
    char* line = strstr(fileIP, "nameserver");
    if (line){
            ipTmp = strstr(line, " ");
            char* help = strtok(ipTmp, " ");
            ipTmp = help;
            break;
        }
        
    }
    
    fclose(fip);
    //end file opening 

    //qname

    printf("\n\n%s\n\n",argv[baseNum]);

    char *qname = malloc(strlen(argv[baseNum])+1);
    memcpy(qname+1, argv[baseNum], strlen(argv[baseNum]));
    char *ptr = malloc(strlen(argv[baseNum]));
    ptr = strtok(qname+1, ".");
    *qname = strlen(ptr);
    int lenQ = strlen(ptr);
    ptr = strtok(NULL, ".");
    qname[lenQ+1] = strlen(ptr);  

    printf("\n\n%s\n\n", qname);

    
    fp = srcPath == false ? stdin : fopen(argv[srcNum], "r");
    //sending BASE_HOST while will be done just once
    while(1){
        int sockfd; 
        struct sockaddr_in     servaddr; 
        char buffer[MAX_BUFFER_SIZE]; 
        // Creating socket file descriptor 
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
            fprintf(stderr, "socket creation failed \n");
            exit(0);
        }

        memset(&servaddr, 0, sizeof(servaddr)); 
        
        // Filling server information 
        servaddr.sin_family = AF_INET; 
        servaddr.sin_port = htons(PORT);

        if(ipShow == true){
            servaddr.sin_addr.s_addr = inet_addr(argv[ipNum]);
        }else{
            servaddr.sin_addr.s_addr = inet_addr(ipTmp);
        }
        
        unsigned char buf[101] = {0};

        char *c = argv[baseNum];
        
        base32_encode((const unsigned char *)c, strlen(argv[baseNum]), buf, BLOCK);
        
        int lenB = strlen((const char *)buf) + 1;
        char bufLen[100] = {0};
        bufLen[0] = lenB - 1;
        memcpy(bufLen + 1, buf, lenB - 1);
        memcpy(buf, bufLen, lenB);

        unsigned char packet[512] = {0};
        struct dns_header *header = (struct dns_header *)packet;
        header->id = htons(5555);
        header->rd = 1;
        header->qdcount = htons(1);
        
        unsigned char *afterHeader = packet + sizeof(struct dns_header);

        memcpy(afterHeader, buf, strlen((const char *)buf));

        unsigned char *afterData = packet + sizeof(struct dns_header) + strlen((const char *)buf);

        memcpy(afterData, qname, strlen(qname)+1);
     
        unsigned char *afterQname = packet + sizeof(struct dns_header) + strlen((const char *)buf)+strlen((const char *)qname)-1;

        struct dns_response_trailer *trailer = (struct dns_response_trailer *)afterQname;
        trailer->qclass = htons(QTYPE_A);
        trailer->type = htons(QCLASS_INET);

        int len = sizeof(struct dns_header) + strlen((const char *)buf) + strlen(qname) + sizeof(trailer->qclass) + sizeof(trailer->type);
        
        sendto(sockfd, packet, len+1, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
        
        unsigned int n, lenC;

        struct timeval timeout;      
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) > 0)
        {    
            perror("set timeout failed");
            exit(EXIT_FAILURE);
        }
        if((n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &lenC)) == -1){
            fprintf(stderr, "BASEHOST not same or connection timeout \n");
            exit(0);
        }

        buffer[n] = '\0'; 
        printf("Server: %s \n", buffer);
        
        close(sockfd);
        break;
    }
    //sending DST_FILEPATH while will be done just once
    while(1){
        int sockfd; 
        struct sockaddr_in     servaddr; 
        char buffer[MAX_BUFFER_SIZE]; 
        // Creating socket file descriptor 
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
            fprintf(stderr, "socket creation failed \n");
            exit(0);
        } 

        memset(&servaddr, 0, sizeof(servaddr)); 
        
        // Filling server information 
        servaddr.sin_family = AF_INET; 
        servaddr.sin_port = htons(PORT); 
        if(ipShow == true){
            servaddr.sin_addr.s_addr = inet_addr(argv[ipNum]);
        }else{
            servaddr.sin_addr.s_addr = inet_addr(ipTmp);
        }
        unsigned char buf[101] = {0};

        char *c = argv[dstNum];
        
        base32_encode((const unsigned char *)c, strlen(argv[dstNum]), buf, BLOCK);

        int lenB = strlen((const char *)buf) + 1;
        char bufLen[100] = {0};
        bufLen[0] = lenB - 1;
        memcpy(bufLen + 1, buf, lenB - 1);
        memcpy(buf, bufLen, lenB);

        unsigned char packet[512] = {0};
        struct dns_header *header = (struct dns_header *)packet;
        header->id = htons(2323);
        header->rd = 1;
        header->qdcount = htons(1);
        
        unsigned char *afterHeader = packet + sizeof(struct dns_header);

        memcpy(afterHeader, buf, strlen((const char *)buf));

        unsigned char *afterData = packet + sizeof(struct dns_header) + strlen((const char *)buf);

        memcpy(afterData, qname, strlen(qname)+1);
     
        unsigned char *afterQname = packet + sizeof(struct dns_header) + strlen((const char *)buf)+strlen((const char *)qname)-1;

        struct dns_response_trailer *trailer = (struct dns_response_trailer *)afterQname;
        trailer->qclass = htons(QTYPE_A);
        trailer->type = htons(QCLASS_INET);


        int len = sizeof(struct dns_header) + strlen((const char *)buf) + strlen(qname) + sizeof(trailer->qclass) + sizeof(trailer->type);

        sendto(sockfd, packet, len+1, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
        
        unsigned int n, lenC;
        
         
        struct timeval timeout;      
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) > 0)
        {    
            perror("set timeout failed");
            exit(EXIT_FAILURE);
        }
        if((n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &lenC)) == -1){
            fprintf(stderr, "DST_FILEPATH did not reach / connection timeout \n");
            exit(0);
        }
    
        close(sockfd);
        break;
    }
    //data sending
    while(1){
        int sockfd; 
        struct sockaddr_in     servaddr; 
        char buffer[MAX_BUFFER_SIZE]; 
        // Creating socket file descriptor 
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
            fprintf(stderr, "socket creation failed \n");
            exit(0);
        } 
        
        memset(&servaddr, 0, sizeof(servaddr)); 
        
        // Filling server information 
        servaddr.sin_family = AF_INET; 
        servaddr.sin_port = htons(PORT); 
        if(ipShow == true){
            servaddr.sin_addr.s_addr = inet_addr(argv[ipNum]);
        }else{
            servaddr.sin_addr.s_addr = inet_addr(ipTmp);
        }
        unsigned char buf[101] = {0};

        int fredNum = fread(c, 1, BLOCK/2, fp);
        
        if (fredNum == 0){
            break;
        }
        
        base32_encode(c, fredNum, buf, BLOCK);

        int lenB = strlen((const char *)buf) + 1;
        char bufLen[100] = {0};
        bufLen[0] = lenB - 1;
        memcpy(bufLen + 1, buf, lenB - 1);
        memcpy(buf, bufLen, lenB);

        unsigned char packet[512] = {0};
        struct dns_header *header = (struct dns_header *)packet;
        header->id = htons(2222);
        header->rd = 1;
        header->qdcount = htons(1);
        
        unsigned char *afterHeader = packet + sizeof(struct dns_header);

        memcpy(afterHeader, buf, strlen((const char *)buf));

        unsigned char *afterData = packet + sizeof(struct dns_header) + strlen((const char *)buf);

        memcpy(afterData, qname, strlen(qname)+1);
     
        unsigned char *afterQname = packet + sizeof(struct dns_header) + strlen((const char *)buf)+strlen((const char *)qname)-1;

        struct dns_response_trailer *trailer = (struct dns_response_trailer *)afterQname;
        trailer->qclass = htons(QTYPE_A);
        trailer->type = htons(QCLASS_INET);


        int len = sizeof(struct dns_header) + strlen((const char *)buf) + strlen(qname) + sizeof(trailer->qclass) + sizeof(trailer->type);

        sendto(sockfd, packet, len+1, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
        
        unsigned int n, lenC;
         
        struct timeval timeout;      
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) > 0)
        {    
            perror("set timeout failed");
            exit(EXIT_FAILURE);
        }
        if((n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &lenC)) == -1){
            fprintf(stderr, "DATA did not reach / connection timeout \n");
            exit(0);
        }
    
        close(sockfd);
    }
    //sending ending packet to end comunication while will be done just once
    while(1){
        int sockfd; 
        struct sockaddr_in     servaddr; 
        char buffer[MAX_BUFFER_SIZE]; 
        // Creating socket file descriptor 
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
            fprintf(stderr, "socket creation failed \n");
            exit(0);
        } 
        
        memset(&servaddr, 0, sizeof(servaddr)); 
        
        // Filling server information 
        servaddr.sin_family = AF_INET; 
        servaddr.sin_port = htons(PORT); 
        if(ipShow == true){
            servaddr.sin_addr.s_addr = inet_addr(argv[ipNum]);
        }else{
            servaddr.sin_addr.s_addr = inet_addr(ipTmp);
        }
        unsigned char buf[101] = {0};

        char *c = "end";
        
        base32_encode((const unsigned char *)c, strlen("end"), buf, BLOCK);
        printf("\n%s\n", buf);

        int lenB = strlen((const char *)buf) + 1;
        char bufLen[100] = {0};
        bufLen[0] = lenB - 1;
        memcpy(bufLen + 1, buf, lenB - 1);
        memcpy(buf, bufLen, lenB);

        unsigned char packet[512] = {0};
        struct dns_header *header = (struct dns_header *)packet;
        header->id = htons(5252);
        header->rd = 1;
        header->qdcount = htons(1);
        
        unsigned char *afterHeader = packet + sizeof(struct dns_header);

        memcpy(afterHeader, buf, strlen((const char *)buf));

        unsigned char *afterData = packet + sizeof(struct dns_header) + strlen((const char *)buf);

        memcpy(afterData, qname, strlen(qname)+1);
     
        unsigned char *afterQname = packet + sizeof(struct dns_header) + strlen((const char *)buf)+strlen((const char *)qname)-1;

        struct dns_response_trailer *trailer = (struct dns_response_trailer *)afterQname;
        trailer->qclass = htons(QTYPE_A);
        trailer->type = htons(QCLASS_INET);


        int len = sizeof(struct dns_header) + strlen((const char *)buf) + strlen(qname) + sizeof(trailer->qclass) + sizeof(trailer->type);

        sendto(sockfd, packet, len+1, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
        
        unsigned int n, lenC;
         
        struct timeval timeout;      
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) > 0)
        {    
            perror("set timeout failed");
            exit(EXIT_FAILURE);
        }
        if((n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *) &servaddr, &lenC)) == -1){
            fprintf(stderr, "OFF signal did not reach / connection timeout \n");
            exit(0);
        }
    
        close(sockfd);
        break;
    }

    fclose(fp);
    return 0;
}
/*****end of main function*****/

/****functions****/

//file existencion check
bool file_exists(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    bool is_exist = false;
    if (fp != NULL)
    {
        is_exist = true;
        fclose(fp); // close the file
    }
    return is_exist;
}

//function for positioning of argumnets
void positioning (int argc, bool *ipShow, bool *srcPath, int *ipNum, int *baseNum, int *dstNum, int *srcNum){
    
    if(*ipShow){
        
        switch (*ipNum){
        case 2:
            if (*srcPath == true){
                *baseNum = 3;
                *dstNum = 4;
                *srcNum = 5;
            }else{
                *baseNum = 3;
                *dstNum = 4;
                *srcNum = 99;
            }
            break;
        case 3:
            if (*srcPath == true){
                *baseNum = 1;
                *dstNum = 2;
                *srcNum = 5;
            }else{ 
                *baseNum = 1;
                *dstNum = 4;
                *srcNum = 99;
            }
            break;
        case 4:
            if (*srcPath == true){
                *baseNum = 1;
                *dstNum = 2;
                *srcNum = 5;
            }else{
                *baseNum = 1;
                *dstNum = 2;
                *srcNum = 99;
            }
            break;
        case 5:
            if (*srcPath == true){
                *baseNum = 1;
                *dstNum = 2;
                *srcNum = 3;
            }
            break;
        default:  //well something went  wrong
            fprintf(stderr, "Internal error.\n");
           exit(0);  //error
        }
    }else{
        
        if (*srcPath == true){
            *baseNum = 1;
            *dstNum = 2;
            *srcNum = 3;
        }else {
            *baseNum = 1;
            *dstNum = 2;
            *srcNum = 99;       //not existing load from stdin
        }
    }
    
}

// checking arguments 
void argvs(int argc, char *argv[], bool *ipShow, bool *srcPath, int *ipNum){
    bool found = false;

    if (argc < 3 || argc > 6){
        fprintf(stderr, "Invalid number of arguments.\n");
        fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
        exit(0);  //error
    }

    for (int a = 1; a < argc; a++){
        if (strcmp(argv[a] , "-u") == 0){
            *ipShow = true;
            found = true;
            
            //check ip if it is correct
            isValidIp4(argv[a+1]);

            if (argc == 5){             //if argc == 5 there is not [SRC_FILEPATH] of 6 we do have it
                *srcPath = false;
            }else if(argc == 6){
                *srcPath = true;
            }else{
                fprintf(stderr, "Invalid input in arguments.\n");
                fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
                exit(0);  //error
            }
            *ipNum = a+1;
        }
    }

    if (found == false){
        *ipShow = false;
            
        if (argc == 3){         //if argc == 3 there is not [SRC_FILEPATH] of 4 we do have it
            *srcPath = false;
        }else if(argc == 4){
            *srcPath = true;
        }else{
            fprintf(stderr, "Invalid input in arguments.\n");
            fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
            exit(0);  //error
        }
    }
}

//validation of IPv4 
int isValidIp4 (char *str) {
    int segs = 0;   //segment counter
    int chCount = 0;  //internal segment counter
    int accum = 0;  //acumulator
    
    if (str == NULL){
        fprintf(stderr, "Invalid IPv4.\n");
        exit(0);
    }
    while (*str != '\0'){
        if (*str == '.'){
            if (chCount == 0){
                fprintf(stderr, "Invalid IPv4.\n");
                exit(0);
            }
            if (++segs == 4){
                fprintf(stderr, "Invalid IPv4.\n");
                exit(0);
            }
            chCount = accum = 0;        //reset loop
            str++;
            continue;
        }
        if ((*str < '0') || (*str > '9')){
            fprintf(stderr, "Invalid IPv4.\n");
            exit(0);
        }
        if ((accum = accum * 10 + *str - '0') > 255){
            fprintf(stderr, "Invalid IPv4.\n");
            exit(0);
        }
        chCount++;      //advance and continue 
        str++;
    }
    if (segs != 3){         //segment chceck
        fprintf(stderr, "Invalid IPv4.\n");
        exit(0);
    }
    if (chCount == 0){
        fprintf(stderr, "Invalid IPv4.\n");
        exit(0);
    }
    return 1;
}