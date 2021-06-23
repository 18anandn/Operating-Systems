#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h> 
#include <sys/wait.h> 

//Splits s at index and returns the later part
void split(char *s, char *s1, int index){
    int i = 0;
    while(s[i+index] != '\0'){
        s1[i] = s[i+index];
        i++;
    }
    s1[i] = '\0';
}

//This method is used for cd implementation. Removes " from str and returns str1.
//If there is space outside "" then the num is returned as -1 showing that 
//there are too many arguments for cd
void filter(char *str, char *str1, int *num){
    int i = 0;
    int j = 0;
    bool allow = false;
    while(str[i] != '\0'){
        if(str[i] == '\"'){
            i++;
            if(allow) allow = false;
            else allow = true;
        }
        else if(str[i] == ' '){
            if(allow){
                str1[j] = str[i];
                i++;
                j++;
                continue;
            }
            else{
                *num = -1;
                break;
            }
        }
        else{
            str1[j] = str[i];
            i++;
            j++;
        }
    }
    if(*num == 0){
        str1[j] = '\0';
        *num = 1;
    }
}

//Splits string str into substrings separated by " "
void arguments(char* str, char** argu) {
    for (int i = 0; i < 1000; i++) { 
        argu[i] = strsep(&str, " "); 
  
        if (argu[i] == NULL) 
            break; 
        if (strlen(argu[i]) == 0) 
            i--; 
    }
} 

int main(){
    int size = 4096;

    //Max size of path
    #ifdef PATH_MAX
        size = PATH_MAX;
    #endif

    //Home directory
    char home[size];
    getcwd(home, sizeof(home));

    //Current directory
    char dir[size];
    strcpy(dir, "~");
    char *cwd = dir;

    //Record for 5 recent commands
    char record[5][128];
    int stored = 0;
    int h = 0;

    while(1){
        printf("MTL458:%s$", cwd);
        char line[129];
        if(fgets(line, 131, stdin) == NULL) continue;
        if(strlen(line) == 1) continue;
		
		//Taking commands within character length 128
        if(strlen(line) < 129){
            line[strlen(line)-1] = '\0';
        }
        else if(strlen(line) == 129){
            line[128] = '\0';
        }
        else if(strlen(line) > 128){
            printf("Error: Command size over limit\n");
            while(getchar() != '\n');
            continue;
        }
        strcpy(record[h], line);
        if(stored < 5) stored++;
        h = (h+1)%5;
        char ch[1000];
        strcpy(ch, line);
        strtok(ch, " ");
        char arg[1000];
        split(line, arg, strlen(ch)+1);
        if(strcmp("cd", ch) == 0){
            
            if(strlen(arg) == 0){}
            
            else{
                if(strcmp(arg, "~") == 0){
                    chdir(home);
                    strcpy(cwd, "~");
                }
                else{
                    int y = 0;
                    int *num = &y;
                    char arg1[strlen(arg)];
                    filter(arg, arg1, num);
                    if(*num < 0) {
                        printf("-bash: cd: too many arguments\n");
                        continue;
                    }
                    if(chdir(arg1) < 0){
                        printf("-bash: cd: %s: No such file or directory\n", arg);
                    }
                    else{
                        getcwd(cwd, size);

                        //using ~ when in subdirectories of home directory
                        if(strstr(cwd, home) != NULL){
                            char dir[strlen(cwd)];
                            dir[0] = '~';
                            int i = 1;
                            while(cwd[i+strlen(home)-1] != '\0'){
                                dir[i] = cwd[i+strlen(home)-1];
                                i++;
                            }
                            dir[i] = '\0';
                            strcpy(cwd, dir);
                        }
                    }
                }
            }
        }
        else if(strcmp("history", ch) == 0){
            if(stored < 5){
                for(int i = 0; i < stored; i++){
                    printf("%d %s\n", i+1, record[i]);
                }
            }
            else{
                for(int i = 0; i < stored; i++){
                    printf("%d %s\n", i+1, record[(i+h)%5]);
                }
            }
        }
        else{
            char *argum[1000];
            arguments(line, argum);

            //execvp is called in a separate precess
            if(fork() == 0){
                if(execvp(ch, argum) < 0){
                    printf("Command \'%s\' not found\n", ch);
                };
            }
            else{
                wait(NULL);
            }
        }
    }
    return 0;
}