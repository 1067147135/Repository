class Node:
    def __init__(self,element,parent = None,left = None,right = None):
        self.element = element
        self.parent = parent
        self.left = left
        self.right = right

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

# Load training data: train.csv 
# Return each sample in training data and the features of data
def getData(num):
    if num == 1119:
        print('Start training for All data...')
    else:
        print('Start training for first',num,'data...')
    train_data = open('train.csv','r')
    line = train_data.readline()
    title = line.split(', ')
    title[-1] = title[-1][:-1]
    dataList = []
    n=0
    while n < num:
        line = train_data.readline()
        n += 1
        if line:
            data = line.split(', ')
            if data[-1][-1] == '\n':
                data[-1] = data[-1][:-1]
            data = list(map(eval, data)) 
            dataList.append(dict(zip(title[:],data[:])))
        else:
            break 
    train_data.close()
    return dataList,title[:-1]

# Get the Gini index for each dividing point of features
# By calling cut and getGini functions
# Return the feature name, dividing point, 2 subsets ,and the number of 2 types 
def nextBranch(newdata):
    diffGini = []
    for column in feature:
        trends = sorted(newdata, key = lambda e:e[column])
        length = len(trends)
        for e in range(length-1):
            val = (trends[e][column]+trends[e+1][column])/2
            d1,d2,var = cut(val,newdata,column)
            if e == 0:
                minvar = var
                bound = val
                gini,t1,t2 = getGini(d1,d2)  
                branch1,branch2 = d1,d2
            elif var < minvar:
                minvar = var
                bound = val
                gini,t1,t2 = getGini(d1,d2)  
                branch1,branch2 = d1,d2     
        diffGini.append((column,bound,gini,branch1,branch2,t1,t2))
    return sorted(diffGini,key=lambda t:t[2])[0]

# Divide domain into 2 sets based on the value of the point 
# Compute the total variance of 2 sets
# Return 2 sets and the total variance
def cut(point,domain,typ):
    set1,set2 = [],[]
    mean1,mean2 = [],[]
    var1,var2 = 0,0
    for i in range(len(domain)):
        if domain[i][typ] <= point:
            set1.append(domain[i])
            mean1.append(domain[i][typ])
        else:
            set2.append(domain[i])   
            mean2.append(domain[i][typ])     
    if mean1 != []: 
        for j in mean1:
            var1 += (j-sum(mean1)/len(mean1))**2
    if mean2 != []:
        for k in mean2:
            var2 += (k-sum(mean2)/len(mean2))**2
    return set1,set2,var1+var2

# Compute Gini index
# Return Gini index and the number of 2 types
def getGini(d1,d2):
    gini = []
    type1 = 0
    type2 = 0
    for s in [d1,d2]:
        label1,label2 = 0,0
        if len(s) == 0:
            prob = 0
        else:
            for h in s:
                if h['quality'] <= 6:
                    label1 += 1
                else:
                    label2 += 1
            type1 += label1
            type2 += label2
            prob = 1-(label1/len(s))**2-(label2/len(s))**2
        gini.append(prob)
    return (len(d1)/len(d1+d2))*gini[0]+(len(d2)/len(d1+d2))*gini[1],type1,type2

# Check different conditions for finding subtree or not
def trainTree(L=None,R=None,parent=None,numL=None,numR=None):
    global leave
    if numL == 0:
        leave += 1
    elif numR == 0:
        leave += 1
    else:
        if len(L) == 1:
            if L[0]['quality'] <= 6:
                myTree.add_left(parent,(parent.element[0],-1,0,1,0)) 
            else:
                myTree.add_left(parent,(parent.element[0],-1,0,0,1))
            leave += 1 
        else:
            addLeftBranch(L,parent)
        if len(R) == 1:
            if R[0]['quality'] <= 6:
                myTree.add_right(parent,(parent.element[0],-1,0,1,0)) 
            else:
                myTree.add_right(parent,(parent.element[0],-1,0,0,1))
            leave += 1 
        else:
            addRightBranch(R,parent)
    
# Add left subtree
def addLeftBranch(newdata,parent):
    label,divid,gini,left,right,nl,nr = nextBranch(newdata)
    condition = (label,divid,gini,nl,nr)
    p = myTree.add_left(parent,condition)
    return trainTree(left,right,p,nl,nr)

# Add right subtree 
def addRightBranch(newdata,parent):
    label,divid,gini,left,right,nl,nr = nextBranch(newdata)
    condition = (label,divid,gini,nl,nr)
    p = myTree.add_right(parent,condition)
    return trainTree(left,right,p,nl,nr)

# Load testing data: test.csv
def getTestData():
    test_data = open('test.csv','r')
    line = test_data.readline()
    title = line.split(', ')
    title[-1] = title[-1][:-1]
    dataList = []
    while True:
        line = test_data.readline()
        if line:
            data = line.split(', ')
            if data[-1][-1] == '\n':
                data[-1] = data[-1][:-1]
            data = list(map(eval, data)) 
            dataList.append(dict(zip(title[:],data[:])))
        else:
            break 
    test_data.close()
    return dataList

# Compute accuracy
def testT(data):
    print('Start testing...')
    incorrect = 0
    total = len(data)
    for test_item in data:
        node = myTree.find_root()   
        while myTree.num_child(node) != 0:
            label = node.element[0]
            bound = node.element[1]
            if test_item[label] <= bound:
                if node.left:
                    node = node.left
            else:
                node = node.right
        if node.element[-1] == 0 and test_item['quality'] <= 6:
            pass
        elif node.element[-2] == 0 and test_item['quality'] > 6:
            pass   
        else:
            incorrect += 1 
    accuracy = 1-incorrect/total
    print('The accuracy is',accuracy)
    return accuracy

testData = getTestData()
n = [100,200,300,400,500,600,700,800,900,1000,1119]
for i in n:
    dataSet,feature = getData(i)
    myTree = binary_tree()
    leave = 0
    label,divid,gini,L,R,numL,numR = nextBranch(dataSet)
    condition = (label,divid,gini,numL,numR)
    parent = myTree.add_root(condition)
    trainTree(L,R,parent,numL,numR)
    accuracy = testT(testData)
    print('The size of CART is',myTree.size,'\n')
print('-----Finished-----')