#include <stdio.h>
#include <stdlib.h>

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




int main(){

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

    /*
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
    */

    imprimir(grafo);
    printf("\n\n");

    Node* caminho = encontrarCaminho(grafo, 0, 6);
    imprimir(grafo);
    printf("\n\n");

    imprimirCaminho(caminho);

    //teste1 cavalca
    return 0;
}
