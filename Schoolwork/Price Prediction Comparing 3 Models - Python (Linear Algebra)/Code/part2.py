import numpy as np

# least square solution
def least_square(A,b):
    middle = np.dot(A.T,A)
    middle = np.linalg.inv(middle) 
    middle = np.dot(middle,A.T)
    x = np.dot(middle,b)
    return x

# Autoregressive model
def Autoregressive(data_list,N,n):
    # n = 1(product 5),2(product 4),3(product 3),4(product 2),5(product 1)
    length = int(len(data_list)/5)
    A = np.zeros((length-N-1,N+4))
    for i in range(N+1,length):
        data = data_list[5*i-n:5*i-5*N-n:-5]
        if n != 1:
            data.append(data_list[5*i-1])
        if n != 2:
            data.append(data_list[5*i-2])
        if n != 3:
            data.append(data_list[5*i-3])
        if n != 4:
            data.append(data_list[5*i-4])
        if n != 5:
            data.append(data_list[5*i-5])
        A[i-N-1] = data
    b = np.zeros((length-N-1,1))
    for j in range(N+1,length):
        b[j-N-1] = data_list[5*i-n]
    return A,b

# data entry
print('data entry...')
train_data = open('Part2_training_data.txt','r')
data_list = []
while True:
    line = train_data.readline()
    if line:
        element1 = line.split(' ')
        for i in range(5):
            element2 = element1[i].split('e')
            number = float(element2[0]) * 10 ** float(element2[1])
            if number < 0:
                number = - number
            data_list.append(number)
    else:
        break
train_data.close()
test_data = open('Part2_testing_data.txt','r')
data_list_test = []
while True:
    line = test_data.readline()
    if line:
        element1 = line.split(' ')
        for i in range(5):
            element2 = element1[i].split('e')
            number = float(element2[0]) * 10 ** float(element2[1])
            if number < 0:
                number = - number
            data_list_test.append(number)
    else:
        break
train_data.close()

print('model: Autoregressive model')
print('1(product 5),2(product 4),3(product 3),4(product 2),5(product 1)')
try_all = int(len(data_list_test)/5)-1
for j in range(1,6):
    best_N = 0
    best_error = 100000000000
    best_x = 0
    for N in range(1,try_all):
        A1, b1 = Autoregressive(data_list,N,j)
        A2, b2 = Autoregressive(data_list_test,N,j)
        x = least_square(A1,b1)
        y = np.dot(A2,x)
        error = 0
        for i in range(len(y)):
            error = error + (y[i]-b2[i])**2
        error = (error/len(y))**(1/2)
        if error <= best_error:
            best_N = N
            best_error = error        
            best_x = x
    print("best_N for",j,":",best_N)
    print("best_error for",j,":",best_error)
    print("best_x for",j,":",best_x)