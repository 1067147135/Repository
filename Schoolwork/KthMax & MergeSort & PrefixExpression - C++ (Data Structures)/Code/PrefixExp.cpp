#include <iostream>
#include <string>
using namespace std;

string expression[2000000];
long long numbers[1000000];


int main() {
    int n;
    string element;
    
    cin >> n;
    for (int i = 0; i < n; i++) {
        cin >> element;
        expression[i] = element;
    }
    if (n == 0){
        cout << 0;
        return 0;
    } 
    int pointer = -1;
    for (int i = n-1; i >= 0; i--) {
        if (expression[i][0] == '+'){
            if (pointer >= 1){
                long long num1 = numbers[pointer--];
                long long num2 = numbers[pointer--];
                long long result = (num1 + num2) % 1000000007;
                numbers[++pointer] = result;
            }
            else{
                cout << "Invalid";
                return 0;
            }
        }
        else if (expression[i][0] == '-'){
            if (pointer >= 1){
                long long num1 = numbers[pointer--];
                long long num2 = numbers[pointer--];
                long long result = (num1 - num2) % 1000000007;
                numbers[++pointer] = result;
            }
            else{
                cout << "Invalid";
                return 0;
            }    
        }
        else if (expression[i][0] == '*'){
            if (pointer >= 1){
                long long num1 = numbers[pointer--];
                long long num2 = numbers[pointer--];
                long long result = (num1 * num2) % 1000000007;
                numbers[++pointer] = result;
            }
            else{
                cout << "Invalid";
                return 0;
            }  
        }
        else{
            long long number = stoll(expression[i]) % 1000000007;
            numbers[++pointer] = number;
        }
    }
    cout << (numbers[0]+1000000007)  % 1000000007;   
    return 0;
}