#pragma once
#include <list>
#include <string>

// Class to store commands.
// input_file and output_file indicate files to open for read/write
// args are the list of arguments to each program
class Command {
public:
    std::string name; // comand to executre
    std::list<std::string> args;

    std::string output_file; 
    std::string input_file;
};

class Parser {
public:
    static std::list<Command>* Parse(std::string raw_input);
};