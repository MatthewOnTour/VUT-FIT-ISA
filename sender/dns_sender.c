#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* function declarations */
int argvs(int argc, char *argv[] ,bool ipShow, bool srcPath, int ipNum);


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

if (argc < 3 || argc > 6){
    fprintf(stderr, "Invalid number of arguments.\n");
    fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
    return(1);  //error
}

for (int a = 0; argv[a] != NULL; a++){
    if (strcmp(argv[a] , "-u") == 0){
        ipShow == true;
        
        //check ip if it is correct TODO



        if (argc == 5){             //if 5 argc there is not [SRC_FILEPATH] of 6 we do have it
            srcPath = false;
        }else if(argc == 6){
            srcPath == true;
        }else{
            fprintf(stderr, "Invalid input in arguments.\n");
            fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
            return(1);  //error
        }
    }else{
        ipShow = false;
        
        if (argc == 3){         //if 3 argc there is not [SRC_FILEPATH] of 4 we do have it
            srcPath = false;
        }else if(argc == 4){
            srcPath == true;
        }else{
            fprintf(stderr, "Invalid input in arguments.\n");
            fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
            return(1);  //error
        }
    }
}



}
