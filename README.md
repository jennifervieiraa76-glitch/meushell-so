# meushell

Trabalho semestral de **Sistemas Operacionais** — Engenharia da Computação,
UNISAGRADO. Prof. Luiz Ricardo Mantovani da Silva · 2026-2

Cada grupo escreve um **shell**: o programa que lê uma linha, cria um processo,
liga descritores e espera. E que, a cada entrega, passa a **enxergar mais do
sistema** — até medir o próprio escalonador e a própria memória virtual.

Não é um shell pelo shell. É a bancada pela qual vocês vão observar, com as
próprias mãos, aquilo que a teoria descreve: a árvore de processos, o
descritor como recurso, o sinal como interrupção de software, o tempo que o
processo passou esperando em vez de calculando.

---

## Comece por aqui

```bash
git clone https://github.com/LuizRMSilva1973/so-lab.git
cd so-lab
```

```bash
make verificar E=1
```

Vai dar vermelho — é para dar. O shell compila e responde à linha de comando,
mas as partes que importam estão vazias. O vermelho é o ponto de partida.

Precisa de Linux com `gcc` e `make`. Se o computador de vocês complicar, o
[Google Cloud Shell](https://shell.cloud.google.com) já vem com tudo, é
gratuito, não pede cartão — e é **o ambiente da correção**.

---

## Os dois documentos que mandam

| Arquivo | O que decide |
|---|---|
| [CONTRATOS.md](CONTRATOS.md) | como o shell conversa com o mundo: linha de comando, saídas, códigos |
| [entregas/](entregas/) | o enunciado de cada entrega, com o que vale nota |

---

## As entregas

| # | Entrega | Data | Vale |
|---|---|---|---|
| [E1](entregas/E1.md) | Criar processos: `fork`, `exec`, `wait` | 02/10 | 0,8 |
| [E2](entregas/E2.md) | Descritores: redirecionamento e cano | 16/10 | 1,2 |
| [E3](entregas/E3.md) | Sinais e segundo plano | 30/10 | 1,4 |
| [E4](entregas/E4.md) | O shell mede o sistema | 13/11 | 1,6 |
| [Apres.](entregas/APRESENTACAO.md) | Demonstração e defesa | 04/12 | 1,0 |

São **quatro entregas sobre o mesmo shell**, não quatro trabalhos. O
verificador da E4 confere tudo o que veio antes.

---

## O que o verificador faz — e por que ele não se contenta com a saída

Ele alimenta o shell com roteiros e compara a saída byte a byte. Mas isso
sozinho seria fácil de enganar, então ele também **olha o estado do sistema**:

- conta processos **zumbis** cujo pai é o shell de vocês;
- lê `/proc/<pid>/fd` para conferir que o descritor do cano aponta para
  `pipe:[...]`, e não para um arquivo comum;
- manda `SIGINT` no grupo e confere que o **filho morreu** (código 130) e o
  **shell continuou**;
- na Entrega 4, exige que um programa que só calcula e um que só espera
  produzam números **diferentes** — medição que não distingue nada não mede
  nada.

Dá para produzir a saída certa de um cano gravando um arquivo temporário no
meio. O verificador tem uma prova só para isso, e ela é implacável: os dez
roteiros passam e a entrega reprova. **Saída igual com mecanismo errado não é
o mesmo trabalho.**

---

## Regras do jogo

**A entrega é o repositório, nunca a máquina de vocês.** A correção clona o
repositório numa máquina limpa e roda `make verificar E=n`. Antes de entregar,
rodem `make prova`: ele faz isso aqui, com o repositório de vocês, e pega o
defeito mais comum de todos — *funciona aqui e não no clone*.

**C, biblioteca padrão, sem bibliotecas externas.** Aqui a linguagem não é
escolha de estilo: `fork`, `execvp` e `dup2` são a matéria.

**Grupos de até 3.** O mesmo grupo do começo ao fim.

**Escrever o shell é a tarefa.** Usar IA para entender um conceito ou uma
mensagem de erro é bem-vindo, e eu faço isso também. Entregar um shell que
vocês não sabem alterar é outra coisa — e a apresentação tem uma alteração ao
vivo, sorteada na hora, justamente para separar os dois casos.

---

## Como entregar

1. `git push` no repositório do grupo.
2. Abram a [página do trabalho](https://profluiz.mantovanitec.com/disciplinas/aulas/so-ec/trabalho.html).
3. No formulário: escolham a entrega, identifiquem os integrantes, colem a URL
   do repositório e anexem o `evidencias/verificacao-N.txt`.
4. Cada integrante recebe uma cópia por e-mail. **Guardem esse e-mail**: é o
   comprovante.

## Entrega 1 — Árvore de processos ao rodar `ls -l`

    bash (terminal)
     └── meushell          ← pai: lê a linha e chama fork()
          └── ls -l        ← filho: nasce como cópia do meushell e
                             vira o ls quando chama execvp()

Os dois processos passam a existir no instante em que o fork() retorna:
antes dele há só o meushell; depois dele o pai recebe o pid do filho e o
filho recebe 0. O filho troca de programa com execvp e o pai espera com waitpid.

## Entrega 2 — Descritores em `ls | wc -l`

O shell cria o cano com `pipe()` (ponta de leitura e ponta de escrita, que
aqui ficam nos descritores 3 e 4) e depois faz dois `fork`. Os dois filhos
herdam a mesma tabela; cada um ajusta a sua com `dup2` antes do `execvp`.

| Descritor | Antes do dup2 (os dois filhos) | Esquerda (`ls`) depois | Direita (`wc -l`) depois |
|---|---|---|---|
| 0 (entrada) | terminal | terminal | **cano (leitura)** |
| 1 (saída)   | terminal | **cano (escrita)** | terminal |
| 2 (erro)    | terminal | terminal | terminal |
| 3 (cano, leitura) | aberto | fechado | fechado (já copiado no 0) |
| 4 (cano, escrita) | aberto | fechado (já copiado no 1) | fechado |

O pai também fecha as duas pontas: se alguma ponta de escrita ficasse aberta,
o `wc -l` nunca receberia o fim dos dados e o shell travaria.
