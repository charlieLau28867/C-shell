# C-shell
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
