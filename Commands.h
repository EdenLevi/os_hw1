#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <list>

using namespace std;


#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
// TODO: Add your data members
public:
    char *cmd_line_new;
    char *cmd_line;
    bool is_bg;
    char** args;
    int arg_size;

    explicit Command(const char *cmd_line);

    virtual ~Command();

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    explicit BuiltInCommand(const char *cmd_line);

    virtual ~BuiltInCommand() {

    }
};

class ExternalCommand : public Command {
public:
    explicit ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand() {}

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    explicit ChangePromptCommand(const char *cmd_line);

    virtual ~ChangePromptCommand() {}

    void execute() override;
};

class FareCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    explicit FareCommand(const char *cmd_line);

    virtual ~FareCommand() {}

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
// TODO: Add your data members public:
    static std::list<string> path_history;

    explicit ChangeDirCommand(const char *cmd_line); // char** plastPwd was here
    virtual ~ChangeDirCommand() {}

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    explicit GetCurrDirCommand(const char *cmd_line);

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    explicit ShowPidCommand(const char *cmd_line);

    virtual ~ShowPidCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {}

    void execute() override;
};

class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    public:
        int job_id;
        Command* command;
        int process_id;
        bool stopped;
        time_t time_s;

        JobEntry(int job_id, Command* command, int process_id, bool stopped,time_t time_s = time(0)) : job_id(job_id),
                                                                                          command(command),
                                                                                          process_id(process_id),
                                                                                          stopped(stopped),
                                                                                          time_s(time_s) {};

        ~JobEntry();
    };

    static vector<JobEntry *> jobs;
    static vector<JobEntry *> times;
    // TODO: Add your data members
public:
    //JobsList();

    //~JobsList();

    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(const char *cmd_line);

    virtual ~BackgroundCommand() {}

    void execute() override;
};

class TimeoutCommand : public Command {
/* Optional */
// TODO: Add your data members
public:
    explicit TimeoutCommand(const char *cmd_line);

    virtual ~TimeoutCommand() {}

    void execute() override;
};


class SetcoreCommand : public BuiltInCommand {
    /* Optional */
    // TODO: Add your data members
public:
    SetcoreCommand(const char *cmd_line);

    virtual ~SetcoreCommand() {}

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    /* Bonus */
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {}

    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members
    SmallShell();

public:
    static string prompt;
    static JobsList::JobEntry * cur_job;
    static Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator

    ~SmallShell();

    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
