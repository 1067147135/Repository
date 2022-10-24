import numpy as np
import math

# least square solution
def least_square(A,b):
    middle = np.dot(A.T,A)
    middle = np.linalg.inv(middle) 
    middle = np.dot(middle,A.T)
    x = np.dot(middle,b)
    return x

# Autoregressive model
def Autoregressive(data_list,N):
    length = len(data_list)
    A = np.zeros((length-N-1,N))
    for i in range(N,length-1):
        A[i-N] = data_list[i:i-N:-1]
    b = np.zeros((length-N-1,1))
    for j in range(N+1,length):
        b[j-N-1] = data_list[j]
    return A,b

# Fourier series
def Fourier(data_list,N):
    length = len(data_list)
    A = np.zeros((length,2*N))
    for i in range(1,length+1):
        elements = []
        for j in range(1,N+1):
            elements.append(math.cos(i*j))
            elements.append(math.sin(i*j))
        A[i-1] = elements
    b = np.zeros((length,1))
    for k in range(length):
        b[k] = data_list[k]
    return A,b

# Taylor formula
def Taylor(data_list,N):
    length = len(data_list)
    A = np.zeros((length,N))
    for i in range(length):
        elements = []
        for j in range(N):
            elements.append(i**j)
        A[i] = elements
    b = np.zeros((length,1))
    for k in range(length):
        b[k] = data_list[k]
    return A,b

# data entry
print('data entry...')
train_data = open('Part1_training_data.txt','r')
data_list = []
while True:
    line = train_data.readline()
    if line:
        element = line.split('e')
        number = float(element[0]) * 10 ** float(element[1])
        data_list.append(number)
    else:
        break
train_data.close()
test_data = open('Part1_testing_data.txt','r')
data_list_test = []
while True:
    line_test = test_data.readline()
    if line_test:
        element_test = line_test.split('e')
        number_test = float(element_test[0]) * 10 ** float(element_test[1])
        data_list_test.append(number_test)
    else:
        break
test_data.close()

# calculation
print('model: Autoregressive model')
try_all = len(data_list_test)-1
best_N = 0
best_error = 100000000000
best_x = 0
for N in range(1,try_all):
    A1, b1 = Autoregressive(data_list,N)
    A2, b2 = Autoregressive(data_list_test,N)
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
print("best_N:",best_N)
print("best_error:",best_error)
print("best_x:",best_x)

print('model: Fourier series')
best_N = 0
best_error = 100000000000
best_x = 0
for N in range(1,try_all):
    A1, b1 = Fourier(data_list,N)
    A2, b2 = Fourier(data_list_test,N)
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
print("best_N:",best_N)
print("best_error:",best_error)
print("best_x:",best_x)

print('model: Taylor formula')
best_N = 0
best_error = 100000000000
best_x = 0
for N in range(1,try_all):
    A1, b1 = Taylor(data_list,N)
    A2, b2 = Taylor(data_list_test,N)
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
print("best_N:",best_N)
print("best_error:",best_error)
print("best_x:",best_x)

# Interface
# while True:
#     try:
#         N = int(input('Enter N:'))
#         mode = int(input('Enter mode number (1. Autoregressive model 2. Fourier series 3. Taylor formula)：'))
#         print('Get.')
#         if mode == 1:
#             A1, b1 = Autoregressive(data_list,N)
#             A2, b2 = Autoregressive(data_list_test,N)
#         elif mode == 2:
#             A1, b1 = Fourier(data_list,N)
#             A2, b2 = Fourier(data_list_test,N)
#         elif mode == 3:
#             A1, b1 = Taylor(data_list,N)
#             A2, b2 = Taylor(data_list_test,N)
#         else:
#             print('Invalid enter!')
#             continue
#         print('Start calculating...')
#         x = least_square(A1,b1)
#         y = np.dot(A2,x)
#         error = 0
#         for i in range(len(y)):
#             error = error + (y[i]-b2[i])**2
#         print('Finish calculation!')
#         print("Coefficients are",x)
#         print("Error is",error)

#     except:
#         print('Invalid enter!')
#         continue