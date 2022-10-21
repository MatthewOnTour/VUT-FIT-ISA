#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>



//main function
int main(int argc, char *argv[]){

FILE *fp;
int c;

if (argc < 3 || argc > 6){
    fprintf(stderr, "Invalid number of arguments.\n");
    return(1);  //error
}

if (argc == 3){
    // kontrola argv ci su spravne
}

if (argc == 4){
    if (argv[1] == "-u"){ 
        fprintf(stderr, "Invalid arguments.\n");
        return(1);  //error
    }else if(){
        // check last argv and load if needed
    }
}



    
}
