#include <stdio.h>       /* Input/Output */
#include <stdlib.h>      /* General Utilities */
#include <unistd.h>      /* Symbolic Constants */
#include <sys/types.h>   /* Primitive System Data Types */
#include <sys/wait.h>    /* Wait for Process Termination */
#include <errno.h>       /* Errors */
#include <string.h>      /* String Manipulation*/
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>

#define LINE_LEN_MAX 1024

#define AND 1
#define PIPE 2  
#define TRUE 0
#define FALSE 1

int builtInCommands(char line[LINE_LEN_MAX], FILE *fp, char *PATH, char *HOME, int *cmdCount){
    char buffer[LINE_LEN_MAX];
    char line_copy[LINE_LEN_MAX];
    strcpy(line_copy, line);
    int i = 0;

    fseek(fp, 0, SEEK_SET);


    if (strcmp("echo $PATH", line) == 0){
        printf("%s\n", PATH);
        return 1;
    }

    if (strcmp("echo $HOME", line) == 0){
        printf("%s\n", HOME);
        return 1;
    }

    char *token;  
    char *endString;                
    token = strtok_r(line_copy," ", &endString);



    while (token != NULL){
        if(strcmp("cd", token) == 0){
            token = strtok_r(NULL," ", &endString);
            if (chdir(token) != 0){
                printf("Directory not found: %s\n", token);
            }
            return 1;
        }
        if (strcmp("history", token) == 0){ //handle all history cmd cases
            token = strtok_r(NULL," ", &endString);

            if(!token){
                while (fgets(buffer, LINE_LEN_MAX, fp) != NULL){
                    printf("%s", buffer);
                }
                return 1;
            }
        
         if (token != NULL && isdigit(*token)){  //command is history n where n is a number
            fseek(fp, 0, SEEK_END);
            int num = atoi(token);
            int pos = ftell(fp) - 1;    //skip to end of file - the last newline char
        
            while(pos){
                if (!fseek(fp, --pos, SEEK_SET)){
                    if (fgetc(fp) == '\n') {    
                        i++;            //i tracks how many newlines from the eof we are
                    }
                    // stop reading when n newlines 
                    // is found 
                    if (i == num){
                        break;
                    } 
                
            }
        }


        while (fgets(buffer, LINE_LEN_MAX, fp) != NULL){
            printf("%s", buffer);   //print last n lines
        }
        return 1;
    } else {
            if (token != NULL && strcmp(token, "-c") == 0){
                fp = fopen(".CIS3110_history", "w+");
                *cmdCount = 0;
                return 1;
            }
        return -1;  //History cmd was not followed by a number or by -c
    }
}//history cmd handled

    if (strcmp("export", token) == 0){
        token = strtok_r(NULL," ", &endString);
        char *endString2;
        char *token2 = strtok_r(token, "=", &endString2);
        
        if(token != NULL && token2 != NULL){

            if (strcmp(token2,"PATH") == 0){
                token2 = strtok_r(NULL, "=", &endString2);
                printf("%s\n", token2);
                strcpy(PATH, token2);
                return 1;
            } else if (strcmp(token,"HOME") == 0){
                token2 = strtok_r(NULL, "=", &endString2);
                strcpy(HOME, token2);
                return 1;
            }
        }
    }


    if (token){
        token = strtok_r(NULL," ", &endString);
    }

}
    return -1;  //no built in cmds were used
}

int splitLine(char line[LINE_LEN_MAX], char ***argv, int *options, FILE *fp){
    char *token;                  
    token = strtok(line," ");
    int i = 0;
    int j = 0;
    while(token!=NULL){
        if (*token == '<'){
            token = strtok(NULL," ");
            //printf("FILENAME: %s\n", token);
            freopen(token, "r+", stdin);
            token = strtok(NULL," ");
        } if (token != NULL && *token == '>'){
            token = strtok(NULL," ");
            //printf("FILENAME: %s\n", token);
            freopen(token, "w+", stdout);
            token = strtok(NULL," ");
        } if (token != NULL && *token == '|'){
            *options =+ PIPE;
            argv[j][i] = NULL;  //null termination of first cmd
            token = strtok(NULL," ");
            j++;    //next cmds to be placed in next next array.
            i = 0;
            if (!token){
                char *line2 = malloc(sizeof(char)*LINE_LEN_MAX);
                printf("> ");
                if(fgets(line2, LINE_LEN_MAX, stdin)){
                    if (line2[strlen(line2) - 1] == '\n'){
                        line2[strlen(line2) - 1] = '\0';
                    }
                    argv[j][i] = line2;
                    fseek(fp, -1, SEEK_END);
                    fputs(line2, fp);
                    fputs("\n", fp);
                    i++;
                    break;
                }
            }
        }
        if (token!=NULL) {
            argv[j][i]=token; 
            //printf("%s\n", token);     
            token = strtok(NULL," ");
            i++;
        }
    }
    if (*argv[j][i-1] == '&'){
        *options += AND;
        i--;    //ensure null termination before & command
    }
    argv[j][i]=NULL;
    return i;
}

pid_t simpleLaunch(char ***argv){   //a simple execvp call function
    int status;
    pid_t childpid= fork();              //fork child

    if (childpid < 0){
        fprintf(stderr, "fork failed...exiting\n");
        exit(-1);
    }

    else if(childpid==0){               //Child
        status = execvp(argv[0][0],argv[0]);
        fprintf(stderr, "Child process could not do execvp\n");
        exit(-1);

    } else{                    //Parent
        waitpid(childpid,&status,0);
        if (WIFEXITED(status)){
            if (WEXITSTATUS(status) == 255){
                printf("-bash: Command not found\n"); 
            }
        }
    }
    return childpid;
}

pid_t backgroundLaunch(char ***argv, int backgroundNum, char *CWD){
    int status;
    pid_t childpid= fork();              //fork child
    pid_t childpid2;
    int j = 0;
    int i = 0;

    if (childpid < 0){
        fprintf(stderr, "fork failed...exiting\n");
        exit(-1);
    }

    else if (childpid==0){               //Child

        childpid2 = fork(); //grandchild

        if (childpid2 < 0){
        fprintf(stderr, "fork failed...exiting\n");
        exit(-1);
    }

        if (childpid2 ==0){
            printf("[%d] %d\n", backgroundNum, getpid() );
            status = execvp(argv[0][0],argv[0]);
            fprintf(stderr, "Invalid command.\n");
            exit(-1);
        } else {
            waitpid(childpid2,&status,0);   //child 1 waits for its child
            
            printf("\n[%d]+ Done", backgroundNum);   //prints done when background cmd is done  
            printf("\t\t      ");
            j = 0;                            
            while (argv[i][j] != NULL) {
                printf("%s", argv[i][j]);          //print command/args
                j++;
            }   
            getcwd(CWD, 100);
            printf("\n%s> ", CWD);

            
            exit(0); //child exits
        }


    } else{                    //Parent
        signal(SIGCHLD,SIG_IGN); //specifies to not wait for child termination
    }
    return childpid;
}

void pipeLaunch(char ***argv){
    int status;
    int status2;
    pid_t childpid;
    pid_t childpid2;
    int fd[2];
    int i = 0;
    int j = 0;

    /*while (argv[1][j] != NULL){
        printf("ARGV2: %s\n", argv[1][j]);
        j++;
    }
    j = 0;*/

    if (pipe(fd)==-1) { 
        fprintf(stderr, "Pipe Failed" ); 
        exit(-1); 
    }

    childpid = fork();

    if (childpid < 0){
        fprintf(stderr, "fork failed...exiting\n");
        exit(-1);
    } 

    if (childpid == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(fd[0]); 
        dup2(fd[1], STDOUT_FILENO); 
        close(fd[1]); 
  
        status = execvp(argv[0][0], argv[0]);
        fprintf(stderr, "\nCould not execute command 1.."); 
        exit(-1); 
        
    } else { 
        // Parent executing 
        childpid2 = fork(); 
  
        if (childpid2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
  
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (childpid2 == 0) { 
            close(fd[1]); 
            dup2(fd[0], STDIN_FILENO); 
            close(fd[0]); 
            
            status2 = execvp(argv[1][0], argv[1]);
            fprintf(stderr, "\nCould not execute command 2.."); 
            exit(-1); 
            
        } else { 
            close(fd[1]); //parent does not read or write
            close(fd[0]);
            // parent executing, waiting for two children 
            waitpid(childpid, &status, 0); 
            waitpid(childpid2, &status2, 0);
        } 
    } 

}

void redoLoop(int backgroundNum, FILE *fp, int cmdCount, char *CWD, char *PATH, char *HOME){ // a function to redo the loop called when & is used
    char line[LINE_LEN_MAX];
    char ***argv;
    int options = 0;
    int i = 0;
    int j = 0;
    pid_t childpid;
    argv = malloc(sizeof(char**)*5);
    for (i =0; i<5; i++){
		argv[i] = malloc((sizeof(char*) * 100));
		for (j =0; j<100; j++){
			argv[i][j] = malloc((sizeof(char)) * LINE_LEN_MAX);
            argv[i][j] = "";
		}
	}
    int argc;

    if(fgets(line, LINE_LEN_MAX, stdin) == NULL){  //get command and put it in line
        exit(0);                                     //if user hits CTRL+D break
    }

    fprintf(fp, " %d  ", ++cmdCount);
    fputs(line, fp);
    cmdCount++;

    if (line[strlen(line) - 1] == '\n'){
        line[strlen(line) - 1] = '\0';
    }

    if(strlen(line) == 0){
        printf("%s> ", CWD);                               //print shell prompt
        redoLoop(backgroundNum, fp, cmdCount, CWD, PATH, HOME);
        return;
    } else if (strcmp(line, "exit")==0){                 //check if command is exit
        exit(0);
    }


    if (builtInCommands(line, fp, PATH, HOME, &cmdCount) == -1 && strlen(line) > 0){
        argc = splitLine(line, argv, &options, fp);      //Read the input and parse it, set argc to num of argv
    } else {
        options = -1;
    }


    if (options % 2 == 0 && options < 2){                      //options is flag whethe or not & cmd was used
        //("SIMPLE LAUNCH\n");
        childpid = simpleLaunch(argv);
        backgroundNum = 1;
    } else if (options % 2 == 1 && options < 2){
        //printf("BGRND LAUNCH\n");
        childpid = backgroundLaunch(argv, backgroundNum++, CWD);
        redoLoop(backgroundNum, fp, cmdCount, CWD, PATH, HOME);
    } else if (options == 2){
        //printf("PIPING\n");
        pipeLaunch(argv);
        backgroundNum = 1;
    }
}

int main(){
    char line[LINE_LEN_MAX];  //get command line
    char *filename = malloc(sizeof(char) * 20); //for file piping
    char ***argv;        //user command
    int argc;               //arg count
    int options = 0;
    pid_t childpid;
    int saved_stdin = dup(0);
    int i = 0;
    int j = 0;
    int backgroundNum = 1;
    int cmdCount = 0;
    char *PATH = malloc(sizeof(char)*LINE_LEN_MAX);
    strcpy(PATH,"/bin:usr/bin");
    char *HOME = malloc(sizeof(char)*LINE_LEN_MAX);
    char *CWD = malloc(sizeof(char)*LINE_LEN_MAX);
    getcwd(HOME, 100);
    char *HISTFILE = ".CIS3110_history";

    FILE *fp;
    if ((fp = fopen(HISTFILE, "r+"))){
        fseek(fp, 0, SEEK_END);
        int pos = ftell(fp) - 1;
        
            while(pos){
                if (!fseek(fp, --pos, SEEK_SET)){
                    if (fgetc(fp) == '\n') {
                        i++;  
                    }
                    // stop reading when n newlines 
                    // is found 
                    if (i == 1){
                        break;
                    } 
                
            }
        }

        fscanf(fp, " %d", &cmdCount);
    } else {
        fp = fopen(HISTFILE, "w+");
    }
    FILE *fp2 = fopen(".CIS3110_profile", "w+");
    fprintf(fp2, "export PATH=%s\n", PATH);
    fprintf(fp2, "export HOME=%s\n", HOME);
    
    if (fp == NULL || fp2 == NULL){
        fprintf(stderr, "CIS 3110 File unable to open");
    }


    argv = malloc(sizeof(char**)*5);
    for (i =0; i<5; i++){
		argv[i] = malloc((sizeof(char*) * 100));
		for (j =0; j<100; j++){
			argv[i][j] = malloc((sizeof(char)) * LINE_LEN_MAX);
            argv[i][j] = "";
		}
	}
    

while(1){
    freopen ("/dev/tty", "a", stdout);          //restore stdout
    dup2(saved_stdin, 0);                       //restore stdin
    getcwd(CWD, 100);
    options = 0;
    
    printf("%s> ", CWD);                               //print shell prompt

    if(fgets(line, LINE_LEN_MAX, stdin) == NULL){  //get command and put it in line
        break;                                     //if user hits CTRL+D break
    }

    fseek( fp, 0, SEEK_END );
    fprintf(fp, " %d  ", ++cmdCount);
    fprintf(fp, "%s", line);
    

    if (line[strlen(line) - 1] == '\n'){
        line[strlen(line) - 1] = '\0';
    }

    if(strlen(line) == 0){
        printf("%s> ", CWD);                               //print shell prompt
        redoLoop(backgroundNum, fp, cmdCount, CWD, PATH, HOME);
    } else if (strcmp(line, "exit")==0){                 //check if command is exit
        break;
    }

    if (builtInCommands(line, fp, PATH, HOME, &cmdCount) == -1 && strlen(line) > 0){
        argc = splitLine(line, argv, &options, fp);      //Read the input and parse it, set argc to num of argv
    } else {
        options = -1;
    }
    

    if (options % 2 == 0 && options < 2 && options > -1){                      //options is flag whethe or not & cmd was used
        //printf("SIMPLE LAUNCH\n");
        childpid = simpleLaunch(argv);
        backgroundNum = 1;
    } else if (options % 2 == 1 && options < 2 && options > -1){
        //printf("BGRND LAUNCH\n");
        childpid = backgroundLaunch(argv, backgroundNum++, CWD);
        redoLoop(backgroundNum, fp, cmdCount, CWD, PATH, HOME);
    } else if (options == 2 && options > -1){
        //printf("PIPING\n");
        pipeLaunch(argv);
        backgroundNum = 1;
    }
    
}
    printf("Program exited\n");
} 