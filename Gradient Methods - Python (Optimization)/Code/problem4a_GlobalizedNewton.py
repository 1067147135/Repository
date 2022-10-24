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
fprime_sym = [f_sym.diff(x_) for x_ in (x1, x2)]
sympy.Matrix(fprime_sym)
fprime_lmbda = sympy.lambdify((x1, x2), fprime_sym, 'numpy')
fprime = func_XY_to_X_Y(fprime_lmbda)

# Hessian
fhess_sym = [[f_sym.diff(x1_, x2_) for x1_ in (x1, x2)] for x2_ in (x1, x2)]
sympy.Matrix(fhess_sym)
fhess_lmbda = sympy.lambdify((x1, x2), fhess_sym, 'numpy')
fhess = func_XY_to_X_Y(fhess_lmbda)

# stopping tolerance parameter for exact line search
tol = 0.0000001

# parameters for backtracking and the Armijo condition
sig = 0.5
gam = 0.0001

# parameters for the Newton condition
gam1 = 0.000001
gam2 = 0.1

# initial point
x0 = np.array([-1, -0.5])

# counting
c_new = 0   # number of newton direction is utilized
c_ak = 0    # number of full step size sk is used

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

def find_dk(xk):
    global c_new
    sk = np.linalg.solve(fhess(xk), -fprime(xk))
    if -fprime(xk).T @ sk >= gam1 * min(1, norm(sk) ** gam2) * norm(sk) ** 2:
        c_new += 1
        return sk
    else:
        return -fprime(xk)

def newton_glob():
    global c_ak
    x_values = []
    count = 1
    xk = x0
    x_values.append(xk)
    dk = find_dk(xk)
    while norm(fprime(xk)) > tol:
        ak = 1
        cond = f(xk + ak * dk) - f(xk) - gam * ak * norm(dk) ** 2
        # print("count:", count, "cond:", cond)
        while cond > 0:
            ak *= sig
            cond = f(xk + ak * dk) - f(xk) - gam * ak * norm(dk) ** 2
        if ak == 1:
            c_ak += 1   
        xk = xk + ak * dk
        x_values.append(xk)
        count += 1
        dk = find_dk(xk) 
    print("The globalized Newton Method converges in", count, "steps, with accuracy", norm(dk))
    print("final derivative value", xk, ", final objective function value", f(xk))
    print("utilize the Newton direction", c_new, "times, use full step size ak", c_ak, "times")
    draw(x_values, "The globalized Newton Method")
    return

if __name__ == '__main__':
    newton_glob()

    
    