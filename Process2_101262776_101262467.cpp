// Process2.cpp
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
using namespace std;

struct Shared {
    int multiple;
    int counter;
};

static void lock_sem(int semid) {
    sembuf op{0, -1, 0};
    if (semop(semid, &op, 1) == -1) { perror("semop lock"); exit(1); }
}
static void unlock_sem(int semid) {
    sembuf op{0, +1, 0};
    if (semop(semid, &op, 1) == -1) { perror("semop unlock"); exit(1); }
}

int main() {
    cout << "Process2 (Child via exec) started, PID = " << getpid() << endl;

    key_t shm_key = ftok(".", 0x42);
    if (shm_key == -1) { perror("ftok shm"); return 1; }
    int shmid = shmget(shm_key, sizeof(Shared), 0666);
    if (shmid == -1) { perror("shmget"); return 1; }
    auto* sh = (Shared*)shmat(shmid, nullptr, 0);
    if (sh == (void*)-1) { perror("shmat"); return 1; }

    key_t sem_key = ftok(".", 0x24);
    if (sem_key == -1) { perror("ftok sem"); return 1; }
    int semid = semget(sem_key, 1, 0666);
    if (semid == -1) { perror("semget"); return 1; }

    while (true) {
        lock_sem(semid);
        int c = sh->counter;
        unlock_sem(semid);
        if (c > 100) break;
        if (c % 25 == 0) fprintf(stderr, "[Process2 %d] waiting… counter=%d\n", getpid(), c);
        usleep(60000);
    }

    cout << "Process2: detected counter > 100 — starting work.\n";

    int t = 0;
    while (true) {
        lock_sem(semid);
        int m = sh->multiple;
        int c = sh->counter;
        bool done = (c > 500);
        if (!done) {
            cout << "[Process2 " << getpid() << "] counter=" << c << " (multiple=" << m << ")";
            if (c % m == 0) cout << "  -> multiple hit!";
            cout << endl;
        }
        unlock_sem(semid);
        if (done) break;
        usleep(94000 + (t++ % 5) * 4000);
    }

    if (shmdt(sh) == -1) perror("shmdt");
    cout << "Process2: finished (counter > 500). Exiting.\n";
    return 0;
}

