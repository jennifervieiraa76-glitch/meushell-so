/*
 * Entregas 1 e 2 — criar processos e ligar descritores.
 *
 * ENTREGA 1: executar_comando() sem redirecionamento e sem cano.
 *   O caminho e sempre o mesmo: fork() cria a copia; no FILHO voce troca o
 *   programa com execvp(); no PAI voce espera com waitpid() e guarda o codigo.
 *
 *   Tres coisas que valem nota e costumam faltar:
 *     - comando que nao existe: mensagem e codigo 127 (o execvp VOLTA quando
 *       falha — se ele voltou, deu errado);
 *     - o filho tem que reagir ao Ctrl-C: signal(SIGINT, SIG_DFL) antes do
 *       execvp, porque o shell ignora o sinal e o filho herda esse "ignorar";
 *     - _exit() no filho, nunca exit(), para nao esvaziar duas vezes o buffer
 *       que ele herdou do pai.
 *
 * ENTREGA 2: aplicar os redirecionamentos (dup2 depois do fork, ANTES do
 *   execvp) e o cano em executar_com_cano().
 *
 * Leiam antes: CONTRATOS.md secoes 3 e 4, e entregas/E1.md.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include "comum.h"

/* Traduz o status devolvido pelo wait para um codigo de saida. PRONTO. */
int codigo_de(int status)
{
    if (WIFEXITED(status))   return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 1;
}

/* Abre os arquivos do redirecionamento e aponta 0 e 1 para eles.
   Devolve 0, ou -1 se algum arquivo nao abriu. (ENTREGA 2) */
int aplicar_redir(struct redir *r)
{
    (void)r;
    FALTA_ESCREVER("o redirecionamento (Entrega 2)");
    return -1;
}

int executar_comando(char *v[], int n, struct redir *r, int fundo)
{
    (void)n; (void)r; (void)fundo;   /* so serao usados nas proximas entregas */

    pid_t pid = fork();              /* a partir daqui existem DOIS processos */

    if (pid < 0) {                   /* o fork falhou */
        perror("fork");
        return 1;
    }

    if (pid == 0) {                  /* FILHO (o fork devolveu 0 para ele) */
        signal(SIGINT, SIG_DFL);     /* o Ctrl-C volta a matar este processo */
        execvp(v[0], v);             /* troca este processo pelo programa v[0] */
        /* so chega aqui se o execvp FALHOU (comando nao existe) */
        fprintf(stderr, "meushell: comando nao encontrado: %s\n", v[0]);
        _exit(127);                  /* _exit, nao exit: nao duplica buffers */
    }

    /* PAI (o fork devolveu a ele o numero do filho) */
    int status;
    waitpid(pid, &status, 0);        /* espera ESSE filho terminar */
    return codigo_de(status);        /* traduz o status em codigo de saida */
}

int executar_com_cano(char *ve[], int ne, struct redir *re,
                      char *vd[], int nd, struct redir *rd)
{
    (void)ve; (void)ne; (void)re; (void)vd; (void)nd; (void)rd;
    FALTA_ESCREVER("o cano (Entrega 2)");
    return 3;
}
