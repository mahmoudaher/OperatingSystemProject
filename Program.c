#ifndef _unix_
#define _unix_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
// #include "Program.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define MAX_ARKAPLAN 64

int bg_processes[MAX_ARKAPLAN];
int bg_count = 0;
void giris(char *input, char **args)
{
    char *token = strtok(input, " \t\n");
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}
int Arkaplan(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "&") == 0)
        {
            args[i] = NULL;
            return 1;
        }
    }
    return 0;
}

void Arkaplan_ekle(pid_t pid)
{
    if (bg_count < MAX_ARKAPLAN)
    {
        bg_processes[bg_count++] = pid;
    }
    else
    {
        fprintf(stderr, "Arka plan islem kapasitesi doldu.\n");
    }
}

void Komut_isleme(char **args, int background)
{
    pid_t pid = fork();
    int status;

    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("Komut isleme hata cikti");
            exit(EXIT_FAILURE); // Exit with failure code
        }
    }
    else if (pid > 0)
    {
        // Parent process
        if (background)
        {
            printf("[%d] Arka plan da calisiyor\n", pid);
            Arkaplan_ekle(pid);
        }
        else
        {
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
            {
                int exit_code = WEXITSTATUS(status);
                printf("[%d] retval: %d\n", pid, exit_code);
            }
            else if (WIFSIGNALED(status))
            {
                int signal_number = WTERMSIG(status);
                printf("[%d] terminated by signal: %d\n", pid, signal_number);
            }
        }
    }
    else
    {
        // Fork failed
        perror("Fork islem hatasi oldu");
    }
}

void Arkaplan_cikar(pid_t pid)
{
    for (int i = 0; i < bg_count; i++)
    {
        if (bg_processes[i] == pid)
        {
            bg_processes[i] = bg_processes[--bg_count];
            return;
        }
    }
}

void arkaplan_kontrol()
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printf("[arka plan processi%d bitti]\n", pid);
        Arkaplan_cikar(pid);
    }
}

void arkaplana_bekle()
{
    printf("Arkaplan komutlari bitmesine bekleniyor\n");
    for (int i = 0; i < bg_count; i++)
    {
        int status;
        waitpid(bg_processes[i], &status, 0);
        printf("[arka plan processi%d bitti]\n", bg_processes[i]);
    }
    bg_count = 0;
}

void linux_shell()
{
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    while (1)
    {

        if (fgets(input, MAX_INPUT, stdin) == NULL)
        {
            break;
        }

        giris(input, args);

        if (args[0] == NULL)
        {
            continue;
        }

        arkaplan_kontrol();

        if (strcmp(args[0], "quit") == 0)
        {
            arkaplana_bekle();
            break;
        }

        int background = Arkaplan(args);
        Komut_isleme(args, background);
    }
}

int main()
{
    linux_shell();
    printf("Kabuktan Cikildi.\n");
    return 0;
}

#endif
