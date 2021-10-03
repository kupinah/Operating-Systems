#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/time.h>

typedef struct _ps{
    char* pid;
    char* ppid;
    char* status;
    char* name;
    char* dat;
    char* nit;
} ps;

void akcijaSys(char path[]){
    char tmp[] = "/";
    char file[] = "version";
    strcat(path, tmp);
    strcat(path, file);
    char* buffer = malloc(1024*sizeof(char));
    char* bufferVersion = malloc(28*sizeof(char));
    char* bufferGCC = malloc(6*sizeof(char));

    FILE* f = fopen(path, "r");
    
    fread(buffer, strlen(buffer)+1, 1024, f);

    printf("Linux: %.*s\n", 27, buffer+14);
    printf("gcc: %.*s\n", 5, buffer+88);
    fclose(f);
    free(buffer);
    free(bufferVersion);
    free(bufferGCC);
}

int compare(const void *s1, const void *s2){
    const ps *p1 = s1;
    const ps *p2 = s2;

    return (atoi(p1->pid) < atoi(p2->pid)) ? -1 : 1;
}

void psCommon(char path[], char predniki[], int akcija){
    char* file = malloc(128*sizeof(char));
    char* fileName = malloc(128*sizeof(char));
    char* fileDat = malloc(128*sizeof(char));

    memcpy(file, path, strlen(path));
    char* buffer = malloc(128*sizeof(char));
    
    DIR* d = opendir(path);
    struct dirent *dir;
    char* tmp = malloc(20*sizeof(char));
    char* ppid = malloc(20*sizeof(char));
    char* status = malloc(20*sizeof(char));
    char* threads = malloc(15*sizeof(char));
    char* nDat = malloc(15*sizeof(char));

    ps array[256];
    
    int counter = 0;
    int cThreads = 0;
    int cDat = 0;

    if(akcija == 1) 
        printf("%5s %5s %6s %s\n", "PID", "PPID", "STANJE", "IME");
    else
        printf("%5s %5s %6s %6s %6s %s\n", "PID", "PPID", "STANJE", "#NITI", "#DAT.", "IME");

    while((dir = readdir(d))){
        if(strtol(dir->d_name, &tmp, 10) != 0){
            strcat(file, "/");
            strcat(file, dir->d_name);
            strcat(fileName, file);
            strcat(fileDat, file);

            strcat(file, "/stat");
            strcat(fileName, "/comm");
            strcat(fileDat, "/fd");
            
            FILE* f = fopen(file, "r");
            FILE* fName = fopen(fileName, "r");
            DIR* fDir = opendir(fileDat);

            fscanf(f, "%s %s %s %s", buffer, buffer, status, ppid);
            fscanf(fName, "%s", fileName);

            char *tmpName = malloc(25*sizeof(char));
            char *tmpStatus = malloc(25*sizeof(char));
            char *tmpPid = malloc(25*sizeof(char));
            char *tmpPpid  = malloc(25*sizeof(char));

            memcpy(tmpName, fileName, strlen(fileName)+1);
            array[counter].name = tmpName;

            memcpy(tmpStatus, status, strlen(status)+1);
            array[counter].status = tmpStatus;

            memcpy(tmpPid, dir->d_name, strlen(dir->d_name)+1);
            array[counter].pid = tmpPid;

            memcpy(tmpPpid, ppid, strlen(ppid)+1);
            array[counter].ppid = tmpPpid;

            if(akcija == 2){
                char *tmpDat = malloc(25*sizeof(char));
                char *tmpThreads = malloc(25*sizeof(char));
                cThreads = 0;
                cDat = 0;
                fseek(f, 0, SEEK_SET);
                while(cThreads < 20){
                    fscanf(f, "%s", threads);
                    cThreads++;
                }
                while(readdir(fDir))
                    cDat++;
                sprintf(nDat, "%d", cDat-2);
                DIR* fDir = opendir(fileDat);
                struct dirent *files;

                memcpy(tmpThreads, threads, strlen(threads)+1);
                array[counter].nit = tmpThreads;

                memcpy(tmpDat, nDat, strlen(nDat)+1);
                array[counter].dat = tmpDat;

                closedir(fDir);
            }

            counter++;

            //empty file and fileName arrays
            memset(file, 0, strlen(file));
            memset(fileName, 0, strlen(fileName));
            memset(fileDat, 0, strlen(fileDat));
            memcpy(file, path, strlen(path));

            fclose(f);
            fclose(fName); 
            closedir(fDir);
        }   
    }

    qsort(array, counter, sizeof(ps), compare);

    if(predniki != NULL){
        ps *novi = calloc(200, sizeof(ps));
        for(int i = 0; i < 200; i++)
            novi[i].pid = "-1";

        char *prviPid = malloc(25*sizeof(char));
        char *prviName  = malloc(25*sizeof(char));
        char *prviPpid  = malloc(25*sizeof(char));
        char *prviStatus = malloc(25*sizeof(char));
        char *prviDat = malloc(25*sizeof(char));
        char *prviNit = malloc(25*sizeof(char));
        
        memcpy(prviPid, predniki, strlen(predniki)+1);
        novi[0].pid = prviPid;
        
        int ix = 1;
        int first = 0;
        while(strcmp(prviPid, array[first].pid) != 0) first++;
        novi[0].name = array[first].name;
        novi[0].status = array[first].status;
        novi[0].ppid = array[first].ppid;
        if(akcija == 2){
            novi[0].dat = array[first].dat;
            novi[0].nit = array[first].nit;
        }

        for(int i = 0; i < counter; i++){
            for(int j = 0; j < counter; j++){
                char *tmpName = malloc(25*sizeof(char));
                char *tmpStatus = malloc(25*sizeof(char));
                char *tmpPid = malloc(25*sizeof(char));
                char *tmpPpid  = malloc(25*sizeof(char));
                char *tmpNit = malloc(25*sizeof(char));
                char *tmpDat  = malloc(25*sizeof(char));
                if(atoi(novi[i].pid) == atoi(array[j].ppid)){
                    strcpy(tmpPid, array[j].pid);
                    novi[ix].pid = tmpPid;
                    strcpy(tmpName, array[j].name);
                    novi[ix].name = tmpName;
                    strcpy(tmpPpid, array[j].ppid);
                    novi[ix].ppid = tmpPpid;
                    strcpy(tmpStatus, array[j].status);
                    novi[ix].status = tmpStatus;
                    if(akcija == 2){
                        strcpy(tmpNit, array[j].nit);
                        novi[ix].nit = tmpNit;
                        strcpy(tmpDat, array[j].dat);
                        novi[ix].dat = tmpDat;
                    }
                    ix++;
                }
            }
        }
        
        qsort(novi, ix, sizeof(ps), compare);

        for(int i = 0; i < ix; i++){
           if(akcija == 1){
               printf("%5s %5s %6s %s\n", novi[i].pid, novi[i].ppid, novi[i].status, novi[i].name);
               free(novi[i].name);
               free(novi[i].pid);
               free(novi[i].status);
           }
           else{
               printf("%5s %5s %6s %6s %6s %s\n", novi[i].pid, novi[i].ppid, novi[i].status, novi[i].nit, novi[i].dat, novi[i].name);
               free(novi[i].name);
               free(novi[i].pid);
               free(novi[i].status);
               free(novi[i].dat);
               free(novi[i].nit);
           }
        }
    }
    else{
        for(int i = 0; i < counter; i++){
           if(akcija == 1){
               printf("%5s %5s %6s %s\n", array[i].pid, array[i].ppid, array[i].status, array[i].name);
               free(array[i].name);
               free(array[i].pid);
               free(array[i].status);
           }
           else{
               printf("%5s %5s %6s %6s %6s %s\n", array[i].pid, array[i].ppid, array[i].status, array[i].nit, array[i].dat, array[i].name);
               free(array[i].name);
               free(array[i].pid);
               free(array[i].status);
               free(array[i].dat);
               free(array[i].nit);
           }
        }
    }
    closedir(d);
}

void forrec(int nums[], int kom[], int ix, int mainProc, char out[]){
    int kids = nums[ix];
    for(int i = 0; i < kids; i++){
        int k = fork();
        if(k == 0){
            //forrec(nums, kom, ix+kom[i], mainProc, out);
            sleep(2);
            exit(1);
        }
    }
    if(getpid() == mainProc){
        int f = fork();
        if(f == 0){
            sleep(1);
            execlp("pstree","pstree","-c","-A",out,NULL);
        }
        else if(f > 0)
            sleep(2);
    }
}
int main(int argc, char** argv){
    char* akcija = malloc(128*sizeof(char));
    memcpy(akcija, argv[1], strlen(argv[1]));
    if(strcmp(akcija, "sys") == 0){
        akcijaSys(argv[2]);
    }
    else if(strcmp(akcija, "sysext") == 0){
        char* file = malloc(30*sizeof(char));
        strcat(file, argv[2]);
        akcijaSys(file);
        char fileSwaps[50];
        char fileModules[50];

        strcpy(fileSwaps, argv[2]);
        strcpy(fileModules, argv[2]);

        size_t sBuff = 512;
        ssize_t nLines;

        strcat(fileSwaps, "/swaps");
        strcat(fileModules, "/modules");
        char* bufferSwaps = malloc(1024*sizeof(char));
        char* tmp = malloc(1024*sizeof(char));
        FILE* fS = fopen(fileSwaps, "r");

        getline(&tmp, &sBuff, fS);
        getline(&bufferSwaps, &sBuff, fS);
        int modCounter = 0;
        FILE* fM = fopen(fileModules, "r");
        while((nLines = getline(&tmp, &sBuff, fM) != -1))
            modCounter++;

        printf("Swap: %.*s\n", 10, bufferSwaps);
        printf("Modules: %d\n", modCounter);

        fclose(fS);
        fclose(fM);
        free(bufferSwaps);
        free(tmp);
        return modCounter;
    }
    else if(strcmp(akcija, "me") == 0){
        struct utsname podatki;
        uname(&podatki);
        
        char* cwd = malloc(1024*sizeof(char));
        size_t s = 1024;

        int which = PRIO_PROCESS;
        int priority;

        struct timezone *tzone = malloc(sizeof(struct timezone));
        struct timeval *tmp = malloc(sizeof(struct timeval));
        gettimeofday(tmp, tzone);

        struct rlimit cpuLimit;
        getrlimit(RLIMIT_CPU, &cpuLimit);

        pid_t pid = getpid();
        uid_t uid = getuid();
        uid_t euid = geteuid();
        gid_t gid = getgid();
        gid_t egid = getegid();

        char* proc = "/proc/";
        int accessS = access(proc, R_OK);
        char* accessString = malloc(4*sizeof(char));
        if(accessS == 0)
            accessString = "yes";
        else
            accessString = "no";
        
        getcwd(cwd, s);
        priority = getpriority(which, pid);

        printf("Uid: %d\n", uid);
        printf("EUid: %d\n", euid);
        printf("Gid: %d\n", gid);
        printf("EGid: %d\n", egid);
        printf("Cwd: %s\n", cwd);
        printf("Priority: %d\n", priority);
        printf("Process proc path: /proc/%d/\n", pid);
        printf("Process proc access: %s\n", accessString);
        printf("OS name: %s\n", podatki.sysname);
        printf("OS release: %s\n", podatki.release);
        printf("OS version: %s\n", podatki.version);
        printf("Machine: %s\n", podatki.machine);
        printf("Node name: %s\n", podatki.nodename);
        printf("Timezone: %d\n", tzone->tz_dsttime);
        printf("CPU limit: %ld\n", cpuLimit.rlim_cur);
    }
    else if(strcmp(akcija, "pids") == 0){
        DIR* d = opendir(argv[2]);
        struct dirent *dir;
        char* name = malloc(256*sizeof(char));
        char temp[256];
        char files[128][128];
        int counter = 0;
        while((dir = readdir(d))){
           if(strtol(dir->d_name, &name, 10) != 0){
               strcpy(files[counter], dir->d_name);
               counter++;
           }
        }
        for (int i = 0; i < counter-1; i++){
            for (int j = i + 1; j < counter; j++){
                if (atoi(files[i]) > atoi(files[j])){
                    strcpy(temp, files[i]);
                    strcpy(files[i], files[j]);
                    strcpy(files[j], temp);
                }
            }
        }
        for(int i = 0; i < counter; i++)
            printf("%s\n", files[i]);
        closedir(d);
    }
    else if(strcmp(akcija, "names") == 0){
        char* file = malloc(256*sizeof(char));
        memcpy(file, argv[2], strlen(argv[2]));
        
        DIR* d = opendir(argv[2]);
        struct dirent *dir;

        char* buffer = malloc(256*sizeof(char));

        char* name = malloc(256*sizeof(char));
        char strArray[256][256], strNames[256][256], strFin[256][256], temp[256];

        int i = 0;
        while((dir = readdir(d))){
           if(strtol(dir->d_name, &name, 10) != 0){
                strcat(file, "/");
                strcat(file, dir->d_name);
                strcat(file, "/comm");

                FILE* f = fopen(file, "r");
                fscanf(f, "%s", buffer);
                strcat(dir->d_name, " ");
                strcat(dir->d_name, buffer);

                strcpy(strArray[i], buffer);
                strcpy(strNames[i], dir->d_name);
                i++;

                //empty the file array
                memset(file, 0, strlen(file));
                memcpy(file, argv[2], strlen(argv[2]));
                fclose(f);
           }
        }
        
        //sort - first by name, then by PID... Yes, I know it is really ugly
        for (int j = 0; j < i-1; j++){
            for (int k = j + 1; k < i; k++){
                if (strcasecmp(strArray[j], strArray[k]) > 0){
                    strcpy(temp, strNames[j]);
                    strcpy(strNames[j], strNames[k]);
                    strcpy(strNames[k], temp);
                    strcpy(temp, strArray[j]);
                    strcpy(strArray[j], strArray[k]);
                    strcpy(strArray[k], temp);
                }
                else if (strcasecmp(strArray[j], strArray[k]) == 0 && atoi(strNames[j]) > atoi(strNames[k])) {
                    strcpy(temp, strNames[j]);
                    strcpy(strNames[j], strNames[k]);
                    strcpy(strNames[k], temp);
                    strcpy(temp, strArray[j]);
                    strcpy(strArray[j], strArray[k]);
                    strcpy(strArray[k], temp);
                }
            }
        }

        for (int j = 0; j < i; j++)
            printf("%s\n",strNames[j]);

        closedir(d);
    }
    else if(strcmp(akcija, "ps") == 0){
        psCommon(argv[2], argv[3], 1);
    }
    else if(strcmp(akcija, "psext") == 0){
        psCommon(argv[2], argv[3], 2);
    }
    else if(strcmp(akcija, "forktree") == 0){
        int mainProc = getpid();
        char input[128];
        char out[128];
        sprintf(out,"%d", mainProc);
        fgets(input, 128, stdin);
        input[strlen(input)-1] = '\0';
        int nums[128];
        int counter = 0;
        for(int i = 0; i < strlen(input); i+=2){
            nums[counter] = input[i]-48;
            counter++;
        }
        int kom[128];
        for(int i = 0; i < strlen(input)-2; i++){
            int c = 0;
            for(int j = 0; j < i; j++)
                c += nums[j];
            kom[i] = c;
        }
        
        for(int i = 0; i < strlen(input)-2; i++){
           kom[i] = kom[i];
        }
        
        forrec(nums, kom, 0, mainProc, out);
      
    }
    return 0;
}

