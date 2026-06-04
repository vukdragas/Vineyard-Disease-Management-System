#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "VineyardTracker.h"

#define DAMAGE_THRESHOLD 30.0f

volatile sig_atomic_t got_sigusr1 = 0;
volatile sig_atomic_t got_sigusr2 = 0;

static void sigusr1_handler(int s) { (void)s; got_sigusr1 = 1; }
static void sigusr2_handler(int s) { (void)s; got_sigusr2 = 1; }

static void pipe_write_record(int fd, const Record *r) {
    write(fd, r, sizeof(Record));
}
static int pipe_read_record(int fd, Record *r) {
    return read(fd, r, sizeof(Record)) == (ssize_t)sizeof(Record);
}
static void out(const char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}

int main(void) {
    load_records();
    if (num_records == 0) { write(STDOUT_FILENO, "No records.\n", 12); return 0; }

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigusr1_handler;
    sigaction(SIGUSR1, &sa, NULL);
    sa.sa_handler = sigusr2_handler;
    sigaction(SIGUSR2, &sa, NULL);

    sigset_t block_mask, old_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    sigaddset(&block_mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &block_mask, &old_mask);

    pid_t inspector_pid = getpid();

    Record destroy_list[MAX_RECORDS], treat_list[MAX_RECORDS];
    int destroy_count = 0, treat_count = 0;
    for (int i = 0; i < num_records; i++) {
        if (records[i].damage >= DAMAGE_THRESHOLD)
            destroy_list[destroy_count++] = records[i];
        else
            treat_list[treat_count++] = records[i];
    }

    char buf[2048];
    snprintf(buf, sizeof(buf), "[Inspector] Damage threshold : %.0f%%\n", DAMAGE_THRESHOLD);
    out(buf);
    snprintf(buf, sizeof(buf), "[Inspector] Plots to DESTROY : %d\n", destroy_count);
    out(buf);
    snprintf(buf, sizeof(buf), "[Inspector] Plots to TREAT   : %d\n\n", treat_count);
    out(buf);

    int pipe_destroy[2], pipe_treat[2];
    pipe(pipe_destroy);
    pipe(pipe_treat);

    /* ── Fork 1: Destruction Officer ── */
    pid_t pid_destroy = fork();
    if (pid_destroy == 0) {
        sigprocmask(SIG_SETMASK, &old_mask, NULL);
        close(pipe_destroy[1]);
        close(pipe_treat[0]);
        close(pipe_treat[1]);

        /* Signal inspector: ready to receive data */
        kill(inspector_pid, SIGUSR1);

        Record rbuf[MAX_RECORDS];
        int cnt = 0;
        while (cnt < MAX_RECORDS && pipe_read_record(pipe_destroy[0], &rbuf[cnt]))
            cnt++;
        close(pipe_destroy[0]);

        char line[2048];
        snprintf(line, sizeof(line),
            "[Destruction Officer] Received %d plot(s) for destruction:\n\n", cnt);
        out(line);

        for (int i = 0; i < cnt; i++) {
            Record *r = &rbuf[i];
            snprintf(line, sizeof(line),
                "  [%d] %s | %s | %s | %s | %d sq.fathoms | %s | %.1f%%\n",
                i+1, r->location, r->site, r->plot, r->variety,
                r->area, TYPE_NAMES[r->vineyard_type], r->damage);
            out(line);
        }
        out("\n");

        for (int i = 0; i < cnt; i++) {
            Record *r = &rbuf[i];
            snprintf(line, sizeof(line),
                "[Destruction Officer] Dear Chief Engineer of the %s plot, %s, %s.\n"
                "  By order of the national vineyard inspector, within the framework\n"
                "  of the control measures against Flavescence doree, I hereby request\n"
                "  the IMMEDIATE DESTRUCTION of the plot.\n\n",
                r->plot, r->site, r->location);
            out(line);
        }

        out("[Destruction Officer] All plots destroyed.\n\n");

        /* Signal inspector: destruction complete */
        kill(inspector_pid, SIGUSR2);
        exit(0);
    }

    /* ── Fork 2: Control Officer ── */
    pid_t pid_control = fork();
    if (pid_control == 0) {
        sigprocmask(SIG_SETMASK, &old_mask, NULL);
        close(pipe_treat[1]);
        close(pipe_destroy[0]);
        close(pipe_destroy[1]);

        /* Wait for inspector's SIGUSR2 "go ahead" signal */
        sigset_t wait_mask;
        sigfillset(&wait_mask);
        sigdelset(&wait_mask, SIGUSR2);
        while (!got_sigusr2)
            sigsuspend(&wait_mask);

        /* Signal inspector: ready to receive data */
        kill(inspector_pid, SIGUSR1);

        Record rbuf[MAX_RECORDS];
        int cnt = 0;
        while (cnt < MAX_RECORDS && pipe_read_record(pipe_treat[0], &rbuf[cnt]))
            cnt++;
        close(pipe_treat[0]);

        char line[2048];
        snprintf(line, sizeof(line),
            "[Control Officer] Received %d plot(s) for treatment:\n\n", cnt);
        out(line);

        for (int i = 0; i < cnt; i++) {
            Record *r = &rbuf[i];
            snprintf(line, sizeof(line),
                "  [%d] %s | %s | %s | %s | %d sq.fathoms | %s | %.1f%%\n",
                i+1, r->location, r->site, r->plot, r->variety,
                r->area, TYPE_NAMES[r->vineyard_type], r->damage);
            out(line);
        }
        out("\n");

        for (int i = 0; i < cnt; i++) {
            Record *r = &rbuf[i];
            snprintf(line, sizeof(line),
                "[Control Officer] Dear Chief Engineer of the %s plot, %s, %s.\n"
                "  The nationwide large spring chemical treatment is about to begin.\n"
                "  Please do NOT initiate any local spraying or protective measures.\n\n",
                r->plot, r->site, r->location);
            out(line);
        }

        out("[Control Officer] All wine communities visited.\n\n");

        /* Signal inspector: treatment complete */
        kill(inspector_pid, SIGUSR2);
        exit(0);
    }

    /* ── Parent: Inspector ── */
    close(pipe_destroy[0]);
    close(pipe_treat[0]);

    sigset_t wait_mask;
    sigfillset(&wait_mask);
    sigdelset(&wait_mask, SIGUSR1);
    sigdelset(&wait_mask, SIGUSR2);

    /* Step 1: wait for Destruction Officer "ready" (SIGUSR1) */
    out("[Inspector] Waiting for Destruction Officer...\n");
    while (!got_sigusr1) sigsuspend(&wait_mask);
    got_sigusr1 = 0;

    /* Send destroy list */
    for (int i = 0; i < destroy_count; i++)
        pipe_write_record(pipe_destroy[1], &destroy_list[i]);
    close(pipe_destroy[1]);

    /* Step 2: wait for Destruction Officer "done" (SIGUSR2) */
    while (!got_sigusr2) sigsuspend(&wait_mask);
    got_sigusr2 = 0;
    waitpid(pid_destroy, NULL, 0);
    out("[Inspector] Destruction Officer done. Releasing Control Officer...\n\n");

    /* Step 3: release Control Officer */
    kill(pid_control, SIGUSR2);

    /* Step 4: wait for Control Officer "ready" (SIGUSR1) */
    out("[Inspector] Waiting for Control Officer...\n");
    while (!got_sigusr1) sigsuspend(&wait_mask);
    got_sigusr1 = 0;

    /* Send treat list */
    for (int i = 0; i < treat_count; i++)
        pipe_write_record(pipe_treat[1], &treat_list[i]);
    close(pipe_treat[1]);

    /* Step 5: wait for Control Officer "done" (SIGUSR2) */
    while (!got_sigusr2) sigsuspend(&wait_mask);
    got_sigusr2 = 0;
    waitpid(pid_control, NULL, 0);
    out("[Inspector] Control Officer done.\n\n");

    snprintf(buf, sizeof(buf),
        "\n"
        "  STATUS REPORT  -  Minister of Agriculture\n"
        "\n"
        "  Damage threshold  : %.0f%%\n"
        "  Total plots       : %d\n"
        "  Destroyed         : %d  (damage >= %.0f%%)\n"
        "  Treated           : %d  (damage <  %.0f%%)\n"
        "  Destruction Officer : COMPLETED\n"
        "  Control Officer     : COMPLETED\n"
        "\n",
        DAMAGE_THRESHOLD, num_records,
        destroy_count, DAMAGE_THRESHOLD,
        treat_count, DAMAGE_THRESHOLD);
    out(buf);

    return 0;
}
