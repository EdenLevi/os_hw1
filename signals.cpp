#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    JobsList::JobEntry* job = SmallShell::cur_job;
    if (job == nullptr) {
        return;
    }
    job->stopped = true;
    kill(job->process_id, SIGSTOP);
    SmallShell::cur_job = nullptr;
    cout << "smash: process " << job->process_id << " was stopped" << endl;
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    JobsList::JobEntry* job = SmallShell::cur_job;
    if (job == nullptr) {
        return;
    }
    //JobsList::removeJobById(job->job_id);
    //JobsList::removeJobByIdDeep?(job_id); // this is timeout list
    kill(job->process_id, SIGKILL);
    SmallShell::cur_job = nullptr;
    cout << "smash: process " << job->process_id << " was killed" << std::endl;
}


void alarmHandler(int sig_num) {
    cout << "smash: got an alarm" << endl;
    JobsList::removeFinishedJobs();
    JobsList::removeFinishedTimes();
    //JobsList::JobEntry* timeout_job = smash.timeoutlist.getFirst();

    JobsList::JobEntry* firstTime = JobsList::getFirstTime();
    int job_id = firstTime->job_id;
    if (SmallShell::cur_job != nullptr && SmallShell::cur_job->job_id == job_id) {
        SmallShell::cur_job = nullptr;
    }
    if(firstTime->process_id != SmallShell::cur_job->process_id) {
        kill(firstTime->process_id, SIGKILL);
        cout<<"ugabuga"<<endl;
    }
    std::cout << "smash: " << firstTime->command->cmd_line << " timed out!" << std::endl;

    //SmallShell::timeoutlist.removeJobById(job_id, false);
    //JobsList::removeJobById(job_id);
    //SmallShell::timeoutlist.sortTimeout();
    //SmallShell::timeoutlist.setAlarm();
}


