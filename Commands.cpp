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
JobsList::JobEntry * SmallShell::cur_job;
vector<JobsList::JobEntry *> JobsList::jobs;
vector<JobsList::JobEntry *> JobsList::times;


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

bool _isBackgroundCommand(const char *cmd_line) {
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

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {};

ChangePromptCommand::ChangePromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

FareCommand::FareCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

JobsCommand::JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

ForegroundCommand::ForegroundCommand( const char *cmd_line ): BuiltInCommand(cmd_line) {};

BackgroundCommand::BackgroundCommand( const char *cmd_line ): BuiltInCommand(cmd_line) {};

QuitCommand::QuitCommand( const char *cmd_line ): BuiltInCommand (cmd_line) {};

RedirectionCommand::RedirectionCommand( const char *cmd_line ): Command(cmd_line) {};

PipeCommand::PipeCommand( const char *cmd_line ): Command(cmd_line) {};

TimeoutCommand::TimeoutCommand(const char* cmd_line) : Command(cmd_line){}


void ChangePromptCommand::execute() {

    if (this->arg_size >= 2) {  // usage: "chprompt <prompt>"
        SmallShell::prompt = strcat(this->args[1], "> ");
    } else { // usage: "chprompt"
        SmallShell::prompt = "smash> ";
    }
}


void FareCommand::execute() {
    string orig, dest;
    if (this->arg_size == 4) {
        orig=this->args[2];
        dest=this->args[3];
        ifstream file;
        file.open(this->args[1]);

        if (file.fail()) {
            perror("smash error: open failed");
            return;
        }

        string container;
        for(string line; getline(file,line);container.append(line)) {
            size_t pos = line.find(orig);
            while (pos != string::npos)
            {
                line.replace(pos, orig.size(), dest);
                pos = line.find(orig,pos);
            }
            line.append("\n");
        }

        file.close();
        if (file.fail()) {
            perror("smash error: close failed");
            return;
        }

        ofstream ofile;
        ofile.open(this->args[1]);

        if (ofile.fail()) {
            perror("smash error: open failed");
            return;
        }

        ofile<<container;
        ofile.close();

        if (ofile.fail()) {
            perror("smash error: close failed");
            return;
        }

    } else {
        perror("smash error: fare: invalid arguments");
    }
}

void ShowPidCommand::execute() {
    int pid = getpid();
    // if (pid == 0) {
    //
    // }
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

    if (this->arg_size == 2) { // proper usage (cd <argument>)
        if (strcmp(this->args[1], "-") == 0) { // go to previous set cd path
            if (ChangeDirCommand::path_history.empty()) {
                perror("smash error: cd: OLDPWD not set");
            } else {
                result = chdir(ChangeDirCommand::path_history.back().c_str());
                if (result == 0) ChangeDirCommand::path_history.pop_back();
            }
        } else {
            result = chdir(this->args[1]); // proper usage (cd <path>)
            ChangeDirCommand::path_history.push_back(currdir);
        }

        if (result != 0) {
            // you have an error :(
            perror("smash error: cd: chdir failed");
        }
    } else if (this->arg_size > 2) { // improper usage (more than one argument)
        perror("smash error: cd: too many arguments");
    }

    for (int j = 0; j < this->arg_size; j++) {
        free(args[j]);
    }
    delete[] this->args;

}

void JobsCommand::execute() {
    JobsList::printJobsList();
}

void ForegroundCommand::execute() {
    int status;
    JobsList::JobEntry *selectedJob;
    if (this->arg_size > 2) {
        // too many arguments
        perror("smash error: fg: invalid arguments");
        return;
    } else if (this->arg_size == 2) {
        // fg <pid>
        int jobId = atoi(this->args[1]); /// may fail if not int
        selectedJob = JobsList::getJobById(jobId);
        if(selectedJob == nullptr) {
            string error = "smash error: fg: job-id ";
            error.append(to_string(jobId));
            error.append(" does not exist");
            perror(error.c_str());
            return;
        }
    } else {
        // fg (no arguments)
        selectedJob = JobsList::getLastJob(nullptr);
        if (selectedJob == nullptr) {
            // error, no jobs and no argument
            perror("smash error: fg: jobs list is empty");
            return;
        }
    }

    cout << selectedJob->command->cmd_line << endl;

    if (kill(selectedJob->process_id, SIGCONT) != 0) {
        //error
        return;
    }
    SmallShell::cur_job = selectedJob;
    waitpid(selectedJob->process_id, &status, WUNTRACED);
    if (!WIFSTOPPED(status)) { // if process did not stop
        JobsList::removeJobById(selectedJob->job_id);
        SmallShell::cur_job = nullptr;
        return;
    }

    selectedJob->stopped=true;
}

void BackgroundCommand::execute() {
    JobsList::JobEntry *selectedJob;
    if (this->arg_size > 2) {
        // too many arguments
        perror("smash error: bg: invalid arguments");
        return;
    } else if (this->arg_size == 2) {
        // bg <pid>
        int jobId = atoi(this->args[1]); /// may fail if not int
        selectedJob = JobsList::getJobById(jobId);
        if(selectedJob == nullptr) {
            string error = "smash error: bg: job-id ";
            error.append(this->args[1]);
            error.append(" does not exist");
            perror(error.c_str());
            return;
        }
            //smash error: bg: job-id <job-id> is already running in the background
        else if(!(selectedJob->stopped)) {
            string error = "smash error: bg: job-id ";
            error.append(this->args[1]);
            error.append("  is already running in the background");
            perror(error.c_str());
            return;
        }
    } else {
        // bg (no arguments)
        selectedJob = JobsList::getLastJob(nullptr);
        if (selectedJob == nullptr) {
            // error, no jobs and no argument
            perror("smash error: bg: there is no stopped jobs to resume");
            return;
        }
    }

    if (kill(selectedJob->process_id, SIGCONT) != 0) {
        perror("smash error: bg failed");
        return;
    }
    selectedJob->stopped = false;
    cout << selectedJob->command->cmd_line << endl;
}

void JobsList::printKillJobs() {
    for (auto job: JobsList::jobs) {
        cout << job->job_id << ": " << job->command->cmd_line << endl;
    }
}

void QuitCommand::execute() {
    if ((this->arg_size > 1 && strcmp(args[1], "kill") == 0)) {
        std::cout << "smash: sending SIGKILL signal to " << JobsList::jobs.size() << " jobs:" << std::endl;
        JobsList::printKillJobs();
        JobsList::killAllJobs();
    }

    exit(0);
}


/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/

Command *SmallShell::CreateCommand(const char *cmd_line) {

    if ((cmd_line == nullptr) || (cmd_line[0] == '\0')) {
        return nullptr;
    }

    // For example:
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if (firstWord[firstWord.size()-1] == '&') {
        firstWord = firstWord.substr(0, firstWord.size() - 1);
    }

    if (cmd_s.find('>') != std::string::npos) {
        return new RedirectionCommand(cmd_line);
    } else if (cmd_s.find('|') != std::string::npos) {
        return new PipeCommand(cmd_line);
    } else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord.compare("chprompt") == 0) {
        return new ChangePromptCommand(cmd_line);
    } else if (firstWord.compare("fare") == 0) {
        return new FareCommand(cmd_line);
    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
    } else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line);
    } else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_line);
    } else if (firstWord.compare("timeout") == 0) {
        return new TimeoutCommand(cmd_line);
    } else if (firstWord.compare("bg") == 0) {
        return new BackgroundCommand(cmd_line);
    } else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line);
    } else {
        return new ExternalCommand(cmd_line);
    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {

    // TODO: Add your implementation here
    // for example:
    JobsList::removeFinishedJobs();
    JobsList::removeFinishedTimes();
    Command *cmd = CreateCommand(cmd_line);
    if (cmd != nullptr) cmd->execute();
    // Please note that you musCreateCommandt fork smash process for some commands (e.g., external commands....)

}

Command::Command(const char *cmd_line) {

    this->args = new char *[COMMAND_MAX_ARGS];
    this->is_bg =_isBackgroundCommand(cmd_line);
    this->cmd_line_new=new char[CHAR_MAX];
    this->cmd_line=new char[CHAR_MAX];
    this->cmd_line= strcpy(this->cmd_line,cmd_line);
    this->cmd_line_new= strcpy(this->cmd_line_new,cmd_line);

    _removeBackgroundSign(this->cmd_line_new);
    this->arg_size = _parseCommandLine(this->cmd_line_new,args);
};

Command::~Command() {
    for (int j = 0; j < this->arg_size; j++) {
        free(this->args[j]);
    }
    delete[] this->args;
    delete[] this->cmd_line;
    delete[] this->cmd_line_new;
}

JobsList::JobEntry::~JobEntry() {
    command = nullptr;
}


void JobsList::addJob(Command *cmd, int pid, bool isStopped) {
    JobsList::removeFinishedJobs();
    int highest_index = jobs.empty() ? -1 : jobs[jobs.size()-1]->job_id;
    JobEntry * newJob = new JobEntry(++highest_index,cmd, pid,isStopped);
    JobsList::jobs.push_back(newJob);

}
void JobsList::addTime(Command *cmd, int pid) {
    JobsList::removeFinishedJobs();
    int highest_index = times.empty() ? -1 : times[times.size()-1]->job_id;
    JobEntry * newJob = new JobEntry(++highest_index,cmd, pid,false,(time(0)));
    time_t t;
    time(&t);
    alarm(JobsList::times[0]->time_s + time_t(atoi(cmd->args[2])) - t);
    JobsList::times.push_back(newJob);
}

JobsList::JobEntry* JobsList::getFirstTime() {
    if(times.empty()) return nullptr;
    return times[0];
}

void JobsList::printJobsList() { /// check if 'secs' is printed or if need to add manually

    JobsList::removeFinishedJobs();
    for(int i = 0; i < JobsList::jobs.size(); i++) {
        //[<job-id>] <command> : <process id> <seconds elapsed> (stopped)
        int job_id = JobsList::jobs[i]->job_id;
        string command = JobsList::jobs[i]->command->cmd_line;
        int p_id = JobsList::jobs[i]->process_id;
        time_t elapsed = difftime(time(0), JobsList::jobs[i]->time_s);
        bool isStopped = JobsList::jobs[i]->stopped;
        cout << "[" << job_id << "] " << command << " : " << p_id << " " << elapsed << (isStopped ? " (stopped)" : "") << endl;
    }
}

void JobsList::killAllJobs() {
    for(vector<JobEntry*>::iterator  i = jobs.begin(); i != jobs.end(); i++) {
        kill((*i)->process_id,SIGKILL);
        delete (*i)->command;
        delete (*i);
        jobs.erase(i);
    }
}

void JobsList::removeFinishedJobs() {
    int stat;

    for(vector<JobEntry*>::iterator i = jobs.begin(); i != jobs.end();) {
        int pid=(*i)->process_id;
        if(waitpid(pid,&stat,WNOHANG) > 0 || kill(pid,0) == -1 ){
            delete (*i)->command;
            delete (*i);
            i = jobs.erase(i);
        }
        else {
            i++;
        }
    }
}

void JobsList::removeFinishedTimes() {
    int stat;

    for(vector<JobEntry*>::iterator i = times.begin(); i != times.end();) {
        int pid=(*i)->process_id;
        if( (waitpid(pid,&stat,WNOHANG) > 0 || kill(pid,0) == -1 )){
            delete (*i)->command;
            delete (*i);
            i = times.erase(i);
        }
        else if((*i)->time_s <= time(0) ){
            kill((*i)->process_id,SIGKILL);
            delete (*i)->command;
            delete (*i);
            i = times.erase(i);
        }
        else {
            i++;
        }
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    for(int i = 0; i < JobsList::jobs.size(); i++) {
        if(jobs[i]->job_id == jobId) {
            return jobs[i];
        }
    }
    return nullptr;
}

void JobsList::removeJobByIdDeep(int jobId) {
    for(vector<JobEntry*>::iterator  i = jobs.begin(); i != jobs.end(); i++) {
        if((*i)->job_id == jobId) {
            delete (*i)->command;
            delete (*i);
            jobs.erase(i);
            break;
        }
    }
}
void JobsList::removeJobById(int jobId) {
    for(vector<JobEntry*>::iterator  i = jobs.begin(); i != jobs.end(); i++) {
        if((*i)->job_id == jobId) {
            delete (*i);
            jobs.erase(i);
            break;
        }
    }
}

JobsList::JobEntry* JobsList::getLastJob(int *lastJobId) {
    if(lastJobId != nullptr) *lastJobId = jobs.size()-1;
    if(JobsList::jobs.empty()) return nullptr;
    return JobsList::jobs[jobs.size()-1];
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId) {
    for(int i = JobsList::jobs.size()-1; i >= 0; i--) {
        if(jobs[i]->stopped) {
            return jobs[i];
        }
    }
    return nullptr;
}



ExternalCommand::ExternalCommand(const char* cmd_line) : Command(cmd_line) {};

void ExternalCommand::execute() {

    pid_t pid = fork();
    if (pid == 0) {
        setpgrp();
        if (execvp(this->args[0], this->args) == -1) {
            perror("smash error: execvp failed");
        }
        exit(0);
    }
    else {
        // ORIGINAL CODE
        JobsList::addJob(this,pid,false);
        int job_id = JobsList::getLastJob(nullptr)->job_id;
        if (!this->is_bg) {
            SmallShell::cur_job = JobsList::getJobById(job_id);
            int status;
            waitpid(pid, &status, WUNTRACED);
            if (!WIFSTOPPED(status)) {
                JobsList::removeJobById(job_id);
                SmallShell::cur_job = nullptr;
            }
        }
    }
};

void RedirectionCommand::execute() {

    int out_new,out_org;

    bool isapp = (string(cmd_line).find(">>") != std::string::npos);

    string cmd = (string(this->cmd_line).substr(0, string(cmd_line).find(">")));
    string file = _trim(string(cmd_line).substr(string(cmd_line).find_last_of(">") + 1));

    out_org = dup(1);
    if(out_org < 0) {
        perror("smash error: dup failed");
        return;
    }

    if (isapp) {
        out_new = open(file.c_str() , O_CREAT | O_WRONLY | O_APPEND , 0666);
    }
    else {
        out_new = open(file.c_str() , O_CREAT | O_WRONLY | O_TRUNC, 0666);
    }

    if (out_new < 0) {
        perror("smash error: open failed");
        close(out_org);
        return;
    }

    if (dup2(out_new,1) < 0) {
        perror("smash error: dup failed");
        return;
    }

    if (close(out_new) < 0) {
        perror("smash error: close failed");
        return;
    }
    SmallShell::executeCommand(cmd.c_str());
    if (dup2(out_org, 1) < 0) {
        perror("smash error: dup failed");
        return;
    }
    if (close(out_org) < 0) {
        perror("smash error: close failed");
        return;
    }
};



void PipeCommand::execute() {
    bool ischild=false;
    int pipe_fd[2];
    bool iserr = (string(cmd_line).find("|&") != std::string::npos);
    string part_a = string(cmd_line).substr(0, string(cmd_line).find("|"));
    string part_b = string(cmd_line).substr(string(cmd_line).find("|") + (iserr ? 2 : 1));

    if(pipe(pipe_fd) != 0 ) {
        perror("smash error: pipe failed");
        return;
    }

    int pid = fork();
    if (pid == 0) {
        setpgrp();
        ischild = true;
        close(pipe_fd[0]);
        if (dup2(pipe_fd[1], (iserr ? 2 : 1)) < 0) {
            perror("smash error: dup failed");
            exit(0);
        }

        SmallShell::executeCommand(part_a.c_str());

        if (close(pipe_fd[1]) < 0) {
            perror("smash error: close failed");
        }

        exit(0);
    }
    else {
        ischild = false;
        close(pipe_fd[1]);
        int old_out = dup(0);
        if (old_out == -1) {
            perror("smash error: dup failed");
            return;
        }
        if (dup2(pipe_fd[0], 0) < 0) {
            perror("smash error: dup failed");
            return;
        }

        SmallShell::executeCommand(part_b.c_str());
        if (close(pipe_fd[0]) < 0) {
            perror("smash error: close failed");
        }
        if (dup2(old_out, 0) < 0) {
            perror("smash error: dup failed");
            return;
        }
        close(old_out);
    }
};


int isInt(char* arg) {
    int result = -1;
    try {
        result = std::atoi(arg);
    }
    catch (std::invalid_argument&) {
        return -1;
    }
    return result;
}


void TimeoutCommand::execute() {

    if(this->arg_size <= 2) {
        std::cerr << "smash error: timeout: invalid arguments" << std::endl;
        return;
    }
    if(isInt(args[1]) < 0) {
        std::cerr << "smash error: timeout: invalid arguments" << std::endl;
        return;
    }
    string new_cmd = string(this->cmd_line).substr( string(this->cmd_line).find_first_of(" ") + 1);
    new_cmd = new_cmd.substr(new_cmd.find_first_of(" ") + 1);

    pid_t pid = fork();
    if (pid == 0) {
        setpgrp();
        if (execvp(this->args[0], this->args) < 0) { /// need to adjust this for timeout
            perror("smash error: execvp failed");
        }
        exit(0);
    }
    else {
        int status;
        JobsList::addJob(this,pid,false);

        JobsList::addTime(this, pid);

        int job_id = JobsList::getLastJob(nullptr)->job_id;

        if (!this->is_bg) {

            SmallShell::cur_job = JobsList::getJobById(job_id);

            waitpid(pid, &status, WUNTRACED);
            if (!WIFSTOPPED(status)) {
                JobsList::removeJobById(job_id);
                SmallShell::cur_job = nullptr;
            }
        }
    }
}
