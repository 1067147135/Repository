import collections
import csv

class DecisionTree:
    def __init__(self, col=-1, value=None, Branch1=None, Branch2=None, results=None):
        self.col = col
        self.value = value
        self.Branch1 = Branch1
        self.Branch2= Branch2
        self.results = results 

def divideSet(rows, column, value):
    splitting = None
    if isinstance(value, int) or isinstance(value, float):
        splitting =lambda row:row[column]>= value
    else:
        splitting =lambda row:row[column] == value
    list1 = [row for row in rows if splitting(row)]
    list2 = [row for row in rows if not splitting(row)]
    return (list1, list2)

def uniqueCounts(rows):
    results = {}
    for row in rows:
        r = row[-1]
        if r not in results:
            results[r] = 0
        results[r] += 1
    return results

def gini(rows):
    total = len(rows)
    counts = uniqueCounts(rows)
    imp = 0.0
    for k1 in counts:
        p1 = float(counts[k1])/total  
        for k2 in counts:
            if k1 == k2:
                continue
            p2 = float(counts[k2])/total
            imp += p1*p2
    return imp

def var(rows):
    if len(rows) == 0:
        return 0
    data = [float(row[len(row) - 1]) for row in rows]
    mean = sum(data) / len(data)
    var = sum([(d-mean)**2 for d in data]) / len(data)
    return var

def growTree(rows):
    if len(rows) == 0:
        return DecisionTree()
    currentScore = gini(rows)
    bestGain = 0.0
    bestAttribute = None
    bestSets = None
    columnCount = len(rows[0]) - 1 
    for col in range(0, columnCount):
        columnValues = [row[col] for row in rows]
        for value in columnValues:
            (set1, set2) = divideSet(rows, col, value)
            p = float(len(set1)) / len(rows)
            gain = currentScore - p*gini(set1) - (1-p)*gini(set2)
            if gain>bestGain and len(set1)>0 and len(set2)>0:
                bestGain = gain
                bestAttribute = (col, value)
                bestSets = (set1, set2)
    if bestGain > 0:
        Branch1 = growTree(bestSets[0])
        Branch2 = growTree(bestSets[1])
        return DecisionTree(col=bestAttribute[0], value=bestAttribute[1], Branch1=Branch1, Branch2=Branch2)
    else:
        return DecisionTree(results=uniqueCounts(rows))

def prune(tree, minGain,notify=False):
    if tree.Branch1.results ==None:
        prune(tree.Branch1, minGain, notify)
    if tree.Branch2.results == None:
        prune(tree.Branch2, minGain, notify)
    if tree.Branch1.results != None and tree.Branch2.results != None:
        tb, fb = [], []
        for v, c in tree.Branch1.results.items():
            tb += [[v]] * c
        for v, c in tree.Branch2.results.items():
            fb += [[v]] * c
        p = float(len(tb)) / len(tb + fb)
        delta = gini(tb+fb) - p*gini(tb) - (1-p)*gini(fb)
        if delta < minGain:	
            if notify: print('A branch was pruned: gain = %f' % delta)		
            tree.Branch1, tree.Branch2 = None, None
            tree.results = uniqueCounts(tb + fb)

def getdata(file):
    def convertTypes(s):
        s = s.strip()
        try:
            return float(s) if '.' in s else int(s)
        except ValueError:
            return s	
    reader = csv.reader(open(file, 'rt'))
    next(reader)
    return [[convertTypes(item) for item in row] for row in reader]

trainingData = getdata('train.csv') 
decisionTree = growTree(trainingData)		
prune(decisionTree, 0.5,notify=True) 
