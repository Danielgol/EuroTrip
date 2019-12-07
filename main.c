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

    //Não sei por fullscreen :V..........
    //VARIAVEIS
    const int SCREEN_W = 1920;
    const int SCREEN_H = 1080;

    //VARIAVEIS ALLEGRO
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;

    //ADDONS
    al_init();
    al_init_image_addon();
    al_install_mouse();

    //CRIACAO ALLEGRO
    display = al_create_display(SCREEN_W, SCREEN_H);
    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_mouse_event_source());

    //Variaveis do programa
    int pos_x = 0, pos_y = 0;

    //IMPORTAR IMAGENS
    ALLEGRO_BITMAP *image = al_load_bitmap("image/t1.png");
    ALLEGRO_BITMAP *botaoSobreNos = al_load_bitmap("image/Botao05.png");
    ALLEGRO_BITMAP *botaoSobreNos2 = al_load_bitmap("image/Botao06.png");
    ALLEGRO_BITMAP *botaoViajeAgora = al_load_bitmap("image/Botao01.png");
    ALLEGRO_BITMAP *botaoViajeAgora2 = al_load_bitmap("image/Botao02.png");
    ALLEGRO_BITMAP *botaoPacotesTour = al_load_bitmap("image/Botao03.png");
    ALLEGRO_BITMAP *botaoPacotesTour2= al_load_bitmap("image/Botao04.png");

    //Programa------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    while(1){
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_MOUSE_AXES){
                pos_x = ev.mouse.x;
                pos_y = ev.mouse.y;
        }

        if(al_is_event_queue_empty(event_queue)){
            al_draw_scaled_bitmap(image, 0, 0,  al_get_bitmap_width(image), al_get_bitmap_height(image), 0, 0, al_get_bitmap_width(image), al_get_bitmap_height(image), 0);

            if(pos_x>=1700 && pos_x<= 1900 && pos_y>=30 && pos_y<=63){
                al_draw_scaled_bitmap(botaoSobreNos2, 0, 0, al_get_bitmap_width(botaoSobreNos2), al_get_bitmap_height(botaoSobreNos2), 1700, 30, al_get_bitmap_width(botaoSobreNos2), al_get_bitmap_height(botaoSobreNos2), 0);

            }else{
                al_draw_scaled_bitmap(botaoSobreNos, 0, 0, al_get_bitmap_width(botaoSobreNos), al_get_bitmap_height(botaoSobreNos), 1700, 30, al_get_bitmap_width(botaoSobreNos), al_get_bitmap_height(botaoSobreNos), 0);

            }

            if(pos_x>=495 && pos_x<= 914 && pos_y>=530 && pos_y<= 630){
                al_draw_scaled_bitmap(botaoViajeAgora2, 0, 0,  al_get_bitmap_width(botaoViajeAgora2), al_get_bitmap_height(botaoViajeAgora2), 495, 530, al_get_bitmap_width(botaoViajeAgora2), al_get_bitmap_height(botaoViajeAgora2), 0);

            }else{
                al_draw_scaled_bitmap(botaoViajeAgora, 0, 0,  al_get_bitmap_width(botaoViajeAgora), al_get_bitmap_height(botaoViajeAgora), 495, 530, al_get_bitmap_width(botaoViajeAgora), al_get_bitmap_height(botaoViajeAgora), 0);

            }

            if(pos_x>=1000 && pos_x<=1419 && pos_y>=530 && pos_y<= 630){
                al_draw_scaled_bitmap(botaoPacotesTour2, 0, 0,  al_get_bitmap_width(botaoPacotesTour2), al_get_bitmap_height(botaoPacotesTour2), 1000, 530, al_get_bitmap_width(botaoPacotesTour2), al_get_bitmap_height(botaoPacotesTour2), 0);

            }else{
                al_draw_scaled_bitmap(botaoPacotesTour, 0, 0,  al_get_bitmap_width(botaoPacotesTour), al_get_bitmap_height(botaoPacotesTour), 1000, 530, al_get_bitmap_width(botaoPacotesTour), al_get_bitmap_height(botaoPacotesTour), 0);

            }
        }

        al_flip_display();

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
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return 0;
}
