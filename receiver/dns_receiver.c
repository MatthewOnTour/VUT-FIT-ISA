#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>

            /****function declarations****/
int argvs(int argc, char *argv[]);
        /****end of function declarations****/


/*****main function*****/
int main(int argc, char *argv[]){

   argvs(argc, argv);

    return 0;
}
/*****end of main function*****/

/****functions****/
int argvs(int argc, char *argv[]){
    if(argc == 2){
        return;
    }else{
        fprintf(stderr, "Invalid number of arguments.\n");
        fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
        return(1);  //error
    }
}