/*
 * Entrega 4 — o shell mede o sistema.
 *
 * Tres embutidos que rodam um comando e, DEPOIS que ele termina, contam o que
 * o sistema registrou. Tudo sai na saida de ERRO, para nao sujar a saida do
 * comando medido:
 *
 *   tempo   <cmd>   parede=<s> usuario=<s> sistema=<s>
 *   trocas  <cmd>   voluntarias=<n> involuntarias=<n>
 *   paginas <cmd>   menores=<n> maiores=<n> rss_pico_kb=<n>
 *
 * A chave e o wait4(): igual ao waitpid, mas com um quarto argumento
 * `struct rusage *`, que o nucleo preenche com a contabilidade do filho —
 * ru_utime, ru_stime, ru_nvcsw, ru_nivcsw, ru_minflt, ru_majflt, ru_maxrss.
 * O tempo de parede nao esta la: e voce quem mede, com clock_gettime, antes e
 * depois.
 *
 * O que se aprende aqui, e o que a apresentacao vai cobrar: um programa que so
 * calcula e um que so espera produzem numeros OPOSTOS. Se os seus derem
 * parecido, a medicao esta morta.
 *
 * Leiam antes: CONTRATOS.md secao 6, e entregas/E4.md.
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "comum.h"

#include <unistd.h>
#include <signal.h>

int aplicar_redir(struct redir *r);   /* estao em executar.c */
int codigo_de(int status);

static double seg(struct timeval t)   /* timeval -> segundos */
{
    return t.tv_sec + t.tv_usec / 1e6;
}

int medir(const char *qual, char *v[], int n, struct redir *r)
{
    (void)n;
    struct timespec ini, fim;
    clock_gettime(CLOCK_MONOTONIC, &ini);     /* hora de inicio (parede) */

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {                           /* FILHO: igual a Entrega 1 */
        signal(SIGINT, SIG_DFL);
        if (aplicar_redir(r) < 0) _exit(1);
        execvp(v[0], v);
        fprintf(stderr, "meushell: comando nao encontrado: %s\n", v[0]);
        _exit(127);
    }

    int status;
    struct rusage ru;
    wait4(pid, &status, 0, &ru);              /* espera E recebe a ficha do filho */
    clock_gettime(CLOCK_MONOTONIC, &fim);     /* hora do fim */

    double parede = (fim.tv_sec - ini.tv_sec) + (fim.tv_nsec - ini.tv_nsec) / 1e9;

    if (strcmp(qual, "tempo") == 0)
        fprintf(stderr, "parede=%.3f usuario=%.3f sistema=%.3f\n",
                parede, seg(ru.ru_utime), seg(ru.ru_stime));
    else if (strcmp(qual, "trocas") == 0)
        fprintf(stderr, "voluntarias=%ld involuntarias=%ld\n",
                ru.ru_nvcsw, ru.ru_nivcsw);
    else if (strcmp(qual, "paginas") == 0)
        fprintf(stderr, "menores=%ld maiores=%ld rss_pico_kb=%ld\n",
                ru.ru_minflt, ru.ru_majflt, ru.ru_maxrss);

    return codigo_de(status);
}
