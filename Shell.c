// Program written by 
// Guoruizhe Sun
// Zifan Chen

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>




const int MAX_USER_INPUT = 100;
const int MAXARGS = 30;
const int MAXBGPROCESS = 30;




void get_input(char* input);
int parse_input(char* input, char** argv);
void parse_io(char **argv, char *output, char *input, int *io);
void build_in_cmd(char *cmd, pid_t *bgpid);
int is_empty(const char *input);
void addpid(pid_t *bgpid, pid_t pid);
void removepid(pid_t *bgpid, pid_t pid);
void eval(char **argv, int bg, pid_t *bgpid);
void int_handler(int sig);




int main()
{
    pid_t bgpid[MAXBGPROCESS];          // maximum background process


    // signal handler (if require to kill all chilren when parent exit)
    void int_handler(int sig)
    {
        int status;
        int pid;
        pid = waitpid(-1, &status, WNOHANG);
        // printf("PID is %d\n",pid);
        removepid(bgpid,pid);

    }

    // installing signal
    signal(17,int_handler);

    // initialize all process to default
    int count;
    for(count = 0; count < MAXBGPROCESS; count++)
    {
        bgpid[count] = -1;
    }


    // ask user input, parse, evluate
    while(1)
    {
        char input[MAX_USER_INPUT];         // user input
        char *argv[MAXARGS];                // max arguments
        int bg;                             // background or forground
                    
        
        input[0] = '\0';


        get_input(input);                   // get user input

        // check empty line
        if(is_empty(input) == 1)
        {
            bg = parse_input(input, argv);      // parse input
        

            // debug
            // int i = 0;
            // while(argv[i] != NULL)
            // {
            //     printf("%s\n", argv[i]);
            //     i++;
            // }
        
        
            eval(argv, bg, bgpid);                      // run shell

            
            // debug
            // int i;
            // for(i=0; i<MAXBGPROCESS; i++)
            // {
            //     if(bgpid[i] != -1)
            //     {
            //         printf("Current background process: %d\n", bgpid[i]);
            //     }
            // }



        }



    }

    return 0;
}

// signal handler 
// void int_handler(int sig)
// {
//     int status;
//     int pid;
//     pid = waitpid(-1, &status, WNOHANG);
//     // debug
//     // printf("PID is %d\n",pid);

// }

// check if input is empty
int is_empty(const char *input)
{
    int i;
    if(strlen(input) == 0)
    {
        return 0;
    }

    for(i = 0; i < strlen(input); i++)
    {
        if(input[i]!=' ' && input[i] != '\n')
        {
            return 1;
        }
    }

    

    return 0;
}


// get user input
// function is done
void get_input(char* input)
{
    
    printf("prompt> ");
    if(fgets(input,MAX_USER_INPUT,stdin) == NULL)
    {
        exit(0);
    }

    fflush(stdout);

    if(strlen(input) == 0)
    {
        exit(0);
    }


}

// parse user input into linxu command
// returns whether is foreground or background command
// 0 is foreground
// 1 is background
// function is done
int parse_input(char* input, char **argv)
{  

    int bg = 0;
    char *token;
    int index = 0;

    



    //remove backspace
    input[strlen(input)-1] = '\0';


    // seperate each word and store in argv
    token = strtok(input, " ");
    while(token != NULL)
    {
        argv[index] = token;
        argv[index][strlen(token)] = '\0';
        index ++;
        token = strtok(NULL," ");
    }
    argv[index] = NULL;

    // check background foreground
    if(strcmp(argv[index-1],"&") == 0)
    {
        bg = 1;
        argv[index-1] = NULL;               // if &, change to NULL
    }




    
    return bg;

}


// only one build in command
// quit, terminates all children process as well
void build_in_cmd(char *cmd, pid_t *bgpid)
{
    if(strcmp(cmd,"quit") == 0)
    {

        // remove all background children process before parent exit
        int i;
        for(i=0; i<MAXBGPROCESS; i++)
        {
            if(bgpid[i] != -1)
            {
                int status;
                kill(bgpid[i],9);
                waitpid(-1,&status, WNOHANG); 
            }
        }
        
        // exit

        //printf("program exited\n");


        exit(0);
    }
}

// add background process to pid array
void addpid(pid_t *bgpid, pid_t pid)
{
    // add background process to tracker
    int i;
    for(i=0; i<MAXBGPROCESS; i++)
    {
        if(bgpid[i] == -1)
        {
            bgpid[i] = pid;
            break;
        }
    }

}

// remove background process from pid array
void removepid(pid_t *bgpid, pid_t pid)
{
    // remove terminated background process from tracker
    int i;
    for(i=0; i<MAXBGPROCESS; i++)
    {
        if(bgpid[i] == pid)
        {
            bgpid[i] = -1;
            break;
        }
    }
}

// parse io
// modifies argv, output, input, io
void parse_io(char **argv, char *output, char *input, int *io)
{
    int i = 0;
    while(argv[i] != NULL)
    {
        if(strcmp(argv[i],"<")==0)
        {
           
            io[0] = 1;

            if(argv[i+1] != NULL)
            {
                strncpy(input,argv[i+1],100);
            }
            else
            {
                io[0] = 0;
                printf("syntax error near unexpected token \'newline\'\n");
                exit(0);
            }
            
            argv[i] = NULL;
            
        }
        else if(strcmp(argv[i],">")==0)
        {
            io[1] = 1;
            if(argv[i+1] != NULL)
            {
                strncpy(output,argv[i+1],100);
            }
            else
            {
                io[1] = 0;
                printf("syntax error near unexpected token \'newline\'\n");
                exit(0);
            }
            
            
            argv[i] = NULL;
        }
        i++;
    }
}



// execute user command
void eval(char **argv, int bg, pid_t *bgpid)
{
    pid_t pid;

    // io[0] reading from file  (<) yes/no
    // io[1] writing to file    (>) yes/no
    int io[2] = {0,0};
    char input_file[100];
    char output_file[100];
    
    // check if command is build-in
    build_in_cmd(argv[0], bgpid);

    parse_io(argv, output_file, input_file, io);
    
    

    // spawn child process
    if((pid = fork()) == 0)
    {

        // Alien language: parse io part
        

        // determine what io to use
        if(io[0] == 1)
        {
            if(freopen(input_file,"r",stdin) == NULL)
            {
                printf("Redirect input to %s failed\n", input_file);
            }
        }
        if(io[1] == 1)
        {
            
            if(freopen(output_file,"w",stdout) == NULL)
            {
                printf("Redirect output to %s failed\n", output_file);
            }
        }

        if(execv(argv[0],argv) < 0)
        {
            if(execvp(argv[0],argv) < 0)
            {
                printf("%s: command not found.\n", argv[0]);
                exit(0);
            }
            
        }
    }

    // if child is foreground, wait till child exit and reap
    if(bg == 0)
    {
        int status;
        if (waitpid(pid, &status, 0) < 0)
        {
            printf("waitfg: waitpid error\n");
            exit(0);
        }
    }
    else
    {
        // debug
        // printf("------------------Added background process id: %d\n", pid);
        // if children background, add to tracker and continue parent process
        addpid(bgpid,pid);
    }
    
    
}
