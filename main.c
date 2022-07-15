#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
int fileexit=0;
// this is the zombie handler function
void func(int signum)
{
char text[]="child terminated\n";
    if(fileexit !=1){
     write(fileexit,text,strlen(text));
    }

    int wstat;
    pid_t	pid;

    pid = wait3 (&wstat, WNOHANG, (struct rusage *)NULL );
			if (pid == 0)
				return;
			else if (pid == -1)
				return;

    return ;
}

void setup_environment(){
  char wd[1000];
  printf("%s $",getcwd(wd,sizeof(wd)));
}

void shell(){

do{
 char param[2][20]={'\0'}; // here i store the command and parameter
 char input[30];
 int i=0;
 fgets(input, sizeof(input), stdin);

 if(strcmp(strtok(input,"\n"),"exit")==0) //this line for termination
 {
     break;
 }
 char * token = strtok(input, " ");
 while(token !=NULL)
 {
   strcpy(param[i],token);
   token=strtok(NULL ,"\n");
   i++;
 }
//this block fo evaluate expressions contain $ except echo
 if(strcmp(param[0],"echo") != 0  && checkSpecificChar(param[1])){
   char result[20]={'\0'};
    evaluete(param[1],result);
   strcpy(param[1],result);
}

 if(strcmp("cd",param[0])==0 ||strcmp("echo",param[0])==0 ||strcmp("export",param[0])==0)
 {
   execute_shell_builtin(param);
 }
 else {
   execute_command(param);
 }
    setup_environment();
 }while(1);
 exit(0);
}

//this is a helper function used in shell
int checkSpecificChar(char param[])
{
  for(int i=0;i<strlen(param);i++){
   if(param[i]=='$')
   {
     return 1;
   }
  }
  return 0;
}
/////////////////////////////buildin functions/////////////////////////////////////
char arrOfvar[100][20]; // array of export variables
char arrOfval[100][20]; //array of export value
int varIndex=0,valIndex=0; // i for variables j for value
//////////////////////////////////////////////////////////////////

void evaluete(char param[],char result[])
{

   char x[20];
   strcpy(x,strtok(param,"\n"));
   int indResult=0,varInd=0;
   char var[20];
   int m;
   for(m=0;m<strlen(x);m++)
   {
     if(x[m]=='$')
     {
       m++;
       for(;m<strlen(x) && x[m]!=' '&&x[m]!='\"';m++)
       {
         var[varInd]=x[m];
         varInd++;
       }
       var[varInd]='\0';
       int t=0;
      for(t=0;t<varIndex;t++)
      {
       if(strcmp(var,arrOfvar[t])==0)
       {
        int i;
        for(i=0;i<strlen(arrOfval[t]);i++){
          if(arrOfval[t][i]!='\"'){
           result[indResult]=arrOfval[t][i];
           indResult++;
           }
           }
       }
      }
     }
     if(m<strlen(x) ){
       result[indResult]=x[m];
       indResult++;
     }
   }
   result[indResult]='\0';
}





//////////////////////////////////executable or error///////////////////////

void execute_command(char param[2][20])
{

 int child_id=fork();

 if(child_id==0){
    char parametertCommand[3][10] ={'\0'}; // i made this array to store 2nd parameter in param which may be -l -a -h so it 3 rows
    int i=0;
    int status_code=0;
    if(strcmp(param[1],"")!=0){
    char * token = strtok(param[1], " ");
    while(token !=NULL)
    {
     strcpy(parametertCommand[i],token);
     token=strtok(NULL ," ");
     i++;
    }
    if(strcmp(parametertCommand[1],"")==0)
    {
      if(strcmp(strtok(parametertCommand[0],"\n"),"&")!=0){
         char* argument_list[] = {param[0], strtok(parametertCommand[0],"\n"), NULL,NULL};
         status_code = execvp(param[0], argument_list);
         }else{
           //printf("fff\n");
         char* argument_list[] = {param[0], NULL, NULL,NULL};
         status_code = execvp(param[0], argument_list);
         }

    }else if(strcmp(parametertCommand[2],"")==0)
    {
         char* argument_list[]  = {param[0], parametertCommand[0],parametertCommand[1],NULL};
         status_code = execvp(param[0], argument_list);

    }else {
        char* argument_list[] = {param[0], parametertCommand[0],parametertCommand[1],parametertCommand[2]};
        status_code = execvp(param[0], argument_list);
    }
     }
     else{
      char* argument_list[] = {param[0],NULL,NULL,NULL};
      status_code = execvp(param[0], argument_list);
     }
    if (status_code == -1) {
        printf("Process did not terminate correctly\n");
        exit(0);
    }
    exit (rand());

 }else if(child_id !=0 && strcmp(param[1],"")==0 ){
     int status;
     int endID = waitpid(child_id ,&status, 0);
           if (endID == -1) {            /* error calling waitpid       */
              perror("waitpid error");
              exit(EXIT_FAILURE);
           }
 }
}



// here i check if the input data in export already exit or not if not i store it in arrOfval and variable in arrOfvar

void checkAndPut(char param[20])
{
 char arr[2][20];
 char * token = strtok(param, "=");
 int i=0;
 while(token !=NULL)
 {
   strcpy(arr[i],token);
   token=strtok(NULL , "=");
   i++;
 }

 if(varIndex==0)
 {
  strcpy(arrOfvar[varIndex],arr[0]);
  strcpy(arrOfval[varIndex],arr[1]);
  varIndex++;
  valIndex++;
  }
  else{
  int i;
   for(i=0;i<varIndex;i++)
   {
     if(strcmp(arrOfvar[i],arr[0])==0){
      strcpy(arrOfval[i],arr[1]);
      break;
     }

   }
   if(i==varIndex)
   {
     strcpy(arrOfvar[varIndex],arr[0]);
     strcpy(arrOfval[varIndex],arr[1]);
     varIndex++;
     valIndex++;
  }
  }
}

void execute_shell_builtin(char param [2][20]){

  if(strcmp("cd",param[0])==0 ) {
   chdir(strtok(param[1],"\n"));
  }
  else if(strcmp("echo",param[0])==0 ) {

   char x[20];
   strcpy(x,strtok(param[1],"\n"));
   int flag=0,varInd=0;
   char var[20];
   int m;
   for(m=1;m<strlen(x)-1;m++)
   {
     if(x[m]=='$')
     {
       m++;
       for(;m<strlen(x) && x[m]!=' '&&x[m]!='\"';m++)
       {
         var[varInd]=x[m];
         varInd++;
       }
       var[varInd]='\0';
       int t=0;
      for(t=0;t<varIndex;t++)
      {
       if(strcmp(var,arrOfvar[t])==0)
       {
          printf("%s",arrOfval[t]);
       }
      }
     }
     if(m<strlen(x)-1){
       printf("%c",x[m]);
     }
   }
  printf("\n");

  }else{
     checkAndPut(strtok(param[1],"\n"));
  }
}

main ()
{

		signal(SIGCHLD, func);
		fileexit =open("file.text",O_WRONLY |O_CREAT |O_TRUNC,0644);
		setup_environment();
		shell();

}
