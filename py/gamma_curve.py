import numpy as np 
import math as mt
from matplotlib import pyplot as plt


pwr = 1.0 / 2.2
ts = 0.0
imax = 0xffff
mode = 2
g = [0.0,0.0,0.0,0.0,0.0,0.0]
bnd = [0.0, 0.0]
r = 0.0
curve = np.zeros(0x10000)

g[0] = pwr
g[1] = ts
bnd[g[1] >= 1] = 1
if g[1] and (g[1] - 1) * (g[0] - 1) <= 0:
    for i in range(0, 48):
        g[2] = (bnd[0] + bnd[1]) / 2
        if g[0]:
            bnd[((mt.pow(g[2] / g[1], -1 * g[0]) - 1) / g[0]) - (1 / g[2]) > -1] = g[2]
        else:
            bnd[g[2] / mt.exp(1 - 1 / g[2]) < g[1]] = g[2]
    
    g[3] = g[2] / g[1]
    if g[0]:
        g[4] = g[2] * (1 / g[0] - 1)

if g[0]:
    g[5] = 1 / (g[1] * (g[3] * g[3]) / 2 - g[4] * (1 - g[3]) + 
                (1 - mt.pow(g[3], 1 + g[0])) * (1 + g[4]) / (1 + g[0])) - 1
else:
    g[5] = 1 / (g[1] * (g[3] * g[3]) / 2 + 1 - g[2] - g[3] - g[2] * g[3] * mt.log(g[3] - 1)) - 1

for i in range(0, 0x10000):
    curve[i] = 0xffff
    r = i / imax
    if r < 1:
        if r < g[3]:
            __temp = r * g[0]
        else:
            if g[0]:
                __temp = mt.pow(r, g[0]) * (1 + g[4]) - g[4]
            else:
                __temp = mt.log(r) * g[2] + 1
        
        curve[i] = 0x10000 * __temp

plt.plot(np.arange(0, 256), curve[:256])
plt.grid()
plt.show()