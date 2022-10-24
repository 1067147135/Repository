# calculate part of gini
def gini_single(a,b):
    gini = 1 - ((a/(a+b))**2) - ((b/(a+b))**2)
    return gini

# calculate gini by parts
def gini_index(a,b,c,d):
    left = gini_single(a,b)
    right = gini_single(c,d)
    gini = left*((a+b)/(a+b+c+d)) + right*((c+d)/(a+b+c+d))
    return gini

# find the best way to divide, return gini number, boundary, two subsets and subsets' yes/no
def find_criterion(dataset,column):
    sorting = []
    for item in dataset:
        key = float(item[column])
        value = item['quality']
        sorting.append((key,value))
    sorting.sort()
    length = len(sorting)
    boundary_final = 0
    gini_final = 1
    type1_yes_final = 0
    type1_no_final = 0
    type2_yes_final = 0
    type2_no_final = 0
    for i in range(length-1):   # calculate gini number
        type1_yes = 0
        type1_no = 0
        type2_yes = 0
        type2_no = 0
        boundary_try = (float(sorting[i][0])+float(sorting[i+1][0]))/2
        if boundary_try == float(sorting[i][0]) or boundary_try == float(sorting[i+1][0]):
            continue
        for j in range(i+1):    
            if float(sorting[j][1]) > 6:
                type1_yes += 1
            else:
                type1_no += 1
        for k in range(i+1,length):
            if float(sorting[k][1]) > 6:
                type2_yes += 1
            else:
                type2_no += 1
        gini_try = gini_index(type1_yes,type1_no,type2_yes,type2_no)
        if gini_try < gini_final:   #The smaller the gini number, the better the dividing line
            gini_final = gini_try
            boundary_final = boundary_try
            type1_yes_final = type1_yes
            type1_no_final = type1_no
            type2_yes_final = type2_yes
            type2_no_final = type2_no
    new_dataset1 = []   # two subsets of train set
    new_dataset2 = []
    for item in dataset:
        if float(item[column]) < boundary_final:
            new_dataset1.append(item)
        else:
            new_dataset2.append(item)
    return gini_final, boundary_final, type1_yes_final, type1_no_final, type2_yes_final, type2_no_final, new_dataset1, new_dataset2

# code for node 
class Node:
    def __init__(self,element,parent = None,left = None,right = None):
        self.element = element
        self.parent = parent
        self.left = left
        self.right = right

# code for binary tree
class binary_tree:
    def __init__(self):
        self.root = None
        self.size = 0

    def __len__(self):
        return self.size

    def find_root(self):
        return self.root

    def parent(self,node):
        return node.parent

    def left(self,node):
        return node.left

    def right(self,node):
        return node.right

    def num_child(self,node):
        count = 0
        if node.left is not None:
            count += 1
        if node.right is not None:
            count += 1
        return count

    def add_root(self,e):
        if self.root is not None:
            print('Root already exists.')
            return None
        self.size = 1
        self.root = Node(e)
        return self.root

    def add_left(self,node,e):
        if node.left is not None:
            print('Left child already exists.')
            return None
        self.size += 1
        node.left = Node(e,node)
        return node.left

    def add_right(self,node,e):
        if node.right is not None:
            print('Right child already exists.')
            return None
        self.size += 1
        node.right = Node(e,node)
        return node.right

    def replace(self,node,e):
        old = node.element
        node.element = e
        return old

    def delete(self,node):
        if node.parent.left is node:
            node.parent.left = None
        if node.parent.right is node:
            node.parent.right = None    
        return node.element

# code to make tree
def make_tree(dataset, yes, no, gini, accuracy):
    global columns
    global node
    if (len(dataset) == yes) or (len(dataset) == no):   # end condition 1: all tags are the same
        return node
    if gini <= 0.1:    # end condition 2: gini index is less than threshold value
        return node
    if len(dataset) <= 80:  # end condition 3: the number of samples in the node is less than the threshold value
        return node
    compare = []    # search for bifurcation criteria
    for column in columns: 
        compare.append((find_criterion(dataset,column),column))
    compare.sort()    
    left_side = []  # [column_name, < boundary, data_set1, gini, yes1, no1, tag, accuracy1]
    left_side.append(compare[0][1])
    left_side.append(compare[0][0][1])
    left_side.append(compare[0][0][6])
    left_side.append(compare[0][0][0])
    left_side.append(compare[0][0][2])
    left_side.append(compare[0][0][3])
    if compare[0][0][2] >= compare[0][0][3]:
        left_side.append('Yes')
    else:
        left_side.append('No')
    right_side = [] # [column_name, > boundary, data_set2, gini, yes2, no2, tag, accuracy2]
    right_side.append(compare[0][1])
    right_side.append(compare[0][0][1])
    right_side.append(compare[0][0][7])
    right_side.append(compare[0][0][0])
    right_side.append(compare[0][0][4])
    right_side.append(compare[0][0][5])
    if compare[0][0][4] >= compare[0][0][5]:
        right_side.append('Yes')
    else:
        right_side.append('No')
    the_correct = 0 # measure accuracy based on training set
    the_wrong = 0
    if left_side[6] == 'Yes':
        the_correct += left_side[4]
        the_wrong += left_side[5]
        accuracy1 = left_side[4] / (left_side[4] + left_side[5])
    else:
        the_correct += left_side[5]
        the_wrong += left_side[4]
        accuracy1 = left_side[5] / (left_side[4] + left_side[5])
    if right_side[6] == 'Yes':
        the_correct += right_side[4]
        the_wrong += right_side[5]
        accuracy2 = right_side[4] / (right_side[4] + right_side[5])
    else:
        the_correct += right_side[5]
        the_wrong += right_side[4]
        accuracy2 = right_side[5] / (right_side[4] + right_side[5])
    accuracy_now = the_correct / (the_correct + the_wrong)
    if accuracy >= accuracy_now:    # end condition 4: accuracy decreases after bifurcation
        return node
    left_side.append(accuracy1)
    right_side.append(accuracy2)
    left = left_side[:] # copy data to prevent subsequent processes from affecting the node data
    right = right_side[:]
    tree.add_left(node,left)    # The above four conditions are not met, execute the bifurcation
    tree.add_right(node,right)    
    node = node.left
    print('working...')
    make_tree(node.element[2], node.element[4], node.element[5], node.element[3], node.element[7])  # use a recursive loop to build a tree
    node = node.parent
    node = node.right
    print('working...')
    make_tree(node.element[2], node.element[4], node.element[5], node.element[3], node.element[7])
    node = node.parent
    return node

# main function

# data entry
print('data entry...')
train_data = open('train.csv','r')
line = train_data.readline()
title = line.split(', ')
title[-1] = title[-1][:-1]
data_list = []
while True:
    line = train_data.readline()
    if line:
        data = line.split(', ')
        if data[-1][-1] == '\n':
            data[-1] = data[-1][:-1]
        data_list.append(dict(zip(title[:],data[:])))   # creat a training set
    else:
        break
train_data.close()
print('finish!')

# make tree
print('make tree...')
columns = title[:-1]
tree = binary_tree()
node = tree.add_root(data_list)

make_tree(data_list, 0, 0, 1, 0)
print('finish!')

# test code
print('test...')
test_data = open('test.csv','r')    # test data entry
line = test_data.readline()
data_list_test = []
while True:
    line = test_data.readline()
    if line:
        data = line.split(', ')
        if data[-1][-1] == '\n':
            data[-1] = data[-1][:-1]
        data_list_test.append(dict(zip(title[:],data[:])))
    else:
        break
test_data.close()
correct = 0
total = 0
for test_item in data_list_test:    # follow the tree's judgment to find the answer
    while tree.num_child(node):
        if float(test_item[node.left.element[0]]) < node.left.element[1]:
            node = node.left
        else:
            node = node.right
    if ((node.element[6] == 'Yes') and (int(test_item['quality']) > 6)) or ((node.element[6] == 'No') and (int(test_item['quality']) <= 6)): 
        correct += 1
    node = tree.root
    total += 1
accuracy = correct / total  # calculate the accuracy
print('the accuracy is ',accuracy)