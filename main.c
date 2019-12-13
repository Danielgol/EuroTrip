#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include "allegro5/allegro_image.h"
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>



typedef struct{
    //No
    int index;//Id do no
    int status; //0-Aberto; 1-Fechado; (No clique, serve para indicar se eh candidato ao clique)
    int estimativa; //Custo do no escolhido ate este (No clique, serve para salvar o grau)
    struct Out* saidas; //Nos que saem desse no
    struct Node* precedente; //No que precede em um caminho (Serve para unir o clique)
    //Grafo
    struct Node* prox;//Proximo na lista de nos (geral)
}Node;

typedef struct{
    int distancia;
    struct Node* node;
    struct Out* prox;
}Out;


void zerarNodes(Node* grafo){
    if(grafo != NULL){
        grafo->estimativa = 1000;
        grafo->precedente = NULL;
        grafo->status = 0;
        zerarNodes(grafo->prox);
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
        node2->saidas = inserirNasSaidas(node2->saidas, node1, distancia);//MAO DUPLA
    }
}



void fecharNodes(Out* atual){
    if(atual != NULL){
        Node* node = atual->node;
        node->status = 1;
        fecharNodes(atual->prox);
    }
}

int calcularGrau(Out* saida, Node* selecionado, int soma){
    if(saida != NULL){
        Node* node = saida->node;
        if(node->status == 1 && node != selecionado){
            soma += 1;
        }
        return calcularGrau(saida->prox, selecionado, soma);
    }
    return soma;
}

void estimarSubgrafo(Out* atual, Node* selecionado, int tamanho){
    if(atual != NULL){
        Node* node = atual->node;
        Out* saidas = node->saidas;
        node->estimativa = calcularGrau(node->saidas, selecionado, 0);
        if(node->estimativa < tamanho-2){
            node->status = 0;
            node->estimativa = 0;
        }
        estimarSubgrafo(atual->prox, selecionado, tamanho);
    }
}

int verificarConexoes(Out* lista_saidas, Out* lista_node){

    int soma = 0;
    Out* aux_ls = lista_saidas;

    while(lista_node != NULL){
        Node* atual_node = lista_node->node;

        while(aux_ls != NULL){
            Node* atual_saidas = aux_ls->node;
            if(atual_node == atual_saidas && atual_node->status == 1 && atual_saidas->status == 1){
                soma += 1;
            }
            aux_ls = aux_ls->prox;
        }

        aux_ls = lista_saidas;
        lista_node = lista_node->prox;
    }

    return soma;
}

void separarClique(Out* atual, int tamanho){
    if(atual != NULL){
        Node* node = atual->node;
        Out* saidas = node->saidas;

        if(node->status == 1){
            int conexoes = 0;

            while(saidas != NULL){
                Node* saida = saidas->node;
                if(saida->status == 1){
                    conexoes += verificarConexoes(node->saidas, saida->saidas);
                }
                if(saida == node){
                    conexoes += 1;
                }
                saidas = saidas->prox;
            }

            conexoes += 2;
            conexoes /= 2;

            if(conexoes < tamanho-2){
                node->status = 0;
                node->estimativa = 0;
            }else{
                node->status = 1;
                node->estimativa = conexoes;
            }
        }

        separarClique(atual->prox, tamanho);
    }
}

Node* juntarClique(Out* atual){
    if(atual != NULL){
        Node* node = atual->node;
        if(node->status == 1){
            node->precedente = juntarClique(atual->prox);
            return node;
        }else{
            return juntarClique(atual->prox);
        }
    }
    return NULL;
}

Node* acharClique(Node* selecionado, int tamanho){
    fecharNodes(selecionado->saidas);
    estimarSubgrafo(selecionado->saidas, selecionado, tamanho);
    selecionado->status = 0;
    separarClique(selecionado->saidas, tamanho);
    selecionado->precedente = juntarClique(selecionado->saidas);
    return selecionado;
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
    if(anterior->estimativa + distancia < node->estimativa){
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

void imprimir_pontos(Node* caminho, int centroX, int centroY, int SCREEN_H, ALLEGRO_BITMAP* red){
    if(caminho != NULL){

        int index = caminho->index;
        if(index == 1){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-477, centroY+294, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 2){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-400, centroY+370, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 3){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-328 , centroY+255, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 4){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-190, centroY+265, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 5){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-95 , centroY+135, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 6){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-135 , centroY+28, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 7){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-165 , centroY-90, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 8){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-210, centroY-170, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 9){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-73 , centroY-52, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 10){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-5 , centroY+84, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 11){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-40 , centroY+205, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 12){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX+35 , centroY+160, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 13){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX+106 , centroY+276, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 14){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX+10 , centroY-20, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 15){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX-37, centroY-120, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 16){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX+80 , centroY+50, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 17){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX+148 , centroY-3, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 18){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX+129, centroY-110, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 19){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX+210 , centroY+45, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 20){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX+210 , centroY+145, SCREEN_H/20, SCREEN_H/20, 0);
        }
        if(index == 21){
            al_draw_scaled_bitmap(red, 0, 0,  al_get_bitmap_width(red), al_get_bitmap_height(red), centroX+410 , centroY+390, SCREEN_H/20, SCREEN_H/20, 0);
        }
        imprimir_pontos(caminho->precedente, centroX, centroY, SCREEN_H, red);
    }
}




int main(int argc, char **argv){


    Node* grafo = NULL;

    grafo = inserir(grafo, 1);
    grafo = inserir(grafo, 2);
    grafo = inserir(grafo, 3);
    grafo = inserir(grafo, 4);
    grafo = inserir(grafo, 5);
    grafo = inserir(grafo, 6);
    grafo = inserir(grafo, 7);
    grafo = inserir(grafo, 8);
    grafo = inserir(grafo, 9);
    grafo = inserir(grafo, 10);
    grafo = inserir(grafo, 11);
    grafo = inserir(grafo, 12);
    grafo = inserir(grafo, 13);
    grafo = inserir(grafo, 14);
    grafo = inserir(grafo, 15);
    grafo = inserir(grafo, 16);
    grafo = inserir(grafo, 17);
    grafo = inserir(grafo, 18);
    grafo = inserir(grafo, 19);
    grafo = inserir(grafo, 20);
    grafo = inserir(grafo, 21);

    ligarNodes(grafo, 1, 2, 8);
    ligarNodes(grafo, 1, 3, 10);
    ligarNodes(grafo, 2, 3, 8);
    ligarNodes(grafo, 2, 4, 14);
    ligarNodes(grafo, 3, 4, 10);
    ligarNodes(grafo, 4, 5, 12);
    ligarNodes(grafo, 4, 6, 16);
    ligarNodes(grafo, 5, 6, 10);
    ligarNodes(grafo, 5, 10, 6);
    ligarNodes(grafo, 5, 11, 6);
    ligarNodes(grafo, 6, 7, 8);
    ligarNodes(grafo, 6, 9, 6);
    ligarNodes(grafo, 7, 8, 6);
    ligarNodes(grafo, 9, 14, 6);
    ligarNodes(grafo, 9, 15, 4);
    ligarNodes(grafo, 10, 12, 6);
    ligarNodes(grafo, 10, 14, 6);
    ligarNodes(grafo, 10, 16, 4);
    ligarNodes(grafo, 11, 12, 6);
    ligarNodes(grafo, 12, 13, 8);
    ligarNodes(grafo, 12, 16, 6);
    ligarNodes(grafo, 14, 16, 6);
    ligarNodes(grafo, 14, 18, 8);
    ligarNodes(grafo, 15, 18, 10);
    ligarNodes(grafo, 16, 17, 6);
    ligarNodes(grafo, 16, 18, 10);
    ligarNodes(grafo, 16, 19, 8);
    ligarNodes(grafo, 16, 20, 9);
    ligarNodes(grafo, 17, 18, 6);
    ligarNodes(grafo, 17, 19, 6);
    ligarNodes(grafo, 18, 19, 10);
    ligarNodes(grafo, 19, 20, 8);
    ligarNodes(grafo, 20, 21, 22);


    Node* clique = NULL;
    Node* caminho = NULL;


    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_DISPLAY_MODE disp_data;

    al_init();
    al_init_image_addon();
    al_install_mouse();
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(20);

    al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);
    al_set_new_display_flags(ALLEGRO_FULLSCREEN);
    display = al_create_display(disp_data.width, disp_data.height);
    int SCREEN_W = al_get_display_width(display);
    int SCREEN_H = al_get_display_height(display);

    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_flip_display();

    //Menu inicial
    ALLEGRO_BITMAP *image = al_load_bitmap("image/MenuInicial/t1.png");
    ALLEGRO_BITMAP *image2 = al_load_bitmap("image/MenuInicial/wallpaperSobreNos.png");
    ALLEGRO_BITMAP *botaoSobreNos = al_load_bitmap("image/MenuInicial/Botao05.png");
    ALLEGRO_BITMAP *botaoSobreNos2 = al_load_bitmap("image/MenuInicial/Botao06.png");
    ALLEGRO_BITMAP *botaoViajeAgora = al_load_bitmap("image/MenuInicial/Botao01.png");
    ALLEGRO_BITMAP *botaoViajeAgora2 = al_load_bitmap("image/MenuInicial/Botao02.png");
    ALLEGRO_BITMAP *botaoPacotesTour = al_load_bitmap("image/MenuInicial/Botao03.png");
    ALLEGRO_BITMAP *botaoPacotesTour2= al_load_bitmap("image/MenuInicial/Botao04.png");
    ALLEGRO_BITMAP *botaoSair= al_load_bitmap("image/MenuInicial/Botao07.png");
    ALLEGRO_BITMAP *botaoSair2= al_load_bitmap("image/MenuInicial/Botao08.png");

    //MenuViajeAgora
    ALLEGRO_BITMAP *wallpaper03= al_load_bitmap("image/MenuViaje/CidadeChegada.png");
    ALLEGRO_BITMAP *wallpaper01= al_load_bitmap("image/MenuViaje/CidadePartida.png");
    ALLEGRO_BITMAP *wallpaper02 = al_load_bitmap("image/MenuViaje/PaisChegada.png");
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
    ALLEGRO_BITMAP *btviagem25= al_load_bitmap("image/MenuViaje/viagem25.png");
    ALLEGRO_BITMAP *btviagem26= al_load_bitmap("image/MenuViaje/viagem26.png");
    ALLEGRO_BITMAP *btviagem27= al_load_bitmap("image/MenuViaje/viagem27.png");
    ALLEGRO_BITMAP *btviagem28= al_load_bitmap("image/MenuViaje/viagem28.png");

    //Cidades
    ALLEGRO_BITMAP *btalemanha01 = al_load_bitmap("image/MenuViaje/alemanha01.png");
    ALLEGRO_BITMAP *btalemanha02 = al_load_bitmap("image/MenuViaje/alemanha02.png");
    ALLEGRO_BITMAP *btalemanha03 = al_load_bitmap("image/MenuViaje/alemanha03.png");
    ALLEGRO_BITMAP *btaustria = al_load_bitmap("image/MenuViaje/austria.png");
    ALLEGRO_BITMAP *btbelgica = al_load_bitmap("image/MenuViaje/belgica.png");
    ALLEGRO_BITMAP *btcroacia = al_load_bitmap("image/MenuViaje/croacia.png");
    ALLEGRO_BITMAP *btespanha01 = al_load_bitmap("image/MenuViaje/espanha01.png");
    ALLEGRO_BITMAP *btespanha02 = al_load_bitmap("image/MenuViaje/espanha02.png");
    ALLEGRO_BITMAP *btespanha03 = al_load_bitmap("image/MenuViaje/espanha03.png");
    ALLEGRO_BITMAP *btfranca01 = al_load_bitmap("image/MenuViaje/franca01.png");
    ALLEGRO_BITMAP *btfranca02 = al_load_bitmap("image/MenuViaje/franca02.png");
    ALLEGRO_BITMAP *btgrecia = al_load_bitmap("image/MenuViaje/grecia.png");
    ALLEGRO_BITMAP *btholanda = al_load_bitmap("image/MenuViaje/holanda.png");
    ALLEGRO_BITMAP *btinglaterra01 = al_load_bitmap("image/MenuViaje/inglaterra01.png");
    ALLEGRO_BITMAP *btinglaterra02 = al_load_bitmap("image/MenuViaje/inglaterra02.png");
    ALLEGRO_BITMAP *btitalia01 = al_load_bitmap("image/MenuViaje/italia01.png");
    ALLEGRO_BITMAP *btitalia02 = al_load_bitmap("image/MenuViaje/italia02.png");
    ALLEGRO_BITMAP *btmonaco = al_load_bitmap("image/MenuViaje/monaco.png");
    ALLEGRO_BITMAP *btportugal = al_load_bitmap("image/MenuViaje/portugal.png");
    ALLEGRO_BITMAP *btcheca = al_load_bitmap("image/MenuViaje/checa.png");
    ALLEGRO_BITMAP *btsuica = al_load_bitmap("image/MenuViaje/suica.png");
    //MenuPacotesTour

    //Mapa Europa
    ALLEGRO_BITMAP *mapa_europa = al_load_bitmap("image/MapaEuropa.png");
    ALLEGRO_BITMAP *red = al_load_bitmap("image/red.png");
    ALLEGRO_BITMAP *white = al_load_bitmap("image/white.png");

    //Sons
    ALLEGRO_SAMPLE *button;
    ALLEGRO_SAMPLE_INSTANCE *inst_button;
    button = al_load_sample("sounds/button.ogg");
    inst_button = al_create_sample_instance(button);
    al_attach_sample_instance_to_mixer(inst_button,al_get_default_mixer());
    al_set_sample_instance_gain(inst_button,1.0);

    ALLEGRO_SAMPLE *menu_sound1;
    ALLEGRO_SAMPLE_INSTANCE *inst_menu_sound1;
    menu_sound1 = al_load_sample("sounds/menu.ogg");
    inst_menu_sound1 = al_create_sample_instance(menu_sound1);
    al_set_sample_instance_gain(inst_menu_sound1, 0.3);
    al_set_sample_instance_playmode(inst_menu_sound1, ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(inst_menu_sound1, al_get_default_mixer());

    ALLEGRO_SAMPLE *exit_button;
    ALLEGRO_SAMPLE_INSTANCE *inst_exit_button;
    exit_button = al_load_sample("sounds/close_door_1.ogg");
    inst_exit_button = al_create_sample_instance(exit_button);
    al_attach_sample_instance_to_mixer(inst_exit_button,al_get_default_mixer());
    al_set_sample_instance_gain(inst_exit_button, 1.0);

    ALLEGRO_SAMPLE *bus_sound;
    ALLEGRO_SAMPLE_INSTANCE *inst_bus_sound;
    bus_sound = al_load_sample("sounds/porta_abrindo.ogg");
    inst_bus_sound = al_create_sample_instance(bus_sound);
    al_attach_sample_instance_to_mixer(inst_bus_sound,al_get_default_mixer());
    al_set_sample_instance_gain(inst_bus_sound, 1.0);

    //Variaveis do programa
    int pos_x = 0, pos_y = 0;//Coordenada do mouse
    int fechar=0;//fechar programa
    int viajar=0;//Menu do short path
    int pacote=0;//Menu do click
    int sobreNos = 0;//tela sobre nos
    int ponto_partida = 0;
    int ponto_chegada = 0;
    int estagio = 0;
    int cidade = -1;
    int nivel = 0;
    //int partida = 0;



    //Programa
    while(1){

        while(fechar == 0){
            ALLEGRO_EVENT ev;
            al_wait_for_event(event_queue, &ev);

            al_play_sample_instance(inst_menu_sound1);

            if(ev.type == ALLEGRO_EVENT_MOUSE_AXES){
                pos_x = ev.mouse.x;
                pos_y = ev.mouse.y;
            }else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                fechar = 1;
                break;
            }else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if(sobreNos==0){
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(botaoSair))-60 && pos_x<= SCREEN_W-(al_get_bitmap_width(botaoSair))-60+al_get_bitmap_width(botaoSair) && pos_y>=SCREEN_H-(al_get_bitmap_height(botaoSair))-40 && pos_y<=SCREEN_H-(al_get_bitmap_height(botaoSair))-40+al_get_bitmap_height(botaoSair)){
                        al_play_sample_instance(inst_exit_button);
                        fechar = 1;
                        break;
                    }else if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(botaoViajeAgora))-43 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(botaoViajeAgora))-43+al_get_bitmap_width(botaoViajeAgora) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(botaoViajeAgora)*0,9)-1 && pos_y <= (SCREEN_H/2-(al_get_bitmap_height(botaoViajeAgora)*0,9)-1)+al_get_bitmap_height(botaoViajeAgora)){
                        al_play_sample_instance(inst_button);
                        viajar=1;
                        break;
                    }else if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(botaoPacotesTour)*0,9)+31 && pos_x<=SCREEN_W/2+(al_get_bitmap_width(botaoPacotesTour)*0,9)+31+al_get_bitmap_width(botaoPacotesTour) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(botaoPacotesTour)*0,9)-1 && pos_y<= SCREEN_H/2-(al_get_bitmap_height(botaoPacotesTour)*0,9)-1+al_get_bitmap_height(botaoPacotesTour)){
                        al_play_sample_instance(inst_button);
                        pacote=1;
                        break;
                    }else if(pos_x>=SCREEN_W-(al_get_bitmap_width(botaoSobreNos))-25 && pos_x<= SCREEN_W-(al_get_bitmap_width(botaoSobreNos))-25+al_get_bitmap_width(botaoSobreNos) && pos_y>=SCREEN_H/15+(al_get_bitmap_height(botaoSobreNos)-80) && pos_y<=SCREEN_H/15+(al_get_bitmap_height(botaoSobreNos)-80)+al_get_bitmap_height(botaoSobreNos)){
                        al_play_sample_instance(inst_button);
                        sobreNos = 1;
                    }
                }else if(sobreNos==1){
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar2)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar2)+60)+al_get_bitmap_width(btvoltar2) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar2)){
                        al_play_sample_instance(inst_button);
                        sobreNos = 0;
                    }
                }
            }

            if(!pacote && !viajar && al_is_event_queue_empty(event_queue)){
                if(sobreNos==0){
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
                }else{
                    al_draw_scaled_bitmap(image2, 0, 0,  al_get_bitmap_width(image), al_get_bitmap_height(image), 0, 0, SCREEN_W, SCREEN_H, 0);

                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar2)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar2)+60)+al_get_bitmap_width(btvoltar2) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar2)){
                        al_draw_scaled_bitmap(btvoltar, 0, 0, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), SCREEN_W-(al_get_bitmap_width(btvoltar)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar))-90, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), 0);

                    }else{
                        al_draw_scaled_bitmap(btvoltar2, 0, 0, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), SCREEN_W-(al_get_bitmap_width(btvoltar2)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), 0);
                    }
                }
            }

            al_flip_display();
        }


        if(fechar){
            break;
        }


        //Short path
        while(viajar){
            //Cidade representa o pais com suas cidades = 0(alemanha), 1(austria), 2(belgica), 3(checa), 4(croacia), 5(espanha), 6(franca), 7(grecia), 8(holanda), 9(inglaterra), 10(italia), 11(monaco), 12(portugal), 13(suica)
            ALLEGRO_EVENT ev2;
            al_wait_for_event(event_queue, &ev2);

            if(ev2.type == ALLEGRO_EVENT_MOUSE_AXES){
                pos_x = ev2.mouse.x;
                pos_y = ev2.mouse.y;
            }else if(ev2.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if(estagio == 0 || estagio == 2){
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar)+60)+al_get_bitmap_width(btvoltar) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar)){
                        al_play_sample_instance(inst_button);
                        viajar=0;
                        ponto_partida = 0;
                        estagio = 0;
                        cidade = -1;
                        break;
                    }
                    //clicar na alemanha
                    if(pos_x>=SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10) && pos_x<= SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10)+al_get_bitmap_width(btviagem01) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 0;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }

                    }
                    //clicar na inglaterra
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90+al_get_bitmap_height(btviagem03)){
                        cidade = 9;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar na franca
                    if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65 && pos_x<= SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90+al_get_bitmap_height(btviagem03)){
                        cidade = 6;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar na suica
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3) && pos_x<= SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3)+al_get_bitmap_width(btviagem07) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem07)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem07)*0,9)-90+al_get_bitmap_height(btviagem07)){
                        cidade = 13;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar na checa
                    if(pos_x>=SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10) && pos_x<= SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10)+al_get_bitmap_width(btviagem01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 3;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar italia
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 10;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar monaco
                    if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65 && pos_x<= SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 11;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3) && pos_x<= SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3)+al_get_bitmap_width(btviagem07) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 1;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar belgica
                    if(pos_x>=SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10) && pos_x<= SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10)+al_get_bitmap_width(btviagem01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 2;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar croacia
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 4;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar grecia
                    if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65 && pos_x<= SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 7;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar holanda
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3) && pos_x<= SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3)+al_get_bitmap_width(btviagem07) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 8;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar espanha
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2) && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)+al_get_bitmap_width(btviagem10)*0.8 && pos_y>= SCREEN_H/2+(al_get_bitmap_height(btviagem01)) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01))+al_get_bitmap_height(btviagem10)*0.8){
                        cidade = 5;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                    //clicar portugal
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2) && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)+al_get_bitmap_width(btviagem10)*0.8 && pos_y>= SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5)+al_get_bitmap_height(btviagem10)*0.8){
                        cidade = 12;
                        al_play_sample_instance(inst_button);
                        if(estagio == 0){
                            estagio = 1;
                        }else if(estagio == 2){
                            estagio = 3;
                        }
                    }
                }else if(estagio == 1 || estagio == 3){
                    if(cidade == 0){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR BERLIM COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 18;
                                estagio = 2;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR FRANKFURT COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 14;
                                estagio = 2;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR MUNIQUE COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 16;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR BERLIM COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 18;
                                viajar = 0;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR FRANKFURT COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 14;
                                viajar = 0;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR MUNIQUE COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 16;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 1){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR VIENA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 19;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR VIENA COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 19;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 2){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR BRUXELAS COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 9;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR BRUXELAS COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 9;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 3){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR PRAGA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 17;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR PRAGA COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 17;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 4){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR ZAGREBE COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 20;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR ZAGREBE COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 20;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 5){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR SEVILLA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 2;
                                estagio = 2;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR MADRI COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 3;
                                estagio = 2;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR BARCELONA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 4;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR SEVILLA COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 2;
                                viajar = 0;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR MADRI COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 3;
                                viajar = 0;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR BARCELONA COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 4;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 6){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR PARIS COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 6;
                                estagio = 2;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR LYON COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 5;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR PARIS COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 6;
                                viajar = 0;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR LYON COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 5;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 7){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR ATENAS COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 21;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR ATENAS COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 21;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 8){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR AMSTERDAN COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 15;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR AMSTERDAN COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 15;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 9){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR LONDRES COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 7;
                                estagio = 2;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR LIVERPOLL COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 8;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR LONDRES COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 7;
                                viajar = 0;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR LIVERPOLL COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 8;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 10){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR MILAO COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 12;
                                estagio = 2;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR ROMA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 13;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR MILAO COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 12;
                                viajar = 0;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR ROMA COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 13;
                                viajar = 0;
                            }
                        }

                    }else if(cidade == 11){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR MONACO COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 11;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR MONACO COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 11;
                                viajar = 0;
                            }
                        }
                    }else if(cidade == 12){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR LISBOA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 1;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR LISBOA COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 1;
                                viajar = 0;
                            }
                        }
                    }else if(cidade == 13){
                        if(estagio == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR BERNA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 10;
                                estagio = 2;
                            }
                        }else if(estagio == 3){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR BERNA COMO PONTO DE CHEGADA
                                al_play_sample_instance(inst_button);
                                ponto_chegada = 10;
                                viajar = 0;
                            }
                        }
                    }
                }
            }

            if(ponto_chegada != 0 && ponto_partida != 0){
                caminho = encontrarCaminho(grafo, ponto_partida, ponto_chegada);
                viajar = 0;
                estagio = 0;
                cidade = -1;
                break;
            }

            if(al_is_event_queue_empty(event_queue)){
                if(estagio == 0 || estagio == 2){
                    if(estagio == 0){
                        al_draw_scaled_bitmap(background, 0, 0,  al_get_bitmap_width(background), al_get_bitmap_height(background), 0, 0, SCREEN_W, SCREEN_H, 0);
                    }else if(estagio == 2){
                        al_draw_scaled_bitmap(wallpaper02, 0, 0,  al_get_bitmap_width(wallpaper02), al_get_bitmap_height(wallpaper02), 0, 0, SCREEN_W, SCREEN_H, 0);
                    }

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

                    //GRECIA-------------------
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

                    //PORTUGAL------------------
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2) && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)+al_get_bitmap_width(btviagem10)*0.8 && pos_y>= SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5)+al_get_bitmap_height(btviagem10)*0.8){
                        al_draw_scaled_bitmap(btviagem26, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)-10, SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5), al_get_bitmap_width(btviagem10)*0.8, al_get_bitmap_height(btviagem10)*0.8, 0);
                    }else{
                        al_draw_scaled_bitmap(btviagem25, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)-10, SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5), al_get_bitmap_width(btviagem10)*0.8, al_get_bitmap_height(btviagem10)*0.8, 0);
                    }

                    //ESPANHA-----------------
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2) && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)+al_get_bitmap_width(btviagem10)*0.8 && pos_y>= SCREEN_H/2+(al_get_bitmap_height(btviagem01)) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01))+al_get_bitmap_height(btviagem10)*0.8){
                        al_draw_scaled_bitmap(btviagem28, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)-10, SCREEN_H/2+(al_get_bitmap_height(btviagem01)), al_get_bitmap_width(btviagem10)*0.8, al_get_bitmap_height(btviagem10)*0.8, 0);
                    }else{
                        al_draw_scaled_bitmap(btviagem27, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)-10, SCREEN_H/2+(al_get_bitmap_height(btviagem01)), al_get_bitmap_width(btviagem10)*0.8, al_get_bitmap_height(btviagem10)*0.8, 0);
                    }

                }else if(estagio == 1 || estagio == 3){
                    if(estagio == 1){
                        al_draw_scaled_bitmap(wallpaper01, 0, 0,  al_get_bitmap_width(wallpaper01), al_get_bitmap_height(wallpaper01), 0, 0, SCREEN_W, SCREEN_H, 0);
                    }else if(estagio == 3){
                        al_draw_scaled_bitmap(wallpaper03, 0, 0,  al_get_bitmap_width(wallpaper03), al_get_bitmap_height(wallpaper03), 0, 0, SCREEN_W, SCREEN_H, 0);
                    }

                    //alemanha
                    if(cidade==0){
                        //berlim
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btalemanha01, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btalemanha01, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                        //frankfurt
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btalemanha02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btalemanha02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                        //munique
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btalemanha03, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btalemanha03, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //austria
                    else if(cidade==1){
                        //viena
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btaustria, 0, 0, al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), SCREEN_W/2-(al_get_bitmap_width(btaustria)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btaustria, 0, 0, al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), SCREEN_W/2-(al_get_bitmap_width(btaustria)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //belgica
                    else if(cidade==2){
                        //bruxelas
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btbelgica, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btbelgica, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //checa
                    else if(cidade==3){
                        //praga
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btcheca, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btcheca, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //croacia
                    else if(cidade==4){
                        //zagrebe
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btcroacia, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btcroacia, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //espanha
                    else if(cidade==5){
                        //sevilla
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btespanha01, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btespanha01, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                        //madri
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btespanha02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btespanha02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                        //barcelona
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btespanha03, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btespanha03, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //franca
                    else if(cidade==6){
                        //paris
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btfranca01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btfranca01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                        //lyon
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btfranca02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btfranca02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //grecia
                    else if(cidade==7){
                        //atenas
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btgrecia, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btgrecia, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //holanda
                    else if(cidade==8){
                        //amsterdan
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btholanda, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btholanda, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //inglaterra
                    else if(cidade==9){
                        //londres
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btinglaterra01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btinglaterra01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                        //liverpoll
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btinglaterra02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btinglaterra02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //italia
                    else if(cidade==10){
                        //milao
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btitalia01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btitalia01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                        //roma
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btitalia02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btitalia02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //monaco
                    else if(cidade==11){
                        //monaco
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btmonaco, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btmonaco, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //portugal
                    else if(cidade==12){
                        //lisboa
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btportugal, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btportugal, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //suica
                    else if(cidade==13){
                        //berna
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btsuica, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btsuica, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                }

            }

            al_flip_display();
        }



        //Mapa Shortest Path
        while(ponto_chegada != 0 && ponto_partida != 0){

            ALLEGRO_EVENT ev3;
            al_wait_for_event(event_queue, &ev3);

            if(ev3.type == ALLEGRO_EVENT_MOUSE_AXES){
                pos_x = ev3.mouse.x;
                pos_y = ev3.mouse.y;
            }else if(ev3.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar)+60)+al_get_bitmap_width(btvoltar) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar)){
                    al_play_sample_instance(inst_button);
                    ponto_chegada = 0;
                    ponto_partida = 0;
                    zerarNodes(grafo);
                    break;
                }
            }

            if(al_is_event_queue_empty(event_queue)){

                int centroX = ((SCREEN_W-SCREEN_H)+SCREEN_H/2);
                int centroY = SCREEN_H/2;

                al_draw_scaled_bitmap(white, 0, 0,  al_get_bitmap_width(white), al_get_bitmap_height(white), (SCREEN_W-SCREEN_H), 0, SCREEN_H, SCREEN_H, 0);
                imprimir_pontos(caminho, centroX, centroY, SCREEN_H, red);
                al_draw_scaled_bitmap(mapa_europa, 0, 0,  al_get_bitmap_width(mapa_europa), al_get_bitmap_height(mapa_europa), (SCREEN_W-SCREEN_H), 0, SCREEN_H, SCREEN_H, 0);

                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar2)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar2)+60)+al_get_bitmap_width(btvoltar2) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar2)){
                    al_draw_scaled_bitmap(btvoltar, 0, 0, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), SCREEN_W-(al_get_bitmap_width(btvoltar)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar))-90, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), 0);
                }else{
                    al_draw_scaled_bitmap(btvoltar2, 0, 0, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), SCREEN_W-(al_get_bitmap_width(btvoltar2)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), 0);
                }
            }

            al_flip_display();
        }



        //Clique
        while(pacote){

            //Colocoar depois mais condições, botões tipo (voltar) e wallpaper
            //COLOQUEI PRA ABRIR O MESMO WALLPAPER DE PROPOSITO, AINDA N PREPAREI O NOVO PARA POR AQUI..... PARA N FICAR IGUAL SÓ TIREI OS BOTÕES PRA SABER SE TA PEGANDO OU N.

            ALLEGRO_EVENT ev4;
            al_wait_for_event(event_queue, &ev4);

            if(ev4.type == ALLEGRO_EVENT_MOUSE_AXES){
                pos_x = ev4.mouse.x;
                pos_y = ev4.mouse.y;
            }else if(ev4.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if(nivel == 0 ){
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar)+60)+al_get_bitmap_width(btvoltar) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar)){
                        al_play_sample_instance(inst_button);
                        pacote=0;
                        break;
                    }
                    //clicar na alemanha
                    if(pos_x>=SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10) && pos_x<= SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10)+al_get_bitmap_width(btviagem01) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 0;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar na inglaterra
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90+al_get_bitmap_height(btviagem03)){
                        cidade = 9;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }

                    }
                    //clicar na franca
                    if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65 && pos_x<= SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem03)*0,9)-90+al_get_bitmap_height(btviagem03)){
                        cidade = 6;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar na suica
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3) && pos_x<= SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3)+al_get_bitmap_width(btviagem07) && pos_y>=SCREEN_H/5+(al_get_bitmap_height(btviagem07)*0,9)-90 && pos_y<=SCREEN_H/5+(al_get_bitmap_height(btviagem07)*0,9)-90+al_get_bitmap_height(btviagem07)){
                        cidade = 13;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar na checa
                    if(pos_x>=SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10) && pos_x<= SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10)+al_get_bitmap_width(btviagem01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 3;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar italia
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 10;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar monaco
                    if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65 && pos_x<= SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 11;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3) && pos_x<= SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3)+al_get_bitmap_width(btviagem07) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*0,9)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 1;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar belgica
                    if(pos_x>=SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10) && pos_x<= SCREEN_W/3-(al_get_bitmap_width(btviagem01)+10)+al_get_bitmap_width(btviagem01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 2;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar croacia
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60 && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem03))+60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 4;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar grecia
                    if(pos_x>=SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-65 && pos_x<= SCREEN_W/2+(al_get_bitmap_width(btviagem05)*0.8)-60+al_get_bitmap_width(btviagem03) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 7;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar holanda
                    if(pos_x>=SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3) && pos_x<= SCREEN_W-(al_get_bitmap_width(btviagem07)*1.3)+al_get_bitmap_width(btviagem07) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90 && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01)*1.2)-90+al_get_bitmap_height(btviagem01)){
                        cidade = 8;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar espanha
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2) && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)+al_get_bitmap_width(btviagem10)*0.8 && pos_y>= SCREEN_H/2+(al_get_bitmap_height(btviagem01)) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01))+al_get_bitmap_height(btviagem10)*0.8){
                        cidade = 5;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                    //clicar portugal
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2) && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)+al_get_bitmap_width(btviagem10)*0.8 && pos_y>= SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5)+al_get_bitmap_height(btviagem10)*0.8){
                        cidade = 12;
                        al_play_sample_instance(inst_button);
                        if(nivel == 0){
                            nivel = 1;
                        }
                    }
                }else if(nivel == 1 ){
                    if(cidade == 0){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR BERLIM COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 18;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR FRANKFURT COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 14;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR MUNIQUE COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 16;
                            }
                        }

                    }else if(cidade == 1){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR VIENA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 19;
                            }
                        }

                    }else if(cidade == 2){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR BRUXELAS COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 9;
                            }
                        }

                    }else if(cidade == 3){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR PRAGA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 17;
                            }
                        }

                    }else if(cidade == 4){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR ZAGREBE COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 20;
                            }
                        }

                    }else if(cidade == 5){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR SEVILLA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 2;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR MADRI COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 3;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR BARCELONA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 4;
                            }
                        }

                    }else if(cidade == 6){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR PARIS COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 6;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR LYON COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 5;
                            }
                        }

                    }else if(cidade == 7){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR ATENAS COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 21;
                            }
                        }

                    }else if(cidade == 8){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR AMSTERDAN COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 15;
                            }
                        }

                    }else if(cidade == 9){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR LONDRES COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 7;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR LIVERPOLL COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 8;
                            }
                        }

                    }else if(cidade == 10){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR MILAO COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 12;
                            }else if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                                //ADICIONAR ROMA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 13;
                            }
                        }

                    }else if(cidade == 11){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR MONACO COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 11;
                            }
                        }
                    }else if(cidade == 12){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR LISBOA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 1;
                            }
                        }
                    }else if(cidade == 13){
                        if(nivel == 1){
                            if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                                //ADICIONAR BERNA COMO PONTO DE PARTIDA
                                al_play_sample_instance(inst_button);
                                ponto_partida = 10;
                            }
                        }
                    }
                }
            }

            if(ponto_partida != 0){
                clique = acharClique(buscarNode(grafo, ponto_partida), 3);
                nivel = 0;
                cidade = -1;
                break;
            }


            if(al_is_event_queue_empty(event_queue)){
                if(nivel == 0 ){
                    if(nivel == 0){
                        al_draw_scaled_bitmap(background, 0, 0,  al_get_bitmap_width(background), al_get_bitmap_height(background), 0, 0, SCREEN_W, SCREEN_H, 0);
                    }

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

                    //GRECIA-------------------
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

                    //PORTUGAL------------------
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2) && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)+al_get_bitmap_width(btviagem10)*0.8 && pos_y>= SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5)+al_get_bitmap_height(btviagem10)*0.8){
                        al_draw_scaled_bitmap(btviagem26, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)-10, SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5), al_get_bitmap_width(btviagem10)*0.8, al_get_bitmap_height(btviagem10)*0.8, 0);
                    }else{
                        al_draw_scaled_bitmap(btviagem25, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)-10, SCREEN_H/2-(al_get_bitmap_height(btviagem01)*1.5), al_get_bitmap_width(btviagem10)*0.8, al_get_bitmap_height(btviagem10)*0.8, 0);
                    }

                    //ESPANHA-----------------
                    if(pos_x>=SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2) && pos_x<= SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)+al_get_bitmap_width(btviagem10)*0.8 && pos_y>= SCREEN_H/2+(al_get_bitmap_height(btviagem01)) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btviagem01))+al_get_bitmap_height(btviagem10)*0.8){
                        al_draw_scaled_bitmap(btviagem28, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)-10, SCREEN_H/2+(al_get_bitmap_height(btviagem01)), al_get_bitmap_width(btviagem10)*0.8, al_get_bitmap_height(btviagem10)*0.8, 0);
                    }else{
                        al_draw_scaled_bitmap(btviagem27, 0, 0, al_get_bitmap_width(btviagem10), al_get_bitmap_height(btviagem10), SCREEN_W/2-(al_get_bitmap_width(btviagem07)*3.2)-10, SCREEN_H/2+(al_get_bitmap_height(btviagem01)), al_get_bitmap_width(btviagem10)*0.8, al_get_bitmap_height(btviagem10)*0.8, 0);
                    }

                }else if(nivel == 1 ){
                    if(nivel == 1){
                        al_draw_scaled_bitmap(wallpaper01, 0, 0,  al_get_bitmap_width(wallpaper01), al_get_bitmap_height(wallpaper01), 0, 0, SCREEN_W, SCREEN_H, 0);
                    }

                    //alemanha
                    if(cidade==0){
                        //berlim
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btalemanha01, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btalemanha01, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                        //frankfurt
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btalemanha02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btalemanha02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                        //munique
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btalemanha03, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btalemanha03, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //austria
                    else if(cidade==1){
                        //viena
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btaustria, 0, 0, al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), SCREEN_W/2-(al_get_bitmap_width(btaustria)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btaustria, 0, 0, al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), SCREEN_W/2-(al_get_bitmap_width(btaustria)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //belgica
                    else if(cidade==2){
                        //bruxelas
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btbelgica, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btbelgica, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //checa
                    else if(cidade==3){
                        //praga
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btcheca, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btcheca, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //croacia
                    else if(cidade==4){
                        //zagrebe
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btcroacia, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btcroacia, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //espanha
                    else if(cidade==5){
                        //sevilla
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btespanha01, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btespanha01, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*1.5), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                        //madri
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btespanha02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btespanha02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2-(al_get_bitmap_height(btalemanha01)*0.3), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                        //barcelona
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btespanha03, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btespanha03, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.9), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //franca
                    else if(cidade==6){
                        //paris
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btfranca01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btfranca01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                        //lyon
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btfranca02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btfranca02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //grecia
                    else if(cidade==7){
                        //atenas
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btgrecia, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btgrecia, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //holanda
                    else if(cidade==8){
                        //amsterdan
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btholanda, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btholanda, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //inglaterra
                    else if(cidade==9){
                        //londres
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btinglaterra01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btinglaterra01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                        //liverpoll
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btinglaterra02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btinglaterra02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //italia
                    else if(cidade==10){
                        //milao
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btitalia01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btitalia01, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                        //roma
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2) && pos_y<=SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.5)+al_get_bitmap_height(btalemanha01)){
                            al_draw_scaled_bitmap(btitalia02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), 0);
                        }else{
                            al_draw_scaled_bitmap(btitalia02, 0, 0, al_get_bitmap_width(btalemanha01), al_get_bitmap_height(btalemanha01), SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4), SCREEN_H/2+(al_get_bitmap_height(btalemanha01)*0.2), al_get_bitmap_width(btalemanha01)*0.99, al_get_bitmap_height(btalemanha01)*0.99, 0);
                        }
                    }
                    //monaco
                    else if(cidade==11){
                        //monaco
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btmonaco, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btmonaco, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //portugal
                    else if(cidade==12){
                        //lisboa
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btportugal, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btportugal, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                    //suica
                    else if(cidade==13){
                        //berna
                        if(pos_x>= SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4) && pos_x<=SCREEN_W/2-(al_get_bitmap_width(btalemanha01)*0.4)+al_get_bitmap_width(btalemanha01) && pos_y>=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2) && pos_y<=SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2)+al_get_bitmap_height(btaustria)){
                            al_draw_scaled_bitmap(btsuica, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria), al_get_bitmap_height(btaustria), 0);
                        }else{
                            al_draw_scaled_bitmap(btsuica, 0, 0, al_get_bitmap_width(btbelgica), al_get_bitmap_height(btbelgica), SCREEN_W/2-(al_get_bitmap_width(btbelgica)*0.4), SCREEN_H/2-(al_get_bitmap_height(btaustria)*1.2), al_get_bitmap_width(btaustria)*0.99, al_get_bitmap_height(btaustria)*0.99, 0);
                        }
                    }
                }
            }

            al_flip_display();
        }



        //Mapa Clique
        while(ponto_partida != 0 && pacote == 1){

            ALLEGRO_EVENT ev3;
            al_wait_for_event(event_queue, &ev3);

            if(ev3.type == ALLEGRO_EVENT_MOUSE_AXES){
                pos_x = ev3.mouse.x;
                pos_y = ev3.mouse.y;
            }else if(ev3.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar)+60)+al_get_bitmap_width(btvoltar) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar)){
                    al_play_sample_instance(inst_button);
                    ponto_partida = 0;
                    pacote = 0;
                    zerarNodes(grafo);
                    break;
                }
            }

            if(al_is_event_queue_empty(event_queue)){

                int centroX = ((SCREEN_W-SCREEN_H)+SCREEN_H/2);
                int centroY = SCREEN_H/2;

                al_draw_scaled_bitmap(white, 0, 0,  al_get_bitmap_width(white), al_get_bitmap_height(white), (SCREEN_W-SCREEN_H), 0, SCREEN_H, SCREEN_H, 0);
                imprimir_pontos(clique, centroX, centroY, SCREEN_H, red);
                al_draw_scaled_bitmap(mapa_europa, 0, 0,  al_get_bitmap_width(mapa_europa), al_get_bitmap_height(mapa_europa), (SCREEN_W-SCREEN_H), 0, SCREEN_H, SCREEN_H, 0);

                if(pos_x>=SCREEN_W-(al_get_bitmap_width(btvoltar2)+60) && pos_x<= SCREEN_W-(al_get_bitmap_width(btvoltar2)+60)+al_get_bitmap_width(btvoltar2) && pos_y>=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90 && pos_y<=SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90+al_get_bitmap_height(btvoltar2)){
                    al_draw_scaled_bitmap(btvoltar, 0, 0, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), SCREEN_W-(al_get_bitmap_width(btvoltar)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar))-90, al_get_bitmap_width(btvoltar), al_get_bitmap_height(btvoltar), 0);
                }else{
                    al_draw_scaled_bitmap(btvoltar2, 0, 0, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), SCREEN_W-(al_get_bitmap_width(btvoltar2)+60), SCREEN_H/30+(al_get_bitmap_height(btvoltar2))-90, al_get_bitmap_width(btvoltar2), al_get_bitmap_height(btvoltar2), 0);
                }
            }

            al_flip_display();
        }

    }

    //Fim do programa
    al_destroy_bitmap(image);
    al_destroy_bitmap(image2);
    al_destroy_bitmap(botaoViajeAgora);
    al_destroy_bitmap(botaoViajeAgora2);
    al_destroy_bitmap(botaoSobreNos);
    al_destroy_bitmap(botaoSobreNos2);
    al_destroy_bitmap(botaoPacotesTour);
    al_destroy_bitmap(botaoPacotesTour2);
    al_destroy_bitmap(botaoSair);
    al_destroy_bitmap(botaoSair2);

    al_destroy_sample(button);
    al_destroy_sample(menu_sound1);
    al_destroy_sample(exit_button);
    al_destroy_sample(bus_sound);
    al_destroy_sample_instance(inst_button);
    al_destroy_sample_instance(inst_menu_sound1);
    al_destroy_sample_instance(inst_exit_button);
    al_destroy_sample_instance(inst_bus_sound);

    al_destroy_bitmap(wallpaper03);
    al_destroy_bitmap(wallpaper01);
    al_destroy_bitmap(wallpaper02);
    al_destroy_bitmap(btvoltar);
    al_destroy_bitmap(btvoltar2);
    al_destroy_bitmap(btviagem01);
    al_destroy_bitmap(btviagem02);
    al_destroy_bitmap(btviagem03);
    al_destroy_bitmap(btviagem04);
    al_destroy_bitmap(btviagem05);
    al_destroy_bitmap(btviagem06);
    al_destroy_bitmap(btviagem07);
    al_destroy_bitmap(btviagem08);
    al_destroy_bitmap(btviagem09);
    al_destroy_bitmap(btviagem10);
    al_destroy_bitmap(btviagem11);
    al_destroy_bitmap(btviagem12);
    al_destroy_bitmap(btviagem13);
    al_destroy_bitmap(btviagem14);
    al_destroy_bitmap(btviagem15);
    al_destroy_bitmap(btviagem16);
    al_destroy_bitmap(btviagem17);
    al_destroy_bitmap(btviagem18);
    al_destroy_bitmap(btviagem19);
    al_destroy_bitmap(btviagem20);
    al_destroy_bitmap(btviagem21);
    al_destroy_bitmap(btviagem22);
    al_destroy_bitmap(btviagem23);
    al_destroy_bitmap(btviagem24);
    al_destroy_bitmap(btviagem25);
    al_destroy_bitmap(btviagem26);
    al_destroy_bitmap(btviagem27);
    al_destroy_bitmap(btviagem28);

    al_destroy_bitmap(btalemanha01);
    al_destroy_bitmap(btalemanha02);
    al_destroy_bitmap(btalemanha03);
    al_destroy_bitmap(btaustria);
    al_destroy_bitmap(btbelgica);
    al_destroy_bitmap(btcroacia);
    al_destroy_bitmap(btespanha01);
    al_destroy_bitmap(btespanha02);
    al_destroy_bitmap(btespanha03);
    al_destroy_bitmap(btgrecia);
    al_destroy_bitmap(btholanda);
    al_destroy_bitmap(btinglaterra01);
    al_destroy_bitmap(btinglaterra02);
    al_destroy_bitmap(btitalia01);
    al_destroy_bitmap(btitalia02);
    al_destroy_bitmap(btmonaco);
    al_destroy_bitmap(btportugal);
    al_destroy_bitmap(btcheca);
    al_destroy_bitmap(btsuica);

    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return 0;
}
