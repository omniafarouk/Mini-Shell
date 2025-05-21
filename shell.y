
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token	NOTOKEN GREAT NEWLINE DOUBLE_GREAT PIPE SMALL AND

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command AND NEWLINE { 
		printf("   Yacc: Command executed in background \n");	
		Command::_currentCommand._background++;
		Command::_currentCommand.execute();
	}
	| simple_command NEWLINE { 
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
    ;

simple_command:	
	command_and_args
	|
	simple_command iomodifier_opt
	;

command_and_args:
	command_word arg_list {
		printf("    Yacc: command without pipe \n");
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	| command_and_args PIPE command_word arg_list{
		printf("    Yacc: redirect output to second command\n");

		Command::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
		printf("current command is %d \n",Command::_currentSimpleCommand->_numberOfArguments);
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
        printf("   Yacc: insert command \"%s\"\n", $1);
	    Command::_currentSimpleCommand = new SimpleCommand();
	    Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._outFileState = 0;
	}
	| DOUBLE_GREAT WORD {
		printf("   Yacc: append output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._outFileState = 1;
	}
	| SMALL WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	;
%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
