#include<cstdio>
using namespace std;


struct distances{
    int name = 2147483647/2;
    int distance = 2147483647/2;
    distances* next = NULL;
};

struct node{
    int value = 2147483647/2;
    int distance_to_s = 2147483647/2;
    distances* neighbors = NULL; 
};

node* points[1000000];
node* min_heap[10000000];

void perlocateDown(node* array[], int hole, int currentSize){
    int child;
    node* tmp = array[hole];
    while (hole*2 <= currentSize){
        child = hole*2;
        if (child != currentSize && array[child+1]->distance_to_s < array[child]->distance_to_s){
            child ++;
        }
        if (array[child]->distance_to_s < tmp->distance_to_s){
            array[hole] = array[child];
        }else{
            break;
        }
        hole = child;
    }
    array[hole] = tmp;
};

node* deleteMin(node* array[], int &currentSize){
    for (int i = currentSize/2; i > 0; i--){
        perlocateDown(array, i, currentSize);
    }
    node* data = array[1];
    array[1] = array[currentSize--];
    return data;
}

int main(){
    int n, m, s, u, v, w;
    scanf("%d%d%d", &n, &m, &s);
    for (int i = 0; i < n; i++){
        node* new_node = new node;
        new_node->value = i+1;
        points[i] = new_node;
    }
    // construct graph using array and linked list
    for (int i = 0; i < m; i++){    
        scanf("%d%d%d", &u, &v, &w);
        if (points[u-1]->neighbors == NULL){
            distances* new_neighbor = new distances;
            new_neighbor->name = v;
            new_neighbor->distance = w;
            points[u-1]->neighbors = new_neighbor;
        }else{
            distances* pointer = points[u-1]->neighbors;
            while (pointer->next != NULL && pointer->name != v){
                pointer = pointer->next;
            }
            if (pointer->name == v){
                if (pointer->distance > w){
                        pointer->distance = w;
                }
            }else{
                distances* new_neighbor = new distances;
                new_neighbor->name = v;
                new_neighbor->distance = w;
                pointer->next = new_neighbor;
            }
        }  
    }
    // dijkstra
    node* source_node = points[s-1];
    source_node->distance_to_s = 0;
    int currentSize = n;
    for (int i = 1; i <= n; i++){
        min_heap[i] = points[i-1];
    }
    while (currentSize != 0){
        node* vertex = deleteMin(min_heap, currentSize);
        distances* neighbor = vertex->neighbors;
        while (neighbor != NULL){
            if (points[neighbor->name-1]->distance_to_s > vertex->distance_to_s + neighbor->distance){
                points[neighbor->name-1]->distance_to_s = vertex->distance_to_s + neighbor->distance;
            }
            neighbor = neighbor->next;
        }
    }
    for (int i = 0; i < n; i++){
        if (points[i]->distance_to_s == 1073741823){
            printf("%d%s", -1, "\n");
        }else{
            printf("%d%s", points[i]->distance_to_s, "\n");
        }
    }
    return 0;
}