#include "shellpers-2.h"

/*
 * text handling functions, for parsing inputs.
 */
bool splitOnSymbol(std::vector<std::string> &words, int i, char c) {
    if (words[i].size() < 2) { return false; }
    int pos;
    if ((pos = words[i].find(c)) != std::string::npos) {
        if (pos == 0) {
            //starts with symbol
            words.insert(words.begin() + i + 1, words[i].substr(1, words[i].size() - 1));
            words[i] = words[i].substr(0, 1);
        } else {
            //symbol in middle or end
            words.insert(words.begin() + i + 1, std::string{c});
            std::string after = words[i].substr(pos + 1, words[i].size() - pos - 1);
            if (!after.empty()) {
                words.insert(words.begin() + i + 2, after);
            }
            words[i] = words[i].substr(0, pos);
        }
        return true;
    } else {
        return false;
    }
}

std::vector<std::string> tokenize(const std::string &s) {

    std::vector<std::string> ret;
    int pos = 0;
    int space;
    //split on spaces
    while ((space = s.find(' ', pos)) != std::string::npos) {
        std::string word = s.substr(pos, space - pos);
        if (!word.empty()) {
            ret.push_back(word);
        }
        pos = space + 1;
    }

    std::string lastWord = s.substr(pos, s.size() - pos);
    if (!lastWord.empty()) {
        ret.push_back(lastWord);
    }

    for (int i = 0; i < ret.size(); ++i) {
        for (auto c : {'&', '<', '>', '|'}) {
            if (splitOnSymbol(ret, i, c)) {
                --i;
                break;
            }
        }
    }

    return ret;
}

std::ostream &operator<<(std::ostream &outs, const Command &c) {
    outs << c.exec << " argv: ";
    for (const auto &arg : c.argv) {
        if (arg) {
            outs << arg << ' ';
        }
    }
    outs << "fds: " << c.fdStdin << ' ' << c.fdStdout << ' ' << (c.background ? "background" : "");
    return outs;
}

/** builds a set of commands from the input array of tokens.
 *
 * @param tokens a vector of strings, the commands and characters from the command line.
 * @return a vector of comman objects, distilled from the vector of string tokens.
 */
std::vector<Command> getCommands(const std::vector<std::string> &tokens) {
    // count the number of commands needed by counting pipes and adding 1 (there will always be
    // a command on either side of a pipe only one in any other case including redirection.
    std::vector<Command> ret(std::count(tokens.begin(), tokens.end(), "|") + 1);

    int first = 0;
    int last = std::find(tokens.begin(), tokens.end(), "|") - tokens.begin();
    bool error = false;

    for (int i = 0; i < ret.size(); ++i) {

        // error check for leading commands without the appropriate dependencies.
        if ((tokens[first] == "&") || (tokens[first] == "<") ||
            (tokens[first] == ">") || (tokens[first] == "|")) {
            error = true;
            break;
        }
        
        ret[i].exec = tokens[first];
        ret[i].argv.push_back(tokens[first].c_str());
        std::cout << "exec start: " << ret[i].exec << std::endl;
        ret[i].fdStdin = 0;
        ret[i].fdStdout = 1;
        ret[i].background = false;

        // get the rest of the tokens after the command (if any) and evaluate them to create the next
        // command, handling fds, redirection, and background assignment.
         
        for (int j = first + 1; j < last; ++j) {
            if (tokens[j] == ">" || tokens[j] == "<") {
                if (tokens[j] == ">") {
                    ret[i].fdStdout = open(tokens[j + 1].c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777);
                } else {
                    ret[i].fdStdin = open(tokens[j + 1].c_str(), O_RDONLY);
                }

                j++;
            } else if (tokens[j] == "&") {
                ret[i].background = true; // set flag for background assignment of this command.
            } else {
                ret[i].argv.push_back(tokens[j].c_str()); // set final program field for execution.
            }
        }
        //  if there are multiple commands found that need to be connected with pipes,
        //  create a pipe between commands on either side of the pipe by assigning their
        //  in and out fd's to the pipe.

        if (i > 0) {
            int pfd[2];
            pipe(pfd);

            ret[i - 1].fdStdout = pfd[1];
            ret[i].fdStdin = pfd[0];
        }

        ret[i].argv.push_back(nullptr); //obligatory nullptr cap to keep execvp() happy.

        // get the next pipe location, so we can connect the correct commands to each other.
        first = last + 1;
        if (first < tokens.size()) {
            last = std::find(tokens.begin() + first, tokens.end(), "|") - tokens.begin();
        }
    }
    // if an error is encountered, make sure that all processes in and outs are closed.
    // (prevents memory leaks, safe handling of running/waiting processes).
     
    if (error) {
        for (Command command : ret) {
            close(command.fdStdin);
            close(command.fdStdout);
        }
    }

    return ret;
}
