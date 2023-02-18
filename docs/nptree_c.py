import sys
import re

if len(sys.argv) <= 1:
    msg = \
    """
    usage: {0} FILE
    Converts a dump of the tree structure inside biomeEntries, obtained with
    the debugger of IntelliJ IDEA, into a C representation. Currently setup
    the script assumes a tree of order 6, used by 1.19.3 - 1.20.
    """.format(sys.argv[0])
    print(msg)
    sys.exit(0)

in_file = sys.argv[1]

with open(in_file) as f:
    lines = f.readlines()


def torange(x):
    if x[0] == '[':
        m = re.search(r'(-?[0-9]+)-(-?[0-9]+)', x)
        return int(m.group(1)), int(m.group(2))
    if x[-1] == ']':
        x = x[:-1]
    return int(x), int(x)

"""

typedef struct Node Node;
struct Node
{
    short range[12];
    short inner[10];
    short biome;
} 

"""

class Node:
    def __init__(self):
        self.range = None
        self.inner = []
        self.biome = None
        self.id = 0

    def set(self, r, b, idx):
        if len(idx) > 0:
            i = idx[0]
            while i >= len(self.inner):
                self.inner.append(Node())
            self.inner[i].set(r, b, idx[1:])
        else:
            if r:
                self.range = r
            if b:
                self.biome = b
   
    def dump(self, depth = 0):
        print ('{} : {}'.format(self.range, self.biome))
        for i,n in enumerate(self.inner):
            print ('{}[{}]:'.format('   '*depth, i), end='')
            n.dump(depth+1)

    def gen_ids(self, num):
        self.id = num
        num += 1
        for n in self.inner:
            num = n.gen_ids(num)
        return num

    def dump_c(self):
        s = '/*{}*/{{'.format(self.id)
        if self.range:
            s += '{' + ','.join(['{},{}'.format(r[0], r[1]) for r in self.range[:-1]]) + '},'
        else:
            s += '{},'
        s += '{' + ','.join(['{}'.format(n.id) for n in self.inner]) + '},'
        if self.biome:
            s += self.biome + '},'
        else:
            s += 'none},'
        print (s)
        for n in self.inner:
            n.dump_c()


tree = Node()
idx = [0,0,0,0,0]
lo = -1


while lo+1 < len(lines):
    lo += 1
    m = re.search(r'([ ]+)([0-9]+) = {MultiNoiseUtil[^\[]+\[([0-9\-\[\]]+), ([0-9\-\[\]]+), ([0-9\-\[\]]+), ([0-9\-\[\]]+), ([0-9\-\[\]]+), ([0-9\-\[\]]+), ([0-9\-\[\]]+)', lines[lo])
    if not m:
        continue
    dim = (len(m.group(1))-1) // 2
    idx[dim] = int(m.group(2))
    r = [torange(m.group(i)) for i in [3,4,5,6,7,8,9]]
    b = re.search(r'worldgen/biome / minecraft:([a-z_]+)', lines[lo+1])
    if b:
        b = b.group(1)
    tree.set(r, b, idx[:(dim+1)])

tree.gen_ids(0)
tree.dump_c()
print()






