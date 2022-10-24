#include<cstdio>
using namespace std;


struct distances{
    int name;
    int distance;
    distances* next = NULL;
};

struct node{
    int name;
    //int position;
    distances* neighbors = NULL; 
};

//node* points[1000000];
node* min_heap[1000000];
int distance_to_s[1000000];
int position[1000000];

int main(){
    int n, m, s, u, v, w;
    scanf("%d%d%d", &n, &m, &s);
    for (int i = 1; i <= n; i++){
        node* new_node = new node;
        new_node->name = i;
        position[i] = i;
        min_heap[i] = new_node;
        distance_to_s[i] = 1073741823;
    }
    // construct graph using array and linked list
    for (int i = 0; i < m; i++){    
        scanf("%d%d%d", &u, &v, &w);
        if (min_heap[u]->neighbors == NULL){
            distances* new_neighbor = new distances;
            new_neighbor->name = v;
            new_neighbor->distance = w;
            min_heap[u]->neighbors = new_neighbor;
        }else{
            distances* pointer = min_heap[u]->neighbors;
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
    node* source_node = min_heap[s];
    distance_to_s[s] = 0;
    int currentSize = n;

    min_heap[s] = min_heap[1];
    position[s] = 1;
    min_heap[1] = source_node;
    position[1] = s;
    while (currentSize != 0){
        node* vertex = min_heap[1];
        min_heap[1] = min_heap[currentSize--];
        position[min_heap[1]->name] = 1;
        int child;
        int hole = 1;
        node* tmp = min_heap[hole];
        while (hole*2 <= currentSize){
            child = hole*2;
            if (child != currentSize && distance_to_s[min_heap[child+1]->name] < distance_to_s[min_heap[child]->name]){
                child ++;
            }
            if (distance_to_s[min_heap[child]->name] < distance_to_s[tmp->name]){
                min_heap[hole] = min_heap[child];
                position[min_heap[hole]->name] = hole;
            }else{
                break;
            }
            hole = child;
        }
        min_heap[hole] = tmp;
        //min_heap[hole]->position = hole;
        distances* neighbor = vertex->neighbors;
        while (neighbor != NULL){
            if (distance_to_s[neighbor->name] > distance_to_s[vertex->name] + neighbor->distance){
                distance_to_s[neighbor->name] = distance_to_s[vertex->name] + neighbor->distance;
                int hole = position[neighbor->name];
                int parent;
                node* tmp = min_heap[hole];
                while (hole/2 > 0){
                    parent = hole/2;
                    if (distance_to_s[min_heap[parent]->name] > distance_to_s[tmp->name]){
                        min_heap[hole] = min_heap[parent];
                        position[min_heap[hole]->name] = hole;
                    }else{
                        break;
                    }
                    hole = parent;
                }
                min_heap[hole] = tmp;
                position[tmp->name] = hole;
            }
            neighbor = neighbor->next;
        }
    }
    for (int i = 1; i <= n; i++){
        if (distance_to_s[i] == 1073741823){
            printf("%d%s", -1, "\n");
        }else{
            printf("%d%s", distance_to_s[i], "\n");
        }
    }
    return 0;
}