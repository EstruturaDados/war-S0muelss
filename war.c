// ============================================================================
//         PROJETO WAR ESTRUTURADO - DESAFIO DE CÓDIGO
// ============================================================================
//        
// ============================================================================
//
// OBJETIVOS:
// - Modularizar completamente o código em funções especializadas.
// - Implementar um sistema de missões para um jogador.
// - Criar uma função para verificar se a missão foi cumprida.
// - Utilizar passagem por referência (ponteiros) para modificar dados e
//   passagem por valor/referência constante (const) para apenas ler.
// - Foco em: Design de software, modularização, const correctness, lógica de jogo.
//
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TERRITORIOS 5
#define TAM_NOME 30
#define TAM_COR  10

/* Estrutura de território */
struct territorio {
    char nome[TAM_NOME];
    char cor[TAM_COR];
    int tropas;
};

/* --- Protótipos --- */
/* utilitários */
void limparBufferEntrada(void);
void removerNovaLinha(char *str);

/* gerenciamento de mapa */
struct territorio* alocarMapa(int qtd);
void inicializarTerritorios(struct territorio* mapa, int qtd);
void liberarMemoria(struct territorio* mapa);

/* interface */
void exibirMenuPrincipal(void);
void exibirMapa(const struct territorio* mapa, int qtd);
void exibirMissao(int missaoId, const char* corJogador, int alvoNumero);

/* fase de jogo / ataque / missões */
void faseDeAtaque(struct territorio* mapa, int qtd, const char* corJogador);
void simularAtaque(struct territorio* atacante, struct territorio* defensor, const char* corJogador);
int sortearMissao(void);
int verificarVitoria(const struct territorio* mapa, int qtd, int missaoId, const char* corJogador, int alvoNumero);

/* --- Implementações --- */

void limparBufferEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void removerNovaLinha(char *str) {
    str[strcspn(str, "\n")] = '\0';
}

/* alocarMapa():
 * Aloca dinamicamente um vetor de 'struct territorio' usando calloc.
 * Retorna ponteiro ou NULL em caso de falha.
 */
struct territorio* alocarMapa(int qtd) {
    if (qtd <= 0) return NULL;
    struct territorio* mapa = (struct territorio*) calloc((size_t)qtd, sizeof(struct territorio));
    if (mapa == NULL) {
        fprintf(stderr, "Erro: falha na alocacao do mapa.\n");
        return NULL;
    }
    return mapa;
}

/* inicializarTerritorios():
 * Preenche os dados iniciais de cada território no mapa (nome, cor, tropas).
 * Modifica o mapa passado por ponteiro.
 */
void inicializarTerritorios(struct territorio* mapa, int qtd) {
    if (!mapa || qtd <= 0) return;

    printf("\n=== Inicializacao dos %d territorios ===\n", qtd);
    for (int i = 0; i < qtd; ++i) {
        printf("\nTerritorio %d/%d\n", i+1, qtd);

        printf("Nome: ");
        if (fgets(mapa[i].nome, TAM_NOME, stdin) == NULL) mapa[i].nome[0] = '\0';
        removerNovaLinha(mapa[i].nome);

        printf("Cor (dono): ");
        if (fgets(mapa[i].cor, TAM_COR, stdin) == NULL) mapa[i].cor[0] = '\0';
        removerNovaLinha(mapa[i].cor);

        int tropas = 0;
        do {
            printf("Tropas (inteiro >= 1): ");
            if (scanf("%d", &tropas) != 1) {
                printf("Entrada invalida. Tente novamente.\n");
                limparBufferEntrada();
                tropas = 0;
            } else if (tropas < 1) {
                printf("Tropas devem ser >= 1.\n");
            }
        } while (tropas < 1);
        mapa[i].tropas = tropas;
        limparBufferEntrada();
    }
}

/* liberarMemoria():
 * Libera a memória alocada para o mapa.
 */
void liberarMemoria(struct territorio* mapa) {
    if (mapa) free(mapa);
}

/* exibirMenuPrincipal():
 * Imprime o menu de ações.
 */
void exibirMenuPrincipal(void) {
    printf("\n=================================\n");
    printf("           MENU PRINCIPAL        \n");
    printf("=================================\n");
    printf("1 - Exibir mapa\n");
    printf("2 - Fase de ataque\n");
    printf("3 - Verificar missao\n");
    printf("0 - Sair\n");
    printf("=================================\n");
    printf("Escolha uma opcao: ");
}

/* exibirMapa():
 * Mostra o estado atual de todos os territorios (apenas leitura).
 */
void exibirMapa(const struct territorio* mapa, int qtd) {
    if (!mapa) {
        printf("Mapa vazio.\n");
        return;
    }
    printf("\n================= MAPA =================\n");
    printf("%-3s | %-20s | %-10s | %-6s\n", "ID", "NOME", "Dono", "Tropas");
    printf("-----------------------------------------------\n");
    for (int i = 0; i < qtd; ++i) {
        printf("%-3d | %-20s | %-10s | %-6d\n",
               i, mapa[i].nome[0] ? mapa[i].nome : "(sem nome)",
               mapa[i].cor[0] ? mapa[i].cor : "(sem dono)",
               mapa[i].tropas);
    }
    printf("=========================================\n");
}

/* exibirMissao():
 * Exibe a missão sorteada de forma amigável.
 * missaoId: 0 => destruir exército específico; 1 => conquistar N territorios.
 * corJogador: cor do jogador (para referência).
 * alvoNumero: valor associado à missão (por exemplo, número de territórios) ou ignorado se desnecessário.
 */
void exibirMissao(int missaoId, const char* corJogador, int alvoNumero) {
    printf("\n--- MISSÃO SECRETA ---\n");
    if (missaoId == 0) {
        printf("Missao: Eliminar TODO o exército de cor '%s' do mapa (tornar 0 territorios dessa cor).\n", corJogador);
        printf("Obs: Para testes a missao pede eliminar o proprio exército do jogador; voce pode trocar a cor alvo no codigo se quiser.\n");
    } else if (missaoId == 1) {
        printf("Missao: Conquistar pelo menos %d territorios (ter sob seu controle).\n", alvoNumero);
    } else {
        printf("Missao: (desconhecida)\n");
    }
}

/* --- faseDeAtaque e simularAtaque --- */

/* ordenarDesc helper (pequeno) */
static void ordenarDesc(int *arr, int len) {
    for (int i = 0; i < len - 1; ++i)
        for (int j = 0; j < len - 1 - i; ++j)
            if (arr[j] < arr[j+1]) {
                int t = arr[j]; arr[j] = arr[j+1]; arr[j+1] = t;
            }
}

/* simularAtaque():
 * Executa a logica de batalha entre dois territorios.
 * Regras:
 *  - atacante deve ter >1 tropa para atacar.
 *  - atacante rola até 3 dados (max = tropas-1); defensor até 2 dados (max = tropas).
 *  - compara maiores dados por pares; perdedor de cada comparacao perde 1 tropa.
 *  - se defensor ficar com 0 tropas, troca dono para a cor do atacante e move 1 tropa.
 */
void simularAtaque(struct territorio* atacante, struct territorio* defensor, const char* corJogador) {
    if (!atacante || !defensor) return;

    if (atacante->tropas <= 1) {
        printf("Atacante '%s' nao possui tropas suficientes (>1) para iniciar ataque.\n", atacante->nome);
        return;
    }
    if (strcmp(atacante->cor, defensor->cor) == 0) {
        printf("Atencao: Atacante e defensor tem o mesmo dono ('%s'). Nao e permitido atacar seu proprio territorio.\n", atacante->cor);
        return;
    }

    int maxDadosAt = 3;
    int maxDadosDef = 2;

    int dadosAt = atacante->tropas - 1;
    if (dadosAt > maxDadosAt) dadosAt = maxDadosAt;
    if (dadosAt < 1) {
        printf("Atacante nao pode rolar dados.\n");
        return;
    }
    int dadosDef = defensor->tropas;
    if (dadosDef > maxDadosDef) dadosDef = maxDadosDef;
    if (dadosDef < 1) dadosDef = 1;

    printf("\n-> %s (dono: %s, tropas: %d) ataca %s (dono: %s, tropas: %d)\n",
           atacante->nome, atacante->cor, atacante->tropas,
           defensor->nome, defensor->cor, defensor->tropas);
    printf("Atacante rola %d dados. Defensor rola %d dados.\n", dadosAt, dadosDef);

    int rolAt[3] = {0,0,0};
    int rolDef[2] = {0,0};

    for (int i = 0; i < dadosAt; ++i) rolAt[i] = (rand() % 6) + 1;
    for (int i = 0; i < dadosDef; ++i) rolDef[i] = (rand() % 6) + 1;

    ordenarDesc(rolAt, dadosAt);
    ordenarDesc(rolDef, dadosDef);

    printf("Dados atacante: ");
    for (int i = 0; i < dadosAt; ++i) printf("%d ", rolAt[i]);
    printf("\nDados defensor: ");
    for (int i = 0; i < dadosDef; ++i) printf("%d ", rolDef[i]);
    printf("\n");

    int comparacoes = (dadosAt < dadosDef) ? dadosAt : dadosDef;
    int perdasAt = 0, perdasDef = 0;

    for (int i = 0; i < comparacoes; ++i) {
        if (rolAt[i] > rolDef[i]) {
            defensor->tropas -= 1;
            perdasDef++;
        } else {
            atacante->tropas -= 1;
            perdasAt++;
        }
    }

    printf("Resultado da batalha: atacante perdeu %d tropas; defensor perdeu %d tropas.\n", perdasAt, perdasDef);

    if (defensor->tropas <= 0) {
        printf("Territorio '%s' foi conquistado pelo dono '%s'!\n", defensor->nome, atacante->cor);
        /* troca dono */
        strncpy(defensor->cor, atacante->cor, TAM_COR-1);
        defensor->cor[TAM_COR-1] = '\0';

        /* mover 1 tropa do atacante para o defensor (ou o max possivel mantendo 1) */
        int mover = 1;
        if (atacante->tropas - mover < 1) mover = atacante->tropas - 1;
        if (mover < 1) mover = 1; // garante mover pelo menos 1

        atacante->tropas -= mover;
        defensor->tropas += mover;

        printf("%d tropa(s) movida(s) do atacante para o territorio conquistado.\n", mover);
    }

    printf("Estado apos combate:\n");
    printf(" -> %s | dono: %s | tropas: %d\n", atacante->nome, atacante->cor, atacante->tropas);
    printf(" -> %s | dono: %s | tropas: %d\n", defensor->nome, defensor->cor, defensor->tropas);
}

/* faseDeAtaque():
 * Gerencia a interação do jogador para selecionar territorios e iniciar o ataque.
 */
void faseDeAtaque(struct territorio* mapa, int qtd, const char* corJogador) {
    if (!mapa || qtd <= 0) return;

    exibirMapa(mapa, qtd);
    int ia = -1, id = -1;

    /* escolher atacante */
    do {
        printf("Escolha o indice do territorio ATACANTE (0..%d): ", qtd-1);
        if (scanf("%d", &ia) != 1) {
            limparBufferEntrada();
            ia = -1;
            printf("Entrada invalida.\n");
            continue;
        }
        limparBufferEntrada();
        if (ia < 0 || ia >= qtd) {
            printf("Indice fora do intervalo.\n");
            ia = -1;
        }
    } while (ia == -1);

    /* escolher defensor */
    do {
        printf("Escolha o indice do territorio DEFENSOR (0..%d): ", qtd-1);
        if (scanf("%d", &id) != 1) {
            limparBufferEntrada();
            id = -1;
            printf("Entrada invalida.\n");
            continue;
        }
        limparBufferEntrada();
        if (id < 0 || id >= qtd) {
            printf("Indice fora do intervalo.\n");
            id = -1;
        } else if (id == ia) {
            printf("Atacante e defensor devem ser territorios diferentes.\n");
            id = -1;
        }
    } while (id == -1);

    /* validacoes basicas */
    if (mapa[ia].tropas <= 1) {
        printf("Territorio atacante precisa ter mais de 1 tropa para atacar.\n");
        return;
    }
    if (strcmp(mapa[ia].cor, mapa[id].cor) == 0) {
        printf("Nao e possivel atacar um territorio de mesmo dono.\n");
        return;
    }

    simularAtaque(&mapa[ia], &mapa[id], corJogador);
}

/* sortearMissao():
 * Retorna um ID de missao aleatorio (0 ou 1).
 * 0 = destruir exército (a cor do jogador) --> nota: para demostrar, essa missaõ pede destruir um exército identificado pela cor do jogador;
 * 1 = conquistar N territorios (alvoNumero será usado no main).
 */
int sortearMissao(void) {
    return rand() % 2; /* 0 ou 1 */
}

/* verificarVitoria():
 * Verifica se a missao foi cumprida.
 * - missaoId 0: "destruir exército de cor jogador" -> significa não existir mais territorios com a cor informada.
 * - missaoId 1: "conquistar >= alvoNumero territorios" -> conta quantos territorios possuem cor igual a corJogador.
 * Retorna 1 se cumprida, 0 caso contrario.
 */
int verificarVitoria(const struct territorio* mapa, int qtd, int missaoId, const char* corJogador, int alvoNumero) {
    if (!mapa || !corJogador) return 0;

    if (missaoId == 0) {
        /* verificar se não existe mais territorios com a cor alvo (aqui usamos corJogador como cor alvo) */
        for (int i = 0; i < qtd; ++i) {
            if (strcmp(mapa[i].cor, corJogador) == 0) {
                /* ainda existe pelo menos 1 território da cor alvo */
                return 0;
            }
        }
        return 1; /* nenhum territorio com a cor alvo */
    } else if (missaoId == 1) {
        int contador = 0;
        for (int i = 0; i < qtd; ++i) {
            if (strcmp(mapa[i].cor, corJogador) == 0) contador++;
        }
        return (contador >= alvoNumero) ? 1 : 0;
    }
    return 0;
}

/* --- main --- */
int main(void) {
    srand((unsigned int) time(NULL));

    const int qtd = MAX_TERRITORIOS;

    /* 1) aloca o mapa dinamicamente */
    struct territorio* mapa = alocarMapa(qtd);
    if (!mapa) {
        fprintf(stderr, "Nao foi possivel alocar o mapa. Abortando.\n");
        return 1;
    }

    /* 2) inicializa os territorios (entrada pelo usuario) */
    inicializarTerritorios(mapa, qtd);

    /* 3) definicao do jogador e missao */
    char corJogador[TAM_COR];
    printf("\nDigite a cor do jogador (seu exército): ");
    if (fgets(corJogador, TAM_COR, stdin) == NULL) corJogador[0] = '\0';
    removerNovaLinha(corJogador);

    int missaoId = sortearMissao();
    int alvoNumero = 3; /* default para missao tipo 1: conquistar 3 territorios */
    exibirMissao(missaoId, corJogador, alvoNumero);

    /* loop principal */
    int opcao = -1;
    do {
        exibirMenuPrincipal();
        if (scanf("%d", &opcao) != 1) {
            limparBufferEntrada();
            printf("Entrada invalida.\n");
            continue;
        }
        limparBufferEntrada();

        switch (opcao) {
            case 1:
                exibirMapa(mapa, qtd);
                break;
            case 2:
                faseDeAtaque(mapa, qtd, corJogador);
                break;
            case 3: {
                exibirMissao(missaoId, corJogador, alvoNumero);
                int venceu = verificarVitoria(mapa, qtd, missaoId, corJogador, alvoNumero);
                if (venceu) {
                    printf("\nPARABENS! Missao cumprida.\n");
                } else {
                    printf("\nMissao ainda nao cumprida. Continue jogando.\n");
                }
                break;
            }
            case 0:
                printf("Encerrando o jogo. Liberando memoria...\n");
                break;
            default:
                printf("Opcao invalida.\n");
                break;
        }
    } while (opcao != 0);

    /* cleanup */
    liberarMemoria(mapa);
    mapa = NULL;

    return 0;
}
