#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../io/file.h"

int main(){
    startLLFS();
    char args[] = "mkdir /Documents/CSC320";
    parse_arguments(args);
    char args2[] = "addfile /Documents/CSC320/Assignment3 myfile.txt";
    parse_arguments(args2);
    char args3[] = "read /Documents/CSC320/Assignment3";
    parse_arguments(args3);
    char args4[] = "rm /Documents/CSC320/Assignment3";
    parse_arguments(args4);
    char args5[] = "rm /Documents/CSC320";
    parse_arguments(args5);
    char args6[] = "rm /Documents";
    parse_arguments(args6);
    closeLLFS();
    return 1;
}