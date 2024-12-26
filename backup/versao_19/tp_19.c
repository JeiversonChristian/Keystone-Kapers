// versao 19
// ---------------------------------------------------------------------------------------------------
// Bibliotecas

#include <stdio.h>
#include <math.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
// ---------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// Structs

typedef struct Tecla{
	int espaco;
	int d;
	int a;
	int w;
	int s;
	int p;
	int o;
}Tecla;

typedef struct Personagem{
	ALLEGRO_BITMAP *imagem; // imagem do personagem
	float largura; // largura da imagem
	float altura; // altura da imagem
	float x; // coordenada do ponto de referência do retângulo que engloba a imagem
	float x_global; // para controlar globalmente onde o personagem está
	float y; // coordenada do ponto de referência do retângulo que engloba a imagem
	float vx; // velocidade no eixo x
	float vy; // velocidade no eixo y
	float direcao_pulo;
	int orientacao; // virado para direita ou para esquerda
	int andar; // andar onde se encontra
	int pode_andar; // variável de controle
	int pode_pular; // variável de controle
	int pulando; // variável de controle
	float y_chao; // altura do chão onde se encontra
	int num_tela; // cada cenário tem várias telas
	int dentro_elevador; // variável de controle
	int dentro_escada; // variável de controle
	int escada_num; // de controle em conjunto com dentro_escada
	int degrau_num; // de controle em conjunto com dentro_escada
	int na_lama; // variável de controle
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
	ALLEGRO_BITMAP *imagem; // só do corrimao
}Escada;

typedef struct Mundo{
	ALLEGRO_BITMAP *imagem_cidade;
	float g; // gravidade
	float dt; // intervalo de tempo que passa no mundo
	Elevador elevador;
	Lama lamas[6];
	Escada escadas[3];
}Mundo;
// ---------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// Constantes

const float FPS = 60; 
// dimensão 4:3 -> 4x230 = 920 e 3x230 = 690
const int SCREEN_W = 960;
const int SCREEN_H = 690;
// ---------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// Funções

void inicializar_structs(Tecla *teclas, Personagem *policial, ALLEGRO_BITMAP *imagem_policial, Personagem *ladrao, ALLEGRO_BITMAP *imagem_ladrao, Mundo *mundo, ALLEGRO_BITMAP *imagem_cidade, ALLEGRO_BITMAP *imagem_corrimao0, ALLEGRO_BITMAP *imagem_corrimao1, ALLEGRO_BITMAP *imagem_corrimao2){
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

	// Policial
	(*policial).imagem = imagem_policial;
	(*policial).largura = al_get_bitmap_width(imagem_policial);
	(*policial).altura = al_get_bitmap_height(imagem_policial);
	(*policial).x = SCREEN_W/2 - (*policial).largura/2; // no meio da sala (tela)
	(*policial).x_global = 3*SCREEN_W + (*policial).x; // quase no final já
	(*policial).y = 5*SCREEN_H/6.0 - (*policial).altura; // no primeiro andar
	(*policial).vx = 5.0; // ligeiramente mais rápido que o ladrão
	(*policial).vy = 50.0; // valor alto, porque a lógica do pulo é outra
	(*policial).direcao_pulo = 0.0; // pulando em nenhuma direção
	(*policial).orientacao = 1; // virado pra esquerda
	(*policial).andar = 1; // primeiro andar
	(*policial).pode_pular = 1; // sim
	(*policial).pulando = 0; // não
	(*policial).pode_andar = 1; // sim
	(*policial).y_chao = (*policial).y + (*policial).altura; // y do chão abaixo do policial
	(*policial).num_tela = 1; // tela que o jogador vê
	(*policial).dentro_elevador = 0; // começa fora
	(*policial).dentro_escada = 0; // começa fora
	(*policial).escada_num = -1; // nenhum
	(*policial).degrau_num = -1; // nenhum
	(*policial).na_lama = 0;

	// Ladrão
	(*ladrao).imagem = imagem_ladrao;
	(*ladrao).largura = al_get_bitmap_width(imagem_ladrao);
	(*ladrao).altura = al_get_bitmap_height(imagem_ladrao);
	(*ladrao).x = SCREEN_W/4 - (*ladrao).largura/2;; // 1/4 de um do andar
	(*ladrao).x_global = 0*SCREEN_W + (*ladrao).x; // na 4ª sala (tela)
	(*ladrao).y = 4*SCREEN_H/6.0 - (*ladrao).altura; // andar do meio
	(*ladrao).vx = 3.0; // ligeiramente mais lento que o policial
	(*policial).vy = 0; // pra não pular mesmo
	(*ladrao).direcao_pulo = 0.0; // pulando em nenhuma direção
	(*ladrao).orientacao = 0; // virado pra direita
	(*ladrao).andar = 2; // acima do policial
	(*ladrao).pode_pular = 1; // sim, mas não sei se ele vai pular ainda
	(*ladrao).pulando = 0; //não
	(*ladrao).pode_andar = 1; // sim
	(*ladrao).y_chao = (*ladrao).y + (*ladrao).altura; // y do chão abaixo do ladrão
	(*ladrao).num_tela = 1; // tela que o bandido "vê"
	(*ladrao).dentro_elevador = 0; // será que faço ele aprender a entrar?
	(*ladrao).dentro_escada = 0; // idem
	(*ladrao).escada_num = -1; // nenhum
	(*ladrao).degrau_num = -1; // nenhum
	(*ladrao).na_lama = 0;

	// mundo
	(*mundo).imagem_cidade = imagem_cidade;
	(*mundo).g = 25.0; // valor alto, pra lógica do pulo
	(*mundo).dt = 5.0/FPS;
	
	// -> elevador
	(*mundo).elevador.largura = (1.5)*(*policial).largura;
	(*mundo).elevador.altura = (*policial).altura + (*policial).altura/4 + (*policial).altura/8;
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
	(*mundo).lamas[0].largura = 2*(*policial).largura; 
	(*mundo).lamas[0].altura = (*policial).altura/16; // fininha
	(*mundo).lamas[0].x = 0; 
	(*mundo).lamas[0].x_global = 3*SCREEN_W; // 1ª tela
	(*mundo).lamas[0].y = (*policial).y_chao; // 1º andar
	(*mundo).lamas[0].y_chao = (*policial).y_chao - (*mundo).lamas[0].altura; 
	(*mundo).lamas[0].num_sala = 1;
	(*mundo).lamas[0].andar = 1;

	(*mundo).lamas[1].largura = 2*(*policial).largura; 
	(*mundo).lamas[1].altura = (*policial).altura/16; // fininha
	(*mundo).lamas[1].x_global = 2*SCREEN_W + SCREEN_W/2 - ((*mundo).lamas[1].largura)/2; //2ª tela
	(*mundo).lamas[1].x = (int)((*mundo).lamas[1]).x_global % SCREEN_W;
	(*mundo).lamas[1].y = (*policial).y_chao; // 1º andar
	(*mundo).lamas[1].y_chao = (*policial).y_chao - (*mundo).lamas[0].altura;
	(*mundo).lamas[1].num_sala = 2;
	(*mundo).lamas[1].andar = 1;

	(*mundo).lamas[2].largura = 2*(*policial).largura; 
	(*mundo).lamas[2].altura = (*policial).altura/16; // fininha
	(*mundo).lamas[2].x_global = 3*SCREEN_W + SCREEN_W/5; // 1ª tela
	(*mundo).lamas[2].x = (int)((*mundo).lamas[2]).x_global % SCREEN_W;
	(*mundo).lamas[2].y = (*mundo).lamas[1].y - SCREEN_H/6; // 2º andar
	(*mundo).lamas[2].y_chao = (*policial).y_chao - (*mundo).lamas[0].altura - SCREEN_H/6;
	(*mundo).lamas[2].num_sala = 1;
	(*mundo).lamas[2].andar = 2;

	(*mundo).lamas[3].largura = 2*(*policial).largura; 
	(*mundo).lamas[3].altura = (*policial).altura/16; // fininha
	(*mundo).lamas[3].x_global = 0*SCREEN_W + SCREEN_W/4; // 4ª tela
	(*mundo).lamas[3].x = (int)((*mundo).lamas[3]).x_global % SCREEN_W;
	(*mundo).lamas[3].y = (*mundo).lamas[1].y - SCREEN_H/6; // 2º andar
	(*mundo).lamas[3].y_chao = (*policial).y_chao - (*mundo).lamas[0].altura - SCREEN_H/6;
	(*mundo).lamas[3].num_sala = 4;
	(*mundo).lamas[3].andar = 2;

	(*mundo).lamas[4].largura = 2*(*policial).largura; 
	(*mundo).lamas[4].altura = (*policial).altura/16; // fininha
	(*mundo).lamas[4].x_global = 1*SCREEN_W; // 3ª tela
	(*mundo).lamas[4].x = (int)((*mundo).lamas[4]).x_global % SCREEN_W;
	(*mundo).lamas[4].y = (*mundo).lamas[1].y - 2*SCREEN_H/6; // 3º andar
	(*mundo).lamas[4].y_chao = (*policial).y_chao - (*mundo).lamas[0].altura - 2*SCREEN_H/6;
	(*mundo).lamas[4].num_sala = 3;
	(*mundo).lamas[4].andar = 3;

	(*mundo).lamas[5].largura = 2*(*policial).largura; 
	(*mundo).lamas[5].altura = (*policial).altura/16; // fininha
	(*mundo).lamas[5].x_global = 2*SCREEN_W + (*mundo).lamas[5].largura; // 2ª tela
	(*mundo).lamas[5].x = (int)((*mundo).lamas[5]).x_global % SCREEN_W;
	(*mundo).lamas[5].y = (*mundo).lamas[1].y - 2*SCREEN_H/6; // 3º andar
	(*mundo).lamas[5].y_chao = (*policial).y_chao - (*mundo).lamas[0].altura - 2*SCREEN_H/6;
	(*mundo).lamas[5].num_sala = 2;
	(*mundo).lamas[5].andar = 3;

	// -> escada
	// escada 1 -> 1ª tela, 2º andar
	// ------------------------------------------------------------------------------------------------
	(*mundo).escadas[1].num_sala = 1;
	(*mundo).escadas[1].andar = 2;
	(*mundo).escadas[1].imagem = imagem_corrimao1;

	(*mundo).escadas[1].degraus[0].largura = (*policial).largura/2;
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
		(*mundo).escadas[1].degraus[i].largura = (*policial).largura/2;
		(*mundo).escadas[1].degraus[i].altura = 0.8*(SCREEN_H/6)/8;
		(*mundo).escadas[1].degraus[i].x_global = 4*SCREEN_W - SCREEN_W/2 + (i-1)*(*mundo).escadas[1].degraus[i].largura;
		(*mundo).escadas[1].degraus[i].x = (int)(*mundo).escadas[1].degraus[i].x_global % SCREEN_W;
		(*mundo).escadas[1].degraus[i].y = 4*(SCREEN_H/6) - (i-1)*(*mundo).escadas[1].degraus[i].altura;
		(*mundo).escadas[1].degraus[i].y_chao = 4*(SCREEN_H/6) + (*mundo).escadas[1].degraus[i].altura - (i-1)*(*mundo).escadas[1].degraus[i].altura;
		(*mundo).escadas[1].degraus[i].num = i;
	}

	(*mundo).escadas[1].degraus[12].largura = (*policial).largura/2;
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
	// ------------------------------------------------------------------------------------------------

	// escada 0 -> 4ª tela, 1º andar
	// escada 2 -> 4ª tela, 3º andar
	// ------------------------------------------------------------------------------------------------
	for (j=0; j<=2; j+=2){
		(*mundo).escadas[j].num_sala = 4;
		(*mundo).escadas[j].andar = 1+j;
		if(j==0)
			(*mundo).escadas[j].imagem = imagem_corrimao0;
		else
			(*mundo).escadas[j].imagem = imagem_corrimao2;

		(*mundo).escadas[j].degraus[12].largura = (*policial).largura/2;
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

		(*mundo).escadas[j].degraus[0].largura = (*policial).largura/2;
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
			(*mundo).escadas[j].degraus[i].largura = (*policial).largura/2;
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
	}	
	// ------------------------------------------------------------------------------------------------
}

void atualizar_posicao_policial(Personagem *policial, Personagem *ladrao, Tecla teclas, Mundo mundo){
	// lama
	int i;
	int j;
	int t = 10; // tolerancia
	for(i=0; i<6; i++){
		if ( (((*policial).x_global > mundo.lamas[i].x_global && (*policial).x_global < mundo.lamas[i].x_global + mundo.lamas[i].largura - t) || (((*policial).x_global + (*policial).largura > mundo.lamas[i].x_global + t && (*policial).x_global + (*policial).largura < mundo.lamas[i].x_global + mundo.lamas[i].largura))) && mundo.lamas[i].andar == (*policial).andar && (*policial).pulando == 0){
			(*policial).vx = 1.0;
			(*policial).na_lama = 1;
			break; // ao se achar na lama, esteje na lama até não estar mais
		}
		else {
			(*policial).vx = 5.0;
			(*policial).na_lama = 0;
		}
	}
	
	// andar para esquerda
	if (teclas.a == 1 && teclas.d == 0 && (*policial).pode_andar == 1){
		(*policial).orientacao = 1; // orientação da imagem -> pra esquerda
		// não pode passar da parede do final lá da esquerda
		if((*policial).x_global > 0) {
			(*policial).x_global -= (*policial).vx;
			(*policial).x = (int)(*policial).x_global % SCREEN_W;
		}
		else {
			// cola na parede
			(*policial).x_global = 0;
			(*policial).x = (int)(*policial).x_global % SCREEN_W;
		}
	}
	// andar para direita
	if (teclas.a == 0 && teclas.d == 1 && (*policial).pode_andar == 1){
		(*policial).orientacao = 0; // orientação da imagem -> pra direita
		// não pode passar da parede do começo da direita
		if((*policial).x_global + (*policial).largura < 4*SCREEN_W){
			(*policial).x_global += (*policial).vx;
			(*policial).x = (int)(*policial).x_global % SCREEN_W;
		}
		else {
			// cola na parede
			(*policial).x_global = 4*SCREEN_W - (*policial).largura;
			(*policial).x = (int)(*policial).x_global % SCREEN_W;
		}
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
			(*policial).x_global += (*policial).vx * (*policial).direcao_pulo;
			(*policial).x = (int)(*policial).x_global % SCREEN_W;
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

	// entrar e sair do elevador
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
		(*policial).y = mundo.elevador.y_porta + (*policial).altura/4;
		(*policial).y_chao = mundo.elevador.y_chao_porta;
		(*policial).andar = mundo.elevador.andar;
	}

	// usar escada rolante
	for (j=0; j<=2; j++){
		if((*policial).dentro_escada==1){
			break; // pra não ir pra dentro da outra escada
		}
		if ((*policial).x_global >= mundo.escadas[j].x_global && (*policial).x_global <= mundo.escadas[j].x_global + mundo.escadas[j].largura && (*policial).andar == mundo.escadas[j].andar){
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
	//subir
	if((*policial).dentro_escada == 1){
		(*policial).y_chao = mundo.escadas[(*policial).escada_num].degraus[(*policial).degrau_num].y_chao - mundo.escadas[(*policial).escada_num].degraus[0].altura;
		(*policial).y = (*policial).y_chao - (*policial).altura;
		(*policial).x_global = mundo.escadas[(*policial).escada_num].degraus[(*policial).degrau_num].x_global;
		(*policial).x = (int)(*policial).x_global % SCREEN_W;
		if((*policial).y_chao <= mundo.escadas[(*policial).escada_num].teto.y_chao - mundo.escadas[(*policial).escada_num].degraus[0].altura){
			(*policial).dentro_escada = 0;
			(*policial).escada_num = -1;
			(*policial).degrau_num = -1;
			(*policial).pode_andar = 1;
			(*policial).pode_pular = 1;
			(*policial).andar += 1;
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
}

void atualizar_posicao_ladrao(Personagem *ladrao, Personagem policial) {
	// foge ladrão
	// o policial tiver na frente do ladrão, ele foge pro outro lado
	if (policial.x_global < (*ladrao).x_global && policial.andar == (*ladrao).andar && policial.dentro_elevador == 0){
		(*ladrao).orientacao = 0;
	}
	else if (policial.x_global > (*ladrao).x_global && policial.andar == (*ladrao).andar && policial.dentro_elevador == 0){
		(*ladrao).orientacao = 1;
	}

	int direcao_andar;
	if ((*ladrao).orientacao == 0){
		direcao_andar = 1;
	}
	else if ((*ladrao).orientacao == 1){
		direcao_andar = -1;
	}
	(*ladrao).x_global += (*ladrao).vx * direcao_andar;
	(*ladrao).x = (int)(*ladrao).x_global % SCREEN_W;

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
}

void atualiza_posicao_elevador(Mundo *mundo, int tempo){
	// a cada 2 segundos atualiza
	if (tempo % (2*60) == 0){
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
	if (tempo % (60/24) == 0){
		int i;
		int j;
		int num_teto;
		int num_pe;
		if (policial.andar == (*mundo).escadas[1].andar && policial.num_tela == (*mundo).escadas[1].num_sala){

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
			if (policial.andar == (*mundo).escadas[j].andar && policial.num_tela == (*mundo).escadas[j].num_sala){
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

void desenhar_cenario(Mundo mundo, Personagem policial) {
	int i;
	int j;
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
	if(mundo.escadas[1].num_sala == policial.num_tela){
		for (i=0; i<13; i++){
			al_draw_filled_rectangle(mundo.escadas[1].degraus[i].x, mundo.escadas[1].degraus[i].y, mundo.escadas[1].degraus[i].x + mundo.escadas[1].degraus[i].largura, mundo.escadas[1].degraus[i].y_chao, al_map_rgb(255,255,255));		
		}
		// corrimao da escada 1
		float x1 = mundo.escadas[1].x + mundo.escadas[1].largura + mundo.escadas[1].degraus[12].largura, y2 = mundo.escadas[1].pe.y_chao; // vértice no ângulo reto
    	float x2 =  mundo.escadas[1].x+5, y1 = mundo.escadas[1].pe.y_chao; // vértice no final da base
    	float x3 = mundo.escadas[1].x + mundo.escadas[1].largura + mundo.escadas[1].degraus[12].largura, y3 = mundo.escadas[1].teto.y; // vértice da altura
		al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, al_map_rgb(0, 60, 0));
		//al_draw_bitmap(mundo.escadas[1].imagem, mundo.escadas[1].x+5, mundo.escadas[1].y-3, 0);
	}
	// --------------------------------------------------------------------------------------------
	// escada 0 -> 4ª tela, 1º andar
	// escada 2 -> 4ª tela, 3º andar
	// --------------------------------------------------------------------------------------------
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
		//al_draw_bitmap(mundo.escadas[0].imagem, mundo.escadas[0].x-18, mundo.escadas[0].y-2, 1);
	}
	if(mundo.escadas[2].num_sala == policial.num_tela){
		// corrimao da escada 2
    	float x1 = 1.1*mundo.escadas[2].degraus[0].largura, y1 = mundo.escadas[2].pe.y_chao;
    	float x2 =  mundo.escadas[2].largura + 2.5*mundo.escadas[2].degraus[0].largura, y2 = mundo.escadas[2].pe.y_chao;
    	float x3 = 1.1*mundo.escadas[2].degraus[0].largura, y3 = mundo.escadas[2].teto.y;
		al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, al_map_rgb(0, 60, 0));
		// al_draw_bitmap(mundo.escadas[2].imagem, mundo.escadas[2].x-18, mundo.escadas[2].y-2, 1);
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

}

void desenhar_policial(Personagem policial, Mundo mundo) {
	if (policial.dentro_elevador == 0 || (policial.dentro_elevador == 1 && mundo.elevador.porta_aberta == 1)){
		al_draw_bitmap(policial.imagem, policial.x , policial.y, policial.orientacao);
	}
}

void desenhar_ladrao(Personagem ladrao, Personagem policial) {
	if (ladrao.num_tela == policial.num_tela){
		al_draw_bitmap(ladrao.imagem, ladrao.x , ladrao.y, ladrao.orientacao);
	}
}

void verificar_teclas(ALLEGRO_EVENT ev, Tecla *teclas, int pressionado) {
	// pressioando = 0 -> não pressionado
	// pressioando = 1 -> pressionado

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
	if (ev.keyboard.keycode == ALLEGRO_KEY_P) {
		(*teclas).p = pressionado;
    }
	if (ev.keyboard.keycode == ALLEGRO_KEY_O) {
		(*teclas).o = pressionado;
    }
}
// ---------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
// Função principal

int main(int argc, char **argv){
	
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

	// imagem do Ladrão harry hooligan
	ALLEGRO_BITMAP *imagem_ladrao = al_load_bitmap("../../imagens_personagens/harry_hooligan_pos_inicial.png");
	if (!imagem_ladrao) {
        fprintf(stderr, "Falha ao carregar a imagem do harry hooligan!\n");
        return -1;
    }

	// imagem do corrimao da escada rolante
	ALLEGRO_BITMAP *imagem_corrimao0 = al_load_bitmap("../../imagens_cenario/corrimao.png");
	if (!imagem_corrimao0) {
        fprintf(stderr, "Falha ao carregar a imagem do corrimao 0!\n");
        return -1;
    }
	// imagem do corrimao da escada rolante
	ALLEGRO_BITMAP *imagem_corrimao1 = al_load_bitmap("../../imagens_cenario/corrimao.png");
	if (!imagem_corrimao1) {
        fprintf(stderr, "Falha ao carregar a imagem do corrimao 1!\n");
        return -1;
    }
	// imagem do corrimao da escada rolante
	ALLEGRO_BITMAP *imagem_corrimao2 = al_load_bitmap("../../imagens_cenario/corrimao.png");
	if (!imagem_corrimao2) {
        fprintf(stderr, "Falha ao carregar a imagem do corrimao 2!\n");
        return -1;
    }

	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();

	//inicializa o modulo allegro que entende arquivos tff de fontes
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

	//-------------------------- criação das structs -----------------------------------------

	Tecla teclas;
	Personagem policial;
	Personagem ladrao;
	Mundo mundo;

	inicializar_structs(&teclas, &policial, imagem_policial, &ladrao, imagem_ladrao, &mundo, imagem_cidade, imagem_corrimao0, imagem_corrimao1, imagem_corrimao2);

	// ---------------------------------------------------------------------------------------
	
	//--------------------------- looping principal ------------------------------------------

	int playing = 1;
	int pause = 0;
	int tempo = 0;
	int tempo_simulado = 0;
	while(playing) 
	{
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {

			// verifica se pausou
			if (teclas.p == 1 && pause == 0){
				pause = 1;
			}
			else if(teclas.o == 1 && pause == 1){
				pause = 0;
			}

			//limpa a tela
			al_clear_to_color(al_map_rgb(0,0,0));
			//atualiza posicões de tudo

			if (pause == 0) {
				atualizar_posicao_policial(&policial, &ladrao, teclas, mundo);
				atualizar_posicao_ladrao(&ladrao, policial);
				atualiza_posicao_elevador(&mundo, tempo);
				atualiza_posicao_escada(&mundo, tempo, policial);
			}

			//desenha tudo
			desenhar_cenario(mundo, policial);
			desenhar_policial(policial, mundo);
			desenhar_ladrao(ladrao, policial);

			// escreve o tempo que falta no topo
			 al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "Tempo restante: %d s", 60 - tempo_simulado);

			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();

			// registra tempo passado
			if (pause == 0)
				tempo += 1;
				if(tempo%60 == 0){
					tempo_simulado += 1;
				}

			// verificar se acabaou o jogo
			if (tempo_simulado >= 60){
				// finalizar jogo, status perdeu
			}			
		}

		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
		//se o tipo de evento for um clique de mouse
		else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			//printf("\nmouse clicado em: %d, %d", ev.mouse.x, ev.mouse.y);
		}
		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) { 
			verificar_teclas(ev, &teclas, 1);
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			verificar_teclas(ev, &teclas, 0);
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
	al_destroy_bitmap(imagem_ladrao);
	al_destroy_bitmap(imagem_corrimao0);
	al_destroy_bitmap(imagem_corrimao1);
	al_destroy_bitmap(imagem_corrimao2);
 
	return 0;

	// ---------------------------------------------------------------------------------------
}

// ---------------------------------------------------------------------------------------------------