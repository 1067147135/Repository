#include<cstdio>
using namespace std;

struct node{
    node* left = NULL;
    node* right = NULL;
    int num_child = 0;
    int value = -1;
};

int array[1500];
int count = 0;

node* insert(node* root, node* new_node){
    node* pointer = root;
    int y = new_node->value;
    while (pointer != NULL){
        if (pointer->value > y){
            if (pointer->left != NULL){
                pointer = pointer->left;
            }else{
                pointer->left = new_node;
                break;
            }     
        }else{
            if (pointer->right != NULL){
                pointer = pointer->right;
            }else{
                pointer->right = new_node;
                break;
            }  
        }
    }
    return pointer;
}

bool check(int value){
    for (int i = 0; i < count; i++){
        if (array[i] == value){
            return false;
        }
    }
    return true;
}

int main(){
    int n, x, y, leaf_count;
    scanf("%d", &n);
    if (n == 0){
        printf("%s", "NO");
        return 0;
    } 
    scanf("%d%d", &x, &y);
    node* root = new node;
    node* new_node0 = new node;
    if (x != -1 && y != -1 && x != y){  
        root->value = x;
        new_node0->value = y;
        if (x > y){
            root->left = new_node0;
        }else{
            root->right = new_node0;
        }
        root->num_child++;
        leaf_count = 1;
        array[count++] = x;
        array[count++] = y;
    }else{
        printf("%s", "NO");
        return 0;
    }
    for (int i = 1; i < n; i++) {
        scanf("%d%d", &x, &y);
        if (x == -1){
            printf("%s", "NO");
            return 0;
        }
        if (y == -1){
            continue;
        }
        if (!check(y)){
            printf("%s", "NO");
            return 0;
        }
        node *new_node = new node;
        new_node->value = y;
        node* parent = insert(root, new_node);
        array[count++] = y;
        parent->num_child++;
        if(parent->left != NULL && parent->left->value == y && parent->right != NULL){
            printf("NO");
            return 0;
        }
        if (parent->value != x){
            printf("%s", "NO");
            return 0;
        }
        if (parent->num_child == 2){
            leaf_count++;
        }
    }
    printf("%s %d", "YES", leaf_count);
    return 0;
}
