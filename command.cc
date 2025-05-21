
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}

	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	if(_numberOfSimpleCommands == 1){
		   
		int defaultin = dup(0);
		int defaultout = dup(1);
		int defaulterr = dup(2);

		if(strcmp(_simpleCommands[0]->_arguments[0],"exit") == 0){
			printf("exiting shell...\n");
			exit(0);
		}
		else if(strcmp(_simpleCommands[0]->_arguments[0],"cd") == 0){
			char* newdir ;
			if(_simpleCommands[0]->_numberOfArguments > 1 ){
				newdir = _simpleCommands[0]->_arguments[1];
			}
			else{
				newdir = "/home/omnia";
			}
			printf({"Changing directory...\n"});
			if( chdir(newdir)== 0 ){
				printf("Directory changed to \"%s\"\n",getcwd(NULL,0)); // using getenv("PWD") doesn't change the actual directory that has the current shell
			}
			else{
				printf("Error changing in directory \n");      
    		}
			clear();
			prompt();
			return;
		}
		   
		if( _outFile){
			int outfd;
			if(_outFileState == 0){
				outfd = creat(_outFile,0666);
				if(outfd < 0){
					perror( "error at creating outfile" );
					return; 
				}
			}
			else if(_outFileState == 1){
				outfd = open(_outFile, O_WRONLY | O_CREAT | O_APPEND ,0666);
				if(outfd < 0){
					perror( "error at creating outfile" );
					return; 
				}
			}
			dup2(outfd, 1);
			close(outfd);
		}
		if(_inputFile){
			int inputfd = open(_inputFile,O_RDONLY);
			if(!inputfd)
			{	
				perror("error at openin input file");
				return;
			}
			dup2(inputfd,0);
			close(inputfd);
		}

		pid_t pid = fork();
		if(pid < 0){
			perror("error at forking child ");
			exit(1);
		}
		else if(pid == 0)
		{
			close( defaultin );
			close( defaultout );
			close( defaulterr );

			execvp(_simpleCommands[0]->_arguments[0] ,_simpleCommands[0]->_arguments);
			
			perror( "error at terminating child process");
			exit(2);
		}
		
		dup2( defaultin, 0 );
		dup2( defaultout, 1 );
		dup2( defaulterr, 2 );

		close( defaultin );
		close( defaultout );
		close( defaulterr );

		if(!_background){
			waitpid( pid, 0, 0 );		//ensure this implementation
		}
		else{
			printf("command running in background \n");
		}
	}
	else{	// more than one simple command  --> piping
		int defaultin = dup(0);
		int defaultout = dup(1);
		int defaulterr = dup(2);

		int fdpipe[2];
		int input_fd = defaultin;  // Start with the default input
		
		pid_t pids[_numberOfSimpleCommands]; 

		for (int i = 0; i < _numberOfSimpleCommands; i++) {
			// Set up the pipe, except for the last command
			if (i < _numberOfSimpleCommands - 1) {
				if (pipe(fdpipe) == -1) {
					perror("problem creating pipe");
					return;
				}
			}

			if(i==0){
				if(_inputFile){
					int inputfd = open(_inputFile,O_RDONLY);
					if(!inputfd)
					{	
						perror("error at openin input file");
						exit(2);
					}
					input_fd = inputfd;
				}
			}

			pids[i]= fork();
			if (pids[i] < 0) {
				perror("problem forking child in piping");
				exit(2);
			}

			if (pids[i] == 0) {  // Child process
				// Input redirection
				if (input_fd != defaultin) {
					dup2(input_fd, 0);
					close(input_fd);  
				}

				// Output redirection
				if (i < _numberOfSimpleCommands - 1) {  // Not the last command
					dup2(fdpipe[1], 1);
					close(fdpipe[0]);  // Close the unused read end
					close(fdpipe[1]);  
				} else if (_outFile) {  // Last command, check for output file
					int outfd;
					if (_outFileState == 0) {
						outfd = creat(_outFile, 0666);
						if (outfd < 0) {
							perror("error creating outfile");
							exit(1);
						}
					} else if (_outFileState == 1) {
						outfd = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
						if (outfd < 0) {
							perror("error opening outfile\n");
							exit(1);
						}
					}
					dup2(outfd, 1);  // Redirect output to the file
					close(outfd);
				}

				// Execute the command
				execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
				perror("error executing command\n");
				exit(2);
			}

			// Parent process
			if (input_fd != defaultin) {
				close(input_fd);  // Close the previous input pipe end
			}
			if (i < _numberOfSimpleCommands - 1) {
				input_fd = fdpipe[0];  // Save the read end for the next command
				close(fdpipe[1]);      // Close the write end in the parent
			}
		}

		// Restore the default file descriptors
		dup2(defaultin, 0);
		dup2(defaultout, 1);
		dup2(defaulterr, 2);
		close(defaultin);
		close(defaultout);
		close(defaulterr);

		// Wait for the last command in the pipeline
		if (!_background){
			for(int i=0; i< _numberOfSimpleCommands; i++){
				printf("child with pid %d is terminating \n",pids[i]);
				waitpid(pids[i],0,0);
			}
		}
		else
		{
			printf("command running in background \n");
		}
	}
	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation
void handle_sigint(int sig) {
    // Do nothing, effectively ignoring the signal
}

void insertLog(int sig){
	// insert into log file if exit , if not create a new one
	char buf[80];
	time_t now = time(NULL);
	struct tm tstruct = *localtime(&now);
	strftime(buf,sizeof(buf),"%c",&tstruct);
	printf("CHILD IS TERMINATED AT %s \n",buf);

	FILE *fptr = fopen("logFile.txt","a");
	if(fptr == NULL){
		perror("problem in opening log file in main \n");
	}
	fprintf(fptr,"CHILD IS TERMINATED AT %s \n",buf);
	fclose(fptr);
}

void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	Command::_currentCommand.prompt();
	signal(SIGINT, handle_sigint);
	FILE *fptr = fopen("logFile.txt","w");
	if(fptr == NULL){
		perror("problem in opening log file in main \n");
	}
	fclose(fptr);
	signal(SIGCHLD,insertLog);
	yyparse();
	fclose(fptr);
	return 0;
}

