import sympy
import numpy as np
import matplotlib.pyplot as plt

def func_XY_to_X_Y(f):
    """
    Wrapper for f(X) -> f(X[0], X[1])
    """
    return lambda X: np.array(f(X[0], X[1]))

x1, x2 = sympy.symbols("x_1, x_2")

# objective function
f_sym = 100 * (x2 - x1 ** 2) ** 2 + (1 - x1) ** 2
f_lmbda = sympy.lambdify((x1, x2), f_sym, 'numpy')
f = func_XY_to_X_Y(f_lmbda)

# gradiant
fprime_sym = np.array([f_sym.diff(x_) for x_ in (x1, x2)])
sympy.Matrix(fprime_sym)
fprime_lmbda = sympy.lambdify((x1, x2), fprime_sym, 'numpy')
fprime = func_XY_to_X_Y(fprime_lmbda)

# maximum nummber of iterations
maxit = 100
# the interval of the step size
s = 2
# stopping tolerance parameter for exact line search
tol = 0.0000001

# parameters for backtracking and the Armijo condition
sig = 0.5
gam = 0.0001

# initial point
x0 = np.array([-1, -0.5])


# Golden Section Method iteration
def iteration(a,b,xk,dk):
    i = 0
    while i < maxit:
        i += 1
        x1 = b - 0.618*(b-a) # 0.618 * a + (1 - 0.618) * b
        x2 = a + 0.618*(b-a) # 0.618 * b + (1 - 0.618) * a
        f1 = f(xk + x1 * dk)
        f2 = f(xk + x2 * dk)
        if f1 < f2:
            b = x2
        else:
            a = x1
        if (b - a) <= tol:
            break
    final_x = 0.5 * (a + b)
    return final_x        

def norm(dk):
    return np.linalg.norm(dk)

def draw(x_values, name):
    # plt.style.use('_mpl-gallery-nogrid')
    #设置绘图风格
    plt.style.use('ggplot')
    #处理中文乱码
    plt.rcParams['font.sans-serif'] = ['Microsoft YaHei']
    #坐标轴负号的处理
    plt.rcParams['axes.unicode_minus'] = False

    # make data
    X, Y = np.meshgrid(np.linspace(-4, 4, 256), np.linspace(-4, 4, 256))
    Z = 100 * (Y - X ** 2) ** 2 + (1 - X) ** 2
    levels = np.linspace(np.min(Z), np.max(Z), 7)

    # plot
    fig, ax = plt.subplots()

    ax.contour(X, Y, Z, levels=levels)

    #横坐标是区间
    #纵坐标是函数值
    x1_values = []
    x2_values = []
    # x_values.sort()  #默认列表中的元素从小到大排列
    for x in x_values:
        x1_values.append(x[0])
        x2_values.append(x[1])
    #绘制折线图
    plt.plot(x1_values,
             x2_values,
             color = 'steelblue', # 折线颜色
             marker = 'o', # 折线图中添加圆点
             markersize = 3, # 点的大小
             )
    # 修改x轴和y轴标签
    plt.xlabel('x1')
    plt.ylabel('x2')
    # 添加图形标题
    plt.title(name)
    # 显示图形
    plt.show()

def gradient_method(flag):
    x_values = []
    count = 1
    xk = x0
    x_values.append(xk)
    dk = - fprime(xk)                       # gradient descent
    if flag == True:                        # Exact line search
        while norm(dk) > tol:
            ak = iteration(0, s, xk, dk)    # Golden Section Method iteration
            xk = xk + ak * dk
            x_values.append(xk)
            count += 1
            dk = - fprime(xk)  
        print("Exact line search method converges in", count, "steps, with accuracy", norm(dk))
        print("final derivative value", xk, ", final objective function value", f(xk))
        draw(x_values, "Exact line search method")
        return
    else:                                   # Backtraking / Armijo line search
        while norm(dk) > tol:
            ak = 1
            cond = f(xk + ak * dk) - f(xk) - gam * ak * norm(dk) ** 2
            # print("count:", count, "cond:", cond)
            while cond > 0:
                ak *= sig
                cond = f(xk + ak * dk) - f(xk) - gam * ak * norm(dk) ** 2   
            xk = xk + ak * dk
            x_values.append(xk)
            count += 1
            dk = - fprime(xk)  
        print("Backtraking / Armijo line search method converges in", count, "steps, with accuracy", norm(dk))
        print("final derivative value", xk, ", final objective function value", f(xk))
        draw(x_values, "Backtraking / Armijo line search method")
        return


if __name__ == '__main__':
    gradient_method(True)
    
# Backtraking / Armijo line search method converges in 17205 steps, with accuracy 9.927503520556047e-08
# final derivative value [1.00000008 1.00000016] , final objective function 
# value 6.1697955620125855e-15

# Exact line search method converges in 16547 steps, with accuracy 9.991710703420053e-08
# final derivative value [0.99999991 0.99999982] , final objective function 
# value 7.863326697028809e-15