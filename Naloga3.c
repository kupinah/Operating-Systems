#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> 

char shellName[128] = {'m', 'y', 's', 'h', '\0'};
char *a; 
char *tokensP[1024];
char narekovaji[128];
int statusCode;
int cTokens = 0;
int fd1;
int fd2;

void tokenize(char* p){
    a = malloc(1024*sizeof(char));
    bool narekovaji = false;

    int counter = 0;
    int tC = 0;
    cTokens = 0;
    int len = 0;
    bool reset = false;
    while(*p != '\n'){ 
        //printf("p: '%c', %d\n", *p, narekovaji);
        if(*p != ' '){
            if(*p == '"'){
                if(narekovaji)
                    narekovaji = false;
                else
                    narekovaji = true;
                p++;
                continue;
            }
            a[counter] = *p;
            len++;
        }
        else if(*p == ' ' && narekovaji){
            a[counter] = *p;
            p++;
            counter++;
            len++;
            continue;
        }
        else{
            a[counter] = '\0';
            // printf("%s %s\n", &a[0], &a[5]);
            // printf("%d %d\n", counter, len);
            tokensP[tC] = &a[counter-len];
            tC++;
            cTokens++;
            len = 0;
        }
        p++;
        counter++;
    }
    tokensP[tC] = &a[counter-len];
    //cTokens++;
}

int name(){
    statusCode = 0;
    if(cTokens == 2)
        strcpy(shellName, tokensP[1]);
    else
       printf("%s\n", shellName);
    fflush(stdout);
}

void print(){
    statusCode = 0;
    for(int i = 1; i <= cTokens; i++)
        printf("%s", tokensP[i]);
    fflush(stdout);
}

void echo(){
    statusCode = 0;
    fflush(stdout);
    for(int i = 1; i <= cTokens; i++)
        printf("%s ", tokensP[i]);
    printf("\n");
    fflush(stdout);
}

int exitStatus(){
    return statusCode;
}

void dirmake(){
    statusCode = 0;
    if (mkdir(tokensP[1], 0700) != 0){
        statusCode = errno;
        perror("dirmake");
    }
}

void dirchange(){
    statusCode = 0;
    if(cTokens == 0)
        chdir(getenv("HOME"));
    else {
        chdir(tokensP[1]);
        statusCode = errno;
    }
}

void dirremove(){
    statusCode = 0;
    struct stat s = {0};
    if(stat(tokensP[1], &s) == 0)
        rmdir(tokensP[1]);
    else
        statusCode = errno;
}

void dirlist(){
    struct dirent *str;
    DIR* d;
    if(cTokens == 0)
        d = opendir(".");
    else
        d = opendir(tokensP[1]);
    while((str = readdir(d)) != NULL)
        printf("%s  ", str->d_name);
    printf("\n");
    fflush(stdout);
}

void dirwhere(){
    char out[128];
    getcwd(out, 128);   
    printf("%s\n", out);
    fflush(stdout);
}

void status(){
    printf("%d\n", statusCode);
    fflush(stdout);
}

void pid(){
    printf("%d\n", getpid());
    fflush(stdout);
}

void ppid(){
    printf("%d\n", getppid());
    fflush(stdout);
}

void linkhard(){
    statusCode = 0;
    if(link(tokensP[1], tokensP[2]) == 0)
       statusCode = errno;
}

void linksoft(){
    statusCode = 0;
    if(symlink(tokensP[1], tokensP[2]) == 0)
       statusCode = errno;
    //perror("symlink");
}

void linkread(){
    char link[128];
    ssize_t len = readlink(tokensP[1], link, sizeof(link));
    link[len] = '\0';
    if(len != -1){
        printf("%s\n", link);
    }
    else
        statusCode = errno;
    //perror("linkread");
    fflush(stdout);
}

void linklist(){
    statusCode = 0;
    struct stat s;
    struct dirent *str;
    long cFile;
    DIR* d = opendir(".");

    if(stat(tokensP[1], &s) == 0)
        cFile = s.st_ino;
    else
        statusCode = errno;

    while((str = readdir(d)) != NULL){
        stat(str->d_name, &s);
        if(s.st_ino == cFile)
            printf("%s ", str->d_name);
    }
    printf("\n");
    //perror("linklist");
    fflush(stdout);
}

void myunlink(){
    if(unlink(tokensP[1]) != 0) 
        statusCode = errno;
}

void myrename(){
    statusCode = 0;
    if(rename(tokensP[1], tokensP[2])!=0);
        statusCode = errno;
}

void cpcat(){
    printf("%s %s\n", tokensP[1], tokensP[2]);
    int vhod;
    vhod = open(tokensP[1], O_RDONLY);
    
    int izhod;
    izhod = open(tokensP[2],  O_CREAT | O_TRUNC | O_WRONLY, 0644);
    char *c = malloc(4*sizeof(char));
    if(cTokens == 2)
        izhod = 1;
    
    if(vhod < 0 || izhod < 0)
        perror("Error: ");

    int r = read(vhod,c,1);

    if(r < 0)
        perror("Error: ");

    while(r != 0 && c[0] != '\n'){
        if(write(izhod,c,1) < 0)
            perror("Error: ");

        r = read(vhod,c,1);
        if(r < 0)
            perror("Error: ");
    }
    
    if(izhod == 1)
        write(izhod, "\n", 1);
}

int main(int argc, char** argv){
    if (isatty(0)) 
        printf("%s> ", shellName);

    while(true){
        char* input = NULL;
        size_t size;
        tokensP[1024];
        cTokens = 0;

        if(getline(&input, &size, stdin) == -1) 
          break;

        if(isspace(*input))
            continue;

        if(input[0] == '#') 
            continue;

        tokenize(input);
        
        bool ozadje = false;
        char newIn[25];
        char newOut[25];
        bool nOut = false;
        bool nIn = false;

        if(*tokensP[cTokens] == '&') {
            tokensP[cTokens] = '\0';
            cTokens--;
            ozadje = true;
            // if(cTokens >= 1 && *tokensP[cTokens-1] == '>')
            //     nOut = true;
            // if(cTokens >= 2 && *tokensP[cTokens-2] == '<')
            //     nIn = true;
        }

        char fOut[25];
        char fIn[25];

        if(*tokensP[cTokens] == '>'){
            strcpy(fOut, tokensP[cTokens]+1);
            printf("%s\n", fOut);
            cTokens--;
            nOut = true;
        }
        if(*tokensP[cTokens] == '<'){
            strcpy(fIn, tokensP[cTokens]+1);
            cTokens--;
            nIn = true;
        }
        
        if(nOut)
            fd1 = open(fOut,  O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if(nIn)
            fd2 = open(fIn,  O_RDONLY);
        printf("Test prije\n");
        if(nOut) dup2(fd1, 1);
        if(nIn) dup2(fd2, 0);
        printf("Test poslije\n");
        if(strcmp(tokensP[0], "name") == 0)
            name();
        else if(strcmp(tokensP[0], "print") == 0)
            print();
        else if(strcmp(tokensP[0], "echo") == 0)
            echo();
        else if(strcmp(tokensP[0], "exit") == 0)
            exit(atoi(tokensP[1]));
        else if(strcmp(tokensP[0], "dirmake") == 0)
            dirmake();
        else if(strcmp(tokensP[0], "dirchange") == 0)
            dirchange();
        else if(strcmp(tokensP[0], "dirwhere") == 0)
            dirwhere();
        else if(strcmp(tokensP[0], "dirremove") == 0)
            dirremove();
        else if(strcmp(tokensP[0], "dirlist") == 0)
            dirlist();
        else if(strcmp(tokensP[0], "rename") == 0)
            myrename();
        else if(strcmp(tokensP[0], "status") == 0)
            status();
        else if(strcmp(tokensP[0], "pid") == 0)
            pid();
        else if(strcmp(tokensP[0], "ppid") == 0)
            ppid();
        else if(strcmp(tokensP[0], "linkhard") == 0)
            linkhard();
        else if(strcmp(tokensP[0], "linksoft") == 0)
            linksoft();
        else if(strcmp(tokensP[0], "linklist") == 0)
            linklist();
        else if(strcmp(tokensP[0], "linkread") == 0)
            linkread();
        else if(strcmp(tokensP[0], "unlink") == 0)
            myunlink();
        else if(strcmp(tokensP[0], "cpcat") == 0)
            cpcat();
        else{
            tokensP[cTokens+1] = NULL;
            int zun;
            if((zun = fork()) == 0){
                return execvp(tokensP[0], tokensP);
            }
            else{
                if(!ozadje)
                    wait(&statusCode);
            }
        }
        nOut = false;
        nIn = false;
        close(fd1);
        close(fd2);
        if (isatty(0)) 
            printf("%s> ", shellName);
    }
}

