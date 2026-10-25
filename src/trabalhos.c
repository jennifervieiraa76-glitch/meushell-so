/*
 * Entrega 3 — segundo plano, `jobs` e sinais.
 *
 * O que precisa acontecer:
 *   - `comando &` nao trava o shell: ele anota o pid e volta a ler linhas;
 *   - `jobs` lista o que ainda esta vivo, um por linha: [n] pid comando
 *   - quem termina no segundo plano tem que ser COLHIDO — senao vira zumbi, e
 *     o verificador conta zumbis cujo pai e o shell de voces;
 *   - Ctrl-C mata o comando em primeiro plano e NAO mata o shell.
 *
 * Sobre colher: waitpid(-1, &status, WNOHANG) devolve >0 quando havia alguem
 * para colher e 0 quando nao havia. Num laco, antes de cada linha, resolve —
 * e evita a corrida que um tratador de SIGCHLD traria junto.
 *
 * Leiam antes: CONTRATOS.md secao 5, e entregas/E3.md.
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "comum.h"

/* Tabela dos trabalhos em segundo plano */
#define MAX_TRAB 64
static struct { int n; pid_t pid; char cmd[256]; int vivo; } trab[MAX_TRAB];
static int qtd = 0;        /* quantos ja foram anotados */

void preparar_sinais(void)
{
    signal(SIGINT, SIG_IGN);   /* o SHELL ignora o Ctrl-C e nao morre */
}

void anotar_trabalho(pid_t pid, const char *comando)
{
    if (qtd >= MAX_TRAB) return;
    trab[qtd].n = qtd + 1;
    trab[qtd].pid = pid;
    snprintf(trab[qtd].cmd, sizeof trab[qtd].cmd, "%s", comando);
    trab[qtd].vivo = 1;
    qtd++;
}

void colher_trabalhos(void)
{
    int status;
    pid_t pid;
    /* WNOHANG: se ninguem terminou, devolve 0 na hora em vez de esperar */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < qtd; i++)
            if (trab[i].pid == pid) trab[i].vivo = 0;   /* colhido: nao e mais zumbi */
    }
}

void listar_trabalhos(void)
{
    for (int i = 0; i < qtd; i++)
        if (trab[i].vivo)
            printf("[%d] %d %s\n", trab[i].n, (int)trab[i].pid, trab[i].cmd);
    fflush(stdout);
}
