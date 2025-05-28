#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncursesw/ncurses.h>
#include <time.h>
#include <locale.h>
#include <wchar.h>
#include <ctype.h>
#include <unistd.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define TAM 15

void desenharJogo();
void revelarCelulasVazias(int l, int c);

int acertou_bomba = 0;

typedef struct {
    int eBomba;
    int estaAberta;
    int vizinhos;
    char proposicao[50];
} Celula;

typedef struct {
    char nome;
    int valor;
} VariavelLogica;

Celula jogo[TAM][TAM];
int l, c;
int linha_selecionada = 0, coluna_selecionada = 0;
int game_over = 0;
int vitoria = 0;
int pontuacao = 0;

VariavelLogica vars[] = {
    {'P', 0}, {'Q', 0}
};
int num_vars = 2;

typedef struct {
    char *proposicao;
    char *descricao;
} ProposicaoLogica;

ProposicaoLogica proposicoes[] = {
    {"P", "Variável simples P"},
    {"¬P", "Negação de P"},
    {"P ∧ Q", "P e Q"},
    {"P ∨ Q", "P ou Q"},
    {"P → Q", "Se P então Q"},
    {"¬(P ∧ Q)", "Não é verdade que P e Q"},
    {"P ↔ Q", "P se e somente se Q"}
};
int num_proposicoes = sizeof(proposicoes) / sizeof(ProposicaoLogica);

typedef struct {
    int disponivel;
    char *texto;
} Dica;

Dica dicas[TAM][TAM];

void inicializarJogo() {
    for(l = 0; l < TAM; l++) {
        for(c = 0; c < TAM; c++) {
            jogo[l][c].eBomba = 0;
            jogo[l][c].estaAberta = 0;
            jogo[l][c].vizinhos = 0;
            int idx = rand() % num_proposicoes;
            strcpy(jogo[l][c].proposicao, proposicoes[idx].proposicao);
            dicas[l][c].disponivel = 1;
            dicas[l][c].texto = NULL;
        }
    }
    linha_selecionada = coluna_selecionada = 0;
    acertou_bomba = 0;
    game_over = vitoria = 0;
    pontuacao = 100;
}

void sortearBombas(int n) {
    int i;
    srand(time(NULL));
    for(i = 1; i <= n; i++) {
        l = rand() % TAM;
        c = rand() % TAM;
        if(jogo[l][c].eBomba == 0) {
            jogo[l][c].eBomba = 1;
        }
        else
            i--;
    }
}

int coordenadaEhValida(int l, int c) {
    return (l >= 0 && l < TAM && c >= 0 && c < TAM);
}

int quantBombasVizinhas(int l, int c) {
    int quantidade = 0;
    int dl[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};
    
    for(int i = 0; i < 4; i++) {
        int nova_l = l + dl[i];
        int nova_c = c + dc[i];
        if(coordenadaEhValida(nova_l, nova_c) && jogo[nova_l][nova_c].eBomba) {
            quantidade++;
        }
    }
    return quantidade;
}

void contarBombas() {
    for(l = 0; l < TAM; l++) {
        for(c = 0; c < TAM; c++) {
            jogo[l][c].vizinhos = quantBombasVizinhas(l, c);
        }
    }
}

void animarExplosao() {
    int frames = 30;
    int offsetX = 0, offsetY = 0;
    
    const char* explosion[] = {
        "⣿⣿⣿⣿⣿⣿⣿⣿⠿⢛⣛⡭⠙⣉⣉⣉⠙⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿",
        "⣿⣿⣿⣿⣿⡿⢁⣴⣀⣿⣿⣷⣴⣿⣿⣿⣾⣦⣈⠙⠛⠿⣿⣿⣿⣿⣿",
        "⣿⡿⢁⣉⠋⣄⣿⣿⣯⠉⣡⣾⣿⡿⣿⣿⣿⠿⣿⣦⣿⣆⡉⠿⣿⣿⣿",
        "⡟⢀⣿⣿⣨⣿⣿⣿⣿⣼⣿⣿⡏⠰⣾⣿⣿⣷⢰⣶⣿⣿⣇⢰⡜⣿⣿",
        "⠅⣽⣿⠩⣿⣿⣿⣿⡿⢿⡿⠆⠙⢂⠙⠋⠛⠿⡟⢿⣿⡿⣿⣮⣃⠸⣿",
        "⣆⢠⢻⡷⠖⣿⣿⣍⣁⣤⢤⣴⣶⣿⢿⡿⠀⣦⣁⠙⢯⡀⢾⣿⢿⡇⢿",
        "⣿⡀⠈⢿⣿⡥⢿⣷⡶⢌⡊⢻⡟⣿⣸⠀⠀⣭⣥⣲⣾⡿⠛⢏⣤⣤⣾",
        "⣿⣿⣷⣤⣤⣴⣷⣶⣶⣦⣬⠈⣇⠃⠇⡄⠤⣬⣉⣡⣬⣤⣶⣿⣿⣿⣿",
        "⣿⣿⣿⣿⣿⡿⠟⢋⠡⠒⠀⢻⣴⢸⡇⡚⢂⡌⠛⢿⣿⣿⣿⣿⣿⣿⣿",
        "⣿⣿⣿⣿⣿⣯⠐⢾⠶⣤⡰⠇⠸⠈⠶⠀⢋⡘⣷⣵⣆⠘⢿⣿⣿⣿⣿",
        "⣿⣿⣿⣿⣿⣿⣦⣉⠈⠹⠿⠳⣾⣧⠾⠗⢿⡷⠿⠏⠟⠀⣸⣿⣿⣿⣿",
        "⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣤⠀⠈⣤⢴⠰⣦⣀⣤⣶⣶⣾⣿⣿⣿⣿⣿",
        "⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⡄⣿⢸⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿",
        "⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠀⡇⣼⢸⡄⢹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿",
        "⣿⣿⣿⣿⣿⣿⣿⣿⢿⠋⠀⣠⣿⣿⠈⣷⡀⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿",
        "⣿⣿⣿⣿⣿⣿⣿⣿⣯⣴⣾⣟⣼⣿⣷⣿⣷⣦⣽⣿⣿⣿⣿⣿⣿⣿⣿"
    };
    
    int art_height = 16;
    int art_width = 60;
    
    int term_height, term_width;
    getmaxyx(stdscr, term_height, term_width);
    
    int centerX = (term_width - art_width) / 2;
    int centerY = (term_height - art_height) / 2;

    for(int frame = 0; frame < frames; frame++) {
        clear();
        
        offsetX = (rand() % 3) - 1;
        offsetY = (rand() % 3) - 1;

        box(stdscr, 0, 0);
        
        attron(COLOR_PAIR(2) | A_BOLD);
        for(int i = 0; i < art_height; i++) {
            int yPos = centerY + i + offsetY;
            int xPos = centerX + offsetX;
            
            if(yPos >= 1 && yPos < term_height - 1) {
                move(yPos, xPos);
                addstr(explosion[i]);
            }
        }
        attroff(COLOR_PAIR(2) | A_BOLD);
        
        refresh();
        usleep(50000);
    }
}

void gerarValoresLogicos() {
    for(int i = 0; i < num_vars; i++) {
        vars[i].valor = rand() % 2;
    }
}

int getValorVariavel(char nome) {
    for(int i = 0; i < num_vars; i++) {
        if(vars[i].nome == nome) {
            return vars[i].valor;
        }
    }
    return 0;
}

int avaliarProposicao(int l, int c) {
    char *prop = jogo[l][c].proposicao;
    int p = getValorVariavel('P');
    int q = getValorVariavel('Q');
    
    if (strcmp(prop, "P") == 0) {
        return p;
    }
    else if (strcmp(prop, "¬P") == 0) {
        return !p;
    }
    else if (strcmp(prop, "P ∧ Q") == 0) {
        return p && q;
    }
    else if (strcmp(prop, "P ∨ Q") == 0) {
        return p || q;
    }
    else if (strcmp(prop, "P → Q") == 0) {
        return !p || q;
    }
    else if (strcmp(prop, "¬(P ∧ Q)") == 0) {
        return !p || !q;
    }
    else if (strcmp(prop, "P ↔ Q") == 0) {
        return p == q;
    }
    return 0;
}

void gerarDica(int l, int c) {
    if (!dicas[l][c].disponivel) return;
    
    char *prop = jogo[l][c].proposicao;
    char buffer[200];
    
    if (strcmp(prop, "P") == 0) {
        snprintf(buffer, sizeof(buffer), "Dica: Esta é uma proposição atômica. O resultado é igual ao valor de P.");
    }
    else if (strcmp(prop, "¬P") == 0) {
        snprintf(buffer, sizeof(buffer), "Dica: Esta é uma negação. O resultado é o oposto do valor de P.");
    }
    else if (strcmp(prop, "P ∧ Q") == 0) {
        snprintf(buffer, sizeof(buffer), "Dica: Uma conjunção (E) só é verdadeira quando AMBOS P e Q são verdadeiros.");
    }
    else if (strcmp(prop, "P ∨ Q") == 0) {
        snprintf(buffer, sizeof(buffer), "Dica: Uma disjunção (OU) é verdadeira quando P é verdadeiro OU Q é verdadeiro (ou ambos).");
    }
    else if (strcmp(prop, "P → Q") == 0) {
        snprintf(buffer, sizeof(buffer), "Dica: Uma implicação é falsa APENAS quando P é verdadeiro e Q é falso.");
    }
    else if (strcmp(prop, "¬(P ∧ Q)") == 0) {
        snprintf(buffer, sizeof(buffer), "Dica: Esta é a negação de uma conjunção. É verdadeira quando P é falso OU Q é falso.");
    }
    else if (strcmp(prop, "P ↔ Q") == 0) {
        snprintf(buffer, sizeof(buffer), "Dica: Uma bicondicional é verdadeira quando P e Q têm o mesmo valor lógico.");
    }
    
    if (dicas[l][c].texto != NULL) {
        free(dicas[l][c].texto);
    }
    
    dicas[l][c].texto = strdup(buffer);
    dicas[l][c].disponivel = 0;
}

void atualizarPontuacao(int acertou, int usouDica) {
    if (acertou) {
        if (!usouDica) {
            pontuacao += 15;
        } else {
            pontuacao += 8;
        }
    } else {
        pontuacao -= 10;
        if (usouDica) {
            pontuacao -= 5;
        }
    }

    if (pontuacao < 0) {
        pontuacao = 0;
    }
}

void mostrarValoresLogicos() {
    move(TAM*2 + 7, 2);
    printw("Valores: P = %s, Q = %s", 
           vars[0].valor ? "V" : "F",
           vars[1].valor ? "V" : "F");
    refresh();
}

void revelarCelulasVazias(int l, int c) {
    if (!coordenadaEhValida(l, c) || jogo[l][c].estaAberta) {
        return;
    }

    if (jogo[l][c].eBomba) {
        return;
    }

    jogo[l][c].estaAberta = 1;

    if (jogo[l][c].vizinhos > 0) {
        return;
    }

    int dl[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    for(int i = 0; i < 4; i++) {
        int nova_l = l + dl[i];
        int nova_c = c + dc[i];
        revelarCelulasVazias(nova_l, nova_c);
    }
}

void abrirCelula(int l, int c) {
    if (!coordenadaEhValida(l, c) || jogo[l][c].estaAberta)
        return;

    gerarValoresLogicos();
    
    for(int i = TAM*2 + 5; i <= TAM*2 + 9; i++) {
        move(i, 2);
        clrtoeol();
    }
    
    mvprintw(TAM*2 + 5, 2, "Proposicao: %s", jogo[l][c].proposicao);
    mvprintw(TAM*2 + 6, 2, "Pressione H para uma dica (-5 pontos) ou V/F para responder");
    mostrarValoresLogicos();
    refresh();

    int usouDica = 0;
    char resposta;
    do {
        resposta = getch();
        resposta = toupper(resposta);
        
        if (resposta == 'H' && dicas[l][c].disponivel) {
            pontuacao -= 5;
            gerarDica(l, c);
            mvprintw(TAM*2 + 9, 2, "%s", dicas[l][c].texto);
            refresh();
            usouDica = 1;
        }
    } while (resposta != 'V' && resposta != 'F');

    move(TAM*2 + 9, 2);
    clrtoeol();

    int avaliacao = avaliarProposicao(l, c);
    int acertou = (resposta == 'V' && avaliacao) || (resposta == 'F' && !avaliacao);

    if (acertou) {
        atualizarPontuacao(1, usouDica);
        
        if (!jogo[l][c].eBomba) {
            jogo[l][c].estaAberta = 1;
            
            if (jogo[l][c].vizinhos == 0) {
                int dl[] = {-1, 1, 0, 0};
                int dc[] = {0, 0, -1, 1};
                
                for(int i = 0; i < 4; i++) {
                    int nova_l = l + dl[i];
                    int nova_c = c + dc[i];
                    revelarCelulasVazias(nova_l, nova_c);
                }
            }
        } else {
            game_over = 1;
            acertou_bomba = 1;
            jogo[l][c].estaAberta = 1;
            pontuacao -= 30;
            if (pontuacao < 0) pontuacao = 0;
            
            for(int i = 0; i < TAM; i++) {
                for(int j = 0; j < TAM; j++) {
                    if(jogo[i][j].eBomba) {
                        jogo[i][j].estaAberta = 1;
                    }
                }
            }
            animarExplosao();
        }
    } else {
        game_over = 1;
        acertou_bomba = 0;
        atualizarPontuacao(0, usouDica);
        
        for(int i = 0; i < TAM; i++) {
            for(int j = 0; j < TAM; j++) {
                if(jogo[i][j].eBomba) {
                    jogo[i][j].estaAberta = 1;
                }
            }
        }
        animarExplosao();
    }
    
    desenharJogo();
}

int ganhou() {
    for(l = 0; l < TAM; l++) {
        for(c = 0; c < TAM; c++) {
            if(!jogo[l][c].estaAberta && !jogo[l][c].eBomba)
                return 0;
        }
    }
    vitoria = 1;
    return 1;
}

void desenharJogo() {
    clear();
    attron(A_BOLD);
    box(stdscr, 0, 0);
    attroff(A_BOLD);

    for(l = 0; l < TAM; l++) {
        for(c = 0; c < TAM; c++) {
            move(l*2 + 2, c*4 + 2);

            if(jogo[l][c].estaAberta) {
                if(l == linha_selecionada && c == coluna_selecionada && !game_over && !vitoria) {
                    attron(COLOR_PAIR(3) | A_BOLD);
                    if(jogo[l][c].eBomba) {
                        addstr(" * ");
                    } else if(jogo[l][c].vizinhos > 0) {
                        printw(" %d ", jogo[l][c].vizinhos);
                    } else {
                        addstr("   ");
                    }
                    attroff(COLOR_PAIR(3) | A_BOLD);
                } else {
                    if(jogo[l][c].eBomba) {
                        attron(COLOR_PAIR(5) | A_BOLD);
                        addstr(" * ");
                        attroff(COLOR_PAIR(5) | A_BOLD);
                    } else if(jogo[l][c].vizinhos > 0) {
                        attron(COLOR_PAIR(1));
                        printw(" %d ", jogo[l][c].vizinhos);
                        attroff(COLOR_PAIR(1));
                    } else {
                        addstr("   ");
                    }
                }
            } else {
                if(l == linha_selecionada && c == coluna_selecionada && !game_over && !vitoria) {
                    attron(COLOR_PAIR(3) | A_BOLD);
                    addstr(" ◆ ");
                    attroff(COLOR_PAIR(3) | A_BOLD);
                } else {
                    attron(COLOR_PAIR(4) | A_BOLD);
                    addstr(" ◆ ");
                    attroff(COLOR_PAIR(4) | A_BOLD);
                }
            }
        }
    }

    move(TAM*2 + 3, 2);
    printw("Pontuacao: %d", pontuacao);

    move(TAM*2 + 4, 2);
    if(game_over) {
        attron(COLOR_PAIR(5) | A_BOLD);
        if (acertou_bomba) {
            printw("GAME OVER! Voce acertou uma bomba!");
        } else {
            printw("GAME OVER! Voce errou a resposta!");
        }
        attroff(COLOR_PAIR(5) | A_BOLD);
        move(TAM*2 + 6, 2);
        printw("Aperte a tecla 1 para jogar novamente");
    }
    else if(vitoria) {
        attron(COLOR_PAIR(4) | A_BOLD);
        printw("PARABENS! VOCE GANHOU!");
        attroff(COLOR_PAIR(4) | A_BOLD);
        move(TAM*2 + 5, 2);
        printw("Aperte a tecla 1 para jogar novamente");
    }
    else {
        printw("Use as setas para navegar. ENTER para abrir celula.");
    }

    refresh();
}

void desenharMenu() {
    clear();
    attron(A_BOLD);
    box(stdscr, 0, 0);
    attroff(A_BOLD);

    int term_height, term_width;
    getmaxyx(stdscr, term_height, term_width);

    const char *title[] = {
        "███▄ ▄███▓ ██▓ ███▄    █  ▄▄▄      ▓█████▄  ▒█████   ██▀███  ",
        "▓██▒▀█▀ ██▒▓██▒ ██ ▀█   █ ▒████▄    ▒██▀ ██▌▒██▒  ██▒▓██ ▒ ██▒",
        "▓██    ▓██░▒██▒▓██  ▀█ ██▒▒██  ▀█▄  ░██   █▌▒██░  ██▒▓██ ░▄█ ▒",
        "▒██    ▒██ ░██░▓██▒  ▐▌██▒░██▄▄▄▄██ ░▓█▄   ▌▒██   ██░▒██▀▀█▄  ",
        "▒██▒   ░██▒░██░▒██░   ▓██░ ▓█   ▓██▒░▒████▓ ░ ████▓▒░░██▓ ▒██▒",
        "░ ▒░   ░  ░░▓  ░ ▒░   ▒ ▒  ▒▒   ▓▒█░ ▒▒▓  ▒ ░ ▒░▒░▒░ ░ ▒▓ ░▒▓░",
        "░  ░      ░ ▒ ░░ ░░   ░ ▒░  ▒   ▒▒ ░ ░ ▒  ▒   ░ ▒ ▒░   ░▒ ░ ▒░",
        "░      ░    ▒ ░   ░   ░ ░   ░   ▒    ░ ░  ░ ░ ░ ░ ▒    ░░   ░ ",
        "       ░    ░           ░       ░  ░   ░        ░ ░     ░     ",
        "                                      ░                        "
    };

    int art_height = 9;
    int art_width = 65;
    int start_y = (term_height / 4) - (art_height / 2);
    int start_x = (term_width - art_width) / 2;

    if (start_x < 1) start_x = 1;
    if (start_y < 1) start_y = 1;

    attron(COLOR_PAIR(4) | A_BOLD);
    for(int i = 0; i < art_height; i++) {
        mvprintw(start_y + i, start_x, "%s", title[i]);
    }
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(1));
    mvprintw(term_height/2 + 4, (term_width-15)/2, "1. Novo Jogo");
    mvprintw(term_height/2 + 6, (term_width-15)/2, "2. Como Jogar");
    mvprintw(term_height/2 + 8, (term_width-15)/2, "3. Sair");
    attroff(COLOR_PAIR(1));

    refresh();
}

void mostrarInstrucoes() {
    clear();
    box(stdscr, 0, 0);

    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(2, 2, "Como Jogar:");
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(1));
    mvprintw(4, 2, "- Use as setas do teclado para mover o cursor");
    mvprintw(5, 2, "- Pressione ENTER para revelar uma celula");
    mvprintw(6, 2, "- Avalie a proposicao logica (V/F) para evitar bombas");
    mvprintw(7, 2, "- Proposicoes: ∧ (AND), ∨ (OR), ¬ (NOT), → (IMPLICA), ↔ (BICOND)");
    mvprintw(9, 2, "Pressione qualquer tecla para voltar ao menu...");
    attroff(COLOR_PAIR(1));

    refresh();
    getch();
}

void desenharSelecaoDificuldade() {
    clear();
    attron(A_BOLD);
    box(stdscr, 0, 0);
    attroff(A_BOLD);

    int term_height, term_width;
    getmaxyx(stdscr, term_height, term_width);

    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(term_height/4, (term_width-25)/2, "SELECIONE A DIFICULDADE");
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(1));
    mvprintw(term_height/2 - 2, (term_width-35)/2, "1. Fácil    (15 bombas)");
    mvprintw(term_height/2, (term_width-35)/2, "2. Médio    (20 bombas)");
    mvprintw(term_height/2 + 2, (term_width-35)/2, "3. Difícil  (30 bombas)");
    mvprintw(term_height/2 + 4, (term_width-35)/2, "ESC - Voltar ao menu");
    attroff(COLOR_PAIR(1));

    refresh();
}

void executarJogo() {
    int ch = 0;
    int estado = 0;
    int num_bombas = 20;
    
    setlocale(LC_ALL, "");
    initscr();

    if(has_colors()) {
        start_color();
        init_pair(1, COLOR_BLUE, COLOR_BLACK);
        init_pair(2, COLOR_WHITE, COLOR_BLACK);
        init_pair(3, COLOR_BLACK, COLOR_WHITE);
        init_pair(4, COLOR_GREEN, COLOR_BLACK);
        init_pair(5, COLOR_RED, COLOR_BLACK);
    }

    use_default_colors();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    keyboardInit();
    resize_term(TAM*2 + 30, TAM*4 + 10);

    while(ch != 27) {
        if(estado == 0) {
            desenharMenu();
            ch = getch();
            
            switch(ch) {
                case '1':
                    estado = 1;
                    break;
                case '2':
                    mostrarInstrucoes();
                    break;
                case '3':
                    ch = 27;
                    break;
            }
        }
        else if(estado == 1) {
            desenharSelecaoDificuldade();
            ch = getch();
            
            switch(ch) {
                case '1':
                    num_bombas = 15;
                    estado = 2;
                    inicializarJogo();
                    sortearBombas(num_bombas);
                    contarBombas();
                    desenharJogo();
                    break;
                case '2':
                    num_bombas = 20;
                    estado = 2;
                    inicializarJogo();
                    sortearBombas(num_bombas);
                    contarBombas();
                    desenharJogo();
                    break;
                case '3':
                    num_bombas = 30;
                    estado = 2;
                    inicializarJogo();
                    sortearBombas(num_bombas);
                    contarBombas();
                    desenharJogo();
                    break;
                case 27:
                    estado = 0;
                    break;
            }
        }
        else if(estado == 2) {
            if(keyhit()) {
                ch = getch();
                int houve_mudanca = 0;

                if(!game_over && !vitoria) {
                    if(ch == KEY_UP && linha_selecionada > 0) {
                        linha_selecionada--;
                        houve_mudanca = 1;
                    }
                    else if(ch == KEY_DOWN && linha_selecionada < TAM-1) {
                        linha_selecionada++;
                        houve_mudanca = 1;
                    }
                    else if(ch == KEY_LEFT && coluna_selecionada > 0) {
                        coluna_selecionada--;
                        houve_mudanca = 1;
                    }
                    else if(ch == KEY_RIGHT && coluna_selecionada < TAM-1) {
                        coluna_selecionada++;
                        houve_mudanca = 1;
                    }
                    else if(ch == 10) {
                        abrirCelula(linha_selecionada, coluna_selecionada);
                        ganhou();
                        houve_mudanca = 1;
                    }
                    else if(ch == 27) {
                        estado = 0;
                        continue;
                    }
                }
                else if((game_over || vitoria) && ch == '1') {
                    estado = 0;
                    continue;
                }

                if(houve_mudanca) {
                    desenharJogo();
                }
            }
            napms(50);
        }
    }

    keyboardDestroy();
    endwin();
}


