import numpy as np

def data_preprocessing(file_name):
    # Import data
    # print("Start preprocess data...")
    # print("Read in data from", file_name, "...")
    data = np.loadtxt(open(file_name,"rb"), delimiter=",", skiprows=1, usecols=[i for i in range(2, 25)])
    # print("Clean rows with nan...")
    pointer = 0
    while pointer < data.shape[0]:
        hasNaN = False
        row = data[pointer]
        for ele in row:
            if np.isnan(ele):
                data = np.delete(data,(pointer), axis = 0)
                hasNaN = True
                break
        if not hasNaN:
            pointer += 1
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
    # Get from a training process of Iteration 295000
    xk = np.array([[ 9.89390306e-01,  9.88901062e-01],
                [ 4.53688545e-01,  4.39521077e-01],
                [ 4.98791807e-01,  5.16961337e-01],
                [-1.80189319e-01, -7.30362527e-02],
                [-4.80140096e-02, -5.70230066e-02],
                [ 6.44635414e-01,  6.01833968e-01],
                [ 5.84540344e-01,  5.90659062e-01],
                [ 7.47533423e-01,  7.77344292e-01],
                [ 1.36537451e-02, -8.61956257e-04],
                [ 9.83619261e-01,  9.87863741e-01],
                [ 9.80674422e-01,  9.84567358e-01],
                [ 9.82249143e-01,  9.85496169e-01],
                [ 9.84280873e-01,  9.87480277e-01],
                [ 8.27425847e-01,  8.31933876e-01],
                [ 8.37143337e-01,  8.35555622e-01],
                [ 9.02164367e-01,  9.06255080e-01],
                [ 9.24308408e-01,  9.27010155e-01],
                [ 6.01927436e-01,  5.83463049e-01],
                [-3.47963825e-01, -4.10021256e-01],
                [-7.76744426e-03, -1.17358146e-02],
                [ 9.51806663e-01,  9.50641276e-01],
                [-4.07628712e-04, -7.78927775e-04]])
    
    # Objective function，minimize RMSE
    rmse = RMSE(a, xk, b)
    # print("Iteration", count, ": xk =", xk, ", rmse =", rmse)
    while True:
        for i in range(100):
            count += 1
            dk = - np.dot(a.transpose(), (np.dot(a, xk) - b))
            norm = np.linalg.norm(dk)
            if (norm) < 0.0001:
                return xk, rmse
            # Golden Section Method iteration
            ak = iteration(0, 2, xk, dk, a, b)
            xk = xk + ak * dk
            if (rmse - RMSE(a, xk, b)) < 0.0001:
                return xk, rmse
            rmse = RMSE(a, xk, b)
        # print("Iteration", count, "xk =", xk, "norm of gradient =", norm, ": rmse =", rmse)
    

def train(train_data):
    train_X = train_data[:, :21]
    b = np.ones(len(train_X))
    train_X = np.insert(train_X,0,b,axis=1)
    train_Y = train_data[:, 21:]
    train_W, rmse = gradient_descent(train_X, train_Y)
    return train_W, rmse

def test(test_data, train_W):
    test_X = test_data[:, :21]
    b = np.ones(len(test_data))
    test_X = np.insert(test_X,0,b,axis=1)
    test_Y = test_data[:, 21:]
    # Calculate RMSE
    rmse = RMSE(test_X, train_W, test_Y)
    return rmse


# Main function

for i in range(10):
    train_data, test_data = data_preprocessing("Regression.csv")
    train_W, train_rmse= train(train_data)
    test_rmse = test(test_data, train_W)
    print("Trail", i)
    print("train RMSE =", train_rmse)
    print("test RMSE =", test_rmse)


# python Regression.py > RegRes.txt

