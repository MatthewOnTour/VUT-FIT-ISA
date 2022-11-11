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

#define PORT     49152     //port used for DNS comunication 
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
            fp = fopen(argv[srcNum-1], "r");
            if (fp == NULL) {
                fprintf(stderr, "cannot open %s\n", argv[srcNum-1]);
                return 1;
            }
        }
    }else{
        char *filename = argv[srcNum-1];
        if (file_exists(filename)){
            //This is good no action needed
            //:D
        }else{
            fprintf(stderr, "File %s doesn't exist or path is invalid \n", filename);
            return 1;
        }
    }
    //end file opening 
    fp = fopen(argv[srcNum-1], "r");

    
    

    while(1){
        //printf(".");
        int sockfd; 
        struct sockaddr_in     servaddr; 
    
        // Creating socket file descriptor 
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
            fprintf(stderr, "socket creation failed \n");
            return 1;
        } 
    
        memset(&servaddr, 0, sizeof(servaddr)); 
        
        // Filling server information 
        servaddr.sin_family = AF_INET; 
        servaddr.sin_port = htons(PORT); 
        servaddr.sin_addr.s_addr = INADDR_ANY; 


        unsigned char buf[101] = {0};
        int fredNum ;

        fredNum = fread(c, 1, BLOCK, fp);

        if (fredNum == 0){
            break;
        }

        base32_encode(c, BLOCK, buf, fredNum);

        

        int lenB = strlen((const char *)buf);
        char bufLen[100] = {0};
        *bufLen = lenB;
        memcpy(bufLen + 1, buf, lenB);


        unsigned char packet[512] = {0};
        struct dns_header *header = (struct dns_header *)packet;
        header->id = htons(0002);
        header->rd = 1;
        header->qdcount = htons(1);

        

        unsigned char *afterHeader = packet + sizeof(struct dns_header);

        memcpy(afterHeader, buf, strlen((const char *)buf));

        unsigned char *afterData = packet + sizeof(struct dns_header) + strlen((const char *)buf);

        memcpy(afterData, argv[baseNum-1], strlen(argv[baseNum-1]));
        
        int len = sizeof(struct dns_header) + strlen((const char *)buf) + strlen(argv[baseNum - 1]);
        

        sendto(sockfd, packet, len, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
        

        //unsigned int n, len;
        //char buffer[MAX_BUFFER_SIZE];  
        /* n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE,  
                MSG_WAITALL, (struct sockaddr *) &servaddr, 
                &len); 
        buffer[n] = '\0'; 
        printf("Server : %s\n", buffer); */
    
        close(sockfd);
    }
    


    fclose(fp);
    return 0;
}
/*****end of main function*****/

/****functions****/

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

void positioning (int argc, bool *ipShow, bool *srcPath, int *ipNum, int *baseNum, int *dstNum, int *srcNum){
    if(*ipShow){
        switch (*ipNum){
        case 3:
            if (*srcPath == true){
                *baseNum = 4;
                *dstNum = 5;
                *srcNum = 6;
            }else{
                *baseNum = 4;
                *dstNum = 5;
                *srcNum = 99;
            }
            break;
        case 4:
            if (*srcPath == true){
                *baseNum = 2;
                *dstNum = 5;
                *srcNum = 6;
            }else{ 
                *baseNum = 2;
                *dstNum = 5;
                *srcNum = 99;
            }
            break;
        case 5:
            if (*srcPath == true){
                *baseNum = 2;
                *dstNum = 3;
                *srcNum = 6;
            }else{
                *baseNum = 2;
                *dstNum = 3;
                *srcNum = 99;
            }
            break;
        case 6:
            if (*srcPath == true){
                *baseNum = 2;
                *dstNum = 3;
                *srcNum = 4;
            }
            break;
        default:  //well something went  wrong
            fprintf(stderr, "Internal error.\n");
            return;  //error
        }
    }else{
        if (*srcPath == true){
            *baseNum = 2;
            *dstNum = 3;
            *srcNum = 4;
        }else {
            *baseNum = 2;
            *dstNum = 3;
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
        return;  //error
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
                return;  //error
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
            return;  //error
        }
    }
}

//validation of IPv4 
int isValidIp4 (char *str) {
    int segs = 0;   //segment counter
    int chCount = 0;  //internal segment counter
    int accum = 0;  //acumulator

    if (str == NULL){
        return 0;
    }
    while (*str != '\0'){
        if (*str == '.'){
            if (chCount == 0){
                return 0;
            }
            if (++segs == 4){
                return 0;
            }
            chCount = accum = 0;        //reset loop
            str++;
            continue;
        }
        if ((*str < '0') || (*str > '9')){
            return 0;
        }
        if ((accum = accum * 10 + *str - '0') > 255){
            return 0;
        }
        chCount++;      //advance and continue 
        str++;
    }
    if (segs != 3){         //segment chceck
        return 0;
    }
    if (chCount == 0){
        return 0;
    }
    return 1;
}
