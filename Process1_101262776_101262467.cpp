// Process1.cpp
//GABRIEL BUGARIJA 101262776
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>   
using namespace std;

struct Shared {
    int multiple;
    int counter;
};

static void lock_sem(int semid) {
    sembuf op{0, -1, 0};
    if (semop(semid, &op, 1) == -1) { 
        perror("semop lock"); exit(1); }
}
static void unlock_sem(int semid) {
    sembuf op{0, +1, 0};
    if (semop(semid, &op, 1) == -1) { 
        perror("semop unlock"); exit(1); }
}

int main(int argc, char** argv) {
    cout << "Process1 (Parent) started, PID = " << getpid() << endl;

    key_t shm_key = ftok(".", 0x42);
    if (shm_key == -1) { perror("ftok shm"); return 1; }
    int shmid = shmget(shm_key, sizeof(Shared), IPC_CREAT | 0666);

    if (shmid == -1) { 
        perror("shmget"); return 1; }

    auto* sh = (Shared*)shmat(shmid, nullptr, 0);
    if (sh == (void*)-1) { 
        perror("shmat"); return 1; }

    key_t sem_key = ftok(".", 0x24);
    if (sem_key == -1) { 
        perror("ftok sem"); return 1; }

    int semid = semget(sem_key, 1, IPC_CREAT | 0666);
    if (semid == -1) { 
        perror("semget"); return 1; }

    if (semctl(semid, 0, SETVAL, 1) == -1) {
         perror("semctl SETVAL"); return 1; }

    lock_sem(semid);
    sh->multiple = (argc > 1 ? atoi(argv[1]) : 3);
    if (sh->multiple <= 0) sh->multiple = 3;
    sh->counter = 0;
    unlock_sem(semid);

    pid_t kid = fork();
    if (kid < 0) { perror("fork"); return 1; }
    if (kid == 0) {
        char* args[] = {(char*)"./Process2", nullptr};
        execvp(args[0], args);
        perror("execvp Process2");
        _exit(1);
    }

    cout << "Process1: running and incrementing shared counter..." << endl;

    int tick = 0;
    while (true) {
        lock_sem(semid);
        int m = sh->multiple;
        int c = sh->counter + 1;
        sh->counter = c;
        cout << "[Process1 " << getpid() << "] counter=" << c << " (multiple=" << m << ")";
        if (c % m == 0) cout << "  -> multiple hit!";
        cout << endl;
        bool done = (c > 500);
        unlock_sem(semid);

        if (done) break;
        usleep(78000 + (tick++ % 4) * 5000);
    }

    cout << "Process1: reached counter > 500. Finishing..." << endl;

    int status = 0;
    waitpid(kid, &status, 0);

    if (shmdt(sh) == -1) perror("shmdt");
    if (shmctl(shmid, IPC_RMID, nullptr) == -1) perror("shmctl IPC_RMID");
    if (semctl(semid, 0, IPC_RMID) == -1) perror("semctl IPC_RMID");

    cout << "Process1: cleanup done. Exiting.\n";
    return 0;
}
