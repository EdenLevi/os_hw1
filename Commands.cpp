#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

#define CHAR_MAX 255

using namespace std;
string SmallShell::prompt;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

void ChangePromptCommand::execute() {
    char **args = new char *[COMMAND_MAX_ARGS];
    int i = _parseCommandLine(this->cmd_line, args);

    if (i >= 2) {  // usage: "chprompt <prompt>"
        SmallShell::prompt = strcat(args[1], "> ");
    } else { // usage: "chprompt"
        SmallShell::prompt = "smash> ";
    }
    cout << "i: " << i << endl;

    for (int j = 0; j < i; j++) {
        free(args[j]);
    }
    delete[] args;
}

void ShowPidCommand::execute() {
    int pid = getpid();
    if (pid == 0) {
        //wait();
    }
    cout << "smash pid is " << pid << endl;
}

void GetCurrDirCommand::execute() {
    char dir[CHAR_MAX];
    getcwd(dir, sizeof(dir));
    cout << dir << endl;
}

void ChangeDirCommand::execute() {

    int result = 0;
    char currdir[CHAR_MAX];
    getcwd(currdir, sizeof(currdir));
    char **args = new char *[COMMAND_MAX_ARGS];
    int i = _parseCommandLine(this->cmd_line, args);

    if (i == 2) { // proper usage (cd <argument>)
        if (strcmp(args[1], "-") == 0) { // go to previous set cd path
            if (ChangeDirCommand::path_history.empty()) {
                perror("smash error: cd: OLDPWD not set");
            } else {
                result = chdir(ChangeDirCommand::path_history.back().c_str());
                if (result == 0) ChangeDirCommand::path_history.pop_back();
            }
        } else {
            result = chdir(args[1]); // proper usage (cd <path>)
            ChangeDirCommand::path_history.push_back(currdir);
        }

        if (result != 0) {
            // you have an error :(
            perror("smash error: cd: chdir failed");
        }
    } else if (i > 2) { // improper usage (more than one argument)
        perror("smash error: cd: too many arguments");
    }

    for (int j = 0; j < i; j++) {
        free(args[j]);
    }
    delete[] args;

}

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line) {
    // For example:

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord.compare("chprompt") == 0) {
        return new ChangePromptCommand(cmd_line);
    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
    } else {
        // return new ExternalCommand(cmd_line);
    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {

    // TODO: Add your implementation here
    // for example:
    Command *cmd = CreateCommand(cmd_line);
    if (cmd != nullptr) cmd->execute();
    // Please note that you musCreateCommandt fork smash process for some commands (e.g., external commands....)

}

Command::~Command() = default;

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {};

Command::Command(const char *cmd_line) : cmd_line(cmd_line) {};

ChangePromptCommand::ChangePromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};