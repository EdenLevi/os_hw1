#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include <string.h>

using namespace std;

list<string> ChangeDirCommand::path_history;

int main(int argc, char *argv[]) {
    SmallShell::prompt = "smash> "; // default value

    if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    /*if (signal(SIGALRM, alarmHandler) == SIG_ERR) {
        perror("smash error: failed to set alarm handler");
    }*/


    //TODO: setup sig alarm handler
    struct sigaction a;
    a.sa_handler = alarmHandler;
    sigemptyset(&a.sa_mask);
    a.sa_flags = SA_RESTART;
    if (sigaction(SIGALRM, &a, nullptr) == -1) {
        perror("smash error: failed to set alarm handler");
    }

    while (true) {
        setbuf(stdout, NULL);
        std::cout << SmallShell::prompt;
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        SmallShell::executeCommand(cmd_line.c_str());
    }
    return 0;
}