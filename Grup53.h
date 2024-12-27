#ifndef PROGRAM_H
#define PROGRAM_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>

void giris(char *, char **);
void boru_isleme(char *, char *);
void tek_Komut_isleme(char **, int);
void Arkaplan_ekle(pid_t);
void arkaplan_kontrol();
void arkaplana_bekle();
void list_files();
void Arkaplan_cikar(pid_t);
int Arkaplan(char **);
void Virgul_boru(char *);
void linux_shell();
void giris_yonlendirme(char *);
void cikis_yonlendirme(char *);
#endif
