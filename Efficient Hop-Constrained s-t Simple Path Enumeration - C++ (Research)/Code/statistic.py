fc1 = open("standard.txt")
fc2 = open("compare.txt")
standard = []
compare = []
for i in range(10):
  tmp1 = fc1.readline()
  tmp2 = fc2.readline()
  # print(tmp1)
  # print(tmp2)
count = 500

try:
  fc2.readline()
  for i in range(count):
    s = fc1.readline()
    c = fc2.readline()
    s_data = s.split()
    # print(s_data)
    standard.append(float(s_data[-2]))  # -float(s_data[-6])
    c_data = c.split()
    # print(c_data)
    compare.append(float(c_data[-2]))   # -float(c_data[-6])
    # count += 1
    print([standard[-1], compare[-1]])
    # fc.readline()
except:
  print("Data collecting finished.")

dfs_avg = 0
para_avg = 0
for i in range(count):
  dfs_avg += standard[i]
  para_avg += compare[i]
dfs_avg = dfs_avg / count
para_avg = para_avg / count

print("dfs avg = ", dfs_avg, ", parallel dfs avg = ", para_avg)
print("acceleration rate = ", dfs_avg / para_avg)