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
    if (r == NULL) return 0;

    if (r->entrada) {                          /* comando < arquivo */
        int fd = open(r->entrada, O_RDONLY);
        if (fd < 0) { perror(r->entrada); return -1; }
        dup2(fd, 0);                           /* o 0 (entrada) passa a ser o arquivo */
        close(fd);                             /* a copia original nao serve mais */
    }

    if (r->saida) {                            /* comando > arquivo  ou  >> arquivo */
        int flags = O_WRONLY | O_CREAT | (r->anexar ? O_APPEND : O_TRUNC);
        int fd = open(r->saida, flags, 0644);
        if (fd < 0) { perror(r->saida); return -1; }
        dup2(fd, 1);                           /* o 1 (saida) passa a ser o arquivo */
        close(fd);
    }
    return 0;
}

int executar_comando(char *v[], int n, struct redir *r, int fundo)
{
    (void)n; (void)fundo;            /* fundo sera usado na Entrega 3 */

    pid_t pid = fork();              /* a partir daqui existem DOIS processos */

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {                  /* FILHO */
        signal(SIGINT, SIG_DFL);     /* o Ctrl-C volta a matar este processo */
        if (aplicar_redir(r) < 0)    /* dup2 DEPOIS do fork e ANTES do execvp */
            _exit(1);
        execvp(v[0], v);
        fprintf(stderr, "meushell: comando nao encontrado: %s\n", v[0]);
        _exit(127);
    }

    int status;                      /* PAI */
    waitpid(pid, &status, 0);
    return codigo_de(status);
}

int executar_com_cano(char *ve[], int ne, struct redir *re,
                      char *vd[], int nd, struct redir *rd)
{
    (void)ne; (void)nd;

    int fd[2];                       /* fd[0] = ponta de LEITURA, fd[1] = ponta de ESCRITA */
    if (pipe(fd) < 0) { perror("pipe"); return 1; }

    /* filho da ESQUERDA: escreve no cano */
    pid_t pe = fork();
    if (pe < 0) { perror("fork"); close(fd[0]); close(fd[1]); return 1; }
    if (pe == 0) {
        signal(SIGINT, SIG_DFL);
        dup2(fd[1], 1);              /* a saida (1) passa a ser o cano */
        close(fd[0]);                /* fecha as pontas que nao usa */
        close(fd[1]);
        if (aplicar_redir(re) < 0) _exit(1);
        execvp(ve[0], ve);
        fprintf(stderr, "meushell: comando nao encontrado: %s\n", ve[0]);
        _exit(127);
    }

    /* filho da DIREITA: le do cano */
    pid_t pd = fork();
    if (pd < 0) {
        perror("fork");
        close(fd[0]); close(fd[1]);
        waitpid(pe, NULL, 0);
        return 1;
    }
    if (pd == 0) {
        signal(SIGINT, SIG_DFL);
        dup2(fd[0], 0);              /* a entrada (0) passa a ser o cano */
        close(fd[0]);
        close(fd[1]);
        if (aplicar_redir(rd) < 0) _exit(1);
        execvp(vd[0], vd);
        fprintf(stderr, "meushell: comando nao encontrado: %s\n", vd[0]);
        _exit(127);
    }

    /* PAI: fecha as DUAS pontas, senao o leitor nunca ve o fim e trava */
    close(fd[0]);
    close(fd[1]);

    int se, sd;
    waitpid(pe, &se, 0);
    waitpid(pd, &sd, 0);
    return codigo_de(sd);            /* o codigo do cano e o da DIREITA */
}
