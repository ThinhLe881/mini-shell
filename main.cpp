#include <string>
#include <list>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "Parser.h"

int main(int args, char **argv)
{
    struct stat status;
    fstat(STDIN_FILENO, &status);
    std::string input;

    system("clear");

    // Display a character to show the user we are in an active shell.
    if (S_ISCHR(status.st_mode))
        std::cout << "? ";

    // Pipe files or read from the user prompt
    // TODO: Groups will need to swap out getline() to detect the tab character.
    while (getline(std::cin, input))
    {
        // Add another case here if you are in a group to handle \t!
        if (input == "exit")
        {
            return 1;
        }
        else
        {
            // TODO: Put your command execution logic here
            std::list<Command> *cmds = Parser::Parse(input);

            int numCmds = 0;

            int in, out;

            int prev[2];

            for (auto cmd = cmds->begin(); cmd != cmds->end(); cmd++)
            {
                numCmds++;

                int next[2];

                // If only 1 command, don't need to use pipe
                if (cmds->size() > 1)
                    pipe(next);

                int pid = fork();
                if (pid < 0)
                {
                    std::cerr << "Error creating child process!" << std::endl;
                    exit(1);
                }
                else if (pid == 0)
                {
                    // If only 1 command, don't need to use pipe
                    if (cmds->size() > 1)
                    {
                        if (cmd == cmds->begin()) // First command
                        {
                            if (dup2(next[1], STDOUT_FILENO) == -1)
                            {
                                std::cerr << "Error duplicating output file descriptor!" << std::endl;
                                exit(1);
                            }
                        }
                        else if (numCmds == cmds->size()) // Last command
                        {
                            if (dup2(prev[0], STDIN_FILENO) == -1)
                            {
                                std::cerr << "Error duplicating input file descriptor!" << std::endl;
                                exit(1);
                            }
                        }
                        else // Middle command(s)
                        {
                            if (dup2(prev[0], STDIN_FILENO) == -1)
                            {
                                std::cerr << "Error duplicating input file descriptor!" << std::endl;
                                exit(1);
                            }

                            if (dup2(next[1], STDOUT_FILENO) == -1)
                            {
                                std::cerr << "Error duplicating output file descriptor!" << std::endl;
                                exit(1);
                            }
                        }

                        // Closes all the pipe ends after using
                        if (cmd != cmds->begin())
                        {
                            close(prev[0]);
                            close(prev[1]);
                        }
                        close(next[0]);
                        close(next[1]);
                    }

                    // If there is input file, redirect to input file
                    if (cmd->input_file != "")
                    {
                        in = open(const_cast<char *>(cmd->input_file.c_str()), O_RDONLY);
                        if (in < 0)
                        {
                            std::cerr << "Error opening input file!" << std::endl;
                            exit(1);
                        }

                        if (dup2(in, STDIN_FILENO) == -1)
                        {
                            std::cerr << "Error duplicating input file descriptor!" << std::endl;
                            exit(1);
                        }

                        close(in);
                    }

                    // If there is output file, redirect to output file
                    if (cmd->output_file != "")
                    {
                        out = open(const_cast<char *>(cmd->output_file.c_str()), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                        if (out < 0)
                        {
                            std::cerr << "Error opening output file!" << std::endl;
                            exit(1);
                        }

                        if (dup2(out, STDOUT_FILENO) == -1)
                        {
                            std::cerr << "Error duplicating output file descriptor!" << std::endl;
                            exit(1);
                        }

                        close(out);
                    }

                    // Convert string to char* array to work with execvp
                    char *arguments[cmd->args.size() + 2];
                    int i = 0;

                    char *command = const_cast<char *>(cmd->name.c_str());
                    arguments[i++] = command;

                    for (auto arg = cmd->args.begin(); arg != cmd->args.end(); arg++)
                    {
                        arguments[i++] = const_cast<char *>((*arg).c_str());
                    }

                    // Append NULL terminator at the end of arguments list to work with execvp
                    arguments[i] = NULL;

                    execvp(command, arguments);

                    std::cerr << "Command did not execute correctly!" << std::endl;
                    exit(1);
                }

                if (cmds->size() > 1)
                {
                    // Don't need to close in first command since there is not "prev" pipe yet
                    if (cmd != cmds->begin())
                    {
                        close(prev[0]);
                        close(prev[1]);
                    }
                    // Reassign "next" to "prev" pipe for next command
                    else if (numCmds != cmds->size())
                    {
                        prev[0] = next[0];
                        prev[1] = next[1];
                    }
                    // Last command closes all the pipe ends
                    else
                    {
                        close(next[0]);
                        close(next[1]);
                    }
                }

                waitpid(pid, NULL, 0);
            }
        }

        // Display a character to show the user we are in an active shell.
        if (S_ISCHR(status.st_mode))
        {
            std::cout << std::endl;
            std::cout << "? ";
        }
    }
}