#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>

            /****function declarations****/
int argvs(int argc, char *argv[] ,bool *ipShow, bool *srcPath, int *ipNum);
int isValidIp4 (char *str);
int positioning (bool *ipShow, bool *srcPath, int *ipNum, int *baseNum, int *dstNum, int *srcNum);
        /****end of function declarations****/

/*****main function*****/
int main(int argc, char *argv[]){
    FILE *fp;
    int c;
        /****flags and intgrs****/
    bool ipShow = false;  //flag for "-u"
    bool srcPath = false;   //flag for [SRC_FILEPATH]
    int ipNum = 0;          // position of "-u", position of ip is ipNum + 1 
    int baseNum = 0;        //position of {BASE_HOST}
    int dstNum = 0;         //position of {DST_FILEPATH}
    int srcNum = 0;         //position of [SRC_FILEPATH]
        /****end of flags and intgrs****/

    argvs(argc, argv, &ipShow, &srcPath, &ipNum);
    positioning(&ipShow, &srcPath, &ipNum, &baseNum, &dstNum, &srcNum);
    
    
    /**file opening**/ //TODO
    
    fp = fopen(argv[srcNum], "r");
    if (fp == NULL) {
        fprintf(stderr, "cannot open %s\n", argv[srcNum]);
        return 1;
    }else {
        fp = stdin; /* read from standard input if no argument */
    }
    while(1) {
      c = fgetc(fp);
      if( feof(fp) ) { 
         break ;
      }
      printf("%c", c);   //TODO   idk ci to je dobre 
   }
    



    //make sending and everything else :( TODO


    fclose(fp);
    return 0;
}
/*****end of main function*****/

/****functions****/

int positioning (bool *ipShow, bool *srcPath, int *ipNum, int *baseNum, int *dstNum, int *srcNum){
    if(ipShow){
        switch (*ipNum){
        case 3:
            *baseNum = 4;
            *dstNum = 5;
            *srcNum = 6;
            break;
        case 4:
            *baseNum = 2;
            *dstNum = 5;
            *srcNum = 6;
            break;
        case 5:
            *baseNum = 2;
            *dstNum = 3;
            *srcNum = 6;
            break;
        case 6:
            *baseNum = 2;
            *dstNum = 3;
            *srcNum = 4;
            break;
        default:
            fprintf(stderr, "Internal error.\n");
            return(1);  //error
        }
    }else{
        *baseNum = 2;
        *dstNum = 3;
        *srcNum = 4;
    }
}

int argvs(int argc, char *argv[], bool *ipShow, bool *srcPath, int *ipNum){
    bool found = false;

    if (argc < 3 || argc > 6){
        fprintf(stderr, "Invalid number of arguments.\n");
        fprintf(stderr, "Usage: dns_sender [-u UPSTREAM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH].\n");
        return(1);  //error
    }

    for (int a = 1; a < 6; a++){
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
                return(1);  //error
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
