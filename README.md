This is a mini shell project that simulates the shell used in linux 
IT implements for the mini shell manipulation 
* lex and yacc (for tokenization)
* C (for process manipulation and forking)
* shell and bash manipulation

key idea 
* The tokens and tokenization "grammer" are done using lex and yacc
* The main process forks children to excute the command
* The commands are executed using the built-in library of linux commands
* There are some signals manipulation to do different work like ctrl-c for exiting the shell , exiting with printing a goodby for example.

