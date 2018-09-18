#ifndef ASSIGNMENT02_SHELLPERS_2_H
#define ASSIGNMENT02_SHELLPERS_2_H

#endif //ASSIGNMENT02_SHELLPERS_2_H

#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <cassert>

/*
 * Helper method, splits string ons a given symbol and returns true if found.
 */
bool splitOnSymbol(std::vector<std::string>& words, int i, char c);

/*
 * Helper method, tokenizes the input string.
 */
std::vector<std::string> tokenize(const std::string& s);

// Assumes the user tries to only execute 1 command.
struct Command{
    std::string exec; //the name of the executable
    // Reminder, argv should end with a nullptr!
    std::vector<const char*> argv;
    int fdStdin, fdStdout;
    bool background;
};

/**
 * Helper method for debugging commands.
 */
std::ostream& operator<<(std::ostream& outs, const Command& c);

/**
 * Gets the commands from the input sequence, returns array of commands if 
 * viable commands are found.
 */
std::vector<Command> getCommands(const std::vector<std::string>& tokens);
