#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* function declarations */
int argvs(int argc, char *argv[] ,bool ipShow, bool srcPath, int ipNum);
int isValidIp4 (char *str);

//main function
int main(int argc, char *argv[]){

    FILE *fp;
    int c;

    //flags
    bool ipShow = false;  //flag for "-u"
    bool srcPath = false;   //falag for [SRC_FILEPATH]
    int ipNum = 0;

    argvs(argc, argv, ipShow, srcPath, ipNum);
}


//functions
int argvs(int argc, char *argv[], bool ipShow, bool srcPath, int ipNum){
    bool found = false;

    if (argc < 3 || argc > 6){
        fprintf(stderr, "Invalid number of arguments.\n");
        fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
        return(1);  //error
    }

    for (int a = 0; argv[a] != NULL; a++){
        if (strcmp(argv[a] , "-u") == 0){
            ipShow = true;
            found = true;
            //check ip if it is correct TODO
            isValidIp4(argv[a+1]);


            if (argc == 5){             //if argc == 5 there is not [SRC_FILEPATH] of 6 we do have it
                srcPath = false;
            }else if(argc == 6){
                srcPath = true;
            }else{
                fprintf(stderr, "Invalid input in arguments.\n");
                fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
                return(1);  //error
            }
            ipNum = a+1;
        }
    }

    if (found == false){
        ipShow = false;
            
        if (argc == 3){         //if argc == 3 there is not [SRC_FILEPATH] of 4 we do have it
            srcPath = false;
        }else if(argc == 4){
            srcPath = true;
        }else{
            fprintf(stderr, "Invalid input in arguments.\n");
            printf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
            return(1);  //error
        }
    }
}

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
