import sys
import re

if len(sys.argv) <= 1:
    msg = \
    """
    usage: {0} FILE
    Compresses the C biome tree into a 64-bit binary table where the bytes
    index the noise points in a secondary table. Biomes are converted to their
    numeric ID and are also stored inside the binary table.
    """.format(sys.argv[0])
    print(msg)
    sys.exit(0)

in_file = sys.argv[1]


bdic = dict()

auto_cnt = 0
def auto(n=None):
    global auto_cnt
    if n is not None:
        auto_cnt = n
    ret = auto_cnt
    auto_cnt += 1
    return ret

bdic['ocean'] = auto(0)
bdic['plains'] = auto()
bdic['desert'] = auto()
bdic['mountains'] = auto()
bdic['forest'] = auto()
bdic['taiga'] = auto()
bdic['swamp'] = auto()
bdic['river'] = auto()
bdic['nether_wastes'] = auto()
bdic['the_end'] = auto()
#    // 10
bdic['frozen_ocean'] = auto()
bdic['frozen_river'] = auto()
bdic['snowy_tundra'] = auto()
bdic['snowy_mountains'] = auto()
bdic['mushroom_fields'] = auto()
bdic['mushroom_field_shore'] = auto()
bdic['beach'] = auto()
bdic['desert_hills'] = auto()
bdic['wooded_hills'] = auto()
bdic['taiga_hills'] = auto()
#    // 20
bdic['mountain_edge'] = auto()
bdic['jungle'] = auto()
bdic['jungle_hills'] = auto()
bdic['jungle_edge'] = auto()
bdic['deep_ocean'] = auto()
bdic['stone_shore'] = auto()
bdic['snowy_beach'] = auto()
bdic['birch_forest'] = auto()
bdic['birch_forest_hills'] = auto()
bdic['dark_forest'] = auto()
#    // 30
bdic['snowy_taiga'] = auto()
bdic['snowy_taiga_hills'] = auto()
bdic['giant_tree_taiga'] = auto()
bdic['giant_tree_taiga_hills'] = auto()
bdic['wooded_mountains'] = auto()
bdic['savanna'] = auto()
bdic['savanna_plateau'] = auto()
bdic['badlands'] = auto()
bdic['wooded_badlands_plateau'] = auto()
bdic['badlands_plateau'] = auto()
#    // 40  --  1.13
bdic['small_end_islands'] = auto()
bdic['end_midlands'] = auto()
bdic['end_highlands'] = auto()
bdic['end_barrens'] = auto()
bdic['warm_ocean'] = auto()
bdic['lukewarm_ocean'] = auto()
bdic['cold_ocean'] = auto()
bdic['deep_warm_ocean'] = auto()
bdic['deep_lukewarm_ocean'] = auto()
bdic['deep_cold_ocean'] = auto()
#    // 50
bdic['deep_frozen_ocean'] = auto()

bdic['the_void'] = auto(127)

bdic['sunflower_plains']                = bdic['plains']+128
bdic['desert_lakes']                    = bdic['desert']+128
bdic['gravelly_mountains']              = bdic['mountains']+128
bdic['flower_forest']                   = bdic['forest']+128
bdic['taiga_mountains']                 = bdic['taiga']+128
bdic['swamp_hills']                     = bdic['swamp']+128
bdic['ice_spikes']                      = bdic['snowy_tundra']+128
bdic['modified_jungle']                 = bdic['jungle']+128
bdic['modified_jungle_edge']            = bdic['jungle_edge']+128
bdic['tall_birch_forest']               = bdic['birch_forest']+128
bdic['tall_birch_hills']                = bdic['birch_forest_hills']+128
bdic['dark_forest_hills']               = bdic['dark_forest']+128
bdic['snowy_taiga_mountains']           = bdic['snowy_taiga']+128
bdic['giant_spruce_taiga']              = bdic['giant_tree_taiga']+128
bdic['giant_spruce_taiga_hills']        = bdic['giant_tree_taiga_hills']+128
bdic['modified_gravelly_mountains']     = bdic['wooded_mountains']+128
bdic['shattered_savanna']               = bdic['savanna']+128
bdic['shattered_savanna_plateau']       = bdic['savanna_plateau']+128
bdic['eroded_badlands']                 = bdic['badlands']+128
bdic['modified_wooded_badlands_plateau'] = bdic['wooded_badlands_plateau']+128
bdic['modified_badlands_plateau']       = bdic['badlands_plateau']+128
#    // 1.14
bdic['bamboo_jungle']                   = 168
bdic['bamboo_jungle_hills']             = 169
#    // 1.16
bdic['soul_sand_valley']                = 170
bdic['crimson_forest']                  = 171
bdic['warped_forest']                   = 172
bdic['basalt_deltas']                   = 173
#    // 1.17
bdic['dripstone_caves']                 = 174
bdic['lush_caves']                      = 175
#    // 1.18
bdic['meadow']                          = 177
bdic['grove']                           = 178
bdic['snowy_slopes']                    = 179
bdic['jagged_peaks']                    = 180
bdic['frozen_peaks']                    = 181
bdic['stony_peaks']                     = 182
bdic['old_growth_birch_forest']         = bdic['tall_birch_forest']
bdic['old_growth_pine_taiga']           = bdic['giant_tree_taiga']
bdic['old_growth_spruce_taiga']         = bdic['giant_spruce_taiga']
bdic['snowy_plains']                    = bdic['snowy_tundra']
bdic['sparse_jungle']                   = bdic['jungle_edge']
bdic['stony_shore']                     = bdic['stone_shore']
bdic['windswept_hills']                 = bdic['mountains']
bdic['windswept_forest']                = bdic['wooded_mountains']
bdic['windswept_gravelly_hills']        = bdic['gravelly_mountains']
bdic['windswept_savanna']               = bdic['shattered_savanna']
bdic['wooded_badlands']                 = bdic['wooded_badlands_plateau']
#    // 1.19
bdic['deep_dark']                       = 183
bdic['mangrove_swamp']                  = 184
#    // 1.20
bdic['cherry_grove']                    = 185


with open(in_file) as f:
    lines = f.readlines()

ln = [re.sub(r'/\*[0-9]*\*/','',x) for x in lines]
ln = [re.sub(r'[\{\}]','',x) for x in ln]
ln = [re.sub(r',,',',',x)[:-1] for x in ln]
ln = [x for x in ln if len(x) > 0]
ln[0] = '0,0,0,0,0,0,0,0,0,0,0,0'+ln[0]

ln = [l[:-1].split(',') for l in ln]
np = [[(int(x[i]),int(x[i+1])) for i in range(0,12,2)] + x[12:] for x in ln]
nps = set()
for x in np:
    for n in x[:6]:
        nps.add(n)

nps = sorted(list(nps))
npdic = dict()
for i in range(len(nps)):
    npdic[nps[i]] = i

for i,n in enumerate(nps):
    s = '{{{:6d},{:6d}}}'.format(n[0],n[1])
    print(s,end=',')
    if i%4 ==3:
        print(' // {:02X}-{:02X}'.format(i-3, i))
print ('\n')

bp = []
for i,x in enumerate(np):
    p = ['{:02X}'.format(npdic[p]) for p in x[:6]]
    p.reverse()
    if len(x) == 7:
        da = 0xFF00 + bdic[x[6]]
    else:
        da = int(x[6])
    s = '0x'+'{:04X}'.format(da)+''.join(p)
    print(s,end=',')
    if i % 4 == 0:
        print()









