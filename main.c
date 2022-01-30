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
        if(strcmp(tokens->items[0],"echo")==0){                         //if echo is input print out second arguement
            printf("%s\n",tokens->items[1]);
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
    int pid = fork();
    FILE *file;
    char wholePath[strlen(getenv("PATH"))]; 
    strcpy(wholePath,getenv("PATH"));
    char *path = strtok(wholePath,":"); //splits the path by colon;
    char pathCommand[strlen(wholePath)];
    int found = -1;
    
    if (pid == 0){
        //in child
        while(path != NULL){
            strcpy(pathCommand,path);

            // adds a / and then the command
            strcat(pathCommand,"/");
            strcat(pathCommand,tokens->items[0]);
        
            // if file exists
            if( access(pathCommand, R_OK) == 0){
                execv(pathCommand,tokens->items);
                found = 0;
            }
            // continues splitting until there is no more string left to split
            path = strtok(NULL,":");
        }
        if (found != 0){
            printf("%s", "command not found");
        }
    }
    else{
        waitpid(pid,NULL,0);
    }
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