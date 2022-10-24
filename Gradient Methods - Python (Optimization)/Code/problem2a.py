import sympy
import numpy as np

def func_XY_to_X_Y(f):
    """
    Wrapper for f(X) -> f(X[0], X[1])
    """
    return lambda X: np.array(f(X[0], X[1]))

x1, x2 = sympy.symbols("x_1, x_2")

# objective function
f_sym = x1 ** 4 + 2/3 * x1 ** 3 + 1/2 * x1 ** 2 - 2 * x1 ** 2 * x2 + 4/3 * x2 ** 2
f_lmbda = sympy.lambdify((x1, x2), f_sym, 'numpy')
f = func_XY_to_X_Y(f_lmbda)

# gradiant
fprime_sym = np.array([f_sym.diff(x_) for x_ in (x1, x2)])
sympy.Matrix(fprime_sym)
fprime_lmbda = sympy.lambdify((x1, x2), fprime_sym, 'numpy')
fprime = func_XY_to_X_Y(fprime_lmbda)

# stopping tolerance parameter for exact line search
tol = 0.000001
# maximum nummber of iterations
maxit = 100
# the interval of the step size
s = 2

# parameters for backtracking and the Armijo condition
sig = 0.5
gam = 0.1

# initial point
x0 = np.array([3, 3])


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

def gradient_method(flag):
    count = 1
    xk = x0
    dk = - fprime(xk)                       # gradient descent
    if flag == True:                        # Exact line search
        while norm(dk) > 0.00001:
            ak = iteration(0, s, xk, dk)    # Golden Section Method iteration
            xk = xk + ak * dk
            count += 1
            dk = - fprime(xk)  
        print("Exact line search method converges in", count, "steps, with accuracy", norm(dk))
        print("final derivative value", xk, ", final objective function value", f(xk))
        return
    else:                                   # Backtraking / Armijo line search
        while norm(dk) > 0.00001:
            ak = 1
            cond = f(xk + ak * dk) - f(xk) - gam * ak * norm(dk) ** 2
            # print("count:", count, "cond:", cond)
            while cond > 0:
                ak *= sig
                cond = f(xk + ak * dk) - f(xk) - gam * ak * norm(dk) ** 2   
            xk = xk + ak * dk
            count += 1
            dk = - fprime(xk)  
        print("Backtraking / Armijo line search method converges in", count, "steps, with accuracy", norm(dk))
        print("final derivative value", xk, ", final objective function value", f(xk))
        return


if __name__ == '__main__':
    gradient_method(True)
    gradient_method(False)
    