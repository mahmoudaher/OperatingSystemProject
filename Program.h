#ifndef PROGRAM_H
#define PROGRAM_H
void giris(char *, char **);

void tek_Komut_isleme(char **, int);
void Arkaplan_ekle(pid_t);
void arkaplan_kontrol();
void arkaplana_bekle();
void Arkaplan_cikar(pid_t);
int Arkaplan(char **);
void linux_shell();
void giris_yonlendirme(char *);
void cikis_yonlendirme(char *);
#endif