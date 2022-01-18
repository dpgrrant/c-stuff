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

char *get_input(void);
tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);


int main(){
while (1) {
    char *user = getenv("USER");
    char *pwd = getenv("PWD");
    char *machine = getenv("MACHINE");
    char *path = getenv("PATH");
    char *home = getenv("HOME");
    printf("\n%s@%s : %s >",user,machine,pwd);


    char *input = get_input();
    tokenlist *tokens = get_tokens(input);

    for (int i = 0; i < tokens->size; i++) {
        char*temp=tokens->items[i];
        if (temp[0] == '$') {
            temp++;
            tokens->items[i]=getenv(temp);
        }
        if (temp[0] == '~') {
            temp++;
            char*new=strcat(home,temp);
            tokens->items[i]=new;
        }

    }
    for (int i = 0; i < tokens->size; i++) {
        printf("token %d: (%s)\n", i, tokens->items[i]);
    }

    if(strcmp(tokens->items[0],"echo")==0){
        printf("%s\n",tokens->items[1]);
    }

    free(input);
    free_tokens(tokens);
}

return 0;
}

tokenlist *new_tokenlist(void)
{
tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
tokens->size = 0;
tokens->items = (char **) malloc(sizeof(char *));
tokens->items[0] = NULL; /* make NULL terminated */
return tokens;
}
void add_token(tokenlist *tokens, char *item)
{
int i = tokens->size;
tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
tokens->items[i] = (char *) malloc(strlen(item) + 1);
tokens->items[i + 1] = NULL;
strcpy(tokens->items[i], item);
tokens->size += 1;
}
char *get_input(void)
{
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
tokenlist *get_tokens(char *input)
{
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