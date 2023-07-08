import numpy as np
import xlrd

def translate(v):
    if v == "Iris-setosa":
        return np.array([1, 0, 0])
    elif v == "Iris-versicolor":
        return np.array([0, 1, 0])
    elif v == "Iris-virginica":
        return np.array([0, 0, 1])

def classify(v):
    v1 = v[0]
    v2 = v[1]
    v3 = v[2]
    if v1 > v2 and v1 > v3:
        return np.array([1, 0, 0])
    elif v2 > v1 and v2 > v3:
        return np.array([0, 1, 0])
    elif v3 > v1 and v3 > v2:
        return np.array([0, 0, 1])

def data_preprocessing(file_name):
    # Import data
    # print("Start preprocess data...")
    # print("Read in data from", file_name, "...")
    wb = xlrd.open_workbook(file_name) 
    ws = wb.sheets()[0] 
    nrows = ws.nrows  # 行数
    ncols = ws.ncols  # 列数
    data = np.zeros((nrows, ncols+2))
    for i in range(ncols-1):
        cols = ws.col_values(i)
        data[:, i] = cols
    col = ws.col_values(ncols-1)
    for i in range(nrows):
        ele = col[i]
        data[i, ncols-1:] = translate(ele)
    # print(data)
        
    # print("Shuffle data...")
    np.random.shuffle(data)
    divide = len(data) * 4 // 5 
    # print("Devide data...")
    train_data = data[:divide]
    test_data = data[divide:]
    # print(train_data.shape)
    # print(test_data.shape)
    # print("Done.")
    return train_data, test_data

# ax = b
def RMSE(a, x, b):
    loss_E = np.dot(a, x) - b
    return (np.trace(np.dot(loss_E.transpose(), loss_E)) / len(a)) ** 0.5

def iteration(a, b, xk, dk, A, B):
    i = 0
    # print("0: ", RMSE(A, xk, B))
    # print("a: ", RMSE(A, xk + a * dk, B))
    # print("b: ", RMSE(A, xk + b * dk, B))
    while i < 100:
        i += 1
        x1 = b - 0.618*(b-a) # 0.618 * a + (1 - 0.618) * b
        x2 = a + 0.618*(b-a) # 0.618 * b + (1 - 0.618) * a
        f1 = RMSE(A, xk + x1 * dk, B)
        f2 = RMSE(A, xk + x2 * dk, B)
        # print(f1, "vs.", f2)
        if f1 < f2:
            b = x2
        else:
            a = x1
    ak = 0.5 * (a + b)
    # print("f: ", RMSE(A, xk + ak * dk, B))
    return ak

def gradient_descent(a, b):
    count = 0
    # Set initial values
    # Get from a training process of Iteration 5700
    xk = np.array([[ 0.20508023,  1.61909519, -0.43545734],
        [ 0.04313224, -0.04177524, -0.05417703],
        [ 0.24198351, -0.42537841,  0.14561678],
        [-0.20593611,  0.22340828, -0.01799003],
        [-0.07404898, -0.47368887,  0.58336008]])
    
    # Objective function，minimize RMSE
    rmse = RMSE(a, xk, b)
    # print("Iteration", count, ": xk =", xk, ", rmse =", rmse)
    while True:
        for i in range(100):
            count += 1
            dk = - np.dot(a.transpose(), (np.dot(a, xk) - b))
            norm = np.linalg.norm(dk)
            if (norm) < 0.000001:
                return xk
            # Golden Section Method iteration
            ak = iteration(0, 2, xk, dk, a, b)
            xk = xk + ak * dk
            if (rmse - RMSE(a, xk, b)) < 0.000001:
                return xk
            rmse = RMSE(a, xk, b)
        # print("Iteration", count, "xk =", xk, "norm of gradient =", norm, ": rmse =", rmse)

def rate(x, w, y):
    res = np.dot(x, w)
    count = 0
    for i in range(len(res)):
        r = classify(res[i])
        if all(r == y[i]):
            count += 1
    return 1 - count / len(res)

def train(train_data):
    train_X = train_data[:, :4]
    b = np.ones(len(train_X))
    train_X = np.insert(train_X,0,b,axis=1)
    train_Y = train_data[:, 4:]
    train_W = gradient_descent(train_X, train_Y)
    error_rate = rate(train_X, train_W, train_Y)
    return train_W, error_rate

def test(test_data, train_W):
    test_X = test_data[:, :4]
    b = np.ones(len(test_data))
    test_X = np.insert(test_X,0,b,axis=1)
    test_Y = test_data[:, 4:]
    # Calculate RMSE
    error_rate = rate(test_X, train_W, test_Y)
    return error_rate


# Main function
for i in range(10):
    train_data, test_data = data_preprocessing("Classification iris.xlsx")
    train_W, train_error_rate= train(train_data)
    test_error_rate = test(test_data, train_W)
    print("Trail", i)
    print("train classification error rate =", train_error_rate)
    print("test classification error rate =", test_error_rate)

# python Classification.py > ClaRes.txt