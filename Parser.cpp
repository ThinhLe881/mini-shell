#include "Parser.h"
#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <string_view>
#include <regex>

/*
    Parses a line and returns a list of commands to execute.
*/
std::list<Command> *Parser::Parse(std::string raw_input)
{
    std::stringstream ss(raw_input);
    std::string pipe_item;
    std::list<Command> *commands = new std::list<Command>();

    // Seperate commands by |, so that we can link them together when iterating through them
    while (std::getline(ss, pipe_item, '|'))
    {
        Command new_command;

        // Isolate the command from possible file redirection
        auto command_end = pipe_item.find_first_of("<>");
        auto command_text = (command_end == std::string::npos) ? pipe_item : pipe_item.substr(0, command_end);

        // Find words seperated by spaces and add them to a list
        std::regex text_regex("\\S+");
        auto iter = std::sregex_iterator(command_text.begin(), command_text.end(), text_regex);
        auto iter_end = std::sregex_iterator();
        auto values = new std::list<std::string>();
        for (auto i = iter; i != iter_end; i++)
            values->push_back((*i).str());

        // Remove the command name, store arguments in a list
        new_command.name = values->front();
        values->pop_front();
        new_command.args = *values;

        // If we have file redicrection, lets find all the file names
        if (command_end != std::string::npos)
        {
            // Grab the isolated string of file names
            auto without_command = pipe_item.substr(command_end);
            auto no_ws = std::remove_if(without_command.begin(),
                                        without_command.end(),
                                        [](char x)
                                        { return std::isspace(x); });

            // matches any text group of text, keeping < or >.
            std::regex cmd_regex("(<|>)(\\w|[0-9]|\\.|-|_)*");
            iter = std::sregex_iterator(without_command.begin(), no_ws, cmd_regex);
            iter_end = std::sregex_iterator();
            auto clean_command = std::string(without_command.begin(), no_ws);

            bool in = false, out = false;
            auto command_end = no_ws;

            // Iterate through input and output file names, store them in the command structure
            for (auto i = iter; i != iter_end; i++)
            {
                auto match = (*i).str();

                // Reject file redirection that has no file name
                if (match.length() == 1)
                {
                    std::cerr << "Please provide a file to redirect to." << std::endl;
                    return nullptr;
                }

                // We don't want multiple input/output files for this assignment (unless you want to add that)
                if (in && out)
                {
                    break;
                }
                else if (match[0] == '<')
                {
                    new_command.input_file = match.substr(1);
                    in = true;
                }
                else if (match[0] == '>')
                {
                    new_command.output_file = match.substr(1);
                    out = true;
                }
            }
        }
        // Uncomment to see the comand structure.
        /*
        std::cout << "for command: " << new_command.name << std::endl;
        std::cout << "    input: " << new_command.input_file << std::endl;
        std::cout << "    output: " << new_command.output_file << std::endl;
        */

        commands->push_back(new_command);
    }

    return commands;
}