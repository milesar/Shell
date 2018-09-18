#include <iostream>

#include "shellpers-2.h"

int main(int argc, char *argv[]) {
    std::string line;
    std::vector<pid_t> backgrounds;
    // main program loop, continues the user exits the program or an error is
    while (getline(std::cin, line)) {
        // clean up background processes that have finished running.
        while (pid_t background = waitpid(-1, 0, WNOHANG) > 0) {
            backgrounds.erase(std::remove(backgrounds.begin(), backgrounds.end(),
            background), backgrounds.end());
        }

        // get and parse the user input string
        auto args = tokenize(line);

        // handle change directory requests, using chdir() system command.
        // handle errors changing directories to user request or home.

        if (args[0] == "cd") {

            int i = 0;

            if (args.size() == 2) {
                i = chdir(args[1].c_str());

            } else { i = chdir(getenv("HOME")); }

            if (i < 0) { std::cout << "directory could not be changed." << "\n"; }
            continue;
        }

        // storage for commands and background processes.
        std::vector<Command> commands = getCommands(args);

        // setup a loop to work through all the commands in the command array.
        for (Command command : commands) {

            pid_t  pid;
            pid = fork();

            // if the fork() was unsuccessful, exit().
            if (pid == -1) {

                perror("error, cannot fork()\n");
                exit(EXIT_FAILURE);
            }

            // child instructions.
            // if the fork() was successful, assign its stdin and stout and call execvp().
            else if (pid == 0) {

                if (command.fdStdin != 0) { dup2(command.fdStdin, 0); }
                if (command.fdStdout != 1) { dup2(command.fdStdout, 1); }

                execvp(command.exec.c_str(), (const_cast<char* const*>(command.argv.data())));
            }

            //  parent instructions.
            //  the parent waits for the children to finish running the program, and closes any
            //  commands that have had their fd ins and outs reassigned. Additionally checks if
            //  the process is intended to run in the background.
            
            else {
                //  check the background flag for the command, and do not tell the parent (shell)
                //  to wait for the child to finish. assigns background processes to an array to
                //  keep track of them, for foregrounding or inspection (later implementation).
                //  otherwise, the parent waits for the child process to complete.
                
                if (command.background) {
                    backgrounds.push_back(pid);
                } else {
                    waitpid(pid, nullptr, 0);
                    if (command.fdStdin != 0) { close(command.fdStdin); }
                    if (command.fdStdout != 1) { close(command.fdStdout); }
                }
            }
        }
    }

    return 0;
}
