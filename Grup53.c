/*
   Ahmed bahaa Ahmed momtaz 2-B G221210569
   Mahmoud Aldaher 2-C G221210588
   Göktuğ Yüceer 1-A B221210058
   Khalid almuanen 2-B G211210575
*/

#define MAX_INPUT 1024  // Maksimum girdi boyutu
#define MAX_ARGS 64     // Maksimum argüman sayısı
#define MAX_ARKAPLAN 64 // Maksimum arka plan süreci sayısı

#include "Grup53.h"

// Arka planda çalışan süreçlerin PID'lerini saklayan dizi ve sayaç
int bg_processes[MAX_ARKAPLAN];
int bg_count = 0;

// Komut satırından girilen girdiyi ayrıştırarak argümanlara böler
void giris(char *input, char **args)
{
    char *token = strtok(input, " \t\n"); // Komut satırını tab, boşluk ve yeni satır karakterine göre ayır
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token; // Her bir argüman diziye eklenir
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL; // Argüman listesinin sonunu belirtmek için NULL eklenir
}

// Komutun arka planda çalışıp çalışmayacağını kontrol eder
int Arkaplan(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "&") == 0) // Komutun sonunda '&' varsa arka planda çalışacak
        {
            args[i] = NULL; // '&' işareti komut listesinden kaldırılır
            return 1;       // Arka plan komutu işaret edilir
        }
    }
    return 0; // Arka plan komutu yoksa 0 döner
}

// Yeni bir arka plan süreci ekler
void Arkaplan_ekle(pid_t pid)
{
    if (bg_count < MAX_ARKAPLAN)
    {
        bg_processes[bg_count++] = pid; // PID diziye eklenir
    }
    else
    {
        fprintf(stderr, "Arka plan islem kapasitesi doldu.\n"); // Maksimum arka plan süreci sayısı aşıldı
    }
}

// Belirtilen PID'yi arka plan süreç listesinden çıkarır
void Arkaplan_cikar(pid_t pid)
{
    for (int i = 0; i < bg_count; i++)
    {
        if (bg_processes[i] == pid)
        {
            bg_processes[i] = bg_processes[--bg_count]; // PID bulunduğunda listeden çıkarılır ve liste sıkıştırılır
            return;
        }
    }
}

// Tamamlanmış arka plan süreçlerini kontrol eder
void arkaplan_kontrol()
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) // Arka planda tamamlanmış süreçleri bekler
    {
        printf("[arka plan processi %d bitti]\n", pid);
        Arkaplan_cikar(pid); // Tamamlanan süreç listeden çıkarılır
    }
}

// Arka planda çalışan tüm süreçlerin tamamlanmasını bekler
void arkaplana_bekle()
{
    printf("Arkaplan komutlarinin bitmesi bekleniyor\n");
    for (int i = 0; i < bg_count; i++)
    {
        int status;
        waitpid(bg_processes[i], &status, 0); // Tüm arka plan süreçlerini bekler
        printf("[arka plan processi %d bitti]\n", bg_processes[i]);
    }
    bg_count = 0; // Tüm süreçler tamamlandıktan sonra sayaç sıfırlanır
}

// Giriş yönlendirmesi yapan fonksiyon
void giris_yonlendirme(char *command)
{
    char *args[64];
    char *input_file = NULL;
    int i = 0;

    char *token = strtok(command, " ");
    while (token != NULL)
    {
        if (strcmp(token, "<") == 0) // Giriş yönlendirme operatörü bulunduğunda
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                fprintf(stderr, "Hata: Giris dosyasi belirtilmedi.\n");
                return;
            }
            input_file = token; // Giriş dosyası belirlenir
            break;
        }
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (input_file == NULL)
    {
        fprintf(stderr, "Hata: Giris yonlendirme operatoru '<' eksik.\n");
        return;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Fork hatasi");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        int fd = open(input_file, O_RDONLY); // Giriş dosyası açılır
        if (fd < 0)
        {
            fprintf(stderr, "Giris dosyasi bulunamadi: %s\n", input_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO); // Standart girdiyi dosyadan okur
        close(fd);
        if (execvp(args[0], args) == -1)
        {
            perror("Komut calistirma hatasi");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        wait(NULL); // Ana süreç çocuk süreci tamamlanana kadar bekler
    }
}

// Çıkış yönlendirmesi yapan fonksiyon
void cikis_yonlendirme(char *command)
{
    char *args[MAX_ARGS];
    char *output_file = NULL;
    int arg_index = 0;

    char *token = strtok(command, " ");
    while (token != NULL)
    {
        if (strcmp(token, ">") == 0) // Çıkış yönlendirme operatörü bulunursa
        {
            token = strtok(NULL, " ");
            if (token == NULL)
            {
                fprintf(stderr, "Hata: Cikisi dosyasi belirtilmedi.\n");
                return;
            }
            output_file = token; // Çıkış dosyası belirlenir
            break;
        }
        else
        {
            args[arg_index++] = token; // Komut argümanları toplanır
        }
        token = strtok(NULL, " ");
    }
    args[arg_index] = NULL;

    if (args[0] == NULL)
    {
        fprintf(stderr, "Hata: Komut belirtilmedi.\n"); // Komut girilmemişse hata verir
        return;
    }

    if (output_file == NULL)
    {
        fprintf(stderr, "Hata: Cikis yonlendirme operatoru '>' eksik.\n");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) // Çocuk süreç oluşturulur
    {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Çıkış dosyası oluşturulur veya sıfırlanır
        if (fd < 0)
        {
            perror("Cikis dosyasi acilamadi.");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDOUT_FILENO) < 0) // Çıkış standard out'a yönlendirilir
        {
            perror("Dosya tanimlayicisi kopyalanamadi");
            close(fd);
            exit(EXIT_FAILURE);
        }
        close(fd);

        if (execvp(args[0], args) < 0)
        {
            perror("Komut calistirilamadi");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0)
    {
        wait(NULL); // Ana süreç çocuk süreci bekler
    }
    else
    {
        perror("Fork islemi basarisiz"); // Fork hatası durumunda hata mesajı verir
    }
}
// Komut işleme fonksiyonu
void tek_Komut_isleme(char **args, int background)
{
    if (args[0] == NULL) // Argüman yoksa fonksiyon sona erer
    {
        return;
    }

    // 'cd' komutu için dızın değiştirme
    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL) // Dizin adı eksikse hata verir
        {
            fprintf(stderr, "cd: eksik giris \n");
        }
        else if (chdir(args[1]) != 0) // Dizin değiştirme hatası durumunda hata verir
        {
            perror("cd");
        }
        return;
    }
    // Arka plan süreçlerini listeleme komutu
    else if (strcmp(args[0], "Arka") == 0)
    {
        for (int i = 0; i < bg_count; i++)
        {
            printf("[%d] %d\n", i + 1, bg_processes[i]);
        }
        return;
    }
    // Arka planda çalışan süreci öne alır
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
    // 'mkdir' komutu için yeni dizin oluşturma
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

    pid_t pid = fork(); // Yeni bir süreç oluştur
    if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            perror("execvp"); // Komut hatalıysa hata mesajı yazdır
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
        perror("fork"); // Fork hatası varsa hata mesajı verir
    }
}

// 'ls' komutuyla dosya listeleme ve 'grep' komutuyla dizinleri filtreleme
void list_files()
{
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1) // Pipe oluşturulur
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork(); // Birinci çocuk süreci oluşturulur
    if (pid1 == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) // Birinci çocuk süreci ('ls' komutunu çalıştır)
    {
        close(pipe_fd[0]);               // Okuma ucunu kapat
        dup2(pipe_fd[1], STDOUT_FILENO); // Pipe yazma ucunu stdout'a yönlendir
        close(pipe_fd[1]);               // Pipe'in yazma ucunu kapat

        execlp("ls", "ls", "-p", NULL); // 'ls -p' komutunu çalıştır
        perror("execlp (ls)");          // Hata olursa mesaj bas
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork(); // İkinci çocuk süreci oluşturulur
    if (pid2 == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) // İkinci çocuk süreci ('grep' komutunu çalıştır)
    {
        close(pipe_fd[1]);              // Yazma ucunu kapat
        dup2(pipe_fd[0], STDIN_FILENO); // Pipe okuma ucunu stdin'e yönlendir
        close(pipe_fd[0]);              // Pipe'in okuma ucunu kapat

        execlp("grep", "grep", "-v", "/", NULL); // 'grep -v /' komutunu çalıştır
        perror("execlp (grep)");
        exit(EXIT_FAILURE);
    }

    close(pipe_fd[0]); // Pipe'in okuma ucunu kapat
    close(pipe_fd[1]); // Pipe'in yazma ucunu kapat

    waitpid(pid1, NULL, 0); // Birinci çocuk sürecini bekle
    waitpid(pid2, NULL, 0); // İkinci çocuk sürecini bekle
}

void Virgul_boru(char *komutlar)
{
    char *command = strtok(komutlar, ";"); // Komutları ';' ile ayır
    char *args[MAX_ARGS];

    while (command != NULL)
    {
        pid_t pid = fork(); // Her komut için ayrı süreç oluştur

        if (pid < 0)
        {
            perror("fork");
            return;
        }

        if (pid == 0) // Çocuk süreçte çalıştır
        {
            giris(command, args);                   // Komutu ayrıştır
            tek_Komut_isleme(args, Arkaplan(args)); // Komutu çalıştır
            exit(EXIT_SUCCESS);                     // Çocuk süreç tamamlandığında çık
        }
        else
        {
            waitpid(pid, NULL, 0); // Her komutun bitmesini bekle
        }

        // Bir sonraki komuta geç
        command = strtok(NULL, ";");
    }
}

void boru_isleme(char *komutlar[], int komut_sayisi)
{
    int pipe_fd[komut_sayisi - 1][2];

    // Gerekli pipe'ları oluştur
    for (int i = 0; i < komut_sayisi - 1; i++)
    {
        if (pipe(pipe_fd[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Her komut için fork ve execvp
    for (int i = 0; i < komut_sayisi; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        { // Çocuk süreç
            if (i > 0)
            {
                dup2(pipe_fd[i - 1][0], STDIN_FILENO); // Önceki pipe'dan oku
            }
            if (i < komut_sayisi - 1)
            {
                dup2(pipe_fd[i][1], STDOUT_FILENO); // Sonraki pipe'a yaz
            }

            // Pipe uçlarını kapat
            for (int j = 0; j < komut_sayisi - 1; j++)
            {
                close(pipe_fd[j][0]);
                close(pipe_fd[j][1]);
            }

            char *args[MAX_ARGS];
            giris(komutlar[i], args); // Komutu ayrıştır
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Ebeveyn süreç pipe uçlarını kapatır
    for (int i = 0; i < komut_sayisi - 1; i++)
    {
        close(pipe_fd[i][0]);
        close(pipe_fd[i][1]);
    }

    // Tüm çocuk süreçlerin tamamlanmasını bekle
    for (int i = 0; i < komut_sayisi; i++)
    {
        wait(NULL);
    }
}

// Basit bir komut satırı kabuğu uygular
void linux_shell()
{
    char komut[MAX_INPUT]; // Kullanıcıdan gelen komutları saklar
    char *args[MAX_ARGS];

    while (1)
    {
        printf("> "); // Komut satırı işareti
        fflush(stdout);

        if (fgets(komut, sizeof(komut), stdin) == NULL) // Komut al, hata durumunda döngüden çık
        {
            break;
        }

        size_t len = strlen(komut);
        if (len > 0 && komut[len - 1] == '\n')
        {
            komut[len - 1] = '\0'; // Yeni satır karakterini kaldır
        }

        // Pipe komutlarını işler
        if (strstr(komut, "|") != NULL)
        {
            char *komutlar[MAX_ARGS];
            int komut_sayisi = 0;

            // '|' ile ayır ve komutları diziye ekle
            char *token = strtok(komut, "|");
            while (token != NULL && komut_sayisi < MAX_ARGS - 1)
            {
                komutlar[komut_sayisi++] = token;
                token = strtok(NULL, "|");
            }
            komutlar[komut_sayisi] = NULL;

            // Birden fazla pipe işlemi yap
            if (komut_sayisi > 1)
            {
                boru_isleme(komutlar, komut_sayisi);
            }
            else
            {
                fprintf(stderr, "Hata: Pipe kullanimi hatali.\n");
            }
            continue;
        }

        // Noktalı virgül komutlarını sırayla işler
        if (strstr(komut, ";") != NULL)
        {
            Virgul_boru(komut);
            continue;
        }

        // Giriş yönlendirmesini işler
        if (strstr(komut, "<") != NULL)
        {
            giris_yonlendirme(komut);
            continue;
        }

        // Çıkış yönlendirmesini işler
        if (strstr(komut, ">") != NULL)
        {
            cikis_yonlendirme(komut);
            continue;
        }

        // Kabuktan çıkış komutu
        if (strcmp(komut, "quit") == 0)
        {
            arkaplana_bekle(); // Arka plan süreçlerini bekle
            break;
        }

        // ls komutu için dosya listeleme
        if (strcmp(komut, "ls") == 0)
        {
            list_files();
        }

        giris(komut, args); // Komutu ayrıştır
        arkaplan_kontrol(); // Arka plan süreçlerini kontrol et
        int background = Arkaplan(args);
        tek_Komut_isleme(args, background); // Komutu çalıştır
    }
}

int main()
{
    linux_shell();
    printf("Kabuktan cikildi.\n");
    return 0;
}
