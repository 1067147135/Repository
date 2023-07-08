#include<cstdio>
using namespace std;

void merge(long long a[], long long tempArray[], int leftPos, int rightPos, int rightEnd) {
    int leftEnd = rightPos - 1;
    int numElements = rightEnd - leftPos + 1;
    for (int i = leftPos; i <= leftEnd; i++){
        tempArray[i] = a[i];
    }
    tempArray[leftEnd+1] = 9223372036854775807;
    for (int i = rightPos; i <= rightEnd; i++){
        tempArray[i+1] = a[i];
    }
    tempArray[rightEnd+2] = 9223372036854775807;
    rightPos++;

    int start = leftPos;
    int end = numElements+leftPos;
    for (int i = start; i < end; i++){
        if (tempArray[leftPos] <= tempArray[rightPos]){
            a[i] = tempArray[leftPos++];
        }
        else{
            a[i] = tempArray[rightPos++];
        }
    }

}

void mergeSort(long long a[], long long tempArray[], int left, int right) {
    if (left < right) {
        int center = (left + right)/2;
        mergeSort(a, tempArray, left, center);
        mergeSort(a, tempArray, center+1, right);
        merge(a, tempArray, left, center+1, right);
    }
}

long long nums[500000];
long long temp[500002];	
int main() {
    int n;
    long long num;

    scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        scanf("%llu", &num);
        nums[i] = num;
    }
    if (n == 0){
        printf("%llu", 0ll);
        return 0;
    } 
    mergeSort(nums, temp, 0, n-1);
    for (int i = 0; i < n; i++) {
        printf("%llu\n", nums[i]);
    }
    
    return 0;
}