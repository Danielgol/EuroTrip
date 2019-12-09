#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include "allegro5/allegro_image.h"
#include <allegro5/allegro_native_dialog.h>

typedef struct{
    //No
    int index;//Id do no
    int status; //0-Aberto; 1-Fechado;
    int estimativa; //Custo do no escolhido ate este
    struct Out* saidas; //Nos que saem desse no
    struct Node* precedente; //No que precede em um caminho
    //Grafo
    struct Node* prox;//Proximo na lista de nos (geral)
}Node;

typedef struct{
    int distancia;
    struct Node* node;
    struct Out* prox;
}Out;

typedef struct{
    struct Node* node;
    struct Out* prox;
}List;


//COLOCAR UM METODO Q ZERE AS ESTIMATIVAS (PARA OUTRAS CONSULTAS)

void imprimirSaidas(Out* atual){
    if(atual == NULL){
    }else{
        Node* node = atual->node;
        printf("(%i)", node->index);
        imprimirSaidas(atual->prox);
    }
}

void imprimir(Node* atual){
    if(atual == NULL){
        printf("NULL");
    }else{
        printf("%i(E:%i) ", atual->index, atual->estimativa);
        if(atual->saidas != NULL){
            //imprimirSaidas(atual->saidas);
        }
        printf(" => ");
        imprimir(atual->prox);

    }
}

void imprimirCaminho(Node* atual){
    if(atual == NULL){
        printf("CAMINHO");
    }else{
        imprimirCaminho(atual->precedente);
        printf(" => %i", atual->index);
    }
}

Node* buscarNode(Node* atual, int index){
    if(atual == NULL){
        return NULL;
    }

    if(atual->index == index){
        return atual;
    }else{
        return buscarNode(atual->prox, index);
    }
}

Node* inserir(Node* atual, int index){

    if(atual == NULL){
        Node* novo = malloc(sizeof(Node));
        novo->estimativa = 1000;
        novo->index = index;
        novo->saidas = NULL;
        novo->precedente = NULL;
        novo->prox = NULL;
        novo->status = 0;
        return novo;
    }

    Node* busca = buscarNode(atual, index);
    if(busca != NULL){
        return atual;
    }

    atual->prox = inserir(atual->prox, index);
    return atual;

}

Out* inserirNasSaidas(Out* atual, Node* node, int distancia){

    if(atual == NULL){
        Out* novo = malloc(sizeof(Out));
        novo->node = node;
        novo->prox = NULL;
        novo->distancia = distancia;
        return novo;
    }

    if(atual->distancia < distancia){
        Out* novo = malloc(sizeof(Out));
        novo->node = node;
        novo->prox = atual;
        novo->distancia = distancia;
        return novo;
    }else{
        atual->prox = inserirNasSaidas(atual->prox, node, distancia);
        return atual;
    }

}

void ligarNodes(Node* grafo, int index1, int index2, int distancia){
    Node* node1 = buscarNode(grafo, index1);
    Node* node2 = buscarNode(grafo, index2);
    if(node1 != NULL && node2 != NULL){
        node1->saidas = inserirNasSaidas(node1->saidas, node2, distancia);
    }
}

Node* buscarNodeMenorEstimativa(Node* atual, int estimativa){
    if(atual != NULL){
        if(atual->estimativa < estimativa && atual->status == 0){
            Node* resultado = buscarNodeMenorEstimativa(atual->prox, atual->estimativa);
            if(resultado != NULL){
                if(resultado->estimativa < atual->estimativa){
                    return resultado;
                }else{
                    return atual;
                }
            }else{
                return atual;
            }
        }
        return buscarNodeMenorEstimativa(atual->prox, estimativa);
    }
    return NULL;
}

void dijkstra(Node* grafo){
    Node* nodeMenorEstimativa = buscarNodeMenorEstimativa(grafo, 1000);
    if(nodeMenorEstimativa != NULL){
        nodeMenorEstimativa->status = 1;
        estimarSaidas(nodeMenorEstimativa->saidas, nodeMenorEstimativa);
        dijkstra(grafo);
    }
}

void estimarNode(Node* node, Node* anterior, int distancia){
    if(anterior->estimativa + distancia < node->estimativa){ //estimativa atual + distancia do proximo < estimativa proximo
        node->estimativa = anterior->estimativa + distancia;
        node->precedente = anterior;
    }
}

void estimarSaidas(Out* atual, Node* anterior){
    if(atual != NULL){
        Node* node = atual->node;
        estimarNode(node, anterior, atual->distancia);
        estimarSaidas(atual->prox, anterior);
    }
}

Node* encontrarCaminho(Node* grafo, int indexInicio, int indexFim){
    Node* inicio = buscarNode(grafo, indexInicio);
    inicio->estimativa = 0;
    dijkstra(grafo);
    return buscarNode(grafo, indexFim);
}




int main(int argc, char **argv){
    /*
    Node* grafo = NULL;

    grafo = inserir(grafo, 0);
    grafo = inserir(grafo, 1);
    grafo = inserir(grafo, 2);
    grafo = inserir(grafo, 3);
    grafo = inserir(grafo, 4);
    grafo = inserir(grafo, 5);
    grafo = inserir(grafo, 6);

    ligarNodes(grafo, 0, 1, 4);
    ligarNodes(grafo, 0, 2, 6);
    ligarNodes(grafo, 0, 3, 9);
    ligarNodes(grafo, 1, 5, 3);
    ligarNodes(grafo, 1, 4, 2);
    ligarNodes(grafo, 4, 5, 2);
    ligarNodes(grafo, 2, 5, 2);
    ligarNodes(grafo, 2, 3, 4);
    ligarNodes(grafo, 4, 6, 7);
    ligarNodes(grafo, 5, 6, 3);
    ligarNodes(grafo, 3, 6, 5);

    ------------------------------------------------------------------------

    grafo = inserir(grafo, 0);
    grafo = inserir(grafo, 1);
    grafo = inserir(grafo, 2);
    grafo = inserir(grafo, 3);
    grafo = inserir(grafo, 4);

    ligarNodes(grafo, 0, 1, 5);
    ligarNodes(grafo, 0, 2, 10);
    ligarNodes(grafo, 1, 2, 3);
    ligarNodes(grafo, 1, 3, 2);
    ligarNodes(grafo, 1, 4, 9);
    ligarNodes(grafo, 2, 1, 3);
    ligarNodes(grafo, 2, 4, 1);
    ligarNodes(grafo, 3, 0, 7);
    ligarNodes(grafo, 3, 4, 6);
    ligarNodes(grafo, 4, 3, 4);


    ------------------------------------------------------------------------------
    imprimir(grafo);
    printf("\n\n");

    Node* caminho = encontrarCaminho(grafo, 0, 6);
    imprimir(grafo);
    printf("\n\n");

    imprimirCaminho(caminho);
    */

    //COMANDOS DO ALLEGRO-----------------------------------------------------------------------------------------------------------------------------------------------------------
    //VARIAVEIS ALLEGRO
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_DISPLAY_MODE disp_data;

    //ADDONS--------------------------------------------------
    al_init();
    al_init_image_addon();
    al_install_mouse();

    //CRIACAO ALLEGRO-------------------------------------------------------
    al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);
    al_set_new_display_flags(ALLEGRO_FULLSCREEN);
    display = al_create_display(disp_data.width, disp_data.height);
    int SCREEN_W = al_get_display_width(display);
    int SCREEN_H = al_get_display_height(display);

    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_flip_display();

    //IMPORTAR IMAGENS---------------------------------------------------------
    //Menu inicial
    ALLEGRO_BITMAP *image = al_load_bitmap("image/MenuInicial/t1.png");
    ALLEGRO_BITMAP *botaoSobreNos = al_load_bitmap("image/MenuInicial/Botao05.png");
    ALLEGRO_BITMAP *botaoSobreNos2 = al_load_bitmap("image/MenuInicial/Botao06.png");
    ALLEGRO_BITMAP *botaoViajeAgora = al_load_bitmap("image/MenuInicial/Botao01.png");
    ALLEGRO_BITMAP *botaoViajeAgora2 = al_load_bitmap("image/MenuInicial/Botao02.png");
    ALLEGRO_BITMAP *botaoPacotesTour = al_load_bitmap("image/MenuInicial/Botao03.png");
    ALLEGRO_BITMAP *botaoPacotesTour2= al_load_bitmap("image/MenuInicial/Botao04.png");
    ALLEGRO_BITMAP *botaoSair= al_load_bitmap("image/MenuInicial/Botao07.png");
    ALLEGRO_BITMAP *botaoSair2= al_load_bitmap("image/MenuInicial/Botao08.png");

    //MenuViajeAgora
    ALLEGRO_BITMAP *background = al_load_bitmap("image/MenuViaje/wallpaper.png");
    ALLEGRO_BITMAP *btvoltar = al_load_bitmap("image/MenuViaje/voltar.png");
    ALLEGRO_BITMAP *btvoltar2 = al_load_bitmap("image/MenuViaje/voltar2.png");
    ALLEGRO_BITMAP *btviagem01 = al_load_bitmap("image/MenuViaje/viagem01.png");
    ALLEGRO_BITMAP *btviagem02= al_load_bitmap("image/MenuViaje/viagem02.png");
    ALLEGRO_BITMAP *btviagem03 = al_load_bitmap("image/MenuViaje/viagem03.png");
    ALLEGRO_BITMAP *btviagem04= al_load_bitmap("image/MenuViaje/viagem04.png");
    ALLEGRO_BITMAP *btviagem05 = al_load_bitmap("image/MenuViaje/viagem05.png");
    ALLEGRO_BITMAP *btviagem06= al_load_bitmap("image/MenuViaje/viagem06.png");
    ALLEGRO_BITMAP *btviagem07 = al_load_bitmap("image/MenuViaje/viagem07.png");
    ALLEGRO_BITMAP *btviagem08= al_load_bitmap("image/MenuViaje/viagem08.png");
    ALLEGRO_BITMAP *btviagem09 = al_load_bitmap("image/MenuViaje/viagem09.png");
    ALLEGRO_BITMAP *btviagem10= al_load_bitmap("image/MenuViaje/viagem10.png");
    ALLEGRO_BITMAP *btviagem11= al_load_bitmap("image/MenuViaje/viagem11.png");
    ALLEGRO_BITMAP *btviagem12= al_load_bitmap("image/MenuViaje/viagem12.png");
    ALLEGRO_BITMAP *btviagem13= al_load_bitmap("image/MenuViaje/viagem13.png");
    ALLEGRO_BITMAP *btviagem14= al_load_bitmap("image/MenuViaje/viagem14.png");
    ALLEGRO_BITMAP *btviagem15= al_load_bitmap("image/MenuViaje/viagem15.png");
    ALLEGRO_BITMAP *btviagem16= al_load_bitmap("image/MenuViaje/viagem16.png");
    ALLEGRO_BITMAP *btviagem17= al_load_bitmap("image/MenuViaje/viagem17.png");
    ALLEGRO_BITMAP *btviagem18= al_load_bitmap("image/MenuViaje/viagem18.png");
    ALLEGRO_BITMAP *btviagem19= al_load_bitmap("image/MenuViaje/viagem19.png");
    ALLEGRO_BITMAP *btviagem20= al_load_bitmap("image/MenuViaje/viagem20.png");
    ALLEGRO_BITMAP *btviagem21= al_load_bitmap("image/MenuViaje/viagem21.png");
    ALLEGRO_BITMAP *btviagem22= al_load_bitmap("image/MenuViaje/viagem22.png");
    ALLEGRO_BITMAP *btviagem23= al_load_bitmap("image/MenuViaje/viagem23.png");
    ALLEGRO_BITMAP *btviagem24= al_load_bitmap("image/MenuViaje/viagem24.png");

    //MenuPacotesTour


    //Variaveis do programa-----------------------------------------------------
    int pos_x = 0, pos_y = 0;//Coordenada do mouse
    int fechar=0;//fechar programa
    int viajar=0;//Menu do short path
    int pacote=0;//Menu do click


    //Programa------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    while(1){
        while(fechar == 0){
            ALLEGRO_EVENT ev;
            al_wait_for_event(event_queue, &ev);

            if(ev.type == ALLEGRO_EVENT_MOUSE_AXES){
                    pos_x = ev.mouse.x;
                    pos_y = ev.mouse.y;
            }else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                fechar = 1;
                break;
            }else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if(pos_x>=SCREEN_W-(al_get_bitmap_width(botaoSair))-60 && pos_x<= SCREEN_W-(al_get_bitmap_width(botaoSair))-60+al_get_bitmap_width(botaoSair) && pos_y>=SCREEN_H-(al_get_bitmap_height(botaoSair))-40 && pos_y<=SCREEN_H-(al_get_bitmap_height(botaoSair))-40+al_get_bitmap_height(botaoSair)){
                    fechar = 1;
                    break;
                }else if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(botaoViajeAgora))-43 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(botaoViajeAgora))-43+al_get_bitmap_width(botaoViajeAgora) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(botaoViajeAgora)*0,9)-1 && pos_y <= (SCREEN_H/2-(al_get_bitmap_height(botaoViajeAgora)*0,9)-1)+al_get_bitmap_height(botaoViajeAgora)){
                    viajar=1;
                    break;
                }else if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(botaoPacotesTour)*0,9)+31 && pos_x<=SCREEN_W/2+(al_get_bitmap_width(botaoPacotesTour)*0,9)+31+al_get_bitmap_width(botaoPacotesTour) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(botaoPacotesTour)*0,9)-1 && pos_y<= SCREEN_H/2-(al_get_bitmap_height(botaoPacotesTour)*0,9)-1+al_get_bitmap_height(botaoPacotesTour)){
                    pacote=1;
                    break;
                }
            }

            if(!pacote && !viajar && al_is_event_queue_empty(event_queue)){
                al_draw_scaled_bitmap(image, 0, 0,  al_get_bitmap_width(image), al_get_bitmap_height(image), 0, 0, SCREEN_W, SCREEN_H, 0);

                if(pos_x>=SCREEN_W-(al_get_bitmap_width(botaoSobreNos))-25 && pos_x<= SCREEN_W-(al_get_bitmap_width(botaoSobreNos))-25+al_get_bitmap_width(botaoSobreNos) && pos_y>=SCREEN_H/15+(al_get_bitmap_height(botaoSobreNos)-80) && pos_y<=SCREEN_H/15+(al_get_bitmap_height(botaoSobreNos)-80)+al_get_bitmap_height(botaoSobreNos)){
                    al_draw_scaled_bitmap(botaoSobreNos2, 0, 0, al_get_bitmap_width(botaoSobreNos2), al_get_bitmap_height(botaoSobreNos2), SCREEN_W-(al_get_bitmap_width(botaoSobreNos2))-25, SCREEN_H/15+(al_get_bitmap_height(botaoSobreNos2))-80, al_get_bitmap_width(botaoSobreNos2), al_get_bitmap_height(botaoSobreNos2), 0);
                }else{
                    al_draw_scaled_bitmap(botaoSobreNos, 0, 0, al_get_bitmap_width(botaoSobreNos), al_get_bitmap_height(botaoSobreNos), SCREEN_W-(al_get_bitmap_width(botaoSobreNos))-25, SCREEN_H/15+(al_get_bitmap_height(botaoSobreNos))-80, al_get_bitmap_width(botaoSobreNos), al_get_bitmap_height(botaoSobreNos), 0);
                }

                if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(botaoViajeAgora))-43 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(botaoViajeAgora))-43+al_get_bitmap_width(botaoViajeAgora) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(botaoViajeAgora)*0,9)-1 && pos_y <= (SCREEN_H/2-(al_get_bitmap_height(botaoViajeAgora)*0,9)-1)+al_get_bitmap_height(botaoViajeAgora)){
                    al_draw_scaled_bitmap(botaoViajeAgora2, 0, 0,  al_get_bitmap_width(botaoViajeAgora2), al_get_bitmap_height(botaoViajeAgora2), SCREEN_W/2-(al_get_bitmap_width(botaoViajeAgora2))-43, SCREEN_H/2-(al_get_bitmap_height(botaoViajeAgora2)*0,9)-1, al_get_bitmap_width(botaoViajeAgora2), al_get_bitmap_height(botaoViajeAgora2), 0);
                }else{
                    al_draw_scaled_bitmap(botaoViajeAgora, 0, 0,  al_get_bitmap_width(botaoViajeAgora), al_get_bitmap_height(botaoViajeAgora), SCREEN_W/2-(al_get_bitmap_width(botaoViajeAgora))-43, SCREEN_H/2-(al_get_bitmap_height(botaoViajeAgora)*0,9)-1, al_get_bitmap_width(botaoViajeAgora), al_get_bitmap_height(botaoViajeAgora), 0);
                }

                if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(botaoPacotesTour)*0,9)+31 && pos_x<=SCREEN_W/2+(al_get_bitmap_width(botaoPacotesTour)*0,9)+31+al_get_bitmap_width(botaoPacotesTour) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(botaoPacotesTour)*0,9)-1 && pos_y<= SCREEN_H/2-(al_get_bitmap_height(botaoPacotesTour)*0,9)-1+al_get_bitmap_height(botaoPacotesTour)){
                    al_draw_scaled_bitmap(botaoPacotesTour2, 0, 0,  al_get_bitmap_width(botaoPacotesTour2), al_get_bitmap_height(botaoPacotesTour2), SCREEN_W/2+(al_get_bitmap_width(botaoPacotesTour)*0,9)+31, SCREEN_H/2-(al_get_bitmap_height(botaoPacotesTour)*0,9)-1, al_get_bitmap_width(botaoPacotesTour2), al_get_bitmap_height(botaoPacotesTour2), 0);
                }else{
                    al_draw_scaled_bitmap(botaoPacotesTour, 0, 0,  al_get_bitmap_width(botaoPacotesTour), al_get_bitmap_height(botaoPacotesTour), SCREEN_W/2+(al_get_bitmap_width(botaoPacotesTour)*0,9)+31, SCREEN_H/2-(al_get_bitmap_height(botaoPacotesTour)*0,9)-1, al_get_bitmap_width(botaoPacotesTour), al_get_bitmap_height(botaoPacotesTour), 0);
                }

                if(pos_x>=SCREEN_W-(al_get_bitmap_width(botaoSair))-60 && pos_x<= SCREEN_W-(al_get_bitmap_width(botaoSair))-60+al_get_bitmap_width(botaoSair) && pos_y>=SCREEN_H-(al_get_bitmap_height(botaoSair))-40 && pos_y<=SCREEN_H-(al_get_bitmap_height(botaoSair))-40+al_get_bitmap_height(botaoSair)){
                    al_draw_scaled_bitmap(botaoSair2, 0, 0, al_get_bitmap_width(botaoSair2), al_get_bitmap_height(botaoSair2), SCREEN_W-(al_get_bitmap_width(botaoSair2)+60), SCREEN_H-(al_get_bitmap_height(botaoSair2))-40, al_get_bitmap_width(botaoSair2), al_get_bitmap_height(botaoSair2), 0);
                }else{
                    al_draw_scaled_bitmap(botaoSair, 0, 0, al_get_bitmap_width(botaoSair), al_get_bitmap_height(botaoSair), SCREEN_W-(al_get_bitmap_width(botaoSair)+60), SCREEN_H-(al_get_bitmap_height(botaoSair))-40, al_get_bitmap_width(botaoSair), al_get_bitmap_height(botaoSair), 0);
                }
            }

            al_flip_display();
        }

        if(fechar){
            break;
        }

        //Short path--------------------------------------------------------------------------------------------------------------------------------------------------------------------
        while(viajar){
            //Colocoar depois mais condições, botões tipo (voltar) e wallpaper
            //COLOQUEI PRA ABRIR O MESMO WALLPAPER DE PROPOSITO, AINDA N PREPAREI O NOVO PARA POR AQUI..... PARA N FICAR IGUAL SÓ TIREI OS BOTÕES PRA SABER SE TA PEGANDO OU N.

            ALLEGRO_EVENT ev2;
            al_wait_for_event(event_queue, &ev2);

            if(ev2.type == ALLEGRO_EVENT_MOUSE_AXES){
                    pos_x = ev2.mouse.x;
                    pos_y = ev2.mouse.y;
            }else if(ev2.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar)+60)+al_get_bitmap_width(btvoltar) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar)){
                    viajar=0;
                    break;
                }
            }

            if(al_is_event_queue_empty(event_queue)){
                al_draw_scaled_bitmap(background, 0, 0,  al_get_bitmap_width(background), al_get_bitmap_height(background), 0, 0, SCREEN_W, SCREEN_H, 0);

                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar2)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar2)+60)+al_get_bitmap_width(btvoltar2) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar2)){
                    al_draw_scaled_bitmap(btvoltar, 0, 0, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), SCREEN_W-(al_get_bitmap_width(btvoltar)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar))-90, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), 0);
                }else{
                    al_draw_scaled_bitmap(btvoltar2, 0, 0, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), SCREEN_W-(al_get_bitmap_width(btvoltar2)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), 0);
                }

                //ALEMANHA------------------
                if(pos_x>=SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10) && pos_x<= SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10)+al_get_bitmap_width(btviagem01) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                    al_draw_scaled_bitmap(btviagem02, 0, 0, al_get_bitmap_width(btviagem02), al_get_bitmap_height(btviagem02), SCREEN_W/3-(al_get_bitmap_width(btviagem02)+10), SCREEN_H/5+(al_get_bitmap_height(btviagem02)*0,9)-90, al_get_bitmap_width(btviagem02), al_get_bitmap_height(btviagem02), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem01, 0, 0, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10), SCREEN_H/5+(al_get_bitmap_height(btviagem01)*0,9)-90, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), 0);
                }

                //INGLATERRA-------------------
                if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90+al_get_bitmap_height(btviagem03)){
                    al_draw_scaled_bitmap(btviagem04, 0, 0, al_get_bitmap_width(btviagem04), al_get_bitmap_height(btviagem04), SCREEN_W/2-(al_get_bitmap_width(btviagem04))+60, SCREEN_H/5+(al_get_bitmap_height(btviagem04)*0,9)-90, al_get_bitmap_width(btviagem04), al_get_bitmap_height(btviagem04), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem03, 0, 0, al_get_bitmap_width(btviagem03), al_get_bitmap_height(btviagem03), SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60, SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90, al_get_bitmap_width(btviagem03), al_get_bitmap_height(btviagem03), 0);
                }

                //FRANÇA-------------------
                if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65 && pos_x<= SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90+al_get_bitmap_height(btviagem03)){
                    al_draw_scaled_bitmap(btviagem06, 0, 0, al_get_bitmap_width(btviagem06), al_get_bitmap_height(btviagem06), SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65, SCREEN_H/5+(al_get_bitmap_height(btviagem06)*0,9)-90, al_get_bitmap_width(btviagem06), al_get_bitmap_height(btviagem06), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem05, 0, 0, al_get_bitmap_width(btviagem05), al_get_bitmap_height(btviagem05), SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65, SCREEN_H/5+(al_get_bitmap_height(btviagem05)*0,9)-90, al_get_bitmap_width(btviagem05), al_get_bitmap_height(btviagem05), 0);
                }

                //SUIÇA-------------------
                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3) && pos_x<= SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3)+al_get_bitmap_width(btviagem07) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem07)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem07)*0,9)-90+al_get_bitmap_height(btviagem07)){
                    al_draw_scaled_bitmap(btviagem08, 0, 0, al_get_bitmap_width(btviagem08), al_get_bitmap_height(btviagem08), SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3), SCREEN_H/5+(al_get_bitmap_height(btviagem08)*0,9)-90, al_get_bitmap_width(btviagem08), al_get_bitmap_height(btviagem08), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem07, 0, 0, al_get_bitmap_width(btviagem07), al_get_bitmap_height(btviagem07), SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3), SCREEN_H/5+(al_get_bitmap_height(btviagem07)*0,9)-90, al_get_bitmap_width(btviagem07), al_get_bitmap_height(btviagem07), 0);
                }

                //REPUBLICA CHECA
                if(pos_x>=SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10) && pos_x<= SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10)+al_get_bitmap_width(btviagem01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                    al_draw_scaled_bitmap(btviagem10, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/3-(al_get_bitmap_width(btviagem10)+10), SCREEN_H/2+(al_get_bitmap_height(btviagem10)*0,9)-90, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem09, 0, 0, al_get_bitmap_width(btviagem09), al_get_bitmap_height(btviagem09), SCREEN_W/3-(al_get_bitmap_width(btviagem09)+10), SCREEN_H/2+(al_get_bitmap_height(btviagem09)*0,9)-90, al_get_bitmap_width(btviagem09), al_get_bitmap_height(btviagem09), 0);
                }

                //ITALIA-------------------
                if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                    al_draw_scaled_bitmap(btviagem12, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem04))+60, SCREEN_H/2+(al_get_bitmap_height(btviagem10)*0,9)-90, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem11, 0, 0, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), SCREEN_W/2-(al_get_bitmap_width(btviagem04))+60, SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), 0);
                }

                //Monaco-------------------
                if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65 && pos_x<= SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                    al_draw_scaled_bitmap(btviagem14, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65, SCREEN_H/2+(al_get_bitmap_height(btviagem10)*0,9)-90, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem13, 0, 0, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65, SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), 0);
                }

                //AUSTRIA-------------------
                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3) && pos_x<= SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3)+al_get_bitmap_width(btviagem07) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                    al_draw_scaled_bitmap(btviagem16, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3), SCREEN_H/2+(al_get_bitmap_height(btviagem10)*0,9)-90, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem15, 0, 0, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3), SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), 0);
                }

                //BELGICA-------------------
                if(pos_x>=SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10) && pos_x<= SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10)+al_get_bitmap_width(btviagem01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                    al_draw_scaled_bitmap(btviagem18, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10), SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem17, 0, 0, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10), SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), 0);
                }

                //CROACIA-------------------
                if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                    al_draw_scaled_bitmap(btviagem20, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60, SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem19, 0, 0, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60, SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), 0);
                }

                //gRECIA-------------------
                if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65 && pos_x<= SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                    al_draw_scaled_bitmap(btviagem22, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65, SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem21, 0, 0, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65, SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), 0);
                }

                //HOLANDA-------------------
                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3) && pos_x<= SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3)+al_get_bitmap_width(btviagem07) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                    al_draw_scaled_bitmap(btviagem24, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3), SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), 0);
                }else{
                    al_draw_scaled_bitmap(btviagem23, 0, 0, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3), SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90, al_get_bitmap_width(btviagem01), al_get_bitmap_height(btviagem01), 0);
                }

            }

            al_flip_display();
        }

        //Clique-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        while(pacote){

            //Colocoar depois mais condições, botões tipo (voltar) e wallpaper
            //COLOQUEI PRA ABRIR O MESMO WALLPAPER DE PROPOSITO, AINDA N PREPAREI O NOVO PARA POR AQUI..... PARA N FICAR IGUAL SÓ TIREI OS BOTÕES PRA SABER SE TA PEGANDO OU N.

            ALLEGRO_EVENT ev3;
            al_wait_for_event(event_queue, &ev3);

            if(ev3.type == ALLEGRO_EVENT_MOUSE_AXES){
                    pos_x = ev3.mouse.x;
                    pos_y = ev3.mouse.y;
            }else if(ev3.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar)+60)+al_get_bitmap_width(btvoltar) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar)){
                    pacote=0;
                    break;
                }

            }

            if(al_is_event_queue_empty(event_queue)){
                al_draw_scaled_bitmap(image, 0, 0,  al_get_bitmap_width(image), al_get_bitmap_height(image), 0, 0, SCREEN_W, SCREEN_H, 0);

                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar2)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar2)+60)+al_get_bitmap_width(btvoltar2) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar2)){
                    al_draw_scaled_bitmap(btvoltar, 0, 0, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), SCREEN_W-(al_get_bitmap_width(btvoltar)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar))-90, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), 0);
                }else{
                    al_draw_scaled_bitmap(btvoltar2, 0, 0, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), SCREEN_W-(al_get_bitmap_width(btvoltar2)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), 0);
                }
            }

            al_flip_display();
        }
    }
    //Fim do programa-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

    //DESTRUIR
    al_destroy_bitmap(image);
    al_destroy_bitmap(botaoViajeAgora);
    al_destroy_bitmap(botaoViajeAgora2);
    al_destroy_bitmap(botaoSobreNos);
    al_destroy_bitmap(botaoSobreNos2);
    al_destroy_bitmap(botaoPacotesTour);
    al_destroy_bitmap(botaoPacotesTour2);
    al_destroy_bitmap(botaoSair);
    al_destroy_bitmap(botaoSair2);


    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return 0;
}
