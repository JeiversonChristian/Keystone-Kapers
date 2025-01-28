// versao IA
// ---------------------------------------------------------------------------------------------------
// Bibliotecas

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h> // Para Sleep()
#include <unistd.h> // Para usleep()

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
// ---------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// Structs

typedef struct Tecla{
	int espaco; // pular
	int d; // direita
	int a; // esquerda
	int w; // cima
	int s; // baixo
	int p; // pause
	int o; // play
	int i; // re-iniciar
}Tecla;

typedef struct Personagem{
	ALLEGRO_BITMAP *imagem; // imagem do personagem
	ALLEGRO_BITMAP *imagem_vitoria; // imagem que aparece na tela final
	ALLEGRO_BITMAP *imagens[4]; // guardará todas as imagens da animação de movimento
	int sentido_anima; // variável de controle
	int imagem_atual; // guardará o número da imagem atual
	float largura; // largura da imagem
	float altura; // altura da imagem
	float x; // coordenada do ponto de referência do retângulo que engloba a imagem
	float x_global; // para controlar globalmente onde o personagem está
	float y; // coordenada do ponto de referência do retângulo que engloba a imagem
	float vx; // velocidade no eixo x
	float vx_inicial;
	float vy; // velocidade no eixo y
	float direcao_pulo;
	int orientacao; // virado para direita ou para esquerda
	int andar; // andar onde se encontra
	int pode_andar; // variável de controle
	int andando; // variável de controle
	int pode_pular; // variável de controle
	int pulando; // variável de controle
	float y_chao; // altura do chão onde se encontra
	int num_tela; // cada cenário tem várias telas
	int dentro_elevador; // variável de controle
	int dentro_escada; // variável de controle
	int escada_num; // de controle em conjunto com dentro_escada
	int degrau_num; // de controle em conjunto com dentro_escada
	int na_lama; // variável de controle
	int ganhou; // variável de controle
	int no_poste;

	// para ia
	int num;
	int geracao;
	int pontos;
	float x_abs;
	float x_abs_anterior;
	float dist_x_inicial;
	float dist_x_ladrao_anterior;
	float dist_x_ladrao;
	// dist_x_ladrao | x_lama1 | y_lama1 | ... | x_lama6 | y_lama6 | x_escada1 | y_escada1 | ... | x_escada3 | y_escada3 | x_porta_elevador | y_porta_elevador | x_poste | y_poste |
	float inputs[23];
	float outputs_camada_oculta[5];
	// | w | s | a | d | space |
	float outputs[5];
	// 5 x 23
	float pesos[5][23];
	float pesos_camada_oculta[5][5];
	int eh_o_melhor;
	int rgb[3];
}Personagem;

typedef struct Elevador{
	float largura;
	float altura;
	float x;
	float x_global;
	float y;
	float y_chao;
	float num_sala;
	float andar;
	float y_porta;
	float y_chao_porta;
	float porta_aberta;
	int subir_descer; // para decidir se vai subir ou descer
}Elevador;

typedef struct Lama{
	float largura;
	float altura;
	float x;
	float x_global;
	float y;
	float y_chao;
	float num_sala;
	float andar;
}Lama;

typedef struct Degrau{
	float largura;
	float altura;
	float x_global;
	float x;
	float y;
	float y_chao;
	int num;
}Degrau;

typedef struct Escada{
	Degrau degraus[13];
	Degrau pe; // variável que marca o lugar do degrau que vem atrás do primeiro
	Degrau teto; // variável que marca o lugar do degrau que vem depois do último
	float num_sala;
	float andar;
	float vy; // velocidade pra cima
	float vx; // velocidade pro lado
	float x_global;
	float x;
	float y;
	float largura;
	float altura;
}Escada;

typedef struct Poste{
	float num_sala;
	float andar;
	float x_global;
	float x;
	float y;
	float largura;
	float altura;
}Poste;

typedef struct Mapa{
	Personagem policiais[50];
	Personagem ladrao;
	Elevador elevador;
	Escada escadas[3];
	Lama lamas[6];
	Poste poste;
	float x1;
	float x2;
	float y1;
	float y2;
	float largura;
	float altura;
	float proporcaox;
	float proporcaoy;
}Mapa;

typedef struct Mundo{
	ALLEGRO_BITMAP *imagem_cidade;
	float g; // gravidade
	float dt; // intervalo de tempo que passa no mundo
	Elevador elevador;
	Lama lamas[6];
	Escada escadas[3];
	Poste poste;
	Mapa mapa;
}Mundo;
// ---------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// Constantes

const float FPS = 60; 
// dimensão 4:3 -> 4x230 = 920 e 3x230 = 690
const int SCREEN_W = 960;
const int SCREEN_H = 690;
const int TEMPO_LIMITE = 80;
const int NUM_POLICIAIS = 50;
// ---------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// Funções

void mutar_policial(Personagem *policial, float pesos_melhor[5][23], float pesos_camada_oculta_melhor[5][5], int num_max_policiais, int taxa_mutacao, float pesos_melhor2[5][23], float pesos_camada_oculta_melhor2[5][5], float pesos_melhor3[5][23], float pesos_camada_oculta_melhor3[5][5]){
	// print teste
	int i, j, acrescimo;
	for (i=0; i<5; i++){
		for (j=0; j<23; j++){
			if ((*policial).num == NUM_POLICIAIS)
				(*policial).pesos[i][j] = pesos_melhor[i][j];
			else if((*policial).num % 2 == 0){
				// "acrescimo" de valor int no intervalo taxa_mutacao * [-100, 100]
				acrescimo = taxa_mutacao*((rand() % 201) - 100);
				if (pesos_melhor[i][j] + (float)acrescimo > 1000)
					(*policial).pesos[i][j] = 1000.0;
				else if (pesos_melhor[i][j] + (float)acrescimo < -1000)
					(*policial).pesos[i][j] = -1000.0;
				else
					(*policial).pesos[i][j] = pesos_melhor[i][j] + (float)acrescimo;
			}
			else if((*policial).num % 3 == 0){
				// "acrescimo" de valor int no intervalo taxa_mutacao * [-100, 100]
				acrescimo = (taxa_mutacao+1)*((rand() % 201) - 100);
				if (pesos_melhor2[i][j] + (float)acrescimo > 1000)
					(*policial).pesos[i][j] = 1000.0;
				else if (pesos_melhor2[i][j] + (float)acrescimo < -1000)
					(*policial).pesos[i][j] = -1000.0;
				else
					(*policial).pesos[i][j] = pesos_melhor2[i][j] + (float)acrescimo;
			}
			else{
				// "acrescimo" de valor int no intervalo taxa_mutacao * [-100, 100]
				acrescimo = (taxa_mutacao+2)*((rand() % 201) - 100);
				if (pesos_melhor3[i][j] + (float)acrescimo > 1000)
					(*policial).pesos[i][j] = 1000.0;
				else if (pesos_melhor3[i][j] + (float)acrescimo < -1000)
					(*policial).pesos[i][j] = -1000.0;
				else
					(*policial).pesos[i][j] = pesos_melhor3[i][j] + (float)acrescimo;
			}
		}
	}
	for (i=0; i<5; i++){
		for (j=0; j<5; j++){
			if ((*policial).num == NUM_POLICIAIS)
				(*policial).pesos_camada_oculta[i][j] = pesos_camada_oculta_melhor[i][j];
			else if((*policial).num == NUM_POLICIAIS - 1){
				(*policial).pesos_camada_oculta[i][j] = pesos_camada_oculta_melhor2[i][j];
			}
			else if((*policial).num % 2 == 0){
				// "acrescimo" de valor int no intervalo [-300, 300] convertido para float
				acrescimo = taxa_mutacao*((rand() % 201) - 100);
				if (pesos_camada_oculta_melhor[i][j] + (float)acrescimo > 1000)
					(*policial).pesos_camada_oculta[i][j] = 1000.0;
				else if (pesos_camada_oculta_melhor[i][j] + (float)acrescimo < -1000)
					(*policial).pesos_camada_oculta[i][j] = -1000.0;
				else
					(*policial).pesos_camada_oculta[i][j] = pesos_camada_oculta_melhor[i][j] + (float)acrescimo;
			}
			else{
				// "acrescimo" de valor int no intervalo [-300, 300] convertido para float
				acrescimo = taxa_mutacao*((rand() % 201) - 100);
				if (pesos_camada_oculta_melhor2[i][j] + (float)acrescimo > 1000)
					(*policial).pesos_camada_oculta[i][j] = 1000.0;
				else if (pesos_camada_oculta_melhor2[i][j] + (float)acrescimo < -1000)
					(*policial).pesos_camada_oculta[i][j] = -1000.0;
				else
					(*policial).pesos_camada_oculta[i][j] = pesos_camada_oculta_melhor2[i][j] + (float)acrescimo;
			}
		}
	}
}

void calculcar_pesos(Personagem *policial){
	int i, j;

	// para treinar o policial ------------------------------------------------
	for (i=0; i<5; i++){
		for (j=0; j<23; j++){
			// valor int no intervalo [-1000, 1000] convertido para float
			(*policial).pesos[i][j] = (float)((rand() % 2001) - 1000);
		}
	}
	// -----------------------------------------------------------------------

	// para mostrar o policial treinado -----------------------------------------------------------------------------------------
	/*if ((*policial).num == NUM_POLICIAIS){

		float p[5][23] = { 
		{-720, 1000, -1000, 70, -765, 111, 676, -424, -1000, 940, 861, -473, -351, -599, -956, 752, 223, 50, 376, 1000, 800, 1000, -269},
        {144, 1000, -744, 143, 1000, 67, -522, -1000, 490, -285, -889, 431, -284, 11, 200, -416, -750, -406, -493, -993, 1000, -122, 137},
        {-1000, 910, 1, 741, -470, -1000, -1000, -710, 1000, 33, -94, -16, 84, 420, 6, 1000, -754, 395, 194, 1000, -60, -1000, -665},
        {-502, 1000, -645, -1000, 1000, 122, -1000, 1000, 1000, -31, 88, 1000, -1000, -1000, 218, -1000, -528, -63, 1000, 596, -626, 856, 640},
        {-58, -216, -902, 1000, 450, -1000, 1000, 1000, 311, 92, 215, -925, 58, 682, -439, -480, -920, 170, 350, 1000, 360, -690, 1000}
    	};
		for (i=0; i<5; i++){
			for (j=0; j<23; j++){
				(*policial).pesos[i][j] = p[i][j];
			}
		}
	}
	else{
		for (i=0; i<5; i++){
			for (j=0; j<23; j++){
				// valor int no intervalo [-1000, 1000] convertido para float
				(*policial).pesos[i][j] = (float)((rand() % 2001) - 1000);
			}
		}
	}*/
// ---------------------------------------------------------------------------------------------------------------------------
}

void calculcar_pesos_camada_oculta(Personagem *policial){
	int i, j;

	// para treinar o policial ------------------------------------------------
	for (i=0; i<5; i++){
		for (j=0; j<5; j++){
			// valor int no intervalo [-1000, 1000] convertido para float
			(*policial).pesos_camada_oculta[i][j] = (float)((rand() % 2001) - 1000);
		}
	}
	// -----------------------------------------------------------------------

	// para mostrar o policial treinado -----------------------------------------------------------------------------------------
	/*if ((*policial).num == NUM_POLICIAIS){
		float p_oculta[5][5] = {
		{288, 408, 352, -720, 72},
		{256, 246, -519, 1000, 441},
		{-344, 1000, -696, -175, 124},
		{349, -680, -581, 704, -431},
		{-1000, -161, -1000, -32, 1000}
		};
		for (i=0; i<5; i++){
			for (j=0; j<5; j++){
				(*policial).pesos_camada_oculta[i][j] = p_oculta[i][j];
			}
		}
	}
	else{
		for (i=0; i<5; i++){
			for (j=0; j<5; j++){
				// valor int no intervalo [-1000, 1000] convertido para float
				(*policial).pesos_camada_oculta[i][j] = (float)((rand() % 2001) - 1000);
			}
		}
	}*/
	// ---------------------------------------------------------------------------------------------------------------------------

}

void calcular_inputs(Personagem *policial, Mundo mundo){
	int i;
	(*policial).inputs[0] = (*policial).dist_x_ladrao;
	for (i=0; i<6; i++){
		(*policial).inputs[1+i] = mundo.lamas[i].x_global;
	}
	for (i=0; i<6; i++){
		(*policial).inputs[7+i] = mundo.lamas[i].y;
	}
	for (i=0; i<3; i++){
		(*policial).inputs[13+i] = mundo.escadas[i].x_global;
	}
	for (i=0; i<3; i++){
		(*policial).inputs[16+i] = mundo.escadas[i].y;
	}
	(*policial).inputs[19] = mundo.elevador.x_global;
	(*policial).inputs[20] = mundo.elevador.y_porta;
	(*policial).inputs[21] = mundo.poste.x_global;
	(*policial).inputs[22] = mundo.poste.y;
}

void calcular_outputs_camada_oculta(Personagem *policial){
	int o, i;
	for (o=0; o<5; o++){
		(*policial).outputs_camada_oculta[o] = 0;
	}
	for (o=0; o<5; o++){
		for (i=0; i<23; i++){
			(*policial).outputs_camada_oculta[o] += (*policial).inputs[i] * (*policial).pesos[o][i];
		}
	}
	for (o=0; o<5; o++){
		if ((*policial).outputs_camada_oculta[o] < 0)
			(*policial).outputs_camada_oculta[o] = 0;
	}

}

void calcular_outputs(Personagem *policial){
	int o, i;
	for (o=0; o<5; o++){
		(*policial).outputs[o] = 0;
	}
	for (o=0; o<5; o++){
		for (i=0; i<5; i++){
			(*policial).outputs[o] += (*policial).outputs_camada_oculta[i] * (*policial).pesos_camada_oculta[o][i];
		}
	}
	for (o=0; o<5; o++){
		if ((*policial).outputs[o] > 0)
			(*policial).outputs[o] = 1;
		else
			(*policial).outputs[o] = 0;
	}
}

void calcular_x_abs_policial(Personagem *policial){
	if ((*policial).andar == 1)
		(*policial).x_abs = 4*SCREEN_W - (*policial).x_global;
	else if ((*policial).andar == 2)
		(*policial).x_abs = 4*SCREEN_W + (*policial).x_global;
	else if (((*policial).andar == 3))
		(*policial).x_abs = 2*4*SCREEN_W + (4*SCREEN_W - (*policial).x_global);
	else if (((*policial).andar == 4))
		(*policial).x_abs = 3*4*SCREEN_W + (*policial).x_global;
}

void calcular_x_abs_ladrao(Personagem *ladrao){
	if ((*ladrao).andar == 1)
		(*ladrao).x_abs = 4*SCREEN_W - (*ladrao).x_global;
	else if ((*ladrao).andar == 2)
		(*ladrao).x_abs = 4*SCREEN_W + (*ladrao).x_global;
	else if (((*ladrao).andar == 3))
		(*ladrao).x_abs = 2*4*SCREEN_W + (4*SCREEN_W - (*ladrao).x_global);
	else if (((*ladrao).andar == 4))
		(*ladrao).x_abs = 3*4*SCREEN_W + (*ladrao).x_global;
}

float calcular_dist_policia_ladaro_x(Personagem policial, Personagem ladrao){
	return abs(policial.x_abs - ladrao.x_abs);
}

void calcular_pontos(Personagem *policial, Personagem ladrao){
	if ((*policial).andar == ladrao.andar){
		(*policial).pontos += 4;
		if ((*policial).num_tela == ladrao.num_tela){
			(*policial).pontos += 4;
		}
	}

	if ((*policial).dist_x_ladrao < (*policial).dist_x_ladrao_anterior){
		(*policial).pontos += 5;
	}
	else if ((*policial).dist_x_ladrao > (*policial).dist_x_ladrao_anterior){
		(*policial).pontos -= 3;
	}
	
	if ((*policial).x_abs == (*policial).x_abs_anterior){
		(*policial).pontos -= 1;
	}

	if ((*policial).na_lama == 1){
		(*policial).pontos -= 10;
	}
}

void inicializar_structs(Tecla *teclas, Personagem policiais[NUM_POLICIAIS], ALLEGRO_BITMAP *imagem_policial, Personagem *ladrao, ALLEGRO_BITMAP *imagem_ladrao, Mundo *mundo, ALLEGRO_BITMAP *imagem_cidade, ALLEGRO_BITMAP *imagem_policial_vitorioso, ALLEGRO_BITMAP *imagem_ladrao_vitorioso, ALLEGRO_BITMAP *imagem_policial2, ALLEGRO_BITMAP *imagem_policial3, ALLEGRO_BITMAP *imagem_policial4, ALLEGRO_BITMAP *imagem_ladrao2, ALLEGRO_BITMAP *imagem_ladrao3, ALLEGRO_BITMAP *imagem_ladrao4, int num_policial, int ia_jogando){
	int i;
	int j;

	// teclas
	// 0 -> não pressionada
	(*teclas).espaco = 0;
	(*teclas).d = 0;
	(*teclas).a = 0;
	(*teclas).w = 0;
	(*teclas).s = 0;
	(*teclas).p = 0;
	(*teclas).o = 0;
	(*teclas).i = 0;

	// Ladrão
	(*ladrao).imagem = imagem_ladrao;
	(*ladrao).imagem_vitoria = imagem_ladrao_vitorioso;
	(*ladrao).imagens[0] = imagem_ladrao;
	(*ladrao).imagens[1] = imagem_ladrao2;
	(*ladrao).imagens[2] = imagem_ladrao3;
	(*ladrao).imagens[3] = imagem_ladrao4;
	(*ladrao).sentido_anima = 1; // indo da esquerda pra direita nas imagens
	(*ladrao).imagem_atual = 0;
	(*ladrao).largura = al_get_bitmap_width(imagem_ladrao);
	(*ladrao).altura = al_get_bitmap_height(imagem_ladrao);
	(*ladrao).x = SCREEN_W/4 - (*ladrao).largura/2;; // 1/4 de um do andar
	(*ladrao).x_global = 0*SCREEN_W + (*ladrao).x; // na 4ª sala (tela)
	(*ladrao).y = 4*SCREEN_H/6.0 - (*ladrao).altura; // andar do meio
	//(*ladrao).vx_inicial = 3.1; // ligeiramente mais lento que o policial
	// a velocidade é o tempo suficiente pra ir lá em cima e volta mais ou menos
	(*ladrao).vx_inicial = 3.0; 
	(*ladrao).vx = (*ladrao).vx_inicial;
	(*ladrao).vy = 0;
	(*ladrao).direcao_pulo = 0.0; // pulando em nenhuma direção
	(*ladrao).orientacao = 0; // virado pra direita
	(*ladrao).andar = 2; // acima do policial
	(*ladrao).pode_pular = 1; // sim, mas não sei se ele vai pular ainda
	(*ladrao).pulando = 0; //não
	(*ladrao).pode_andar = 1; // sim
	(*ladrao).andando = 1; // sim
	(*ladrao).y_chao = (*ladrao).y + (*ladrao).altura; // y do chão abaixo do ladrão
	(*ladrao).num_tela = 1; // tela que o bandido "vê"
	(*ladrao).dentro_elevador = 0; // será que faço ele aprender a entrar?
	(*ladrao).dentro_escada = 0; // idem
	(*ladrao).escada_num = -1; // nenhum
	(*ladrao).degrau_num = -1; // nenhum
	(*ladrao).na_lama = 0;
	(*ladrao).ganhou = 0;
	(*ladrao).no_poste = 0;

	// Policial
	for (i=0; i<NUM_POLICIAIS; i++){
		policiais[i].imagem = imagem_policial;
		policiais[i].imagem_vitoria = imagem_policial_vitorioso;
		policiais[i].imagens[0] = imagem_policial;
		policiais[i].imagens[1] = imagem_policial2;
		policiais[i].imagens[2] = imagem_policial3;
		policiais[i].imagens[3] = imagem_policial4;
		policiais[i].sentido_anima = 1; // indo da esquerda pra direita nas imagens
		policiais[i].imagem_atual = 0;
		policiais[i].largura = al_get_bitmap_width(imagem_policial);
		policiais[i].altura = al_get_bitmap_height(imagem_policial);
		policiais[i].x = SCREEN_W/2 - policiais[i].largura/2; // no meio da sala (tela)
		policiais[i].x_global = 3*SCREEN_W + policiais[i].x; // quase no final já
		policiais[i].y = 5*SCREEN_H/6.0 - policiais[i].altura; // no primeiro andar
		policiais[i].vx_inicial = 1.6 * (*ladrao).vx_inicial; // ligeiramente mais rápido que o ladrão
		policiais[i].vx = policiais[i].vx_inicial; 
		policiais[i].vy = 50; // a lógica do pulo é outra
		policiais[i].direcao_pulo = 0.0; // pulando em nenhuma direção
		policiais[i].orientacao = 1; // virado pra esquerda
		policiais[i].andar = 1; // primeiro andar
		policiais[i].pode_pular = 1; // sim
		policiais[i].pulando = 0; // não
		policiais[i].pode_andar = 1; // sim
		policiais[i].andando = 0; // não
		policiais[i].y_chao = policiais[i].y + policiais[i].altura; // y do chão abaixo do policial
		policiais[i].num_tela = 1; // tela que o jogador vê
		policiais[i].dentro_elevador = 0; // começa fora
		policiais[i].dentro_escada = 0; // começa fora
		policiais[i].escada_num = -1; // nenhum
		policiais[i].degrau_num = -1; // nenhum
		policiais[i].na_lama = 0;
		policiais[i].ganhou = 0;
		policiais[i].no_poste = 0;
	}

	// mundo
	(*mundo).imagem_cidade = imagem_cidade;
	(*mundo).g = 25.0; 
	(*mundo).dt = 5.0/FPS;
	
	// -> elevador
	(*mundo).elevador.largura = (1.5)*policiais[0].largura;
	(*mundo).elevador.altura = policiais[0].altura + policiais[0].altura/4 + policiais[0].altura/8;
	(*mundo).elevador.x = SCREEN_W/2 - (*mundo).elevador.largura/5;
	(*mundo).elevador.x_global = 0*SCREEN_W + (*mundo).elevador.x; // última sala (tela)
	(*mundo).elevador.y = 5*SCREEN_H/6.0 - (*mundo).elevador.altura; // no primeiro andar
	(*mundo).elevador.y_chao = (*mundo).elevador.y + (*mundo).elevador.altura; // y do chão
	(*mundo).elevador.num_sala = 4;
	(*mundo).elevador.andar = 1;
	(*mundo).elevador.y_porta = (*mundo).elevador.y;
	(*mundo).elevador.y_chao_porta = (*mundo).elevador.y_chao;
	(*mundo).elevador.porta_aberta = 1; // 1 aberto, 0 fechado
	(*mundo).elevador.subir_descer = 1; // -1 subindo; 1 descendo
	
	// -> lama	
	(*mundo).lamas[0].largura = 2*policiais[0].largura; 
	(*mundo).lamas[0].altura = policiais[0].altura/16; // fininha
	(*mundo).lamas[0].x = 0; 
	(*mundo).lamas[0].x_global = 3*SCREEN_W; // 1ª tela
	(*mundo).lamas[0].y = policiais[0].y_chao; // 1º andar
	(*mundo).lamas[0].y_chao = policiais[0].y_chao - (*mundo).lamas[0].altura; 
	(*mundo).lamas[0].num_sala = 1;
	(*mundo).lamas[0].andar = 1;

	(*mundo).lamas[1].largura = 2*policiais[0].largura; 
	(*mundo).lamas[1].altura = policiais[0].altura/16; // fininha
	(*mundo).lamas[1].x_global = 2*SCREEN_W + SCREEN_W/2 - ((*mundo).lamas[1].largura)/2; //2ª tela
	(*mundo).lamas[1].x = (int)((*mundo).lamas[1]).x_global % SCREEN_W;
	(*mundo).lamas[1].y = policiais[0].y_chao; // 1º andar
	(*mundo).lamas[1].y_chao = policiais[0].y_chao - (*mundo).lamas[0].altura;
	(*mundo).lamas[1].num_sala = 2;
	(*mundo).lamas[1].andar = 1;

	(*mundo).lamas[2].largura = 2*policiais[0].largura; 
	(*mundo).lamas[2].altura = policiais[0].altura/16; // fininha
	(*mundo).lamas[2].x_global = 3*SCREEN_W + SCREEN_W/5; // 1ª tela
	(*mundo).lamas[2].x = (int)((*mundo).lamas[2]).x_global % SCREEN_W;
	(*mundo).lamas[2].y = (*mundo).lamas[1].y - SCREEN_H/6; // 2º andar
	(*mundo).lamas[2].y_chao = policiais[0].y_chao - (*mundo).lamas[0].altura - SCREEN_H/6;
	(*mundo).lamas[2].num_sala = 1;
	(*mundo).lamas[2].andar = 2;

	(*mundo).lamas[3].largura = 2*policiais[0].largura; 
	(*mundo).lamas[3].altura = policiais[0].altura/16; // fininha
	(*mundo).lamas[3].x_global = 0*SCREEN_W + SCREEN_W/4; // 4ª tela
	(*mundo).lamas[3].x = (int)((*mundo).lamas[3]).x_global % SCREEN_W;
	(*mundo).lamas[3].y = (*mundo).lamas[1].y - SCREEN_H/6; // 2º andar
	(*mundo).lamas[3].y_chao = policiais[0].y_chao - (*mundo).lamas[0].altura - SCREEN_H/6;
	(*mundo).lamas[3].num_sala = 4;
	(*mundo).lamas[3].andar = 2;

	(*mundo).lamas[4].largura = 2*policiais[0].largura; 
	(*mundo).lamas[4].altura = policiais[0].altura/16; // fininha
	(*mundo).lamas[4].x_global = 1*SCREEN_W; // 3ª tela
	(*mundo).lamas[4].x = (int)((*mundo).lamas[4]).x_global % SCREEN_W;
	(*mundo).lamas[4].y = (*mundo).lamas[1].y - 2*SCREEN_H/6; // 3º andar
	(*mundo).lamas[4].y_chao = policiais[0].y_chao - (*mundo).lamas[0].altura - 2*SCREEN_H/6;
	(*mundo).lamas[4].num_sala = 3;
	(*mundo).lamas[4].andar = 3;

	(*mundo).lamas[5].largura = 2*policiais[0].largura; 
	(*mundo).lamas[5].altura = policiais[0].altura/16; // fininha
	(*mundo).lamas[5].x_global = 2*SCREEN_W + (*mundo).lamas[5].largura; // 2ª tela
	(*mundo).lamas[5].x = (int)((*mundo).lamas[5]).x_global % SCREEN_W;
	(*mundo).lamas[5].y = (*mundo).lamas[1].y - 2*SCREEN_H/6; // 3º andar
	(*mundo).lamas[5].y_chao = policiais[0].y_chao - (*mundo).lamas[0].altura - 2*SCREEN_H/6;
	(*mundo).lamas[5].num_sala = 2;
	(*mundo).lamas[5].andar = 3;

	// -> escada
	// escada 1 -> 1ª tela, 2º andar
	// ------------------------------------------------------------------------------------------------
	(*mundo).escadas[1].num_sala = 1;
	(*mundo).escadas[1].andar = 2;

	(*mundo).escadas[1].degraus[0].largura = policiais[0].largura/2;
	(*mundo).escadas[1].degraus[0].altura = 0.8*(SCREEN_H/6)/8; // 80% de 1/8 de um andar
	(*mundo).escadas[1].degraus[0].x_global = 4*SCREEN_W - SCREEN_W/2 - (*mundo).escadas[1].degraus[0].largura;
	(*mundo).escadas[1].degraus[0].x = (int)(*mundo).escadas[1].degraus[0].x_global % SCREEN_W;
	(*mundo).escadas[1].degraus[0].y = 4*(SCREEN_H/6);
	(*mundo).escadas[1].degraus[0].y_chao = 4*(SCREEN_H/6) + (*mundo).escadas[1].degraus[0].altura;
	(*mundo).escadas[1].degraus[0].num = 0;

	(*mundo).escadas[1].pe.x_global = (*mundo).escadas[1].degraus[0].x_global;
	(*mundo).escadas[1].pe.x = (*mundo).escadas[1].degraus[0].x;
	(*mundo).escadas[1].pe.y = (*mundo).escadas[1].degraus[0].y;
	(*mundo).escadas[1].pe.y_chao = (*mundo).escadas[1].degraus[0].y_chao;
	(*mundo).escadas[1].pe.num = 0;

	for (i=1; i<12; i++){
		(*mundo).escadas[1].degraus[i].largura = policiais[0].largura/2;
		(*mundo).escadas[1].degraus[i].altura = 0.8*(SCREEN_H/6)/8;
		(*mundo).escadas[1].degraus[i].x_global = 4*SCREEN_W - SCREEN_W/2 + (i-1)*(*mundo).escadas[1].degraus[i].largura;
		(*mundo).escadas[1].degraus[i].x = (int)(*mundo).escadas[1].degraus[i].x_global % SCREEN_W;
		(*mundo).escadas[1].degraus[i].y = 4*(SCREEN_H/6) - (i-1)*(*mundo).escadas[1].degraus[i].altura;
		(*mundo).escadas[1].degraus[i].y_chao = 4*(SCREEN_H/6) + (*mundo).escadas[1].degraus[i].altura - (i-1)*(*mundo).escadas[1].degraus[i].altura;
		(*mundo).escadas[1].degraus[i].num = i;
	}

	(*mundo).escadas[1].degraus[12].largura = policiais[0].largura/2;
	(*mundo).escadas[1].degraus[12].altura = 0.8*(SCREEN_H/6)/8;
	(*mundo).escadas[1].degraus[12].x_global = (*mundo).escadas[1].degraus[11].x_global + (*mundo).escadas[1].degraus[12].largura;
	(*mundo).escadas[1].degraus[12].x = (int)(*mundo).escadas[1].degraus[12].x_global % SCREEN_W;
	(*mundo).escadas[1].degraus[12].y = (*mundo).escadas[1].degraus[11].y;
	(*mundo).escadas[1].degraus[12].y_chao = (*mundo).escadas[1].degraus[11].y_chao;
	(*mundo).escadas[1].degraus[12].num = 12;

	(*mundo).escadas[1].teto.x_global = (*mundo).escadas[1].degraus[12].x_global;
	(*mundo).escadas[1].teto.x = (*mundo).escadas[1].degraus[12].x;
	(*mundo).escadas[1].teto.y = (*mundo).escadas[1].degraus[12].y;
	(*mundo).escadas[1].teto.y_chao = (*mundo).escadas[1].degraus[12].y_chao;
	(*mundo).escadas[1].teto.num = 12;

	// é necessário percorrer um degrau em x ao mesmo tempo em que se percorre em y
	(*mundo).escadas[1].vy = (*mundo).escadas[1].degraus[0].altura/8;
	(*mundo).escadas[1].vx = (*mundo).escadas[1].degraus[0].largura/8;

	(*mundo).escadas[1].x_global = (*mundo).escadas[1].degraus[1].x_global;
	(*mundo).escadas[1].x = (int)(*mundo).escadas[1].x_global % SCREEN_W;
	(*mundo).escadas[1].largura = (*mundo).escadas[1].degraus[11].x_global - (*mundo).escadas[1].degraus[1].x_global;

	(*mundo).escadas[1].y =  (*mundo).escadas[1].degraus[12].y;
	(*mundo).escadas[1].altura = (*mundo).escadas[1].y - (*mundo).escadas[1].degraus[0].y;
	// ------------------------------------------------------------------------------------------------

	// escada 0 -> 4ª tela, 1º andar
	// escada 2 -> 4ª tela, 3º andar
	// ------------------------------------------------------------------------------------------------
	for (j=0; j<=2; j+=2){
		(*mundo).escadas[j].num_sala = 4;
		(*mundo).escadas[j].andar = 1+j;
		(*mundo).escadas[j].degraus[12].largura = policiais[0].largura/2;
		(*mundo).escadas[j].degraus[12].altura = 0.8*(SCREEN_H/6)/8;
		(*mundo).escadas[j].degraus[12].x_global = 0;
		(*mundo).escadas[j].degraus[12].x = (int)(*mundo).escadas[j].degraus[12].x_global % SCREEN_W;
		(*mundo).escadas[j].degraus[12].y = (4-j)*(SCREEN_H/6);
		(*mundo).escadas[j].degraus[12].y_chao = (*mundo).escadas[j].degraus[12].y + (*mundo).escadas[j].degraus[12].altura;
		(*mundo).escadas[j].degraus[12].num = 12;

		(*mundo).escadas[j].teto.x_global = (*mundo).escadas[j].degraus[12].x_global;
		(*mundo).escadas[j].teto.x = (*mundo).escadas[j].degraus[12].x;
		(*mundo).escadas[j].teto.y = (*mundo).escadas[j].degraus[12].y;
		(*mundo).escadas[j].teto.y_chao = (*mundo).escadas[j].degraus[12].y_chao;
		(*mundo).escadas[j].teto.num = 12;

		(*mundo).escadas[j].degraus[0].largura = policiais[0].largura/2;
		(*mundo).escadas[j].degraus[0].altura = 0.8*(SCREEN_H/6)/8;
		(*mundo).escadas[j].degraus[0].x_global = (*mundo).escadas[j].degraus[12].x_global + 12*(*mundo).escadas[j].degraus[12].largura;
		(*mundo).escadas[j].degraus[0].x = (int)(*mundo).escadas[j].degraus[0].x_global % SCREEN_W;
		(*mundo).escadas[j].degraus[0].y = (5-j)*(SCREEN_H/6);
		(*mundo).escadas[j].degraus[0].y_chao = (*mundo).escadas[j].degraus[0].y + (*mundo).escadas[j].degraus[0].altura;
		(*mundo).escadas[j].degraus[0].num = 0;

		(*mundo).escadas[j].pe.x_global = (*mundo).escadas[j].degraus[0].x_global;
		(*mundo).escadas[j].pe.x = (*mundo).escadas[j].degraus[0].x;
		(*mundo).escadas[j].pe.y = (*mundo).escadas[j].degraus[0].y;
		(*mundo).escadas[j].pe.y_chao = (*mundo).escadas[j].degraus[0].y_chao;
		(*mundo).escadas[j].pe.num = 0;

		for (i=1; i<12; i++){
			(*mundo).escadas[j].degraus[i].largura = policiais[0].largura/2;
			(*mundo).escadas[j].degraus[i].altura = 0.8*(SCREEN_H/6)/8;
			(*mundo).escadas[j].degraus[i].x_global = (*mundo).escadas[j].degraus[i-1].x_global - (*mundo).escadas[j].degraus[i-1].largura;
			(*mundo).escadas[j].degraus[i].x = (int)(*mundo).escadas[j].degraus[i].x_global % SCREEN_W;
			if (i != 1 && i != 11){
				(*mundo).escadas[j].degraus[i].y = (*mundo).escadas[j].degraus[i-1].y - (*mundo).escadas[j].degraus[i-1].altura;
				
			}
			else if(i == 1){
				(*mundo).escadas[j].degraus[i].y = (*mundo).escadas[j].degraus[0].y;
			}
			else{
				(*mundo).escadas[j].degraus[i].y = (*mundo).escadas[j].degraus[12].y;
			}
			(*mundo).escadas[j].degraus[i].y_chao = (*mundo).escadas[j].degraus[i].y + (*mundo).escadas[j].degraus[i].altura;
			(*mundo).escadas[j].degraus[i].num = i;
		}

		(*mundo).escadas[j].vy = (*mundo).escadas[j].degraus[0].altura/8;
		(*mundo).escadas[j].vx = (*mundo).escadas[j].degraus[0].largura/8;

		(*mundo).escadas[j].x_global = (*mundo).escadas[j].degraus[11].x_global;
		(*mundo).escadas[j].x = (int)(*mundo).escadas[j].x_global % SCREEN_W;
		(*mundo).escadas[j].largura = (*mundo).escadas[j].degraus[1].x_global - (*mundo).escadas[j].degraus[11].x_global;

		(*mundo).escadas[j].y =  (*mundo).escadas[j].degraus[12].y;
		(*mundo).escadas[j].altura = abs((*mundo).escadas[j].y - (*mundo).escadas[j].degraus[0].y);
	}

	// poste dos bombeiros
	(*mundo).poste.andar = 4;
	(*mundo).poste.num_sala = 1;
	(*mundo).poste.largura = 10;
	(*mundo).poste.x_global = 4*SCREEN_W - 2*(*mundo).poste.largura; // quase no final da tela
	(*mundo).poste.x = (int)(*mundo).poste.x_global % SCREEN_W;
	(*mundo).poste.y = 2*SCREEN_H/6 - 10;
	(*mundo).poste.altura = 5*SCREEN_H/6 - (*mundo).poste.y;	

	// ------------------------------------------------------------------------------------------------
	// --> mapa
	// esses dados eu tirei da função de desenhar o cenário
	(*mundo).mapa.x1 = SCREEN_W/4.0;
	(*mundo).mapa.x2 = 3*SCREEN_W/4.0;
	(*mundo).mapa.y1 = 5*SCREEN_H/6.0 + 2*SCREEN_H/60.0;
	(*mundo).mapa.y2 = SCREEN_H;
	(*mundo).mapa.largura = (*mundo).mapa.x2 - (*mundo).mapa.x1;
	(*mundo).mapa.altura = (*mundo).mapa.y2 - (*mundo).mapa.y1;
	(*mundo).mapa.proporcaox = (*mundo).mapa.largura / (4*SCREEN_W);
	(*mundo).mapa.proporcaoy = (*mundo).mapa.altura / (SCREEN_H);
	// -- > mapa --> policial
	for (i=0; i<NUM_POLICIAIS; i++){
		(*mundo).mapa.policiais[i].x = (*mundo).mapa.x1 + policiais[0].x_global * (*mundo).mapa.proporcaox;
		(*mundo).mapa.policiais[i].y = (*mundo).mapa.y1 + policiais[0].y * (*mundo).mapa.proporcaoy;
		(*mundo).mapa.policiais[i].largura = policiais[0].largura * (*mundo).mapa.proporcaox;
		(*mundo).mapa.policiais[i].altura = policiais[0].altura * (*mundo).mapa.proporcaoy;
	}
	// -- > mapa --> ladrao
	(*mundo).mapa.ladrao.x = (*mundo).mapa.x1 + (*ladrao).x_global * (*mundo).mapa.proporcaox;
	(*mundo).mapa.ladrao.y = (*mundo).mapa.y1 + (*ladrao).y * (*mundo).mapa.proporcaoy;
	(*mundo).mapa.ladrao.largura = (*ladrao).largura * (*mundo).mapa.proporcaox;
	(*mundo).mapa.ladrao.altura = (*ladrao).altura * (*mundo).mapa.proporcaoy;
	// -- > mapa --> escada
	for (i=0; i<3; i++){
		(*mundo).mapa.escadas[i].x = (*mundo).mapa.x1 + (*mundo).escadas[i].x_global * (*mundo).mapa.proporcaox;
		(*mundo).mapa.escadas[i].y = (*mundo).mapa.y1 + (*mundo).escadas[i].y * (*mundo).mapa.proporcaoy;
		(*mundo).mapa.escadas[i].largura = (*mundo).escadas[i].largura * (*mundo).mapa.proporcaox;
		(*mundo).mapa.escadas[i].altura = (*mundo).escadas[i].altura * (*mundo).mapa.proporcaoy;
	}
	// -- > mapa --> elevador
	(*mundo).mapa.elevador.x = (*mundo).mapa.x1 + (*mundo).elevador.x_global * (*mundo).mapa.proporcaox;
	(*mundo).mapa.elevador.y = (*mundo).mapa.y1 + (*mundo).elevador.y_porta * (*mundo).mapa.proporcaoy;
	(*mundo).mapa.elevador.largura = (*mundo).elevador.largura * (*mundo).mapa.proporcaox;
	(*mundo).mapa.elevador.altura = (*mundo).elevador.altura * (*mundo).mapa.proporcaoy;
	// -- > mapa --> lamas
	for (i=0; i<=5; i++){
		(*mundo).mapa.lamas[i].x = (*mundo).mapa.x1 + (*mundo).lamas[i].x_global * (*mundo).mapa.proporcaox;
		(*mundo).mapa.lamas[i].y = (*mundo).mapa.y1 + (*mundo).lamas[i].y * (*mundo).mapa.proporcaoy;
		(*mundo).mapa.lamas[i].largura = (*mundo).lamas[i].largura * (*mundo).mapa.proporcaox;
		(*mundo).mapa.lamas[i].altura = (*mundo).lamas[i].altura * (*mundo).mapa.proporcaoy;
	}
	// -- > mapa --> poste dos bombeiros
	(*mundo).mapa.poste.x = (*mundo).mapa.x1 + (*mundo).poste.x_global * (*mundo).mapa.proporcaox;
	(*mundo).mapa.poste.y = (*mundo).mapa.y1 + (*mundo).poste.y * (*mundo).mapa.proporcaoy;
	(*mundo).mapa.poste.largura = (*mundo).poste.largura * (*mundo).mapa.proporcaox;
	(*mundo).mapa.poste.altura = (*mundo).poste.altura * (*mundo).mapa.proporcaoy;
	// ------------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------------
	// IA
	if (ia_jogando == 1){
		for (i=0; i<NUM_POLICIAIS; i++){
			policiais[i].num = i+1;
			policiais[i].geracao = 1;
			calculcar_pesos(&policiais[i]);
			calculcar_pesos_camada_oculta(&policiais[i]);
			calcular_inputs(&policiais[i], *mundo);
			calcular_outputs_camada_oculta(&policiais[i]);
			calcular_outputs(&policiais[i]);
			calcular_x_abs_policial(&policiais[i]);
			policiais[i].x_abs_anterior = policiais[i].x_abs;
			policiais[i].dist_x_inicial = calcular_dist_policia_ladaro_x(policiais[i], *ladrao);
			policiais[i].dist_x_ladrao_anterior = policiais[i].dist_x_inicial;
			policiais[i].dist_x_ladrao = policiais[i].dist_x_ladrao_anterior;
			policiais[i].pontos = 0;
			calcular_pontos(&policiais[i], *ladrao);
			policiais[i].eh_o_melhor = 0;
			policiais[i].rgb[0] = (rand() % 256);
			policiais[i].rgb[1] = (rand() % 256);
			policiais[i].rgb[2] = (rand() % 256);
		}
	}
	// ------------------------------------------------------------------------------------------------	
}

void verificar_interacoes(Personagem *policial, Personagem *ladrao, Tecla teclas, Mundo mundo, int *tempo_captura, int tempo_simulado, int *record, int ia_jogando){
	int i;
	int j;

	// policia_ladrao
	int tPL = 21; // tolerancia para policia ladrao
	if ((*policial).andar == (*ladrao).andar && ((*policial).num_tela == (*ladrao).num_tela)){
		if ( ((*policial).x_global >= (*ladrao).x_global && (*policial).x_global <= (*ladrao).x_global + (*ladrao).largura - tPL) || ((*policial).x_global + (*policial).largura >= (*ladrao).x_global + tPL && (*policial).x_global + (*policial).largura <= (*ladrao).x_global + (*ladrao).largura) ){
			if((*policial).dentro_elevador == 0){
				*tempo_captura = tempo_simulado;
				(*policial).ganhou = 1;
				if (ia_jogando == 1)
					calcular_pontos(policial, *ladrao);
				(*policial).pode_andar = 0;
				(*policial).pode_pular = 0;
				(*ladrao).pode_andar = 0;
				if(*tempo_captura < *record){
					*record = *tempo_captura;
					FILE *arq = fopen("record.txt", "w");
					fprintf(arq, "%d", *record);
					fclose(arq);
				}
			}		
		}
	}
	
	// lama
	int tL = 10; // tolerancia para lama
	for(i=0; i<6; i++){
		if ( (((*policial).x_global > mundo.lamas[i].x_global && (*policial).x_global < mundo.lamas[i].x_global + mundo.lamas[i].largura - tL) || (((*policial).x_global + (*policial).largura > mundo.lamas[i].x_global + tL && (*policial).x_global + (*policial).largura < mundo.lamas[i].x_global + mundo.lamas[i].largura))) && mundo.lamas[i].andar == (*policial).andar && (*policial).pulando == 0){
			(*policial).vx = (*policial).vx_inicial/6;
			(*policial).na_lama = 1;
			break; // ao se achar na lama, esteje na lama até não estar mais
		}
		else {
			(*policial).vx = (*policial).vx_inicial;
			(*policial).na_lama = 0;
		}
	}

	// elevador
	if (teclas.w == 1){
		if ((*policial).pode_andar == 1 && (*policial).dentro_elevador == 0){
			if ((*policial).x_global >= mundo.elevador.x_global && (*policial).x_global + (*policial).largura <= mundo.elevador.x_global + mundo.elevador.largura){
				if ((*policial).andar == mundo.elevador.andar && mundo.elevador.porta_aberta == 1){
					(*policial).pode_pular = 0;
					(*policial).pode_andar = 0;
					(*policial).dentro_elevador = 1;
					(*policial).y -= (*policial).altura/8;
				}
			}
		}
	}

	// poste dos bombeiros
	if ((*policial).andar == mundo.poste.andar && (*policial).num_tela == mundo.poste.num_sala){
		if((*policial).x_global + (*policial).largura >= mundo.poste.x){
			if (teclas.s == 1) {
				(*policial).no_poste = 1;
				(*policial).pode_andar = 0;
				(*policial).pode_pular = 0;
			}
		}
	}

	// escada rolante
	for (j=0; j<=2; j++){
		if((*policial).dentro_escada==1){
			break; // pra não ir pra dentro da outra escada
		}
		if((*policial).andar == mundo.escadas[j].andar && (*policial).num_tela == mundo.escadas[j].num_sala){
			if ((*policial).x_global >= mundo.escadas[j].x_global && (*policial).x_global <= mundo.escadas[j].x_global + mundo.escadas[j].largura){
				// está dentro da regição da escada
				(*policial).dentro_escada = 1;
				(*policial).escada_num = j;

				for (i=0; i<=12; i++){
					if(j == 1){
						if((*policial).x_global >= mundo.escadas[j].degraus[i].x_global){
							if((*policial).y_chao >= mundo.escadas[j].degraus[i].y_chao - mundo.escadas[j].degraus[i].altura && (*policial).degrau_num == -1){
								// está em cima do degrau
								(*policial).degrau_num = i;
								(*policial).pode_andar = 0;
								(*policial).pode_pular = 0;
								(*policial).y_chao = mundo.escadas[j].degraus[i].y_chao - mundo.escadas[j].degraus[i].altura;
								(*policial).y = (*policial).y_chao - (*policial).altura;
								(*policial).x_global = mundo.escadas[j].degraus[i].x_global;
								(*policial).x = (int)(*policial).x_global % SCREEN_W;
								break;
							}
						}
					}
					if(j == 0 || j == 2){
						if((*policial).x_global <= mundo.escadas[j].degraus[i].x_global + mundo.escadas[j].degraus[i].largura){
							if((*policial).y_chao >= mundo.escadas[j].degraus[i].y_chao - mundo.escadas[j].degraus[i].altura && (*policial).degrau_num == -1){
								// está em cima do degrau
								(*policial).degrau_num = i;
								(*policial).pode_andar = 0;
								(*policial).pode_pular = 0;
								(*policial).y_chao = mundo.escadas[j].degraus[i].y_chao - mundo.escadas[j].degraus[i].altura;
								(*policial).y = (*policial).y_chao - (*policial).altura;
								(*policial).x_global = mundo.escadas[j].degraus[i].x_global;
								(*policial).x = (int)(*policial).x_global % SCREEN_W;
								break;
							}
						}
					}
					
				}
			}
		}
	}
	
}

void animar_policial(Personagem *policial, int tempo){
	// animação dele correndo
	if (tempo % ((int)FPS/24) == 0){
		if((*policial).andando == 1 && (*policial).pulando == 0 && (*policial).dentro_escada == 0){
			if ((*policial).imagem_atual == 0)
				(*policial).sentido_anima = 1;
			else if ((*policial).imagem_atual == 3)
				(*policial).sentido_anima = -1;

			(*policial).imagem_atual += (*policial).sentido_anima;
			(*policial).imagem = (*policial).imagens[(*policial).imagem_atual];
		}
		else{
			(*policial).imagem = (*policial).imagens[0];
		}
	}
	// animação dele pulando
	if ((*policial).pulando == 1)
		(*policial).imagem = (*policial).imagens[3];
	else if ((*policial).andando == 0){
		(*policial).imagem = (*policial).imagens[0];
	}
	// animação dele descendo o poste
	if ((*policial).no_poste == 1){
		(*policial).imagem = (*policial).imagens[3];
	}
	// animação dele na escada
	if ((*policial).dentro_escada == 1){
		(*policial).imagem = (*policial).imagens[0];
	}
	// animação dele no elevador
	if ((*policial).dentro_elevador == 1){
		(*policial).imagem = (*policial).imagens[0];
	}
}

void animar_ladrao(Personagem *ladrao, int tempo){
	// animação dele correndo
	if (tempo % ((int)FPS/16) == 0){
		if((*ladrao).andando == 1){
			if ((*ladrao).imagem_atual == 0)
				(*ladrao).sentido_anima = 1;
			else if ((*ladrao).imagem_atual == 3)
				(*ladrao).sentido_anima = -1;

			(*ladrao).imagem_atual += (*ladrao).sentido_anima;
			(*ladrao).imagem = (*ladrao).imagens[(*ladrao).imagem_atual];
		}
	}
}

void atualizar_posicao_policial(Personagem *policial, Personagem *ladrao, Tecla teclas, Mundo mundo, int tempo){
	int i, j;

	// andar para esquerda
	if (teclas.a == 1 && teclas.d == 0 && (*policial).pulando == 0){
		(*policial).orientacao = 1; // orientação da imagem -> pra esquerda
		if ((*policial).pode_andar == 1){

			(*policial).andando = 1;

			if((*policial).x_global > 0) { // não pode passar da parede do final lá da esquerda
				(*policial).x_global -= (*policial).vx;
				(*policial).x = (int)(*policial).x_global % SCREEN_W;
			}
			else {
				// cola na parede
				(*policial).x_global = 0;
				(*policial).x = (int)(*policial).x_global % SCREEN_W;
			}

		}
	}
	// andar para direita
	else if (teclas.a == 0 && teclas.d == 1 && (*policial).pulando == 0){
		(*policial).orientacao = 0; // orientação da imagem -> pra direita
		if ((*policial).pode_andar == 1){

			(*policial).andando = 1;

			if((*policial).x_global + (*policial).largura < 4*SCREEN_W){ // não pode passar da parede do começo da direita
				(*policial).x_global += (*policial).vx;
				(*policial).x = (int)(*policial).x_global % SCREEN_W;
			}
			else {
				// cola na parede
				(*policial).x_global = 4*SCREEN_W - (*policial).largura;
				(*policial).x = (int)(*policial).x_global % SCREEN_W;
			}

		}
	}
	else{
		(*policial).andando = 0;
	}

	// pular
	if (teclas.espaco == 1 && (*policial).pode_pular == 1 && (*policial).na_lama == 0){
		(*policial).pode_andar = 0; // pra não andar pulando
		(*policial).pode_pular = 0; // pra não pular duas ou mais vezes seguidas
		(*policial).pulando = 1; // pra saber quando tá pulando
		(*policial).vy = 50.0; // porque usa a fórmula da física, valor "experimental"
		// decidindo pra que lado pula, ou se pula só pra cima
		if (teclas.a == 1) {
			(*policial).direcao_pulo = -1.0;
		}
		else if (teclas.d == 1) {
			(*policial).direcao_pulo = 1.0;
		}
		else {
			(*policial).direcao_pulo = 0.0;
		}
	}
	if ((*policial).pulando){
		// subir, descer
		(*policial).vy -= mundo.g * mundo.dt; // diminui a velocidade até zerar e "aumentar" depois
		// decrementa ou incrementa a posicao, dependendo do sinal da velocidade
		(*policial).y -= (*policial).vy * mundo.dt; 
		// só pode pular para o lado se não passar das paredes
		if((*policial).x_global > 0 && (*policial).x_global + (*policial).largura < 4*SCREEN_W){
			calcular_x_abs_policial(policial);
			(*policial).x_abs_anterior = (*policial).x_abs;
			(*policial).dist_x_ladrao_anterior = calcular_dist_policia_ladaro_x(*policial, *ladrao);
			(*policial).x_global += (*policial).vx * (*policial).direcao_pulo;
			(*policial).x = (int)(*policial).x_global % SCREEN_W;
			calcular_x_abs_policial(policial);
			(*policial).dist_x_ladrao = calcular_dist_policia_ladaro_x(*policial, *ladrao);
			calcular_pontos(policial, *ladrao);
		}
		// colando no chão
		if ((*policial).y > (*policial).y_chao - (*policial).altura){
			(*policial).y = (*policial).y_chao - (*policial).altura;
			(*policial).vy = 0;
			// resetando parâmetros
			(*policial).pulando = 0;
			(*policial).pode_pular = 1;
			(*policial).pode_andar = 1;
		}
	}

	// mudar de tela
	if ((*policial).x_global >= 3*SCREEN_W && (*policial).x_global <= 4*SCREEN_W){
		(*policial).num_tela = 1;
	}
	else if ((*policial).x_global >= 2*SCREEN_W && (*policial).x_global < 3*SCREEN_W){
		(*policial).num_tela = 2;
	}
	else if ((*policial).x_global >= 1*SCREEN_W && (*policial).x_global < 2*SCREEN_W){
		(*policial).num_tela = 3;
	}
	else if ((*policial).x_global >= 0*SCREEN_W && (*policial).x_global < 1*SCREEN_W){
		(*policial).num_tela = 4;
	}

	// entrar elevador
	if (teclas.s == 1){
		if((*policial).dentro_elevador == 1 && mundo.elevador.porta_aberta == 1){
			(*policial).pode_pular = 1;
			(*policial).pode_andar = 1;
			(*policial).dentro_elevador = 0;
			(*policial).y += (*policial).altura/8;
		}
	}
	// sobe ou desce com o elevador.
	if ((*policial).dentro_elevador == 1){
		calcular_x_abs_policial(policial);
		(*policial).x_abs_anterior = (*policial).x_abs;
		(*policial).dist_x_ladrao_anterior = calcular_dist_policia_ladaro_x(*policial, *ladrao);
		(*policial).y = mundo.elevador.y_porta + (*policial).altura/4;
		(*policial).y_chao = mundo.elevador.y_chao_porta;
		(*policial).andar = mundo.elevador.andar;
		calcular_x_abs_policial(policial);
		(*policial).dist_x_ladrao = calcular_dist_policia_ladaro_x(*policial, *ladrao);
		calcular_pontos(policial, *ladrao);
	}

	// subir na escada
	if((*policial).dentro_escada == 1){
		(*policial).y_chao = mundo.escadas[(*policial).escada_num].degraus[(*policial).degrau_num].y_chao - mundo.escadas[(*policial).escada_num].degraus[0].altura;
		(*policial).y = (*policial).y_chao - (*policial).altura;
		calcular_x_abs_policial(policial);
		(*policial).x_abs_anterior = (*policial).x_abs;
		(*policial).dist_x_ladrao_anterior = calcular_dist_policia_ladaro_x(*policial, *ladrao);
		(*policial).x_global = mundo.escadas[(*policial).escada_num].degraus[(*policial).degrau_num].x_global;
		(*policial).x = (int)(*policial).x_global % SCREEN_W;
		calcular_x_abs_policial(policial);
		(*policial).dist_x_ladrao = calcular_dist_policia_ladaro_x(*policial, *ladrao);
		calcular_pontos(policial, *ladrao);
		if((*policial).y_chao <= mundo.escadas[(*policial).escada_num].teto.y_chao - mundo.escadas[(*policial).escada_num].degraus[0].altura){
			(*policial).dentro_escada = 0;
			(*policial).escada_num = -1;
			(*policial).degrau_num = -1;
			(*policial).pode_andar = 1;
			(*policial).pode_pular = 1;
			(*policial).andar += 1;
		}
	}

	// descer poste dos bombeiros
	if ((*policial).no_poste == 1){
		if (tempo % ((int)FPS/48) == 0){
			(*policial).y += mundo.g;
		}
		if ((*policial).y >= 5*SCREEN_H/6.0 - (*policial).altura){
			(*policial).y = 5*SCREEN_H/6.0 - (*policial).altura;
			(*policial).y_chao = (*policial).y + (*policial).altura;
			(*policial).andar = 1;
			(*policial).no_poste = 0;
			(*policial).pode_andar = 1;
			(*policial).pode_pular = 1;
		}
	}
}

void atualizar_posicao_ladrao(Personagem *ladrao, Personagem policial) {
	// foge ladrão
	// se o policial tiver na mesma tela e andar do ladrão, ele foge pro outro lado com 25% a mais de velocidade
	/*if (policial.x_global < (*ladrao).x_global && policial.andar == (*ladrao).andar && policial.dentro_elevador == 0 && policial.num_tela == (*ladrao).num_tela){
		(*ladrao).orientacao = 0;
		(*ladrao).vx = 1.25*(*ladrao).vx_inicial;
	}
	else if (policial.x_global > (*ladrao).x_global && policial.andar == (*ladrao).andar && policial.dentro_elevador == 0 && policial.num_tela == (*ladrao).num_tela){
		(*ladrao).orientacao = 1;
		(*ladrao).vx = 1.25*(*ladrao).vx_inicial;
	}
	else {
		(*ladrao).vx = (*ladrao).vx_inicial;
	}*/

	// andando
	int direcao_andar;
	if ((*ladrao).orientacao == 0){
		direcao_andar = 1;
	}
	else if ((*ladrao).orientacao == 1){
		direcao_andar = -1;
	}
	if((*ladrao).pode_andar == 1){
		(*ladrao).x_global += (*ladrao).vx * direcao_andar;
		(*ladrao).x = (int)(*ladrao).x_global % SCREEN_W;
	}

	// mudando de andar ao se chegar na parede esquerda
	if ((*ladrao).x_global + (*ladrao).largura <= 0){
		if ((*ladrao).andar == 1 || (*ladrao).andar == 3){
			(*ladrao).y -=  SCREEN_H/6.0; // sobe
			(*ladrao).andar += 1;
			(*ladrao).x_global = 0;
			(*ladrao).x = (int)(*ladrao).x_global % SCREEN_W;
			(*ladrao).orientacao = 0; // vira pra direita
		}
		else if ((*ladrao).andar == 2 || (*ladrao).andar == 4){
			(*ladrao).y +=  SCREEN_H/6.0; // desce
			(*ladrao).andar -= 1;
			(*ladrao).x_global = 0;
			(*ladrao).x = (int)(*ladrao).x_global % SCREEN_W;
			(*ladrao).orientacao = 0; // vira pra direita
		}
		(*ladrao).y_chao = (*ladrao).y + (*ladrao).altura;
	}

	// mudando de andar ao se chegar no fim da tela direita
	else if ((*ladrao).x_global >= 4*SCREEN_W) {
		if ((*ladrao).andar == 2){
			(*ladrao).y -=  SCREEN_H/6.0; // sobe
			(*ladrao).andar += 1;
			(*ladrao).x_global = 4*SCREEN_W - (*ladrao).largura;;
			(*ladrao).x = (int)(*ladrao).x_global % SCREEN_W;
			(*ladrao).orientacao = 1; // vira pra esquerda		
		}
		else if ((*ladrao).andar == 3){
			(*ladrao).y +=  SCREEN_H/6.0; // desce
			(*ladrao).andar -= 1;
			(*ladrao).x_global = 4*SCREEN_W - (*ladrao).largura;;
			(*ladrao).x = (int)(*ladrao).x_global % SCREEN_W;
			(*ladrao).orientacao = 1; // vira pra esquerda	
		}
		else if ((*ladrao).andar == 4 || (*ladrao).andar == 1){
			(*ladrao).pode_andar = 0;
			(*ladrao).andar = -1; // pra identificar se ganhou
			(*ladrao).num_tela = -1; // pra não desenhar
		}
		(*ladrao).y_chao = (*ladrao).y + (*ladrao).altura;
	}

	// mudar de tela
	if ((*ladrao).x_global >= 3*SCREEN_W && (*ladrao).x_global < 4*SCREEN_W){
		(*ladrao).num_tela = 1;
	}
	else if ((*ladrao).x_global >= 2*SCREEN_W && (*ladrao).x_global < 3*SCREEN_W){
		(*ladrao).num_tela = 2;
	}
	else if ((*ladrao).x_global >= 1*SCREEN_W && (*ladrao).x_global < 2*SCREEN_W){
		(*ladrao).num_tela = 3;
	}
	else if ((*ladrao).x_global >= 0*SCREEN_W && (*ladrao).x_global < 1*SCREEN_W){
		(*ladrao).num_tela = 4;
	}

	// verifica se ladrão ganhou
	if ((*ladrao).andar == -1){
		(*ladrao).ganhou = 1;
		(*ladrao).pode_andar = 0;
	}
}

void atualiza_posicao_elevador(Mundo *mundo, int tempo){
	// a cada 2 segundos atualiza
	if (tempo % (2*(int)FPS) == 0){
		if ((*mundo).elevador.porta_aberta == 1){
			(*mundo).elevador.porta_aberta = 0;
			if((*mundo).elevador.andar < 3 && (*mundo).elevador.subir_descer == -1){
				(*mundo).elevador.y_porta -= SCREEN_H/6;
				(*mundo).elevador.y_chao_porta -= SCREEN_H/6;
				(*mundo).elevador.andar += 1;
			}
			if((*mundo).elevador.andar > 1 && (*mundo).elevador.subir_descer == +1){
				(*mundo).elevador.y_porta += SCREEN_H/6;
				(*mundo).elevador.y_chao_porta += SCREEN_H/6;
				(*mundo).elevador.andar -= 1;
			}
			// inverte o sentido ao chegar nos limites
			if((*mundo).elevador.andar == 3 || (*mundo).elevador.andar == 1){
				(*mundo).elevador.subir_descer *= -1;
			}
		}
		else{
			(*mundo).elevador.porta_aberta = 1;
		}
	}
}

void atualiza_posicao_escada(Mundo *mundo, int tempo, Personagem policial){
	if (tempo % ((int)FPS/24) == 0){
		int i;
		int j;
		int num_teto;
		int num_pe;
		if (/*policial.andar == (*mundo).escadas[1].andar && policial.num_tela == (*mundo).escadas[1].num_sala && policial.dentro_elevador == 0*/1){

			// escada 1 -> 1ª tela, 2º andar
			// --------------------------------------------------------------------------------------------

			num_teto = (*mundo).escadas[1].teto.num;
			num_pe = (*mundo).escadas[1].pe.num;

			for (i=0; i<13; i++){

				if ((*mundo).escadas[1].degraus[i].x >= (*mundo).escadas[1].teto.x - (*mundo).escadas[1].degraus[i].largura  || (*mundo).escadas[1].degraus[i].x <= (*mundo).escadas[1].pe.x + (*mundo).escadas[1].degraus[i].largura){
					(*mundo).escadas[1].degraus[i].x_global += (*mundo).escadas[1].vx;
					(*mundo).escadas[1].degraus[i].x = (int)(*mundo).escadas[1].degraus[i].x_global % SCREEN_W;
				}

				else{
					(*mundo).escadas[1].degraus[i].x_global += (*mundo).escadas[1].vx;
					(*mundo).escadas[1].degraus[i].x = (int)(*mundo).escadas[1].degraus[i].x_global % SCREEN_W;
					(*mundo).escadas[1].degraus[i].y -= (*mundo).escadas[1].vy;
					(*mundo).escadas[1].degraus[i].y_chao -= (*mundo).escadas[1].vy;
				}

			}	

			if ((*mundo).escadas[1].degraus[num_teto].x >= (*mundo).escadas[1].teto.x + (*mundo).escadas[1].degraus[num_teto].largura){
				(*mundo).escadas[1].degraus[num_teto].x_global = (*mundo).escadas[1].degraus[num_pe].x_global - (*mundo).escadas[1].degraus[num_teto].largura;
				(*mundo).escadas[1].degraus[num_teto].x = (int)(*mundo).escadas[1].degraus[num_teto].x_global % SCREEN_W;
				(*mundo).escadas[1].degraus[num_teto].y = (*mundo).escadas[1].pe.y;
				(*mundo).escadas[1].degraus[num_teto].y_chao = (*mundo).escadas[1].pe.y_chao;
				if ((*mundo).escadas[1].teto.num >= 1){
					(*mundo).escadas[1].teto.num -= 1;
				}
				else {
					(*mundo).escadas[1].teto.num = 12;
				}
				if ((*mundo).escadas[1].pe.num >= 1){
					(*mundo).escadas[1].pe.num -= 1;
				}
				else {
					(*mundo).escadas[1].pe.num = 12;
				}
			}
		}
		// --------------------------------------------------------------------------------------------

		// escada 0 -> 4ª tela, 1º andar
		// escada 2 -> 4ª tela, 3º andar
		// --------------------------------------------------------------------------------------------
		for (j=0; j<=2; j+=2){
			if (/*policial.andar == (*mundo).escadas[j].andar && policial.num_tela == (*mundo).escadas[j].num_sala && policial.dentro_elevador == 0*/1){
				num_teto = (*mundo).escadas[j].teto.num;
				num_pe = (*mundo).escadas[j].pe.num;

				for (i=0; i<13; i++){

					if ((*mundo).escadas[j].degraus[i].x <= (*mundo).escadas[j].teto.x + (*mundo).escadas[j].degraus[i].largura  || (*mundo).escadas[j].degraus[i].x + (*mundo).escadas[j].degraus[i].largura >= (*mundo).escadas[j].pe.x){
						(*mundo).escadas[j].degraus[i].x_global -= (*mundo).escadas[j].vx;
						(*mundo).escadas[j].degraus[i].x = (int)(*mundo).escadas[j].degraus[i].x_global % SCREEN_W;
					}

					else{
						(*mundo).escadas[j].degraus[i].x_global -= (*mundo).escadas[j].vx;
						(*mundo).escadas[j].degraus[i].x = (int)(*mundo).escadas[j].degraus[i].x_global % SCREEN_W;
						(*mundo).escadas[j].degraus[i].y -= (*mundo).escadas[1].vy;
						(*mundo).escadas[j].degraus[i].y_chao -= (*mundo).escadas[1].vy;
					}

				}	

				if ((*mundo).escadas[j].degraus[num_teto].x <= 0){
					(*mundo).escadas[j].degraus[num_teto].x_global = (*mundo).escadas[j].degraus[num_pe].x_global + (*mundo).escadas[j].degraus[num_teto].largura;
					(*mundo).escadas[j].degraus[num_teto].x = (int)(*mundo).escadas[j].degraus[num_teto].x_global % SCREEN_W;
					(*mundo).escadas[j].degraus[num_teto].y = (*mundo).escadas[j].pe.y;
					(*mundo).escadas[j].degraus[num_teto].y_chao = (*mundo).escadas[j].pe.y_chao;
					if ((*mundo).escadas[j].teto.num >= 1){
						(*mundo).escadas[j].teto.num -= 1;
					}
					else {
						(*mundo).escadas[j].teto.num = 12;
					}
					if ((*mundo).escadas[j].pe.num >= 1){
						(*mundo).escadas[j].pe.num -= 1;
					}
					else {
						(*mundo).escadas[j].pe.num = 12;
					}
				}
			}
		}
		// --------------------------------------------------------------------------------------------

	}
}

void atualizar_mapa(Mundo *mundo, Personagem policiais[], Personagem ladrao){
	// -- > mapa --> policial
	int po;
	for (po=0;po<NUM_POLICIAIS;po++){
		(*mundo).mapa.policiais[po].x = (*mundo).mapa.x1 + (policiais[po]).x_global * (*mundo).mapa.proporcaox;
		(*mundo).mapa.policiais[po].y = (*mundo).mapa.y1 -4 + (policiais[po]).y * (*mundo).mapa.proporcaoy;
	}
	// -- > mapa --> ladrao
	(*mundo).mapa.ladrao.x = (*mundo).mapa.x1 + (ladrao).x_global * (*mundo).mapa.proporcaox;
	(*mundo).mapa.ladrao.y = (*mundo).mapa.y1 -4 + (ladrao).y * (*mundo).mapa.proporcaoy;
	// -- > mapa --> elevador
	(*mundo).mapa.elevador.x = (*mundo).mapa.x1 + (*mundo).elevador.x_global * (*mundo).mapa.proporcaox;
	(*mundo).mapa.elevador.y = (*mundo).mapa.y1 -4 + (*mundo).elevador.y_porta * (*mundo).mapa.proporcaoy;
}

void desenhar_cenario(Mundo mundo, Personagem policial) {
	int i;
	int j;
	int po;
	// Desenha um retângulo preenchido (x1, y1, x2, y2, cor)
    // x1, y1 -> canto superior esquerdo
    // x2, y2 -> canto inferior direito

	// retângulos de fundo de todas as telas
    al_draw_filled_rectangle(0, 0*SCREEN_H/6.0, SCREEN_W, 1*SCREEN_H/6.0, al_map_rgb(104,116,208));
	al_draw_filled_rectangle(0, 1*SCREEN_H/6.0, SCREEN_W, 2*SCREEN_H/6.0, al_map_rgb(108,108,108));
	al_draw_filled_rectangle(0, 2*SCREEN_H/6.0, SCREEN_W, 3*SCREEN_H/6.0, al_map_rgb(64,124,64));
	al_draw_filled_rectangle(0, 3*SCREEN_H/6.0, SCREEN_W, 4*SCREEN_H/6.0, al_map_rgb(64,124,64));
	al_draw_filled_rectangle(0, 4*SCREEN_H/6.0, SCREEN_W, 5*SCREEN_H/6.0, al_map_rgb(64,124,64));
	al_draw_filled_rectangle(0, 5*SCREEN_H/6.0, SCREEN_W, 6*SCREEN_H/6.0, al_map_rgb(176,176,176));

	// imagem da cidade ao fundo
	al_draw_bitmap(mundo.imagem_cidade, 0, 1*SCREEN_H/6.0, 0);


	// escada
	// escada 1 -> 1ª tela, 2º andar
	// --------------------------------------------------------------------------------------------
	if (policial.num == NUM_POLICIAIS){
		if(mundo.escadas[1].num_sala == policial.num_tela){
			for (i=0; i<13; i++){
				al_draw_filled_rectangle(mundo.escadas[1].degraus[i].x, mundo.escadas[1].degraus[i].y, mundo.escadas[1].degraus[i].x + mundo.escadas[1].degraus[i].largura, mundo.escadas[1].degraus[i].y_chao, al_map_rgb(255,255,255));		
			}
			// corrimao da escada 1
			float x1 = mundo.escadas[1].x + mundo.escadas[1].largura + mundo.escadas[1].degraus[12].largura, y2 = mundo.escadas[1].pe.y_chao; // vértice no ângulo reto
			float x2 =  mundo.escadas[1].x+5, y1 = mundo.escadas[1].pe.y_chao; // vértice no final da base
			float x3 = mundo.escadas[1].x + mundo.escadas[1].largura + mundo.escadas[1].degraus[12].largura, y3 = mundo.escadas[1].teto.y; // vértice da altura
			al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, al_map_rgb(0, 60, 0));
		}
	}
	// --------------------------------------------------------------------------------------------
	// escada 0 -> 4ª tela, 1º andar
	// escada 2 -> 4ª tela, 3º andar
	// --------------------------------------------------------------------------------------------
	if (policial.num == NUM_POLICIAIS){
		for (j=0; j<=2; j+=2){
			if(mundo.escadas[j].num_sala == policial.num_tela){
				for (i=0; i<13; i++){
					al_draw_filled_rectangle(mundo.escadas[j].degraus[i].x, mundo.escadas[j].degraus[i].y, mundo.escadas[j].degraus[i].x + mundo.escadas[j].degraus[i].largura, mundo.escadas[j].degraus[i].y_chao, al_map_rgb(255,255,255));
				}
			}
		}
		if(mundo.escadas[0].num_sala == policial.num_tela){
			// corrimao da escada 0
			float x1 = 1.1*mundo.escadas[0].degraus[0].largura, y1 = mundo.escadas[0].pe.y_chao; // vértice no ângulo reto
			float x2 =  mundo.escadas[0].largura + 2.5*mundo.escadas[0].degraus[0].largura, y2 = mundo.escadas[0].pe.y_chao; // vértice no final da base
			float x3 = 1.1*mundo.escadas[0].degraus[0].largura, y3 = mundo.escadas[0].teto.y; // vértice da altura
			al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, al_map_rgb(0, 60, 0));
		}
		if(mundo.escadas[2].num_sala == policial.num_tela){
			// corrimao da escada 2
			float x1 = 1.1*mundo.escadas[2].degraus[0].largura, y1 = mundo.escadas[2].pe.y_chao;
			float x2 =  mundo.escadas[2].largura + 2.5*mundo.escadas[2].degraus[0].largura, y2 = mundo.escadas[2].pe.y_chao;
			float x3 = 1.1*mundo.escadas[2].degraus[0].largura, y3 = mundo.escadas[2].teto.y;
			al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, al_map_rgb(0, 60, 0));
		}
	}
	// --------------------------------------------------------------------------------------------


	// retângulos separadores dos retângulos de fundo
	// são dois para cada: um mais claro e outro mais escuro
	int largura = SCREEN_H/60.0;
	al_draw_filled_rectangle(0, 2*SCREEN_H/6.0, SCREEN_W, 2*SCREEN_H/6.0 + largura, al_map_rgb(184,184,64));
	al_draw_filled_rectangle(0, 2*SCREEN_H/6.0 + largura, SCREEN_W, 2*SCREEN_H/6.0 + 2*largura, al_map_rgb(160,160,52));
	al_draw_filled_rectangle(0, 3*SCREEN_H/6.0, SCREEN_W, 3*SCREEN_H/6.0 + largura, al_map_rgb(184,184,64));
	al_draw_filled_rectangle(0, 3*SCREEN_H/6.0 + largura, SCREEN_W, 3*SCREEN_H/6.0 + 2*largura, al_map_rgb(160,160,52));
	al_draw_filled_rectangle(0, 4*SCREEN_H/6.0, SCREEN_W, 4*SCREEN_H/6.0 + largura, al_map_rgb(184,184,64));
	al_draw_filled_rectangle(0, 4*SCREEN_H/6.0 + largura, SCREEN_W, 4*SCREEN_H/6.0 + 2*largura, al_map_rgb(160,160,52));
	al_draw_filled_rectangle(0, 5*SCREEN_H/6.0, SCREEN_W, 5*SCREEN_H/6.0 + largura, al_map_rgb(184,184,64));
	al_draw_filled_rectangle(0, 5*SCREEN_H/6.0 + largura, SCREEN_W, 5*SCREEN_H/6.0 + 2*largura, al_map_rgb(160,160,52));

	// retangulo azul
	if (policial.num == NUM_POLICIAIS){
		// tela 1
		if (policial.num_tela == 1) {
			float dist_parede = SCREEN_W/4.0;
			float largura = SCREEN_W/6.0;
			float dist_teto = (5*SCREEN_H/6.0 - 4*SCREEN_H/6.0)/1.5; 
			float altura = 5*SCREEN_H/6.0;
			al_draw_filled_rectangle(0 + dist_parede, 4*SCREEN_H/6.0 + dist_teto, dist_parede + largura, altura, al_map_rgb(80,112,188));
			al_draw_filled_rectangle(SCREEN_W - dist_parede, 4*SCREEN_H/6.0 + dist_teto, SCREEN_W - dist_parede - largura, altura, al_map_rgb(80,112,188));
		}


		// elevador
		// tela 4
		if (policial.num_tela == 4){
			// porta fechada
			// 3º andar
			al_draw_filled_rectangle(mundo.elevador.x, mundo.elevador.y - 2*SCREEN_H/6.0, mundo.elevador.x + mundo.elevador.largura, mundo.elevador.y_chao - 2*SCREEN_H/6.0, al_map_rgb(0,60,0));
			// 2º andar
			al_draw_filled_rectangle(mundo.elevador.x, mundo.elevador.y - 1*SCREEN_H/6.0, mundo.elevador.x + mundo.elevador.largura, mundo.elevador.y_chao - 1*SCREEN_H/6.0, al_map_rgb(0,60,0));
			// 1º andar
			al_draw_filled_rectangle(mundo.elevador.x, mundo.elevador.y, mundo.elevador.x + mundo.elevador.largura, mundo.elevador.y_chao, al_map_rgb(0,60,0));
			// porta aberta
			if(mundo.elevador.porta_aberta == 1){
				al_draw_filled_rectangle(mundo.elevador.x, mundo.elevador.y_porta, mundo.elevador.x + mundo.elevador.largura, mundo.elevador.y_chao_porta, al_map_rgb(80,156,128));
			}		
			// chão do elevador
			// 3º andar
			al_draw_filled_rectangle(mundo.elevador.x, mundo.elevador.y_chao - policial.altura/8 - 2*SCREEN_H/6.0, mundo.elevador.x + mundo.elevador.largura, mundo.elevador.y_chao - 2*SCREEN_H/6.0, al_map_rgb(184,184,64));
			// 2º andar
			al_draw_filled_rectangle(mundo.elevador.x, mundo.elevador.y_chao - policial.altura/8 - 1*SCREEN_H/6.0, mundo.elevador.x + mundo.elevador.largura, mundo.elevador.y_chao - 1*SCREEN_H/6.0, al_map_rgb(184,184,64));
			// 1º andar
			al_draw_filled_rectangle(mundo.elevador.x, mundo.elevador.y_chao - policial.altura/8, mundo.elevador.x + mundo.elevador.largura, mundo.elevador.y_chao, al_map_rgb(184,184,64));
		}

		// lama
		for (i=0; i<6; i++){
			if(mundo.lamas[i].num_sala == policial.num_tela){
				al_draw_filled_rectangle(mundo.lamas[i].x, mundo.lamas[i].y, mundo.lamas[i].x + mundo.lamas[i].largura, mundo.lamas[i].y_chao, al_map_rgb(150,75,0));
			}
		}

		// poste dos bombeiros
		if(mundo.poste.num_sala == policial.num_tela)
			al_draw_filled_rectangle(mundo.poste.x, mundo.poste.y, mundo.poste.x + mundo.poste.largura, mundo.poste.y + mundo.poste.altura, al_map_rgb(0, 60, 0));
	}
	// ---------------------------------------------------------------------------------------------------
	// mapa em baixo da tela

	float map_y0 = 5*SCREEN_H/6.0 + 2*largura;
	float map_sc_h = SCREEN_H - map_y0;

	// -- > mapa --> retângulos de fundo de todas as telas
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 1*map_sc_h/6.0, 3*SCREEN_W/4.0, map_y0 + 2*map_sc_h/6.0, al_map_rgb(108,108,108));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 2*map_sc_h/6.0, 3*SCREEN_W/4.0, map_y0 + 3*map_sc_h/6.0, al_map_rgb(64,124,64));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 3*map_sc_h/6.0, 3*SCREEN_W/4.0, map_y0 + 4*map_sc_h/6.0, al_map_rgb(64,124,64));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 4*map_sc_h/6.0, 3*SCREEN_W/4.0, map_y0 + 5*map_sc_h/6.0, al_map_rgb(64,124,64));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 5*map_sc_h/6.0, 3*SCREEN_W/4.0, map_y0 + 6*map_sc_h/6.0, al_map_rgb(176,176,176));

	// -- > mapa --> retângulos separadores dos retângulos de fundo
	// são dois para cada: um mais claro e outro mais escuro
	int largura2 = SCREEN_H/60.0 * mundo.mapa.proporcaoy;
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 2*map_sc_h/6.0, 3*SCREEN_W/4.0, map_y0 + 2*map_sc_h/6.0 + largura2, al_map_rgb(184,184,64));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 2*map_sc_h/6.0 + largura2, 3*SCREEN_W/4.0, map_y0 + 2*map_sc_h/6.0 + 2*largura2, al_map_rgb(160,160,52));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 3*map_sc_h/6.0, 3*SCREEN_W/4.0, map_y0 + 3*map_sc_h/6.0 + largura2, al_map_rgb(184,184,64));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 3*map_sc_h/6.0 + largura2, 3*SCREEN_W/4.0, map_y0 + 3*map_sc_h/6.0 + 2*largura2, al_map_rgb(160,160,52));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 4*map_sc_h/6.0, 3*SCREEN_W/4.0, map_y0 + 4*map_sc_h/6.0 + largura2, al_map_rgb(184,184,64));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 4*map_sc_h/6.0 + largura2, 3*SCREEN_W/4.0, map_y0 + 4*map_sc_h/6.0 + 2*largura2, al_map_rgb(160,160,52));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 5*map_sc_h/6.0, 3*SCREEN_W/4.0, map_y0 + 5*map_sc_h/6.0 + largura2, al_map_rgb(184,184,64));
	al_draw_filled_rectangle(SCREEN_W/4.0, map_y0 + 5*map_sc_h/6.0 + largura2, 3*SCREEN_W/4.0, map_y0 + 5*map_sc_h/6.0 + 2*largura2, al_map_rgb(160,160,52));

	// -- > mapa --> policial
	for (po=0; po<NUM_POLICIAIS; po++){
		al_draw_filled_rectangle(mundo.mapa.policiais[po].x,mundo.mapa.policiais[po].y,mundo.mapa.policiais[po].x+mundo.mapa.policiais[po].largura,mundo.mapa.policiais[po].y+mundo.mapa.policiais[po].altura,al_map_rgb(0,0,255));
	}

	// -- > mapa --> ladrao
	al_draw_filled_rectangle(mundo.mapa.ladrao.x,mundo.mapa.ladrao.y,mundo.mapa.ladrao.x+mundo.mapa.ladrao.largura,mundo.mapa.ladrao.y+mundo.mapa.ladrao.altura,al_map_rgb(255,0,0));

	// -- > mapa --> elevador
	al_draw_filled_rectangle(mundo.mapa.elevador.x,mundo.mapa.elevador.y,mundo.mapa.elevador.x+mundo.mapa.elevador.largura,mundo.mapa.elevador.y+mundo.mapa.elevador.altura,al_map_rgb(80,156,128));

	// -- > mapa --> escadas
	float x1 = mundo.mapa.escadas[1].x + mundo.mapa.escadas[1].largura, y1 = mundo.mapa.escadas[1].y - mundo.mapa.escadas[1].altura; // vértice no ângulo reto
	float x2 =  mundo.mapa.escadas[1].x, y2 = mundo.mapa.escadas[1].y - mundo.mapa.escadas[1].altura; // vértice no final da base
	float x3 = mundo.mapa.escadas[1].x + mundo.mapa.escadas[1].largura, y3 = mundo.mapa.escadas[1].y; // vértice da altura
	al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, al_map_rgb(0, 60, 0));
	for (i=0; i<=2; i+=2){
		float x1 = mundo.mapa.escadas[i].x, y1 = mundo.mapa.escadas[i].y + mundo.mapa.escadas[i].altura; // vértice no ângulo reto
		float x2 =  mundo.mapa.escadas[i].x + mundo.mapa.escadas[i].largura, y2 = mundo.mapa.escadas[i].y + mundo.mapa.escadas[i].altura; // vértice no final da base
		float x3 =  mundo.mapa.escadas[i].x, y3 = mundo.mapa.escadas[i].y; // vértice da altura
	al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, al_map_rgb(0, 60, 0));
	}

	// -- > mapa --> lamas
	for (i=0; i<=5; i++){
		al_draw_filled_rectangle(mundo.mapa.lamas[i].x, mundo.mapa.lamas[i].y - 4, mundo.mapa.lamas[i].x + mundo.mapa.lamas[i].largura, mundo.mapa.lamas[i].y + + mundo.mapa.lamas[i].altura, al_map_rgb(150,75,0));
	}

	// -- > mapa --> poste dos bombeiros
	al_draw_filled_rectangle(mundo.mapa.poste.x, mundo.mapa.poste.y, mundo.mapa.poste.x + mundo.mapa.poste.largura, mundo.mapa.poste.y + mundo.mapa.poste.altura, al_map_rgb(0, 60, 0));
	// ---------------------------------------------------------------------------------------------------
}

void desenhar_policial(Personagem policial, Mundo mundo, Personagem policial_referencia) {
	if (policial.dentro_elevador == 0 || (policial.dentro_elevador == 1 && mundo.elevador.porta_aberta == 1)){
		if (policial.num != NUM_POLICIAIS){
			if (policial.num_tela == policial_referencia.num_tela){
				al_draw_filled_rectangle(policial.x + policial.largura/8, policial.y + + policial.altura/8, policial.x + 7*policial.largura/8, policial.y + 7*policial.altura/8, al_map_rgb(policial.rgb[0],policial.rgb[1],policial.rgb[2]));
			}
		}
		else
			al_draw_bitmap(policial.imagem, policial.x , policial.y, policial.orientacao);
	}
}

void desenhar_ladrao(Personagem ladrao, Personagem policial) {
	if (policial.num == NUM_POLICIAIS){
		if (ladrao.num_tela == policial.num_tela){
			al_draw_bitmap(ladrao.imagem, ladrao.x , ladrao.y, ladrao.orientacao);
		}
	}
}

void desenhar_tela_final(Personagem policial, Personagem ladrao, int tempo_captura, ALLEGRO_FONT *font, int ia_jogando){
	if (policial.ganhou == 1){		
		al_draw_bitmap(policial.imagem_vitoria, 0, 0, 0);
		al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W -10, 10, ALLEGRO_ALIGN_RIGHT, "Parabéns, policial!");
		al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W -10, 45, ALLEGRO_ALIGN_RIGHT, "Tempo de captura: %ds", tempo_captura);
		int record;
		FILE *arq = fopen("record.txt","r");
		fscanf(arq,"%d",&record);
		fclose(arq);
		if (record != TEMPO_LIMITE){
			al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W -10, 77, ALLEGRO_ALIGN_RIGHT, "Recorde: %ds", record);
		}

		if (ia_jogando == 1){
			al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W -10, 135, ALLEGRO_ALIGN_RIGHT, " Pesos:");
			int k;
			int l = 0;
			for (k=0; k<26; k++){
				if (k%5==0)
					l += 1;
				al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W -10 - k*5, 1170+l*35, ALLEGRO_ALIGN_RIGHT, " [%f] ", policial.pesos[k]);
			}
		}
	}
	else if (ladrao.ganhou == 1){
		al_draw_bitmap(ladrao.imagem_vitoria, 0, 0, 0);
		al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "Quem sabe na próxima?");
	}
}

void escolher_acao(Tecla *teclas, Personagem *policial) {
	(*teclas).w = (*policial).outputs[0];	
	(*teclas).s = (*policial).outputs[1];
	(*teclas).a = (*policial).outputs[2];
	(*teclas).d = (*policial).outputs[3];
	(*teclas).espaco = (*policial).outputs[4];	
}

void verificar_teclas(ALLEGRO_EVENT ev, Tecla *teclas, int pressionado, int ia_jogando) {
	// pressioando = 0 -> não pressionado
	// pressioando = 1 -> pressionado

	if(ia_jogando == 0){
		if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) {
			(*teclas).espaco = pressionado;
		}
		if (ev.keyboard.keycode == ALLEGRO_KEY_D) {
			(*teclas).d = pressionado;
		}
		if (ev.keyboard.keycode == ALLEGRO_KEY_A) {
			(*teclas).a = pressionado;
		}
		if (ev.keyboard.keycode == ALLEGRO_KEY_W) {
			(*teclas).w = pressionado;
		}
		if (ev.keyboard.keycode == ALLEGRO_KEY_S) {
			(*teclas).s = pressionado;
		}
	}
	
	if (ev.keyboard.keycode == ALLEGRO_KEY_P) {
		(*teclas).p = pressionado;
    }
	if (ev.keyboard.keycode == ALLEGRO_KEY_O) {
		(*teclas).o = pressionado;
    }

	if (ev.keyboard.keycode == ALLEGRO_KEY_I) {
		(*teclas).i = pressionado;
    }
}
// ---------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// Função principal

int main(int argc, char **argv){

	// Inicializa o gerador de números aleatórios
    srand(time(NULL));

	
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_FONT *font = NULL;
    
	//----------------------- rotinas de inicializacao ---------------------------------------

	//inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	
    //inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	
	
	//inicializa o modulo que permite carregar imagens no jogo
	if(!al_init_image_addon()){
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}

	// inicializar imagens usadas no game:

	// imagem do  cidade ao fundo
	ALLEGRO_BITMAP *imagem_cidade = al_load_bitmap("../../imagens_cenario/cidade.png");
	if (!imagem_cidade) {
        fprintf(stderr, "Falha ao carregar a imagem da cidade!\n");
       return -1;
    }

	// imagem do Policial Kelly Keystone
	ALLEGRO_BITMAP *imagem_policial = al_load_bitmap("../../imagens_personagens/kelly_keystone_pos_inicial.png");
	if (!imagem_policial) {
        fprintf(stderr, "Falha ao carregar a imagem do kelly keystone!\n");
        return -1;
    }
	ALLEGRO_BITMAP *imagem_policial2 = al_load_bitmap("../../imagens_personagens/kelly_keystone_2.png");
	if (!imagem_policial2) {
        fprintf(stderr, "Falha ao carregar a imagem 2 do kelly keystone!\n");
        return -1;
    }
	ALLEGRO_BITMAP *imagem_policial3 = al_load_bitmap("../../imagens_personagens/kelly_keystone_3.png");
	if (!imagem_policial3) {
        fprintf(stderr, "Falha ao carregar a imagem 3 do kelly keystone!\n");
        return -1;
    }
	ALLEGRO_BITMAP *imagem_policial4 = al_load_bitmap("../../imagens_personagens/kelly_keystone_4.png");
	if (!imagem_policial4) {
        fprintf(stderr, "Falha ao carregar a imagem 4 do kelly keystone!\n");
        return -1;
    }

	// imagem do Ladrão harry hooligan
	ALLEGRO_BITMAP *imagem_ladrao = al_load_bitmap("../../imagens_personagens/harry_hooligan_pos_inicial.png");
	if (!imagem_ladrao) {
        fprintf(stderr, "Falha ao carregar a imagem do harry hooligan!\n");
        return -1;
    }
	ALLEGRO_BITMAP *imagem_ladrao2 = al_load_bitmap("../../imagens_personagens/harry_hooligan_2.png");
	if (!imagem_ladrao2) {
        fprintf(stderr, "Falha ao carregar a imagem 2 do harry hooligan!\n");
        return -1;
    }
	ALLEGRO_BITMAP *imagem_ladrao3 = al_load_bitmap("../../imagens_personagens/harry_hooligan_3.png");
	if (!imagem_ladrao3) {
        fprintf(stderr, "Falha ao carregar a imagem 3 do harry hooligan!\n");
        return -1;
    }
	ALLEGRO_BITMAP *imagem_ladrao4 = al_load_bitmap("../../imagens_personagens/harry_hooligan_4.png");
	if (!imagem_ladrao4) {
        fprintf(stderr, "Falha ao carregar a imagem 4 do harry hooligan!\n");
        return -1;
    }

	// imagem da tela final do policial vitorioso
	ALLEGRO_BITMAP *imagem_policial_vitorioso = al_load_bitmap("../../imagens_tela_final/tela_final_policial.png");
	if (!imagem_policial_vitorioso) {
        fprintf(stderr, "Falha ao carregar a imagem do policial vitorioso!\n");
        return -1;
    }

	// imagem da tela final do ladrao vitorioso
	ALLEGRO_BITMAP *imagem_ladrao_vitorioso = al_load_bitmap("../../imagens_tela_final/tela_final_ladrao.png");
	if (!imagem_ladrao_vitorioso) {
        fprintf(stderr, "Falha ao carregar a imagem do ladrao vitorioso!\n");
        return -1;
    }

	// inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();

	// inicializa o modulo allegro que entende arquivos tff de fontes
	if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	
	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
 
	//cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    font = al_load_font("arial.ttf", 32, 1);   
	if(font == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

 	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
	}
   
	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}
	
	//instala o mouse
	if(!al_install_mouse()) {
		fprintf(stderr, "failed to initialize mouse!\n");
		return -1;
	}

	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	//registra na fila os eventos de mouse (ex: clicar em um botao do mouse)
	al_register_event_source(event_queue, al_get_mouse_event_source());  	


	//inicia o temporizador
	al_start_timer(timer);

	// ---------------------------------------------------------------------------------------

	// ------------------------- para IA -----------------------------------------------------
	int ia_jogando = 1;
	int num_max_policiais = 100;
	int num_policial = 1;
	int num_melhor_policial = 1;
	int num_melhor_policial2 = 1;
	int num_melhor_policial3 = 1;
	int geracao = 1;
	int num_max_geracoes = 500; 
	int geracao_melhor = 1;
	int geracao_melhor2 = 1;
	int geracao_melhor3 = 1;
	int melhor_pontuacao = -32768;
	int melhor_pontuacao2 = -32768;
	int melhor_pontuacao3 = -32768;
	float menor_distancia = 2147483647;
	float menor_distancia2 = 2147483647;
	float menor_distancia3 = 2147483647;
	int cont_vezes_melhor_repetiu = 0;
	int num_max_repeticoes = 3;
	int quantidade_repeticoes = 0;
	int ultima_melhor_pontuacao = melhor_pontuacao;
	int taxa_mutacao = 10;
	float pesos_melhor[5][23];
	float pesos_camada_oculta_melhor[5][5];
	float pesos_melhor2[5][23];
	float pesos_camada_oculta_melhor2[5][5];
	float pesos_melhor3[5][23];
	float pesos_camada_oculta_melhor3[5][5];
	int teve_um_vencedor = 0;
	// ---------------------------------------------------------------------------------------

	//-------------------------- criação das structs -----------------------------------------

	Tecla teclas;
	Personagem policiais[NUM_POLICIAIS];
	Personagem ladrao;
	Mundo mundo;

	inicializar_structs(&teclas, policiais, imagem_policial, &ladrao, imagem_ladrao, &mundo, imagem_cidade, imagem_policial_vitorioso, imagem_ladrao_vitorioso, imagem_policial2, imagem_policial3,  imagem_policial4, imagem_ladrao2, imagem_ladrao3, imagem_ladrao4, num_policial, ia_jogando);

	// ---------------------------------------------------------------------------------------

	// -------------- algumas variáveis usadas no looping principal --------------------------

	int playing = 1; // variável de controle
	int pause = 0; // variável de controle
	int tempo = 0; // tempo real em segundos
	int tempo_simulado = 0; // tempo simulado em segundos
	int tempo_captura; // guardará o tempo que levou para capturar o ladrão
	int record = TEMPO_LIMITE; // será o menor tempo de captura
	
	FILE *arq = fopen("record.txt", "w");
	fprintf(arq, "%d", record);
	fclose(arq);

	// ---------------------------------------------------------------------------------------
	
	//--------------------------- looping principal ------------------------------------------
	while(playing) 
	{
		int po;

		ALLEGRO_EVENT ev;
		// espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		// se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {

			// verifica se pausou
			if (teclas.p == 1 && pause == 0){
				pause = 1;
			}
			else if(teclas.o == 1 && pause == 1){
				pause = 0;
			}

			// reinicializar
			if (teclas.i == 1){
				playing = 1;
				pause = 0;
				tempo = 0;
				tempo_simulado = 0;
				
				inicializar_structs(&teclas, policiais, imagem_policial, &ladrao, imagem_ladrao, &mundo, imagem_cidade, imagem_policial_vitorioso, imagem_ladrao_vitorioso, imagem_policial2, imagem_policial3,  imagem_policial4, imagem_ladrao2, imagem_ladrao3, imagem_ladrao4, num_policial, ia_jogando);
				
				for (po=0; po<NUM_POLICIAIS; po++)
					policiais[po].ganhou = 0;
				
				ladrao.ganhou = 0;
			}

			// limpa a tela
			al_clear_to_color(al_map_rgb(0,0,0));

			for (po=0; po<NUM_POLICIAIS; po++){
				if (policiais[po].ganhou == 1){
					teve_um_vencedor = 1;
					break;
				}
			}
			
			if (teve_um_vencedor == 0 && ladrao.ganhou == 0) {
				
				if (pause == 0) {

					atualiza_posicao_elevador(&mundo, tempo);
					atualiza_posicao_escada(&mundo, tempo, policiais[0]);
					calcular_x_abs_ladrao(&ladrao);
					for (po=0; po<NUM_POLICIAIS; po++){
						(policiais[po]).dist_x_ladrao_anterior = calcular_dist_policia_ladaro_x(policiais[po], ladrao);
					}
					atualizar_posicao_ladrao(&ladrao, policiais[0]);
					calcular_x_abs_ladrao(&ladrao);
					for (po=0; po<NUM_POLICIAIS; po++){
						(policiais[po]).dist_x_ladrao = calcular_dist_policia_ladaro_x(policiais[po], ladrao);
						calcular_pontos(&policiais[po], ladrao);
						/*if((tempo % (int)FPS) == 0){
							calcular_pontos(&policiais[po], ladrao);
						}*/
					}

					// anima personagens
					animar_policial(&policiais[NUM_POLICIAIS-1],  tempo);
					
					desenhar_cenario(mundo, policiais[NUM_POLICIAIS-1]);
					animar_ladrao(&ladrao, tempo);
					desenhar_ladrao(ladrao, policiais[NUM_POLICIAIS-1]);
					
					
					for (po=0; po<NUM_POLICIAIS; po++){
						calcular_x_abs_policial(&policiais[po]);
						(policiais[po]).x_abs_anterior = (policiais[po]).x_abs;
						(policiais[po]).dist_x_ladrao_anterior = calcular_dist_policia_ladaro_x(policiais[po], ladrao);
						calcular_inputs(&policiais[po], mundo);
						calcular_outputs_camada_oculta(&policiais[po]);
						calcular_outputs(&policiais[po]);
						escolher_acao(&teclas, &policiais[po]);
						atualizar_posicao_policial(&policiais[po], &ladrao, teclas, mundo, tempo);
						verificar_interacoes(&policiais[po], &ladrao, teclas, mundo, &tempo_captura, tempo_simulado, &record, ia_jogando);
						calcular_x_abs_policial(&policiais[po]);
						(policiais[po]).dist_x_ladrao = calcular_dist_policia_ladaro_x(policiais[po], ladrao);
						calcular_pontos(&policiais[po], ladrao);
						/*if((tempo % (int)FPS) == 0){
							calcular_pontos(&policiais[po], ladrao);
						}*/
						desenhar_policial(policiais[po], mundo, policiais[NUM_POLICIAIS-1]);
					}
					atualizar_mapa(&mundo, policiais, ladrao);
										
				}

				if (pause == 1) {
					desenhar_cenario(mundo, policiais[NUM_POLICIAIS-1]);
					desenhar_ladrao(ladrao, policiais[NUM_POLICIAIS-1]);
					for (po=0; po<NUM_POLICIAIS; po++){
						desenhar_policial(policiais[po], mundo, policiais[NUM_POLICIAIS-1]);
					}
				}

				// escreve o tempo
				al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "Tempo restante: %ds", TEMPO_LIMITE - tempo_simulado);
				al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 42, ALLEGRO_ALIGN_LEFT, "Tempo que já passou: %ds", tempo_simulado);

				// dados ia
				if (ia_jogando == 1){
					al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W - 10, 10, ALLEGRO_ALIGN_RIGHT, "Geração: %d", geracao);
					al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W - 10, 42, ALLEGRO_ALIGN_RIGHT, "Policial número: %d", policiais[NUM_POLICIAIS-1].num);
					al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W - 10, 74, ALLEGRO_ALIGN_RIGHT, "Dist Ladrao: %.0f", policiais[NUM_POLICIAIS-1].dist_x_ladrao);
				}

				// registra tempo passado
				if (pause == 0 && teve_um_vencedor == 0 && ladrao.ganhou == 0){
					tempo += 1;
					if(tempo % (int)FPS == 0){
						tempo_simulado += 1;
					}
				}

				// verificar se acabaou o jogo
				if (tempo_simulado >= TEMPO_LIMITE){
					ladrao.ganhou = 1;
				}
			}
			else{
				//pausa do jogo
				teclas.p = 1;

				// mostra quem ganhou
				if (ia_jogando == 0)
					desenhar_tela_final(policiais[NUM_POLICIAIS-1], ladrao, tempo_captura, font, ia_jogando);
				else{
					if (ladrao.ganhou == 1){
						int y, z;
	
						for (po=0; po<NUM_POLICIAIS; po++){

							if (policiais[po].dist_x_ladrao <= menor_distancia){

								if (policiais[po].dist_x_ladrao < menor_distancia && geracao > 1){
									if (taxa_mutacao - 1 >= 3)
										taxa_mutacao -= 1;
								}
								
								geracao_melhor = policiais[po].geracao;
								num_melhor_policial = policiais[po].num;
								melhor_pontuacao = policiais[po].pontos;
								menor_distancia = policiais[po].dist_x_ladrao;

								for (y=0; y<5; y++){
									for (z=0; z<23; z++){
										pesos_melhor[y][z] = policiais[po].pesos[y][z];
									}
								}
								for (y=0; y<5; y++){
									for (z=0; z<5; z++){
										pesos_camada_oculta_melhor[y][z] = policiais[po].pesos_camada_oculta[y][z];
									}
								}
							}
							else if (policiais[po].dist_x_ladrao <= menor_distancia2){

								geracao_melhor2 = policiais[po].geracao;
								num_melhor_policial2 = policiais[po].num;
								melhor_pontuacao2 = policiais[po].pontos;
								menor_distancia2 = policiais[po].dist_x_ladrao;

								for (y=0; y<5; y++){
									for (z=0; z<23; z++){
										pesos_melhor2[y][z] = policiais[po].pesos[y][z];
									}
								}
								for (y=0; y<5; y++){
									for (z=0; z<5; z++){
										pesos_camada_oculta_melhor2[y][z] = policiais[po].pesos_camada_oculta[y][z];
									}
								}
							}

							else if (policiais[po].dist_x_ladrao <= menor_distancia3){

								geracao_melhor3 = policiais[po].geracao;
								num_melhor_policial3 = policiais[po].num;
								melhor_pontuacao3 = policiais[po].pontos;
								menor_distancia3 = policiais[po].dist_x_ladrao;

								for (y=0; y<5; y++){
									for (z=0; z<23; z++){
										pesos_melhor3[y][z] = policiais[po].pesos[y][z];
									}
								}
								for (y=0; y<5; y++){
									for (z=0; z<5; z++){
										pesos_camada_oculta_melhor3[y][z] = policiais[po].pesos_camada_oculta[y][z];
									}
								}
							}

						}

						printf("################################################ geracao %d ################################################", geracao);
						printf("\n");
						printf("\n");
						printf("Melhor policial: %d", num_melhor_policial);
						printf("\n");
						printf("Pontos: %d", melhor_pontuacao);
						printf("\n");
						printf("Dist do ladrao: %.0f", menor_distancia);
						printf("\n");
						printf("Pesos: ");
						printf("\n");
						for (y=0; y<5; y++){
							for (z=0; z<23; z++){
								printf("[%.0f]", pesos_melhor[y][z]);
							}
							printf("\n");
						}
						printf("\n");
						printf("Pesos camada oculta: ");
						printf("\n");
						for (y=0; y<5; y++){
							for (z=0; z<5; z++){
								printf("[%.0f]", pesos_camada_oculta_melhor[y][z]);
							}
							printf("\n");
						}
						printf("\n");
						printf("-----------------------------------------------------------------------------------------------------");
						printf("\n");
						printf("Policial numero %d", NUM_POLICIAIS);
						printf("\n");
						printf("Pontos: %d", policiais[NUM_POLICIAIS-1].pontos);
						printf("\n");
						printf("Dist do ladrao: %.0f", policiais[NUM_POLICIAIS-1].dist_x_ladrao);
						printf("\n");
						printf("Pesos: ");
						printf("\n");
						for (y=0; y<5; y++){
							for (z=0; z<23; z++){
								printf("[%.0f]", policiais[NUM_POLICIAIS-1].pesos[y][z]);
							}
							printf("\n");
						}
						printf("\n");
						printf("Pesos camada oculta: ");
						printf("\n");
						for (y=0; y<5; y++){
							for (z=0; z<5; z++){
								printf("[%.0f]", policiais[NUM_POLICIAIS-1].pesos_camada_oculta[y][z]);
							}
							printf("\n");
						}
						printf("-----------------------------------------------------------------------------------------------------");
						printf("\n");
							
						for (po=0; po<NUM_POLICIAIS; po++){
							if (policiais[po].pontos == melhor_pontuacao){
								policiais[po].eh_o_melhor = 1;
							}
							else{
								policiais[po].eh_o_melhor = 0;
							}
						}
						
						geracao += 1;
						if (geracao > num_max_geracoes /*|| quantidade_repeticoes > num_max_repeticoes*/){
							geracao = 1;
							melhor_pontuacao = -32768;
							quantidade_repeticoes = 0;
							taxa_mutacao = 6;
						}
						for (po=0; po<NUM_POLICIAIS; po++)
							policiais[po].geracao = geracao;				

						inicializar_structs(&teclas, policiais, imagem_policial, &ladrao, imagem_ladrao, &mundo, imagem_cidade, imagem_policial_vitorioso, imagem_ladrao_vitorioso, imagem_policial2, imagem_policial3,  imagem_policial4, imagem_ladrao2, imagem_ladrao3, imagem_ladrao4, num_policial, ia_jogando);
						
						playing = 1;
						pause = 0;
						tempo = 0;
						tempo_simulado = 0;
						for(po=0;po<NUM_POLICIAIS;po++)
							policiais[po].ganhou = 0;
						ladrao.ganhou = 0;
						
						if(geracao > 1){
							for (po=0; po<NUM_POLICIAIS; po++)
								mutar_policial(&policiais[po], pesos_melhor, pesos_camada_oculta_melhor,num_max_policiais, taxa_mutacao, pesos_melhor2, pesos_camada_oculta_melhor2, pesos_melhor3, pesos_camada_oculta_melhor3);
						}
					}
					else if (teve_um_vencedor == 1){
						int y, z;
						for (po=0; po<NUM_POLICIAIS; po++){
							if (policiais[po].ganhou == 1){
								geracao_melhor = policiais[po].geracao;
								num_melhor_policial = policiais[po].num;
								melhor_pontuacao = policiais[po].pontos;
								for (y=0; y<5; y++){
									for (z=0; z<23; z++){
										pesos_melhor[y][z] = policiais[po].pesos[y][z];
									}
								}
								for (y=0; y<5; y++){
									for (z=0; z<5; z++){
										pesos_camada_oculta_melhor[y][z] = policiais[po].pesos_camada_oculta[y][z];
									}
								}
							}
						
						}

						printf("################################################ dados do vencedor ################################################");
						printf("\n");
						printf("\n");
						printf("Melhor policial: %d", num_melhor_policial);
						printf("\n");
						printf("Pontos: %d", melhor_pontuacao);
						printf("\n");
						printf("Pesos: ");
						printf("\n");
						for (y=0; y<5; y++){
							for (z=0; z<23; z++){
								printf("[%.0f]", pesos_melhor[y][z]);
							}
							printf("\n");
						}
						printf("\n");
						printf("Pesos camada oculta: ");
						printf("\n");
						for (y=0; y<5; y++){
							for (z=0; z<5; z++){
								printf("[%.0f]", pesos_camada_oculta_melhor[y][z]);
							}
							printf("\n");
						}
						printf("\n");

						teve_um_vencedor = 0;

						//desenhar_tela_final(policial, ladrao, tempo_captura, font, ia_jogando);
						playing = 0;
					}					
				}

			}
			
			// atualiza a tela (quando houver algo para mostrar)
			al_flip_display();			
		}

		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
			break;
		}
		//se o tipo de evento for um clique de mouse
		else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			//printf("\nmouse clicado em: %d, %d", ev.mouse.x, ev.mouse.y);
		}
		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) { 
			verificar_teclas(ev, &teclas, 1, ia_jogando);
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			verificar_teclas(ev, &teclas, 0, ia_jogando);
		}

	} //fim do while

	// ---------------------------------------------------------------------------------------
     
	//----------------------- procedimentos de fim de jogo -----------------------------------
	//                   (fecha a tela, limpa a memoria, etc)
	 
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_destroy_font(font);
	al_destroy_bitmap(imagem_cidade);
	al_destroy_bitmap(imagem_policial);
	al_destroy_bitmap(imagem_policial2);
	al_destroy_bitmap(imagem_policial3);
	al_destroy_bitmap(imagem_policial4);
	al_destroy_bitmap(imagem_policial_vitorioso);
	al_destroy_bitmap(imagem_ladrao);
	al_destroy_bitmap(imagem_ladrao2);
	al_destroy_bitmap(imagem_ladrao3);
	al_destroy_bitmap(imagem_ladrao4);
	al_destroy_bitmap(imagem_ladrao_vitorioso);
 
	return 0;

	// ---------------------------------------------------------------------------------------
}

// ---------------------------------------------------------------------------------------------------