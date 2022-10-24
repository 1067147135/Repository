# from sympy import *
import math
import matplotlib.pyplot as plt

# objective function
def func(x):
    A = - 1 / (x - 1) ** 2 * (math.log(x) - 2 * (x - 1) / (x + 1))
    return A

# Golden Section Method iteration
def iteration(a,b,dx):
    x_values = []
    x_values.append(a)
    x_values.append(b)
    i = 0
    while True:
        i += 1
        x1 = b - 0.618*(b-a) # 0.618 * a + (1 - 0.618) * b
        x2 = a + 0.618*(b-a) # 0.618 * b + (1 - 0.618) * a
        f1 = func(x1)
        f2 = func(x2)
        if f1 < f2:
            b = x2
            x_values.append(x2)
        else:
            a = x1
            x_values.append(x1)
        if (b - a) <= dx:
            final_x = 0.5 * (a + b)
            print("Golden Section Method converges in", i, "steps, with accuracy", (b - a))
            print("final derivative value", final_x, ", final objective function value", func(final_x))
            break
    draw(x_values)

# draw the plot
def draw(x_values):
    #设置绘图风格
    plt.style.use('ggplot')
    #处理中文乱码
    plt.rcParams['font.sans-serif'] = ['Microsoft YaHei']
    #坐标轴负号的处理
    plt.rcParams['axes.unicode_minus'] = False
    #横坐标是区间
    #纵坐标是函数值
    y_values = []
    x_values.sort()  #默认列表中的元素从小到大排列
    for x in x_values:
        y_values.append(func(x))
    #绘制折线图
    plt.plot(x_values,
             y_values,
             color = 'steelblue', # 折线颜色
             marker = 'o', # 折线图中添加圆点
             markersize = 3, # 点的大小
             )
    # 修改x轴和y轴标签
    plt.xlabel('x')
    plt.ylabel('f(x)')
    # 添加图形标题
    plt.title('Golden Section Search Method to find the minimum')
    # 显示图形
    plt.show()

if __name__ == '__main__':
    a = 1.5  # 区间下限
    b = 4.5  # 区间上限
    dx = 0.00001  # 迭代精度
    iteration(a, b, dx)
