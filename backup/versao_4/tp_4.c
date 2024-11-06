#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>


const float FPS = 60;  

// Dimensão 4:3 -> 4x230 = 920 e 3x230 = 690
const int SCREEN_W = 920; // valor original do professor: 960
const int SCREEN_H = 690; // valor original do professor: 540

// --------------------------------------------------------------------------------------------
// funções criadas por mim:

void verificar_teclas_pressionadas(ALLEGRO_EVENT ev, int *espaco, int *seta_direita, int *seta_esquerda, int estado) {
	if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) {
		//printf("\napertou: %d = espaco", ev.keyboard.keycode);
		*espaco = estado;
	}
	if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
    	printf("\napertou: %d = seta direita", ev.keyboard.keycode);
		*seta_direita = estado;
    }
	if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
        printf("\napertou: %d = seta esquerda", ev.keyboard.keycode);
		*seta_esquerda = estado;
    }
}

void desenhar_cenario(int num_tela, ALLEGRO_BITMAP *imagem_cidade) {
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
	al_draw_bitmap(imagem_cidade, 0, 1*SCREEN_H/6.0, 0);

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
	if (num_tela == 1) {
		// retangulo azul
		float dist_parede = SCREEN_W/4.0;
		float largura = SCREEN_W/6.0;
		float dist_teto = (5*SCREEN_H/6.0 - 4*SCREEN_H/6.0)/1.5; 
		float altura = 5*SCREEN_H/6.0;
		al_draw_filled_rectangle(0 + dist_parede, 4*SCREEN_H/6.0 + dist_teto, dist_parede + largura, altura, al_map_rgb(80,112,188));
		al_draw_filled_rectangle(SCREEN_W - dist_parede, 4*SCREEN_H/6.0 + dist_teto, SCREEN_W - dist_parede - largura, altura, al_map_rgb(80,112,188));
	}

}

void atualizar_posicao_personagem_principal(float *x_personagem, float *y_personagem, int *espaco, int *seta_direita, int *seta_esquerda, int *orientacao_personagem, float largura_personagem) {
	// mechendo de acordo com as teclas
	if (*seta_esquerda == 1) {
		*x_personagem -= 5.0;
		*orientacao_personagem = 1;
	}
	if (*seta_direita == 1) {
		*x_personagem += 5.0;
		*orientacao_personagem = 0;
	}

	// subindo de andar se chegar no fim da tela esquerda
	if (*x_personagem + largura_personagem <= 0) {
		*x_personagem = SCREEN_W - largura_personagem;
		*y_personagem -=  SCREEN_H/6.0; // altura de um andar -> depois refatorar código com esse nome
	}
	// descendo de andar se chegar no fim da tela direita
	else if (*x_personagem >= SCREEN_W) {
		*x_personagem = 0;
		*y_personagem +=  SCREEN_H/6.0; // altura de um andar -> depois refatorar código com esse nome
	}

	// ainda não coloquei as colisões e proibições de subir ou descer
}

void desenhar_personagem_principal(ALLEGRO_BITMAP *imagem_kelly_keystone, float x_personagem, float y_personagem, int *orientacao_personagem) {
	al_draw_bitmap(imagem_kelly_keystone, x_personagem , y_personagem, *orientacao_personagem);
	// o zero no final se refere à orientação do personagem
}
// --------------------------------------------------------------------------------------------
 
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

	// -------------------------------------------------------------------------------
	// criado por mim

	// inicializar imagens usadas no game:

	// imagem do  cidade ao fundo
	ALLEGRO_BITMAP *imagem_cidade = al_load_bitmap("../../imagens_cenario/cidade.png");
	if (!imagem_cidade) {
        fprintf(stderr, "Falha ao carregar a imagem da cidade!\n");
       return -1;
    }

	// imagem do  Kelly Keystone (personagem principal)
	ALLEGRO_BITMAP *imagem_kelly_keystone = al_load_bitmap("../../imagens_personagens/kelly_keystone_pos_inicial.png");
	if (!imagem_kelly_keystone) {
        fprintf(stderr, "Falha ao carregar a imagem do kelly keystone!\n");
        return -1;
    }
	// -------------------------------------------------------------------------------

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


	// criado por mim -------------------------------------------
	// variáveis para controlar estado das teclas pressionadas
	int espaco;
	int seta_direita;
	int seta_esquerda;

	// variáveis do personagem
	float largura_personagem = al_get_bitmap_width(imagem_kelly_keystone);
	float altura_personage = al_get_bitmap_height(imagem_kelly_keystone);
	float x_personagem = SCREEN_W/2 - largura_personagem/2;
	float y_personagem = 4*SCREEN_H/6.0 + 50;
	int orientacao_personagem = 0; // virado pra direita
	// ----------------------------------------------------------

	//inicia o temporizador
	al_start_timer(timer);

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

			//aplica fisica
			//atualizar_posicao_personagens(imagem);

			//desenha
			// criado por mim -------------------------------------------
			int num_tela = 1;
			desenhar_cenario(num_tela, imagem_cidade);
			atualizar_posicao_personagem_principal(&x_personagem, &y_personagem, &espaco, &seta_direita, &seta_esquerda, &orientacao_personagem, largura_personagem);
			desenhar_personagem_principal(imagem_kelly_keystone, x_personagem, y_personagem, &orientacao_personagem);
			//-----------------------------------------------------------

			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();
			
		}

		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
		//se o tipo de evento for um clique de mouse
		else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			printf("\nmouse clicado em: %d, %d", ev.mouse.x, ev.mouse.y);
		}
		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) { // tecla pressionada = 1
			//imprime qual tecla foi
			//printf("\ncodigo tecla: %d", ev.keyboard.keycode);

			// criado por mim -------------------------------------------
			verificar_teclas_pressionadas(ev, &espaco, &seta_direita, &seta_esquerda, 1);
			//-----------------------------------------------------------
		}
		// criado por mim ---------------------------------------------------------
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) { // tecla solta = 0
			verificar_teclas_pressionadas(ev, &espaco, &seta_direita, &seta_esquerda, 0);
		}

	} //fim do while
     
	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	
 
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_destroy_font(font);
	// criado por mim ----------------------------------------------
	// Libera a memória da imagem
	al_destroy_bitmap(imagem_cidade);
	al_destroy_bitmap(imagem_kelly_keystone);
	// -------------------------------------------------------------
 
	return 0;
}