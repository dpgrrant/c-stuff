#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 


typedef struct {
int size;
char **items;
} tokenlist;

typedef struct {
    pid_t pid;
    char *cmd;
} job;

typedef struct {
    job* jobs;      // Stores all background processes
    int size;
} jobList;

pid_t waitpid(
    pid_t pid,
    int *stat_loc,
    int options
);

char *get_input(void);
tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
void pathSearch(tokenlist *tokens, int isBackgroundProc);
void doublePiping(tokenlist *tokens,char *path);
void singlePiping(tokenlist *tokens,char *path);


int main(){
   jobList jobs;
   jobs.size  = 0;

    while (1) {
        printf("\n%s@%s : %s >",getenv("USER"),getenv("MACHINE"),getenv("PWD"));                //prints prompt
        char *input = get_input();
        tokenlist *tokens = get_tokens(input);                                  //given input parser/tokenizer


        if(tokens->size==0){
            continue;
        }
        for (int i = 0; i < tokens->size; i++) {                                //printing tokens for debugging
            printf("token %d: (%s)\n", i, tokens->items[i]);
        }
        if(strcmp(tokens->items[0],"echo")==0){         //if echo is input print out second arguement
            for(int i = 1; i < tokens->size; i++){
                printf("%s ",tokens->items[i]);
            }
            
        }
        else if(strcmp(tokens->items[0],"cd")==0){                  //cd
            if(tokens->size==1){                                //if cd is only arg
                chdir(getenv("HOME"));
                char *cwd = getcwd(NULL, 0);
                setenv("PWD", cwd, 1);        // 1 means overwrite
                free(cwd);                   // IMPORTANT!
                continue;
            }else if(tokens->size==3){                                  //if more than 2 args given
                 printf("ERROR: More than one argument is present\n");
                 continue;
            }

            if(chdir(tokens->items[1])!=0){                             //try's chdir if therre is error print error           
                perror("ERROR");
            }
            char *cwd = getcwd(NULL, 0);                            //change PWD
            setenv("PWD", cwd, 1);  // 1 means overwrite
            free(cwd);                   // IMPORTANT!
        }  
        else{
            if(strcmp(tokens->items[tokens->size-1], "&")==0){
                
                
                
                // TODO: cut out the &



                pathSearch(tokens, 1);      // Run as background process    
            }
            else{
                pathSearch(tokens, 0);
            }
        }
        free(input);                                                            //given cleanup
        free_tokens(tokens);
    }
return 0;
}

void singlePiping(tokenlist *tokens,char *path){
    pid_t pid1;
    pid_t pid2;
    pid_t pid3;
    tokenlist *newTokens1 = new_tokenlist();
    tokenlist *newTokens2 = new_tokenlist();
    int isPiped = -1;
    int timesPiped = 0;
    int p_fds[2];
    char *path1 = malloc(strlen(getenv("PATH")+2 + strlen(tokens->items[0])));
    char *path2 = malloc(strlen(getenv("PATH")+2 + strlen(tokens->items[0])));
    int counter = 0;
    char *temp;
    
    
    for (int j = 0; j < tokens->size; j++){
            if (counter == 0 && strcmp(tokens->items[j],"|") != 0){
                add_token(newTokens1,tokens->items[j]);
            }
            else if (strcmp(tokens->items[j],"|") != 0){
                add_token(newTokens2,tokens->items[j]);
            }
            else if (strcmp(tokens->items[j],"|") == 0){
                counter++;
            }
    }

    //creates the path for the two given commands for piping
        strcpy(path1,path);
        strcpy(path2,path);
        strcat(path1,"/");
        strcat(path2,"/");
        strcat(path1,newTokens1->items[0]);
        strcat(path2,newTokens2->items[0]);

        pipe(p_fds);
        pid1 = fork();
        if (pid1 == 0){
            close(3); //closes unused end of pipe
            close(1); //closes standard output
            dup(4); //places pipe into output
            close(4); //closes pipe
            execv(path1,newTokens1->items);
            exit(1);
        }
        pid2 = fork();
        if(pid2 == 0){
            close(4); //closes unused end of pipe
            close(0); //closes standard input
            dup(3); //places pipe into input
            close(3); //closes pipe
            execv(path2,newTokens2->items);
            exit(1);
        }
           
            close(3);
            close(4);
            waitpid(pid1,NULL,0);
            waitpid(pid2,NULL,0);
        
    
    
    free(path1);
    free(path2);
    free_tokens(newTokens1);
    free_tokens(newTokens2);
}

void doublePiping(tokenlist *tokens,char *path){
    
    pid_t pid1;
    pid_t pid2;
    pid_t pid3;
    tokenlist *newTokens1 = new_tokenlist();
    tokenlist *newTokens2 = new_tokenlist();
    tokenlist *newTokens3 = new_tokenlist();
    int isPiped = -1;
    int timesPiped = 0;
    int p_fds[2];
    char *path1 = malloc(strlen(getenv("PATH")+2 + strlen(tokens->items[0])));
    char *path2 = malloc(strlen(getenv("PATH")+2 + strlen(tokens->items[0])));
    char *path3 = malloc(strlen(getenv("PATH")+2 + strlen(tokens->items[0])));
    int counter = 0;
    char *temp;
    
    
    for (int j = 0; j < tokens->size; j++){
            //newTokens1 = new_tokenlist();            
            //newTokens2 = new_tokenlist();
            //newTokens3 = new_tokenlist();
            if (counter == 0 && strcmp(tokens->items[j],"|") != 0){
                add_token(newTokens1,tokens->items[j]);
            }
            else if (counter == 1 && strcmp(tokens->items[j],"|") != 0){
                add_token(newTokens2,tokens->items[j]);
            }
            else if (counter == 2 && strcmp(tokens->items[j],"|") != 0){
                add_token(newTokens3,tokens->items[j]);
            }
            else if (strcmp(tokens->items[j],"|") == 0){
                counter++;
            }
    }

    //creates the path for the two given commands for piping
        strcpy(path1,path);
        strcpy(path2,path);
        strcpy(path3,path);
        strcat(path1,"/");
        strcat(path2,"/");
        strcat(path3,"/");
        strcat(path1,newTokens1->items[0]);
        strcat(path2,newTokens2->items[0]);
        strcat(path3,newTokens3->items[0]);

        pipe(p_fds);
        pid1 = fork();
        if (pid1 == 0){
            close(3);
            close(1); //closes standard output
            dup(4);
            close(4);
            execv(path1,newTokens1->items);
            exit(1);
        }
        pid2 = fork();
        if (pid2 == 0){
            close(1);
            dup(4);
            close(4);
            close(0);
            dup(3);
            close(3);
            execv(path2,newTokens2->items);
            exit(1);

        }
        pid3 = fork();
        if(pid3 == 0){
            close(4);
            close(0);
            dup(3);
            close(3);
            execv(path3,newTokens3->items);
            exit(1);
        }
           
        close(3);
        close(4);
        waitpid(pid1,NULL,0);
        //waitpid(pid2,NULL,0);
        waitpid(pid3,NULL,0);
        
    
    
    free(path1);
    free(path2);
    free(path3);
}

//includes path search, executing external commands, and IO redirection
void pathSearch(tokenlist *tokens, int isBackgroundProc){
    char *wholePath = malloc(strlen(getenv("PATH")+2+strlen(tokens->items[0]))); 
    char *pipePath = malloc(strlen(getenv("PATH")+2+strlen(tokens->items[0]))); 
    strcpy(wholePath,getenv("PATH"));
    char *path = strtok(wholePath,":"); //splits the path by colon;
    char *pathCommand = malloc(strlen(getenv("PATH")+2 + strlen(tokens->items[0])));
    char *pathPipeCommand = malloc(strlen(getenv("PATH")+2 + strlen(tokens->items[0])));
    int found = -1;


    
    char *fromFile;
    tokenlist *newTokens = new_tokenlist();
    int afterTok = -1;
    int writeFile = -1;
    int readFile = -1;
    char* fileName = "empty";
    char* inFileName = "empty";
    int fd;
    int fd2;
    int isPiped = 0;


    char *localCommand = tokens->items[0];
    char *rmLocal = malloc(strlen(localCommand)+1);
    int changed = -1;

    

    // checks if command is a local command
    for (int i = 0; i < strlen(localCommand); i++){
        if (localCommand[i] == '.'){
            strcpy(pathCommand,getenv("PWD"));
            strcpy(rmLocal,&localCommand[i+1]);
            changed = 0;
            
        }
        else if (localCommand[i] == '/'){
            strcpy(rmLocal,&localCommand[i+1]);
            changed = 0;
        }
    }
    // if there is a period or a slash changes command to proper command
    if (changed == 0){
        strcpy(tokens->items[0], rmLocal);
    }
    //free(rmLocal);


    // loops through all the tokens checking whether there is a <, >, or |
    for (int i = 0; i < tokens->size; i++){
        if (strcmp(tokens->items[i],"<") == 0){
            afterTok = 0;
            readFile = 0;
            inFileName = tokens->items[i+1]; 
        }
        else if (strcmp(tokens->items[i], ">") == 0){
            afterTok = 0;
            writeFile = 0;
            fileName = tokens->items[i+1];
        }
        else if (strcmp(tokens->items[i],"|") == 0){
            isPiped ++;
        }
        else{
            if (afterTok == 0){
                afterTok = -1;
            }
            else{
                add_token(newTokens,tokens->items[i]);
            }
        }
    }
    
    char *copyFile = malloc(strlen(fileName)+1);
    char *copyFile2 = malloc(strlen(inFileName)+1);
  
    strcpy(copyFile,fileName);
    strcpy(copyFile2,inFileName);

  
    if (isPiped == 0){
        tokens = newTokens;
        pid_t pid = fork();
        if (pid == 0){
            //in child

            // changed checks whether it is a local command
            if (changed != 0){
                while(path != NULL){
                    strcpy(pathCommand,path);

                    // adds a / and then the command
                    strcat(pathCommand,"/");
                    strcat(pathCommand,tokens->items[0]);
        
                    // if file exists
                    if( access(pathCommand, R_OK) == 0 || access(pathCommand, X_OK)==0){
                        if (writeFile == 0 || readFile == 0){
                            if (writeFile == 0){
                                close(1); //closes STDOUT
                                fd = open(copyFile,O_RDWR | O_CREAT | O_TRUNC,0666);
                                dup(3);
                                close(3);
                                if (readFile == 0){
                                    close(0);
                                    fd2 = open(copyFile2,O_RDONLY,0666);
                                    dup(3);
                                    close(3);
                                }
                                execv(pathCommand,tokens->items);
                                found = 0;
                            }
                            if (readFile == 0){
                                close(0);
                                fd2 = open(copyFile2,O_RDONLY,0666);
                                dup(3);
                                close(3);
                                execv(pathCommand,tokens->items);
                                found = 0;
                            }
                        }
                        else{
                            //printf("%s\n",pathCommand);
                            execv(pathCommand,tokens->items);
                            found = 0; 
                        }
                    }   
                    // continues splitting until there is no more string left to split
                    path = strtok(NULL,":");
                }
            }
            else {
                strcat(pathCommand,"/");
                strcat(pathCommand,tokens->items[0]);
                if( access(pathCommand, R_OK) == 0 || access(pathCommand, X_OK)==0){
                    if (writeFile == 0 || readFile == 0){
                        if (writeFile == 0){
                            close(1); //closes STDOUT
                            fd = open(copyFile,O_RDWR | O_CREAT | O_TRUNC,0666);
                            dup(3);
                            close(3);
                            if (readFile == 0){
                                close(0);
                                fd2 = open(copyFile2,O_RDONLY,0666);
                                dup(3);
                                close(3);
                            }
                            execv(pathCommand,tokens->items);
                            found = 0;
                        }
                        if (readFile == 0){
                            close(0); //closes STDIN
                            fd2 = open(copyFile2,O_RDONLY,0666);
                            dup(3);
                            close(3);
                            execv(pathCommand,tokens->items);
                            found = 0; 
                        } 
                    }
                    else{
                        execv(pathCommand,tokens->items);
                        found = 0; 
                    
                    }
                }    
            }
            if (found != 0){
                printf("command not found");
            }
        }
        else{
            if(isBackgroundProc == 1){
                // Is a background process, call addJob()
                //addJob(jobs,tokens,pid);
            }
            else{
                waitpid(pid,NULL,0);
            }
        }
    }
    else{
        strcpy(pipePath,getenv("PATH"));
        char *otherPath = strtok(pipePath,":"); //splits the path by colon;
       // printf("%s\n", getenv("PATH"));
        while(otherPath != NULL){
           // printf("%s\n", otherPath);
            strcpy(pathPipeCommand,otherPath);
            // adds a / and then the command
            strcat(pathPipeCommand,"/");
            strcat(pathPipeCommand,tokens->items[0]);

          //  printf("%s\n", pathPipeCommand);
            // if file exists
            if( access(pathPipeCommand, R_OK) == 0 || access(pathPipeCommand, X_OK) == 0){
                if (isPiped == 1){
                    singlePiping(tokens,otherPath);
                    break;
                }
                else if (isPiped == 2){
                    doublePiping(tokens,otherPath);
                    break;
                }
                
            
            }


            otherPath = strtok(NULL,":");
        }
    }
    free(copyFile);
    free(copyFile2);
    free(pathCommand);
    free(pathPipeCommand);
    free(wholePath);
    free(pipePath);
    free(rmLocal);
  
}

tokenlist *new_tokenlist(void){
    tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = (char **) malloc(sizeof(char *));
    tokens->items[0] = NULL; /* make NULL terminated */
    return tokens;
}
void add_token(tokenlist *tokens, char *item){
    int i = tokens->size;
    tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
    if (item[0] == '~'){                                                //tilda expansion if first character of input string 
        char *temp = malloc(strlen(getenv("HOME")) * sizeof(char));
        strcpy(temp, getenv("HOME"));
        strcat(temp,&item[1]);
        tokens->items[i]=temp;
    }
    else if (item[0] == '$'){                                           //expansion of enviroment variables
        item++;
        char *temp = malloc(strlen(getenv(item)) * sizeof(char));
        strcpy(temp, getenv(item));
        tokens->items[i] = temp;
    }
    else{
        tokens->items[i] = (char *) malloc(strlen(item) + 1);             //given add token
        tokens->items[i + 1] = NULL;
        strcpy(tokens->items[i], item);
    }
    tokens->size += 1;
}
char *get_input(void){
    char *buffer = NULL;
    int bufsize = 0;
    char line[5];
    while (fgets(line, 5, stdin) != NULL) {
        int addby = 0;
        char *newln = strchr(line, '\n');
        if (newln != NULL)
            addby = newln - line;
        else
            addby = 5 - 1;
        buffer = (char *) realloc(buffer, bufsize + addby);
        memcpy(&buffer[bufsize], line, addby);
        bufsize += addby;
        if (newln != NULL)
        break;
    }
    buffer = (char *) realloc(buffer, bufsize + 1);
    buffer[bufsize] = 0;
    return buffer;
}
tokenlist *get_tokens(char *input){
    char *buf = (char *) malloc(strlen(input) + 1);
    strcpy(buf, input);
    tokenlist *tokens = new_tokenlist();
    char *tok = strtok(buf, " ");
    while (tok != NULL) {
        add_token(tokens, tok);
        tok = strtok(NULL, " ");
    }
    free(buf);
    return tokens;
}
void free_tokens(tokenlist *tokens){
    for (int i = 0; i < tokens->size; i++)
    free(tokens->items[i]);
    free(tokens->items);
    free(tokens);
}

void addJob(jobList *jobs, char* command, pid_t pid){
    jobs->jobs = realloc(jobs->jobs, 1 * sizeof(job));
    jobs->size++;

    jobs->jobs[jobs->size-1].cmd = command;
    jobs->jobs[jobs->size-1].pid = pid;

    printf("[%d][%d]", jobs->size, pid);
}

void removeJob(jobList *jobs, int id){
    // This is a pain point, maybe implement a list or array? (compared to the current pointer "list")
}

void listJobs(jobList *jobs){
    for(int i = 0; i < jobs->size-1; i++){
        printf("[%d][%d][%s]\n", i, jobs->jobs[i].pid, jobs->jobs[i].cmd);
    }
}

void checkJobs(jobList *jobs){
    int statLoc;
    for(int i = 0; i < jobs->size-1; i++){
        if((jobs->jobs[i].pid = waitpid(jobs->jobs[i].pid, &statLoc, WNOHANG)) == -1){     // Job hit an error
            printf("There was an error with a background job");
        }
        else if(jobs->jobs[i].pid != 0){        // Job finished
            printf("SUCCESFUL[%d]+[%s]\n", i, jobs->jobs[i].cmd);
            removeJob(jobs, i);
        }
    }
}