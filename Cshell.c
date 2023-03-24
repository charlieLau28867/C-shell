/*
 name: Lau cheuk Ning
 Development platform: Mac Os Ubuntu docker image
 Remark:
 1.Process creation and execution- foreground :All features done
 
 2.Pipe - !can handle only one pipe ! for example cat makefile | wc -l
          cannot handle more then two pipe input, can implement timeX command
 
 3.use of signals - Correct behavior of the 3230shell process and the
                    foreground child processes in handling the SIGINT signal
                  - All child processes can wait for the SIGUSR1 signal before executing the target commands.
 
 4.Built-in command: timeX: - Correct use of the timeX command (both pipe and                          normal input of command)
                           - can report improper usage
                           - For each terminated foreground child process, the system prints out the process statistics of the process in the correct format
 
 5.Built-in command: exit: - can Correct use of the exit command;
                           - can report improper usage
 

 */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/resource.h>

//sigint handler in child process
void sigint_handler2(int signum){
    printf("Interrupt\n");
}

//sigusr1 handler
void SIGUSR1_handler(int signum){
    //printf("Child has recieved the signal\n");
}

// Function where the system command is running
void execArgs(char** parsed)
{
    struct rusage used;
    signal(SIGINT, sigint_handler2);
    //checking the timeX command
    if (strcmp(parsed[0], "timeX") != 0){
        //fork the child
        pid_t pid = fork();
        
        if (pid == -1) {
            printf("forking child failded");
            return;
        } else if (pid == 0) {
            //handling the SIGUSR1 process
            signal(SIGUSR1, SIGUSR1_handler);
            sleep(3);
            if (execvp(parsed[0], parsed) < 0) {
                printf("\n3230shell: '%s': No such file or directory \n", parsed[0]);
            }
            exit(0);
        } else {
            //sleep and wait for the child process to exec then, send the SIGUSR1 signal to the child porcess
            sleep(1);
            kill(pid, SIGUSR1);
            // waiting for child to terminate
            int status;
            waitpid(pid, &status, 0);
            if(WTERMSIG(status) != 9 && WTERMSIG(status) != 0&& WTERMSIG(status) != 2 ){
                printf("Terminated\n");
            }
            if(WTERMSIG(status) == 9){
                printf("killed\n");
            }
            return;
        }
    }
    //for doing the timeX command
    else{
        int i;
        
        for (i = 0; i < 30; i++){
            if (parsed[i] == NULL){
                break;
            }
            else{
                parsed[i] = parsed [i + 1];
            }
        }
        //fork the child
        pid_t pid = fork();
        signal(SIGINT, sigint_handler2);
        if (pid == -1) {
            printf("\nFailed forking child..");
            return;
        } else if (pid == 0) {
            signal(SIGUSR1, SIGUSR1_handler);
            sleep(3);
            if (execvp(parsed[0], parsed) < 0) {
                printf("\n3230shell: '%s': No such file or directory \n", parsed[0]);
            }
            exit(0);
        } else {
            sleep(1);
            kill(pid, SIGUSR1);
            // waiting for child to terminate
            pid_t child_pid;
            int status;
            waitpid(pid, &status, 0);
            //checking the terminated signal for the child
            if(WTERMSIG(status) != 9 && WTERMSIG(status) != 0&& WTERMSIG(status) != 2 ){
            }
            if(WTERMSIG(status) == 9){
                printf("killed\n");
            }
            
            getrusage(RUSAGE_CHILDREN, &used);
            printf("(PID)%d  (CMD)%s    ",(int)pid,parsed[0]);
            printf("%s%.3f s","(user)", (used.ru_utime.tv_sec + used.ru_utime.tv_usec /1000000.0));
            printf("%s%.3f s\n", "  (sys)", (used.ru_stime.tv_sec + used.ru_stime.tv_usec /1000000.0));
            return;
        }
    }
}

// Function for piped commands to run
void execArgsPiped(char*** parsedpipe, int pipeNum)
{
    
    int i;
    int count = pipeNum;
    struct rusage used1;
    struct rusage used2;
    // 0 = read end, 1 = write end

    int pfd1[2], pfd2[2], pfd3[2], pfd4[2];
    int all_pfd[4][2] = { pfd1[2], pfd2[2], pfd3[2], pfd4[2]};

    pid_t p1, p2;
    
    if (pipe(pfd1) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    //create pipe
    for (i=0; i < pipeNum ;i++){
        if (pipe(all_pfd[i]) < 0)
            printf("Pipe could not be initialized\n");
    }
    
    //checking for timeX command
    if (strcmp(parsedpipe[0][0], "timeX") != 0){
        p1 = fork(); //child starts here
        if (p1 < 0) {
            printf("\nCould not fork");
            return;
        }

        if (p1 == 0) {//child1
            // only need to write at the write end
            close(pfd1[0]);
            dup2(pfd1[1], STDOUT_FILENO);
            if (execvp(parsedpipe[0][0], parsedpipe[0]) == -1) {
                printf("\nCould not execute command 1..");
                exit(-1);
            }
        } else {
            // Parent executing
            p2 = fork();
            if (p2 < 0) {
                printf("\nCould not fork");
                return;
            }
            // only need to read at the read end
            if (p2 == 0) {//child2
                close(pfd1[1]);
                dup2(pfd1[0], STDIN_FILENO);
                if (execvp(parsedpipe[1][0], parsedpipe[1]) == -1) {
                    printf("\nCould not execute command 2..");
                    exit(-1);
                }
            } else {
                // parent executing, waiting for two children
                close(pfd1[0]);
                close(pfd1[1]); // must close all pipes for parent
                int statusP1, statusP2;
                waitpid(p1, &statusP1, 0);
                waitpid(p2, &statusP2, 0);
            }
        }
    }
    //timeX command exec
    else{
        int i;
        
        for (i = 0; i < 30; i++){
            if (parsedpipe[0][i] == NULL){
                break;
            }
            else{
                parsedpipe[0][i] = parsedpipe[0] [i + 1];
            }
        }
        p1 = fork(); //child starts here
        if (p1 < 0) {
            printf("\nCould not fork");
            return;
        }

        if (p1 == 0) {//child1
            // It only needs to write at the write end
            close(pfd1[0]);
            dup2(pfd1[1], STDOUT_FILENO);
            if (execvp(parsedpipe[0][0], parsedpipe[0]) == -1) {
                printf("\nCould not execute command 1..");
                exit(-1);
            }
        } else {
            // Parent executing
            p2 = fork();
            if (p2 < 0) {
                printf("\nCould not fork");
                return;
            }
            // It only needs to read at the read end
            if (p2 == 0) {//child2
                close(pfd1[1]);
                dup2(pfd1[0], STDIN_FILENO);
                if (execvp(parsedpipe[1][0], parsedpipe[1]) == -1) {
                    printf("\nCould not execute command 2..");
                    exit(-1);
                }
            } else {
                // parent executing, waiting for two children
                close(pfd1[0]);
                close(pfd1[1]); // must close all pipes for parent
                int statusP1, statusP2;
                waitpid(p1, &statusP1, 0);
                waitpid(p2, &statusP2, 0);
                
                //print Rusage for the child
                getrusage(RUSAGE_CHILDREN, &used1);
                printf("(PID)%d  (CMD)%s    ",(int)p1,parsedpipe[0][0]);
                printf("%s%.3f s","(user)", (used1.ru_utime.tv_sec + used1.ru_utime.tv_usec /1000000.0));
                printf("%s%.3f s\n", "  (sys)", (used1.ru_stime.tv_sec + used1.ru_stime.tv_usec /1000000.0));
                
                //print Rusage for the parents
                getrusage(RUSAGE_SELF, &used2);
                printf("(PID)%d  (CMD)%s    ",(int)p2,parsedpipe[1][0]);
                printf("%s%.3f s","(user)", (used2.ru_utime.tv_sec + used2.ru_utime.tv_usec /1000000.0));
                printf("%s%.3f s\n", "  (sys)", (used2.ru_stime.tv_sec + used2.ru_stime.tv_usec /1000000.0));
                return;
            }
        }
    }

    
    // this part i try to handle the more then 1 pipes using for loop but it doesn't work, so i just keep it because want to know what is the problem in it
    /*
    // 0 is read end, 1 is write end
    int i,j,k,q;
    int pfd1[2], pfd2[2], pfd3[2], pfd4[2];
    int all_pfd[4][2] = { pfd1[2], pfd2[2], pfd3[2], pfd4[2]};
    pid_t pid;
    int processID[pipeNum+1];
    int count = 0;
    
    //creat pipe
    for (i=0; i < pipeNum ;i++){
        if (pipe(all_pfd[i]) < 0)
            printf("Pipe could not be initialized\n");
    }
    
    //exec piep
    printf("%d\n", pipeNum);
    for (i=0; i<pipeNum+1; i++){
        pid = fork(); //child starts here
        processID[i] = pid ;
        if (pid < 0) {
            printf("\nCould not fork");
            return;
        }
        if (pid == 0){
            //close all the pipe and read end of first pipe
            printf("iamchild%d%d\n",i,pid);
            if (i == 0){
                printf("pipe1\n");

                
                for(j=0; j<pipeNum; j++ ){
                    if (j != 0){
                        close(all_pfd[j][1]);
                    }
                    close(all_pfd[j][0]);
                }
                dup2(all_pfd[0][1], STDOUT_FILENO);
                printf("going to exec/n");
                if (execvp(parsedpipe[i][0], parsedpipe[i]) == -1) {
                    printf("\nCould not execute command 2..");
                    exit(-1);
                }
            }
            //close all the pipe and write end of last pipe
            else if (i == pipeNum){
                printf("pipelast\n");
                for(k=0; k<pipeNum; k++ ){
                    if (k != pipeNum-1){
                        close(all_pfd[k][0]);
                    }
                    close(all_pfd[k][1]);
                }
                printf("going to exec/n");
                dup2(all_pfd[i-1][0], STDIN_FILENO);
                if (execvp(parsedpipe[i][0], parsedpipe[i]) == -1) {
                    printf("\nCould not execute command 2..");
                    exit(-1);
                }
                

            }
            else{
                for (int q = 0; q < pipeNum; q++) {
                    if (q != i) {
                        close(all_pfd[q][1]);
                    }
                    if (q != i + 1) {
                        close(all_pfd[q][0]);
                    }
                }
                dup2(all_pfd[i][1], 1);
                dup2(all_pfd[i-1][0], 0);
                printf("going to exec/n");
                if (execvp(parsedpipe[i][0], parsedpipe[i]) == -1) {
                    printf("\nCould not execute command 2..");
                    exit(-1);
                }
            }
        }
        if (i = pipeNum && pid != 0){
            printf("iamparent%d\n", pid);
            int status1,status2,status3,status4,status5;
            int exit_status;
            int statusA[5] = {status1,status2,status3,status4,status5};
        
            for (i=0; i<pipeNum+1; i++){
                printf("i am waiting\n");
                waitpid(processID[i], NULL, 0);
            }
        }
    }
    */
}

// Function to execute builtin commands
int BuiltIn(char* str, char** parsed)
{
    int NoOfOwnCmds = 2, i, changeflag = 0;
    char* builtinCMD[NoOfOwnCmds];


    builtinCMD[0] = "exit";
    builtinCMD[1] = "timeX";

    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], builtinCMD[i]) == 0) {
            changeflag = i + 1;
            break;
        }
    }

    switch (changeflag) {
    case 1:
        if (parsed[1] == NULL){
            printf("3230shell: Terminated\n");
            exit(0);
        }else{
            printf("3230shell: exit with other argument!!!\n");
            return 1;
        }
    case 2:
        if ((parsed[1]) == NULL){
            printf("3230shell: 'timeX' cannot be a standalone command\n");
            return 1;
        }
        else{
            return 0;
        }
    default:
        break;
    }
    return 0;
}

// checking the exit of pipe and parse seperate the pipe
int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 30; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return i;
    }
}

// for parsing the command
void parseSpace(char* str, char** parsed)
{
    int i;
    const char s[2] = " ";
    char * token;
    
    token = strtok(str, s);
    for(i = 0; i < 30; i++){
        parsed[i] = token;
        token = strtok(NULL, s);
        if (parsed[i] == NULL){
            break;
        }
    }
        
}

//for check the input and the execflag for the command
int checkingflag(char* str, char*** parsed)
{
    char* strpiped[30];
    int piped = 0;
    int check = 0;
    int i = 0;
    
    if (strlen(str) == 1){
        return 0;
    }
    
    piped = parsePipe(str, strpiped);
    
    //checking for the enter input
    if (piped > 0){
        check = 1;
    }
    
    //checking for the consecutive |
    if (check) {
        for (i=0 ; i < piped; i++){
            if (strlen(strpiped[i]) == 1){
                printf("3230shell : Should not have two consecutive | without in-bettween command\n");
                return 0;
            }
            else
                parseSpace(strpiped[i], parsed[i]);
        }
    } else {
        parseSpace(str, parsed[0]);
    }
    if (BuiltIn(str, parsed[0]))
        return 0;
    else
        return 1 + piped;
}

//first sigint handler for the interrupt of the shell
void sigint_handler1(int signum){
    printf("\n$$ 3230shell## ");
    fflush(stdout);
}


int main()
{
    char inputString[1024], *parsedArgs[30];
    char* parsedArgsPiped1[30], *parsedArgsPiped2[30], *parsedArgsPiped3[30], *parsedArgsPiped4[30];
    char** NoOfPIPE[5] = {parsedArgs,parsedArgsPiped1, parsedArgsPiped2, parsedArgsPiped3, parsedArgsPiped4};
    int execFlag = 0;


    while (1) {
        
        printf("$$ 3230shell## ");
        signal(SIGINT, sigint_handler1);
        
        //get the user input
        if (!fgets(inputString, 1024, stdin)){
            break;
        }
            
        
        size_t length = strlen(inputString);
        if (length == 0){
            break;
        }
        
        if (length > 1){
            if (inputString[length - 1] == '\n'){
                inputString[length - 1] = '\0';
            }
        }
    
        
        execFlag = checkingflag(inputString,NoOfPIPE);

        // 1 if it is a simple command
        if (execFlag == 1)
            execArgs(parsedArgs);
       
        // 2 if it is including a pipe.
        if (execFlag >= 2)//execFlag - 2 = no of pipe
            execArgsPiped(NoOfPIPE, execFlag-2);
    }
    
}
