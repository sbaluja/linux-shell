
Saarthi Baluja: 1098654 : sbaluja@uoguelph.ca : 2021-2-5

    The following work is completed by me, individually.
    The program is a simple linux shell environment. 

    What is supported: Commands with/without parameters, file redirection, piping (1 level of pipes), 
                       changing directories(cd), history of commands, echo $PATH & echo $HOME, export command.

    What is not supported: Full implementation of the environment variables $PATH and $HOME. The export
                           command does change the environment variables PATH & HOME, and is shown through the ECHO cmd,
                           but execvp is used, keeping the original path for commands. Some incomplete cmds such as a cmd 
                           that is piped to no file (</>), will cause a segmentation fault, but piping (|) to nothing is handled.

    Export CMD: must be in the form of "export $variable=path" with no spaces between the variable, equal sign, and path.

    The default PATH varaible is set to /bin:usr/bin, and the default HOME variable is whatever directory the shell is called from.
    

    Type "make" at the command line followed by "./myShell" to run the shell.
    