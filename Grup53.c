#define MAX_INPUT 1024
#define MAX_ARGS 64
#define MAX_ARKAPLAN 64

#include "Program.h"

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
    if (args[0] == NULL)
    {
        return;
    }

    if (strcmp(args[0], "cd") == 0)
    {

        if (args[1] == NULL)
        {
            fprintf(stderr, "cd: eksik giris \n");
        }
        else if (chdir(args[1]) != 0)
        {
            perror("cd");
        }
        return;
    }
    else if (strcmp(args[0], "Arka") == 0)
    {
        for (int i = 0; i < bg_count; i++)
        {
            printf("[%d] %d\n", i + 1, bg_processes[i]);
        }
        return;
    }
    else if (strcmp(args[0], "Ana") == 0)
    {

        if (bg_count > 0)
        {
            pid_t pid = bg_processes[--bg_count];
            int status;
            waitpid(pid, &status, 0);
        }
        else
        {
            fprintf(stderr, "Ana: arkaplanda calisan komut yoktur\n");
        }
        return;
    }
    else if (strcmp(args[0], "mkdir") == 0)
    {
        if (args[1] == NULL)
        {
            fprintf(stderr, "mkdir: eksik dizin ismi\n");
        }
        else
        {
            if (mkdir(args[1], 0755) != 0)
            {
                perror("mkdir");
            }
        }
        return;
    }
    // else if (strcmp(args[0], "ls") == 0)
    // {
    //     system("ls -p | grep -v /");
    // }

    pid_t pid = fork();
    if (pid == 0)
    {

        if (execvp(args[0], args) == -1)
        {
            perror("execvp");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {

        if (background)
        {
            bg_processes[bg_count++] = pid;
            printf("[%d] %d\n", bg_count, pid);
        }
        else
        {
            waitpid(pid, NULL, 0);
        }
    }
    else
    {
        perror("fork");
    }
}
void list_files()
{
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0)
    {

        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        execlp("ls", "ls", "-p", NULL);
        perror("execlp (ls)");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0)
    {

        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);

        execlp("grep", "grep", "-v", "/", NULL);
        perror("execlp (grep)");
        exit(EXIT_FAILURE);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void Virgul_boru(char *komutlar)
{
    char *command = strtok(komutlar, ";");
    char *args[MAX_ARGS]; // Declare args
    while (command != NULL)
    {
        giris(command, args); // Parse the command
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return;
        }
        if (pid == 0)
        {
            tek_Komut_isleme(args, Arkaplan(args)); // Pass args
        }

        waitpid(pid, NULL, 0);
        command = strtok(NULL, ";");
    }
}

void boru_isleme(char *sol, char *sag)
{
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        // Child process for the left command
        close(pipe_fd[0]);               // Close unused read end
        dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipe_fd[1]);

        if (strstr(sol, "<") != NULL)
        {
            giris_yonlendirme(sol);
            exit(EXIT_SUCCESS);
        }

        char *args[MAX_ARGS];
        giris(sol, args);
        if (execvp(args[0], args) == -1)
        {
            perror("execvp (sol)");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid1 < 0)
    {
        perror("fork (sol)");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        // Child process for the right command
        close(pipe_fd[1]);              // Close unused write end
        dup2(pipe_fd[0], STDIN_FILENO); // Redirect stdin to pipe
        close(pipe_fd[0]);

        if (strstr(sag, ">") != NULL)
        {
            cikis_yonlendirme(sag);
            exit(EXIT_SUCCESS);
        }

        char *args[MAX_ARGS];
        giris(sag, args);
        if (execvp(args[0], args) == -1)
        {
            perror("execvp (sag)");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid2 < 0)
    {
        perror("fork (sag)");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void linux_shell()
{
    char komut[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1)
    {
        printf("> ");
        fflush(stdout);

        if (fgets(komut, sizeof(komut), stdin) == NULL)
        {
            break;
        }

        size_t len = strlen(komut);
        if (len > 0 && komut[len - 1] == '\n')
        {
            komut[len - 1] = '\0';
        }
        if (strstr(komut, "|") != NULL)
        {
            char *sol = strtok(komut, "|");
            char *sag = strtok(NULL, "");
            if (sol != NULL && sag != NULL)
            {
                boru_isleme(sol, sag);
            }
            else
            {
                fprintf(stderr, "Hata: pipe kullanımı yanlış.\n");
            }
            continue;
        }
        if (strstr(komut, ";") != NULL)
        {

            Virgul_boru(komut);

            continue;
        }

        if (strstr(komut, "<") != NULL)
        {
            giris_yonlendirme(komut);
            continue;
        }

        if (strstr(komut, ">") != NULL)
        {
            cikis_yonlendirme(komut);
            continue;
        }

        if (strcmp(komut, "quit") == 0)
        {
            arkaplana_bekle();
            break;
        }
        if (strcmp(komut, "ls") == 0)
        {
            list_files();
        }

        giris(komut, args);
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
