#include<cstdio>
using namespace std;


struct distances{
    int name = 1073741823;
    int distance = 1073741823;
    distances* next = NULL;
};

int min_heap[1000000];
int position[1000000];
int distance_to_s[1000000];
distances* neighbors[1000000];

int main(){
    int n, m, s, u, v, w;
    scanf("%d%d%d", &n, &m, &s);
    for (int i = 1; i <= n; i++){
        min_heap[i] = i;
        position[i] = i;
        distance_to_s[i] = 1073741823;
        neighbors[i] = NULL;
    }
    // construct graph using array and linked list
    for (int i = 0; i < m; i++){    
        scanf("%d%d%d", &u, &v, &w);
        if (neighbors[u] == NULL){
            distances* new_neighbor = new distances;
            new_neighbor->name = v;
            new_neighbor->distance = w;
            neighbors[u] = new_neighbor;
        }else{
            distances* pointer = neighbors[u];
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
    distance_to_s[s] = 0;
    int currentSize = n;
    min_heap[s] = 1;
    min_heap[1] = s;
    position[1] = s;
    position[s] = 1;
    while (currentSize != 0){
        int vertex = min_heap[1];
        min_heap[1] = min_heap[currentSize--];
        position[min_heap[1]] = 1;
        // perlocateDown
        int child;
        int hole = 1;
        int tmp = min_heap[hole];
        while (hole*2 <= currentSize){
            child = hole*2;
            if (child != currentSize && distance_to_s[min_heap[child+1]] < distance_to_s[min_heap[child]]){
                child ++;
            }
            if (distance_to_s[min_heap[child]] < distance_to_s[tmp]){
                min_heap[hole] = min_heap[child];
                position[min_heap[hole]] = hole;
            }else{
                break;
            }
            hole = child;
        }
        min_heap[hole] = tmp;
        position[tmp] = hole;

        distances* neighbor = neighbors[vertex];
        while (neighbor != NULL){
            if (distance_to_s[neighbor->name] > distance_to_s[vertex] + neighbor->distance){
                distance_to_s[neighbor->name] = distance_to_s[vertex] + neighbor->distance;
                // perlocateUp
                int hole = position[neighbor->name];
                int parent;
                int tmp = min_heap[hole];
                while (hole/2 > 0){
                    parent = hole/2;
                    if (distance_to_s[min_heap[parent]] > distance_to_s[tmp]){
                        min_heap[hole] = min_heap[parent];
                        position[min_heap[hole]] = hole;
                    }else{
                        break;
                    }
                    hole = parent;
                }
                min_heap[hole] = tmp;
                position[tmp] = hole;
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