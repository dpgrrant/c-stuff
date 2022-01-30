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
void pathSearch(tokenlist *tokens);


int main(){
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
            pathSearch(tokens);
        }
        free(input);                                                            //given cleanup
        free_tokens(tokens);
    }
return 0;
}

void pathSearch(tokenlist *tokens){
    pid_t pid = fork();
    char *wholePath = malloc(strlen(getenv("PATH")+2+strlen(tokens->items[0]))); 
    strcpy(wholePath,getenv("PATH"));
    char *path = strtok(wholePath,":"); //splits the path by colon;
    char *pathCommand = malloc(strlen(getenv("PATH")+2 + strlen(tokens->items[0])));
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
    //if (writeFile == 0){
    strcpy(copyFile,fileName);
   // }

    //if (readFile == 0){
    strcpy(copyFile2,inFileName);
   // }
    tokens = newTokens;
  
    
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
                if( access(pathCommand, R_OK) == 0 || access(pathCommand, X_OK)){
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
            if( access(pathCommand, R_OK) == 0 || access(pathCommand, X_OK)){
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
                        //fd2 = open(copyFile2,O_RDONLY,0666);
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
        waitpid(pid,NULL,0);
    }
    free(copyFile);
    free(copyFile2);
    free(pathCommand);
    free(wholePath);
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
void free_tokens(tokenlist *tokens)
{
for (int i = 0; i < tokens->size; i++)
free(tokens->items[i]);
free(tokens->items);
free(tokens);
}