#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 

int main(){

int pid = fork();
char *x[2];
x[0]="ls";
x[1]=NULL;


if(pid == 0)
{
printf("I am a child process\n");
execv(x[0],x);
}
else
{
printf("I am the parent process\n");
waitpid(pid,NULL,0);
}
printf("We did it! We did an ls!\n");
return 0;




}