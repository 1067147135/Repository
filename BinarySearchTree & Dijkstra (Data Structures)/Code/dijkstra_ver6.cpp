#include<cstdio>
using namespace std;

inline void redn(int& ret1, int& ret2, int& ret3) {
    ret1 = 0;
    char ch=getchar();
    while (ch>='0'&&ch<='9'){
        ret1 = ret1*10 + ch - '0';
        ch=getchar();
    }
    ret2 = 0;
    ch=getchar();
    while (ch>='0'&&ch<='9'){
        ret2 = ret2*10 + ch - '0';
        ch=getchar();
    }
    ret3 = 0;
    ch=getchar();
    while (ch>='0'&&ch<='9'){
        ret3 = ret3*10 + ch - '0';
        ch=getchar();
    }
}

struct distances{
    int name = 1073741823;
    int distance = 1073741823;
    distances* next = NULL;
};

struct node{
    int distance_to_s = 1073741823;
    int position = 1073741823;
    distances* neighbors = NULL; 
};

node* points[1000000];
node* min_heap[1000000];

int main(){
    int n, m, s, u, v, w;
    //scanf("%d%d%d", &n, &m, &s);
    redn(n, m, s);
    for (int i = 1; i <= n; i++){
        node* new_node = new node;
        new_node->position = i;
        points[i] = new_node;
        min_heap[i] = new_node;
    }
    // construct graph using array and linked list
    for (int i = 0; i < m; i++){    
        redn(u, v, w);
        distances* new_neighbor = new distances;
        new_neighbor->name = v;
        new_neighbor->distance = w;
        new_neighbor->next = points[u]->neighbors;
        points[u]->neighbors = new_neighbor;
    }
    // dijkstra
    node* source_node = points[s];
    source_node->distance_to_s = 0;
    int currentSize = n;
    
    min_heap[s] = min_heap[1];
    min_heap[s]->position = s;
    min_heap[1] = source_node;
    min_heap[1]->position = 1;
    while (currentSize != 0){
        node* vertex = min_heap[1];
        min_heap[1] = min_heap[currentSize--];
        min_heap[1]->position = 1;
        int child;
        int hole = 1;
        node* tmp = min_heap[hole];
        while (hole*2 <= currentSize){
            child = hole*2;
            if (child != currentSize && min_heap[child+1]->distance_to_s < min_heap[child]->distance_to_s){
                child ++;
            }
            if (min_heap[child]->distance_to_s < tmp->distance_to_s){
                min_heap[hole] = min_heap[child];
                min_heap[hole]->position = hole;
            }else{
                break;
            }
            hole = child;
        }
        min_heap[hole] = tmp;
        min_heap[hole]->position = hole;
        distances* neighbor = vertex->neighbors;
        while (neighbor != NULL){
            if (points[neighbor->name]->distance_to_s > vertex->distance_to_s + neighbor->distance){
                points[neighbor->name]->distance_to_s = vertex->distance_to_s + neighbor->distance;
                int hole = points[neighbor->name]->position;
                int parent;
                node* tmp = min_heap[hole];
                while (hole/2 > 0){
                    parent = hole/2;
                    if (min_heap[parent]->distance_to_s > tmp->distance_to_s){
                        min_heap[hole] = min_heap[parent];
                        min_heap[hole]->position = hole;
                    }else{
                        break;
                    }
                    hole = parent;
                }
                min_heap[hole] = tmp;
                min_heap[hole]->position = hole;
            }
            neighbor = neighbor->next;
        }
    }
    for (int i = 1; i <= n; i++){
        if (points[i]->distance_to_s == 1073741823){
            printf("%d%s", -1, "\n");
        }else{
            printf("%d%s", points[i]->distance_to_s, "\n");
        }
    }
    return 0;
}