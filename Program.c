#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
// #include program.h

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
        printf("[arka plan processi %d bitti]\n", pid);
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
        printf("[arka plan processi %d bitti]\n", bg_processes[i]);
    }
    bg_count = 0;
}

void giris_yonlendirme(char *command)
{
    char *args[64];
    char *input_file = NULL;
    int i = 0;

    char *token = strtok(command, " ");
    while (token != NULL)
    {
        if (strcmp(token, "<") == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                fprintf(stderr, "Hata: Giriş dosyası belirtilmedi.\n");
                return;
            }
            input_file = token;
            break;
        }
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (input_file == NULL)
    {
        fprintf(stderr, "Hata: Giriş yönlendirme operatörü '<' eksik.\n");
        return;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Fork hatası");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        int fd = open(input_file, O_RDONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Giriş dosyası bulunamadı: %s\n", input_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
        if (execvp(args[0], args) == -1)
        {
            perror("Komut çalıştırma hatası");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        wait(NULL);
    }
}

void cikis_yonlendirme(char *command)
{
    char *args[MAX_ARGS];
    char *output_file = NULL;
    int arg_index = 0;

    char *token = strtok(command, " ");
    while (token != NULL)
    {
        if (strcmp(token, ">") == 0)
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                fprintf(stderr, "Hata: Çıkış dosyası belirtilmedi.\n");
                return;
            }
            output_file = token;
            break;
        }
        else
        {
            args[arg_index++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[arg_index] = NULL;

    if (args[0] == NULL)
    {
        fprintf(stderr, "Hata: Komut belirtilmedi.\n");
        return;
    }

    if (output_file == NULL)
    {
        fprintf(stderr, "Hata: Çıkış yönlendirme operatörü '>' eksik.\n");
        return;
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
        {
            perror("Çıkış dosyası açılamadı");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            perror("Dosya tanımlayıcısı kopyalanamadı");
            close(fd);
            exit(EXIT_FAILURE);
        }
        close(fd);

        if (execvp(args[0], args) < 0)
        {
            perror("Komut çalıştırılamadı");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("Fork işlemi başarısız");
    }
}

void tek_Komut_isleme(char **args, int background)
{
    pid_t pid = fork();
    int status;

    if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            perror("Komut çalıştırma hatası");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0)
    {
        if (background)
        {
            printf("[%d] Arka planda çalışıyor\n", pid);
            Arkaplan_ekle(pid);
        }
        else
        {
            waitpid(pid, &status, 0);
        }
    }
    else
    {
        perror("Fork hatası");
    }
}

void linux_shell()
{
    char command[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1)
    {
        printf("> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }

        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n')
        {
            command[len - 1] = '\0';
        }

        if (strstr(command, "<") != NULL)
        {
            giris_yonlendirme(command);
            continue;
        }

        if (strstr(command, ">") != NULL)
        {
            cikis_yonlendirme(command);
            continue;
        }

        if (strcmp(command, "quit") == 0)
        {
            arkaplana_bekle();
            break;
        }

        giris(command, args);
        arkaplan_kontrol();
        int background = Arkaplan(args);
        tek_Komut_isleme(args, background);
    }
}

int main()
{
    linux_shell();
    printf("Kabuktan çıkıldı.\n");
    return 0;
}
