/*
 * This code implements a simple shell program
 * It supports the internal shell command "exit", 
 * backgrounding processes with "&", input redirection
 * with "<" and output redirection with ">".
 * However, this is not complete.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

extern char **getaline();
struct sigaction sig_act;

/*
 * Handle exit signals from child processes
 */
void sig_handler(int signal) {
  int status;
  int result;

	if(result != -1){
		waitpid(-1, &status, WNOHANG);
	}

}

/*
 * The main shell function
 */ 
main() {
  int i;
  char **args;
  int result;
  int block;
  int output;
  int input;
	int pipes;
	int lo_flag = 0;
  char *output_filename;
  char *input_filename;

  // Set up the signal handler
	memset(&sig_act, 0, sizeof(sig_act));
	sig_act.sa_sigaction = sig_handler;
	if(sigaction(SIGHUP, &sig_act, NULL) == -1) {
		//Some sort of perror()

	}
	sig_act.sa_flags = SA_RESTART;

  sigaction(SIGCHLD, &sig_act, NULL);

  // Loop forever
  while(1) {

    // Print out the prompt and get the input
   	printf("->");
   	args = getaline();

 
    // No input, continue
    if(args[0] == NULL)
      continue;

    // Check for internal shell commands, such as exit
    if(internal_command(args))
      continue;
		
		//checking for number of pipe in command
		pipes = (check_pipes(args));

    // Check for an ampersand
    block = (ampersand(args) == 0);

    // Check for redirected input
    input = redirect_input(args, &input_filename);


    switch(input) {
    case -1:
      printf("Syntax error!\n");
      continue;
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting input from: %s\n", input_filename);
      break;
    }

    // Check for redirected output
    output = redirect_output(args, &output_filename);

    switch(output) {
    case -1:
      printf("Syntax error!\n");
      continue;
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting output to: %s\n", output_filename);
      break;
    }
    
		if(pipes > 0){
			do_pipes(args, pipes);
		}
		else{
    	do_command(args, block,input, input_filename, 
	  	output, output_filename);

		}
  }
}

//checking for number of pipes in command
int check_pipes(char **args){
	int i;
	int pipe_count = 0;

	for(i = 1; args[i] != NULL; i++){
		if(args[i][0] == '|'){
			pipe_count++;
		}
	}
	return pipe_count;
}

//trying to check for logical operators
int lo_check(char **args){
	int i;
	for(i = 0; args[i] != NULL; i++){
		if(args[i] == ';'){
			return 1;		  
				
		}
	}
	return 0;
	printf("%s\n", args[i]);
	}



/*
 * Check for ampersand as the last argument
 */
int ampersand(char **args) {
  int i;

  for(i = 1; args[i] != NULL; i++);
  
  //checking that the ampersand exists, if so return a 1.
  if(args[i-1][0] == '&') {
    free(args[i-1]);
    args[i-1] = NULL;
    return 1;
  } else {
    return 0;
  }
  
  return 0;
}

/* 
 * Check for internal commands
 * Returns true if there is more to do, false otherwise 
 */
int internal_command(char **args) {
  if(strcmp(args[0], "exit") == 0) {
    exit(0);
  }

  return 0;
}

int do_pipes(char **args, int pipes) {
    // The number of commands to run
    const int commands = pipes + 1;
    int i = 0;

    int pipefds[2*pipes];

    for(i = 0; i < pipes; i++){
        if(pipe(pipefds + i*2) < 0) {
            perror("Couldn't Pipe");
            exit(EXIT_FAILURE);
        }
    }

    int pid;
    int status;

    int j = 0;
    int k = 0;
    int s = 1;
    int place;
    int commandStarts[10];
    commandStarts[0] = 0;

    // This loop sets all of the pipes to NULL
    // And creates an array of where the next
    // Command starts

    while (args[k] != NULL){
        if(!strcmp(args[k], "|")){
            args[k] = NULL;
            // printf("args[%d] is now NULL", k);
            commandStarts[s] = k+1;
            s++;
        }
        k++;
    }



    for (i = 0; i < commands; ++i) {
        // place is where in args the program should
        // start running when it gets to the execution
        // command
        place = commandStarts[i];

        pid = fork();
        if(pid == 0) {
            //if not last command
            if(i < pipes){
                if(dup2(pipefds[j + 1], 1) < 0){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            //if not first command&& j!= 2*pipes
            if(j != 0 ){
                if(dup2(pipefds[j-2], 0) < 0){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            int q;
            for(q = 0; q < 2*pipes; q++){
                    close(pipefds[q]);
            }

            // The commands are executed here,
            // but it must be doing it a bit wrong
            if( execvp(args[place], args) < 0 ){
                    perror(*args);
                    exit(EXIT_FAILURE);
            }
        }
        else if(pid < 0){
            perror("error");
            exit(EXIT_FAILURE);
        }

        j+=2;
    }

    for(i = 0; i < 2 * pipes; i++){
        close(pipefds[i]);
    }

    for(i = 0; i < pipes + 1; i++){
        wait(&status);
    }
}



/* 
 * Do the command
 */
int do_command(char **args, int block, int input, char *input_filename,
	       int output, char *output_filename) {
  
  int result;
  pid_t child_id;
  int status;

  // Fork the child process
  child_id = fork();

  // Check for errors in fork()
  switch(child_id) {
  case EAGAIN:
    perror("Error EAGAIN: ");
    return;
  case ENOMEM:
    perror("Error ENOMEM: ");
    return;
  }
			


  if(child_id == 0) {
		if(!block){ 
			setpgid(0,0);
			execvp(args[0], args);
			wait(-1, &status, WNOHANG);
			exit(0);
		}
    
    // Set up redirection in the child process
    if(input)
      freopen(input_filename, "r", stdin);

    if(output == 2)
      freopen(output_filename, "a", stdout);

    if(output == 1)
      freopen(output_filename, "w+", stdout);

    // Execute the command
    result = execvp(args[0], args);
    exit(-1);
  }

  // Wait for the child process to complete, if necessary
  if(block) {
    printf("Waiting for child, pid = %d\n", child_id);
    result = waitpid(child_id, &status, 0); 
  }

}





/*
 * Check for input redirection
 */
int redirect_input(char **args, char **input_filename) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Look for the <
    if(args[i][0] == '<') {
      free(args[i]);

      // Read the filename
      if(args[i+1] != NULL) {
	*input_filename = args[i+1];
      } else {
	return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
	args[j] = args[j+2];
      }

      return 1;
    }
  }

  return 0;
}

/*
 * Check for output redirection
 */
int redirect_output(char **args, char **output_filename) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Look for the >
    if(args[i][0] == '>' && args[i+1][0] == '>') {
      free(args[i]);      
     

      // Get the filename 
      if(args[i+2] != NULL) {
	*output_filename = args[i+2];
      } else {
	return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-2] != NULL; j++) {
	args[j] = args[j+3];
      }

      return 2;
    }
    else if(args[i][0] == '>'){
	free(args[i]);

	if(args[i+1] != NULL) {
		*output_filename = args[i+1];
		
	} else{
	  return -1;
	 }
	 for(j = i; args[j-1] != NULL; j++) {
	 	args[j] = args[j+2];
	}
	return 1;
    }
  }

  return 0;
}


