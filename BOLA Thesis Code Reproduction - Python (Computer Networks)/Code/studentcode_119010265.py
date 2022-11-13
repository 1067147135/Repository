# Written by SHI Wenlan
# BOLA

from asyncore import compact_traceback
import math

# Used to save previous bitrate
# In this experiment, it is among 500000, 1000000, 5000000
bitrate = 0  
last_optimal_m = 3


def student_entrypoint(Measured_Bandwidth, Previous_Throughput, Buffer_Occupancy, Available_Bitrates, Video_Time, Chunk, Rebuffering_Time, Preferred_Bitrate):
    # student can do whatever they want from here going forward
    global bitrate
    R_i = list(Available_Bitrates.items())
    R_i.sort(key=lambda tup: tup[1])
    # print("Bitrate: ", bitrate)
    # print("Measured_Bandwidth: ", Measured_Bandwidth)
    # print("Previous_Throughput: ", Previous_Throughput)
    # print("Buffer_Occupancy: ", Buffer_Occupancy)
    # print("Available_Bitrates: ", Available_Bitrates)
    # print("Chunk: ", Chunk)
    # print("Video_Time: ", Video_Time)
    # print("Rebuffering_Time: ", Rebuffering_Time)
    # print("Preferred_Bitrate: ", Preferred_Bitrate)
    # print("--------------------------------------------")
    bitrate = BOLA(bitrate, Chunk, Buffer_Occupancy, R_i, Previous_Throughput)
    # bitrate = bufferbased(
    #     rate_prev=bitrate, buf_now=Buffer_Occupancy, r=Chunk['time']+1, R_i=R_i)
    return bitrate

def match(value, list_of_list):
    for e in list_of_list:
        if value == e[0]:
            return e[1]

# helper function, to find the corresponding size of previous bitrate
# if there's was no previous assume that it was the highest possible value
def prevmatch(value, list_of_list):
    # print("value = ", value, ", list_of_list = ",list_of_list)
    for e in list_of_list:
        if value == e[0]:
            return e
    return list_of_list[-1]

def utility(R_i, index):
    return math.log(int(R_i[index][0]) / int(R_i[0][0]), math.e)

def objective(V_D, index, R_i, buf_level):
    return (V_D * utility(R_i, index) + V_D * 5 - buf_level) / R_i[index][1]

def optimalM(V_D, R_i, buf_level):
    M = len(R_i)
    res = 0
    cond = objective(V_D, 0, R_i, buf_level)
    for m in range(M):
        obj = objective(V_D, m, R_i, buf_level)
        if obj > cond:
            res = m
            cond = obj
    return res

def BOLA(rate_prev, Chunk, Buffer_Occupancy, R_i, Previous_Throughput):
    global last_optimal_m
    # print(type(rate_prev))
    rate_prev = prevmatch(rate_prev, R_i)[0]
    # t = min(playtime from begin, playtime to end) in second
    t = min(int(Chunk['current']) * Chunk['time'], Chunk['left'] * Chunk['time'])
    # t' = max(t/2, 3 * segment time)
    t_prime = max(t / 2, 3 * Chunk['time'])
    # The paper measures the buffer in seconds
    # However, in this system, the buffer is messured in bytes
    # My proposed solution: estimate the buffer size Q_max in segments as:
    # downloaded video time / segment time + remaining space / (previous bitrate / 8) / segment time
    # print(type(rate_prev))
    Q_max = Buffer_Occupancy['time'] / Chunk['time'] + (Buffer_Occupancy['size'] - Buffer_Occupancy['current']) / match(rate_prev, R_i) / Chunk['time']
    # dynamic buffer size: Q_D_max = min(Q_max, t_prime / segment time)
    Q_D_max = min(Q_max, t_prime / Chunk['time'])
    # The paper uses ln(current bitrate / min bitrate) to measure the utility
    v_M = utility(R_i, -1)
    # dynamic control parameter: V_D = (Q_D_max - 1) / performace metric (= utility + gamma * segment size)
    # The paper uses gamma * segment size = 5
    V_D = (Q_D_max - 1) / (v_M + 5)
    # m_optimal = arg max(V_D * v_m + V_D * gamma * segment time - buffer level) / segment size
    m_optimal = optimalM(V_D, R_i, Buffer_Occupancy['time'] / Chunk['time'])
    # print("rate_prev = ", rate_prev)
    # print("t = ", t)
    # print("t_prime = ", t_prime)
    # print("Q_max = ", Q_max)
    # print("Q_D_max = ", Q_D_max)
    # print("v_M = ", v_M)
    # print("V_D = ", V_D)
    # print("m_optimal = ", m_optimal)
    # print("==================================================")
    if m_optimal > last_optimal_m:
        r = Previous_Throughput / 8
        m_prime = 0
        for index in range(len(R_i)-1, -1, -1):
            # print(R_i[index][1], " vs. ", max(r, R_i[0][1]))
            if R_i[index][1] <= max(r, R_i[0][1]):
                m_prime = index
                break    
        # print("m_prime 1 = ", m_prime)               
        if m_prime >= m_optimal:
            m_prime = m_optimal
        elif m_prime < last_optimal_m:
            m_prime = last_optimal_m
        # print("m_prime 1 = ", m_prime)    
        m_optimal = m_prime
    last_optimal_m = m_optimal    
    return R_i[m_optimal][0]
