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

/*
  war_missoes.c
  Exemplo de implementação do sistema de missões solicitado.
  - Usa malloc/free para armazenar missão do jogador dinamicamente.
  - Função atribuirMissao copia usando strcpy.
  - Função verificarMissao analisa a missão e verifica no mapa.
  - Código em português com comentários.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TERRITORIOS 5
#define TAM_NOME 30
#define TAM_COR  16
#define TAM_MISSAO 256

/* Estrutura de território */
typedef struct territorio {
    char nome[TAM_NOME];
    char cor[TAM_COR];
    int tropas;
} Territorio;

/* ---------- Prototipos ---------- */
/* utilitários */
void limparBufferEntrada(void);
void removerNovaLinha(char *str);

/* gerenciamento de mapa */
Territorio* alocarMapa(int qtd);
void inicializarTerritorios(Territorio* mapa, int qtd);
void liberarMemoria(Territorio* mapa);

/* interface */
void exibirMenuPrincipal(void);
void exibirMapa(const Territorio* mapa, int qtd);
void exibirMissaoFormatada(const char* missao);

/* missões */
void atribuirMissao(char* destino, char* missoes[], int totalMissoes);
void personalizarMissao(char* destino, const char* corJogador);
int verificarMissao(char* missao, Territorio* mapa, int tamanho);

/* jogo / combate */
void faseDeAtaque(Territorio* mapa, int qtd, const char* corJogador);
void simularAtaque(Territorio* atacante, Territorio* defensor);

/* util auxiliar */
static void ordenarDesc(int *arr, int len);

/* ---------- Implementações ---------- */

void limparBufferEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void removerNovaLinha(char *str) {
    str[strcspn(str, "\n")] = '\0';
}

/* aloca mapa dinamicamente */
Territorio* alocarMapa(int qtd) {
    if (qtd <= 0) return NULL;
    Territorio* mapa = (Territorio*) calloc((size_t)qtd, sizeof(Territorio));
    if (!mapa) {
        fprintf(stderr, "Erro: falha na alocacao do mapa.\n");
        return NULL;
    }
    return mapa;
}

/* inicializa territórios (entrada do usuário) */
void inicializarTerritorios(Territorio* mapa, int qtd) {
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

void liberarMemoria(Territorio* mapa) {
    if (mapa) free(mapa);
}

void exibirMenuPrincipal(void) {
    printf("\n=================================\n");
    printf("           MENU PRINCIPAL        \n");
    printf("=================================\n");
    printf("1 - Exibir mapa\n");
    printf("2 - Fase de ataque\n");
    printf("3 - Exibir missao do jogador\n");
    printf("4 - Verificar missao (checar vitoria)\n");
    printf("0 - Sair\n");
    printf("=================================\n");
    printf("Escolha uma opcao: ");
}

void exibirMapa(const Territorio* mapa, int qtd) {
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

/* imprime apenas a parte visual da missão (após personalização) */
void exibirMissaoFormatada(const char* missao) {
    if (!missao) {
        printf("Nenhuma missao atribuida.\n");
        return;
    }
    /* a string de missão foi formatada com '|' separando meta legível */
    const char* ppipe = strchr(missao, '|');
    if (ppipe) {
        printf("\n--- MISSÃO SECRETA ---\n%s\n", ppipe + 1); /* texto legível depois do '|' */
    } else {
        printf("\n--- MISSÃO ---\n%s\n", missao);
    }
}

/* Sorteia e copia o template de missão para 'destino' usando strcpy */
void atribuirMissao(char* destino, char* missoes[], int totalMissoes) {
    if (!destino || !missoes || totalMissoes <= 0) return;
    int id = rand() % totalMissoes;
    /* copia do template (pode conter <COR> para ser substituído depois) */
    strcpy(destino, missoes[id]);  /* conforme requisito: usa strcpy */
}

/* substitui todas ocorrencias de "<COR>" na string destino pela corJogador
   pressupõe que 'destino' tenha espaço suficiente (na main alocamos TAM_MISSAO) */
void personalizarMissao(char* destino, const char* corJogador) {
    if (!destino || !corJogador) return;
    char buffer[TAM_MISSAO];
    buffer[0] = '\0';

    const char *p = destino;
    while (*p) {
        const char *pos = strstr(p, "<COR>");
        if (!pos) {
            /* copia restante */
            strncat(buffer, p, TAM_MISSAO - strlen(buffer) - 1);
            break;
        }
        /* copia trecho antes do placeholder */
        size_t lenPrefix = (size_t)(pos - p);
        if (lenPrefix > 0)
            strncat(buffer, p, (TAM_MISSAO - strlen(buffer) - 1) < lenPrefix ? (TAM_MISSAO - strlen(buffer) - 1) : lenPrefix);

        /* concatena a cor do jogador */
        strncat(buffer, corJogador, TAM_MISSAO - strlen(buffer) - 1);

        /* avança ponteiro após "<COR>" */
        p = pos + strlen("<COR>");
    }

    /* copia o resultado de volta a destino */
    strncpy(destino, buffer, TAM_MISSAO - 1);
    destino[TAM_MISSAO - 1] = '\0';
}

/* verificarMissao:
   A missão é uma string formatada contendo tags no formato:
   - "DESTRUIR_COR:<cor>|Descrição legivel..."
   - "CONQUISTAR_N:<n>;COR:<cor>|Descrição legivel..."
   - "CONSECUTIVOS:<n>;COR:<cor>|Descrição legivel..."

   A função interpreta a tag e verifica a condição no mapa.
   Retorna 1 se cumprida, 0 caso contrário.
*/
int verificarMissao(char* missao, Territorio* mapa, int tamanho) {
    if (!missao || !mapa || tamanho <= 0) return 0;

    /* buscar o tipo até ':' */
    if (strncmp(missao, "DESTRUIR_COR:", 13) == 0) {
        /* formato: DESTRUIR_COR:<cor>|... */
        const char* p = missao + 13;
        const char* q = strchr(p, '|');
        char corAlvo[TAM_COR];
        if (!q) q = p + strlen(p);
        size_t len = (size_t)(q - p);
        if (len >= TAM_COR) len = TAM_COR - 1;
        strncpy(corAlvo, p, len);
        corAlvo[len] = '\0';

        /* verificar se existe algum territorio com essa cor */
        for (int i = 0; i < tamanho; ++i) {
            if (strcmp(mapa[i].cor, corAlvo) == 0) return 0; /* ainda existe */
        }
        return 1; /* nenhum encontrado => missão cumprida */

    } else if (strncmp(missao, "CONQUISTAR_N:", 12) == 0) {
        /* formato: CONQUISTAR_N:<n>;COR:<cor>|... */
        const char* p = missao + 12;
        int n = atoi(p);
        const char* corTag = strstr(missao, "COR:");
        char corAlvo[TAM_COR] = {0};
        if (corTag) {
            corTag += 4;
            const char* q = strchr(corTag, '|');
            if (!q) q = corTag + strlen(corTag);
            size_t len = (size_t)(q - corTag);
            if (len >= TAM_COR) len = TAM_COR - 1;
            strncpy(corAlvo, corTag, len);
            corAlvo[len] = '\0';
        } else {
            return 0; /* sem cor definida, não conseguimos checar */
        }

        int contador = 0;
        for (int i = 0; i < tamanho; ++i) {
            if (strcmp(mapa[i].cor, corAlvo) == 0) contador++;
        }
        return (contador >= n) ? 1 : 0;

    } else if (strncmp(missao, "CONSECUTIVOS:", 13) == 0) {
        /* formato: CONSECUTIVOS:<n>;COR:<cor>|... -> verificar se há uma sequência de n territorios adjacentes
           (neste mapa simples sem adjacência real, interpretamos "seguidos" como índices consecutivos no vetor)
        */
        const char* p = missao + 13;
        int n = atoi(p);
        const char* corTag = strstr(missao, "COR:");
        char corAlvo[TAM_COR] = {0};
        if (corTag) {
            corTag += 4;
            const char* q = strchr(corTag, '|');
            if (!q) q = corTag + strlen(corTag);
            size_t len = (size_t)(q - corTag);
            if (len >= TAM_COR) len = TAM_COR - 1;
            strncpy(corAlvo, corTag, len);
            corAlvo[len] = '\0';
        } else return 0;

        int run = 0;
        for (int i = 0; i < tamanho; ++i) {
            if (strcmp(mapa[i].cor, corAlvo) == 0) {
                run++;
                if (run >= n) return 1;
            } else {
                run = 0;
            }
        }
        return 0;
    }

    /* se não reconheceu o formato, retorna falso */
    return 0;
}

/* ordenarDesc: ordena vetor int decrescente (pequeno helper) */
static void ordenarDesc(int *arr, int len) {
    for (int i = 0; i < len - 1; ++i)
        for (int j = 0; j < len - 1 - i; ++j)
            if (arr[j] < arr[j+1]) {
                int t = arr[j]; arr[j] = arr[j+1]; arr[j+1] = t;
            }
}

/* simularAtaque(): lógica simplificada */
void simularAtaque(Territorio* atacante, Territorio* defensor) {
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
        if (mover < 1) mover = 1; /* garante mover pelo menos 1 */
        atacante->tropas -= mover;
        defensor->tropas += mover;

        printf("%d tropa(s) movida(s) do atacante para o territorio conquistado.\n", mover);
    }

    printf("Estado apos combate:\n");
    printf(" -> %s | dono: %s | tropas: %d\n", atacante->nome, atacante->cor, atacante->tropas);
    printf(" -> %s | dono: %s | tropas: %d\n", defensor->nome, defensor->cor, defensor->tropas);
}

/* faseDeAtaque: escolhe indices e chama simularAtaque */
void faseDeAtaque(Territorio* mapa, int qtd, const char* corJogador) {
    (void) corJogador; /* não usado aqui, mas mantido na assinatura */

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

    simularAtaque(&mapa[ia], &mapa[id]);
}

/* ---------- main ---------- */
int main(void) {
    srand((unsigned int) time(NULL));

    const int qtd = MAX_TERRITORIOS;

    /* templates de missões (contêm placeholders e tags)
       - usamos um padrão simples: TAGS antes do '|' e descrição após o '|'
       - placeholders: <COR> será substituído pela cor do jogador.
    */
    char* templates[] = {
        "DESTRUIR_COR:<COR>|Eliminar todas as tropas da cor <COR> (a missão é tornar 0 territorios dessa cor).",
        "CONQUISTAR_N:3;COR:<COR>|Conquistar pelo menos 3 territorios para a cor <COR>.",
        "CONSECUTIVOS:3;COR:<COR>|Conquistar 3 territorios seguidos (indices consecutivos no vetor) com a cor <COR>.",
        "CONQUISTAR_N:4;COR:<COR>|Conquistar pelo menos 4 territorios para a cor <COR>.",
        "CONSECUTIVOS:2;COR:<COR>|Conquistar 2 territorios seguidos (indices consecutivos) com a cor <COR>."
    };
    const int totalTemplates = sizeof(templates) / sizeof(templates[0]);

    /* 1) aloca o mapa dinamicamente */
    Territorio* mapa = alocarMapa(qtd);
    if (!mapa) {
        fprintf(stderr, "Nao foi possivel alocar o mapa. Abortando.\n");
        return 1;
    }

    /* 2) inicializa territorios (entrada pelo usuario) */
    inicializarTerritorios(mapa, qtd);

    /* 3) definir cor do jogador (entrada) */
    char corJogador[TAM_COR];
    printf("\nDigite a cor do jogador (seu exército): ");
    if (fgets(corJogador, TAM_COR, stdin) == NULL) corJogador[0] = '\0';
    removerNovaLinha(corJogador);

    /* 4) aloca dinamicamente a missao do jogador e atribui uma (usando strcpy dentro da funcao) */
    char* missaoJogador = (char*) malloc(TAM_MISSAO);
    if (!missaoJogador) {
        fprintf(stderr, "Erro: falha na alocacao da missao.\n");
        liberarMemoria(mapa);
        return 1;
    }
    /* sorteia e copia template */
    atribuirMissao(missaoJogador, templates, totalTemplates);

    /* personaliza substituindo <COR> pela cor do jogador. Agora missaoJogador tem as tags e a descrição. */
    personalizarMissao(missaoJogador, corJogador);

    /* exibe missão para teste (apenas descrição legível) */
    exibirMissaoFormatada(missaoJogador);

    /* loop principal do jogo */
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
            case 3:
                exibirMissaoFormatada(missaoJogador);
                break;
            case 4: {
                exibirMissaoFormatada(missaoJogador);
                int venceu = verificarMissao(missaoJogador, mapa, qtd);
                if (venceu) {
                    printf("\nPARABENS! Missao cumprida.\n");
                    /* opcional: terminar o jogo ou seguir jogando; aqui apenas exibe mensagem */
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
    free(missaoJogador);
    liberarMemoria(mapa);
    mapa = NULL;

    return 0;
}
