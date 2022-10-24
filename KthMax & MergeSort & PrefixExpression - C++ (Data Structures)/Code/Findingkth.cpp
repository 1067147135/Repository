#include<cstdio>
using namespace std;

int nums[10000000];

int main() {
    int n, k, num;
    scanf("%d%d", &n, &k);
    for (int i = 0; i < n; i++) {
        scanf("%d", &num);
        nums[i] = num;
    }
    if (n == 0){
        printf("%d", 0);
        return 0;
    } 

    int left = 0;
    int right = n - 1;
    while (true) {
        int pivot = left;
        int l = left + 1; 
        int r = right;
        while (l <= r) {
            while (l <= r && nums[l] >= nums[pivot]) l++; 
            while (l <= r && nums[r] <= nums[pivot]) r--; 
            if (l <= r && nums[l] < nums[pivot] && nums[r] > nums[pivot]) {
                int tmp = nums[l];
                nums[l++] = nums[r];
                nums[r--] = tmp;
            }
        }
        int tmp = nums[pivot];
        nums[pivot] = nums[r];
        nums[r] = tmp;

        int position = r;
        if (position == k - 1){
            printf("%d",nums[position]);
            return 0;
        } 
        else if (position > k - 1) right = position - 1; 
        else left = position + 1;
    }
    return 0;
}