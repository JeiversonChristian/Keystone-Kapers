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
}Personagem;

typedef struct Mundo{
	ALLEGRO_BITMAP *imagem_cidade;
	float g; // gravidade
	float dt; // intervalo de tempo que passa no mundo
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

void inicializar_structs(Tecla *teclas, Personagem *policial, ALLEGRO_BITMAP *imagem_policial, Personagem *ladrao, ALLEGRO_BITMAP *imagem_ladrao, Mundo *mundo, ALLEGRO_BITMAP *imagem_cidade){
	// teclas
	(*teclas).espaco = 0; // não pressionada
	(*teclas).d = 0; // não pressionada
	(*teclas).a = 0; // não pressionada

	// Policial
	(*policial).imagem = imagem_policial;
	(*policial).largura = al_get_bitmap_width(imagem_policial);
	(*policial).altura = al_get_bitmap_height(imagem_policial);
	(*policial).x = SCREEN_W/2 - (*policial).largura/2; // no meio da tela
	(*policial).x_global = 3*SCREEN_W + (*policial).x; // quase no final já
	(*policial).y = 5*SCREEN_H/6.0 - (*policial).altura; // no primeiro andar
	(*policial).vx = 5.0; // ligeiramente mais rápido que o ladrão
	(*policial).vy = 50.0; // valor alto, porque a lógica do pulo é outra
	(*policial).direcao_pulo = 0.0; // pulando em nenhuma direção
	(*policial).orientacao = 0; // virado pra direita
	(*policial).andar = 1; // primeiro andar
	(*policial).pode_pular = 1; // sim
	(*policial).pulando = 0; // não
	(*policial).pode_andar = 1; // sim
	(*policial).y_chao = (*policial).y + (*policial).altura; // y do chão abaixo do policial
	(*policial).num_tela = 1; // tela que o jogador vê

	// Ladrão
	(*ladrao).imagem = imagem_ladrao;
	(*ladrao).largura = al_get_bitmap_width(imagem_ladrao);
	(*ladrao).altura = al_get_bitmap_height(imagem_ladrao);
	(*ladrao).x = (*policial).x - 3*(*ladrao).largura; // um pouco longe do policial
	(*ladrao).x_global = 3*SCREEN_W + (*ladrao).x; // quase no final já
	(*ladrao).y = 5*SCREEN_H/6.0 - (*ladrao).altura; // mesmo andar que o policial
	(*ladrao).vx = 3.0; // ligeiramente mais lento que o policial
	(*policial).vy = 0; // pra não pular mesmo
	(*ladrao).direcao_pulo = 0.0; // pulando em nenhuma direção
	(*ladrao).orientacao = 1; // virado pra esquerda
	(*ladrao).andar = 2; // acima do policial
	(*ladrao).pode_pular = 1; // sim, mas não sei se ele vai pular ainda
	(*ladrao).pulando = 0; //não
	(*ladrao).pode_andar = 1; // sim
	(*ladrao).y_chao = (*ladrao).y + (*ladrao).altura; // y do chão abaixo do ladrão
	(*ladrao).num_tela = 1; // tela que o bandido "vê"

	// mundo
	(*mundo).imagem_cidade = imagem_cidade;
	(*mundo).g = 25.0; // valor alto, pra lógica do pulo
	(*mundo).dt = 5.0/FPS;
}

void atualizar_posicao_policial(Personagem *policial, Personagem *ladrao, Tecla teclas, Mundo mundo){
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
	if (teclas.espaco == 1 && (*policial).pode_pular == 1){
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

}

void atualizar_posicao_ladrao(Personagem *ladrao, Personagem policial) {

	// foge ladrão
	// o policial tiver na frente do ladrão, ele foge pro outro lado
	if (policial.x < (*ladrao).x && policial.y <= (*ladrao).y){
		(*ladrao).orientacao = 0;
	}
	else if (policial.x > (*ladrao).x && policial.y >= (*ladrao).y){
		(*ladrao).orientacao = 1;
	}
	int direcao_andar;
	if ((*ladrao).orientacao == 0){
		direcao_andar = 1;
	}
	else if ((*ladrao).orientacao == 1){
		direcao_andar = -1;
	}
	(*ladrao).x += (*ladrao).vx * direcao_andar;

	// subindo de andar se chegar no fim da tela esquerda
	if ((*ladrao).x + (*ladrao).largura <= 0) {
		(*ladrao).x = SCREEN_W - (*ladrao).largura;
		(*ladrao).y -=  SCREEN_H/6.0; // altura de um andar
		(*ladrao).andar += 1;
		(*ladrao).y_chao = (*ladrao).y + (*ladrao).altura;
	}
	// descendo de andar se chegar no fim da tela direita
	else if ((*ladrao).x >= SCREEN_W) {
		(*ladrao).x = 0;
		(*ladrao).y +=  SCREEN_H/6.0;
		(*ladrao).andar -= 1;
		(*ladrao).y_chao = (*ladrao).y + (*ladrao).altura;
	}
}

void desenhar_cenario(Mundo mundo, Personagem policial) {
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

	// elementos estáticos

	// tela 1
	if (policial.num_tela == 1) {
		// retangulo azul
		float dist_parede = SCREEN_W/4.0;
		float largura = SCREEN_W/6.0;
		float dist_teto = (5*SCREEN_H/6.0 - 4*SCREEN_H/6.0)/1.5; 
		float altura = 5*SCREEN_H/6.0;
		al_draw_filled_rectangle(0 + dist_parede, 4*SCREEN_H/6.0 + dist_teto, dist_parede + largura, altura, al_map_rgb(80,112,188));
		al_draw_filled_rectangle(SCREEN_W - dist_parede, 4*SCREEN_H/6.0 + dist_teto, SCREEN_W - dist_parede - largura, altura, al_map_rgb(80,112,188));
	}

}

void desenhar_policial(Personagem policial) {
	al_draw_bitmap(policial.imagem, policial.x , policial.y, policial.orientacao);
}

void desenhar_ladrao(Personagem ladrao) {
	al_draw_bitmap(ladrao.imagem, ladrao.x , ladrao.y, ladrao.orientacao);
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

	inicializar_structs(&teclas, &policial, imagem_policial, &ladrao, imagem_ladrao, &mundo, imagem_cidade);

	// ---------------------------------------------------------------------------------------
	
	//--------------------------- looping principal ------------------------------------------

	int playing = 1;
	while(playing) 
	{
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {

			//limpa a tela
			al_clear_to_color(al_map_rgb(0,0,0));


			//atualiza posicões personagens
			atualizar_posicao_policial(&policial, &ladrao, teclas, mundo);
			atualizar_posicao_ladrao(&ladrao, policial);

			//desenha tudo
			desenhar_cenario(mundo, policial);
			desenhar_policial(policial);
			desenhar_ladrao(ladrao);

			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();
			
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
 
	return 0;

	// ---------------------------------------------------------------------------------------
}

// ---------------------------------------------------------------------------------------------------