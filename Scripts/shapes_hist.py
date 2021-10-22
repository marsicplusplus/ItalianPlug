import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import csv
import sys

if len(sys.argv) < 4:
    print("Usage:\n{} csv feature-idx shape-1 [shape-2 ... shape-N]".format(sys.argv[0]))
    exit()

features=["Path","3D_Area","3D_MVolume","3D_BBVolume","3D_Diameter","3D_Compactness","3D_Eccentricity","3D_A3","3D_D1","3D_D2","3D_D3","3D_D4"]

feature = ""
featureIdx = -1
for i in range(0, len(features)):
    if features[i] == sys.argv[2]:
        featureIdx = i
        feature = features[i]
        break
if i == -1:
    print("Invalid feature name, use one of the following: \n{}".format(features))
    exit()
shapes = []

for i in range(3, len(sys.argv)):
    shapes.append(sys.argv[i])

with open(sys.argv[1]) as csvfile:
    reader = csv.reader(csvfile, delimiter=',')
    for row in reader:
        for shape in shapes:
            if(shape in row[0]):
                hist = {'n_bins':0,'bin_width':0,'bins':[], 'values':[]}
                feat = row[featureIdx].split(":")
                hist['n_bins'] = int(feat[0])
                hist['bin_width'] = float(feat[1])
                for i in range(2, hist['n_bins']+2):
                    hist['bins'].append(float(feat[i]))
                for j in range(i+1, i+hist['n_bins']+1):
                    hist['values'].append(float(feat[j]))
                plt.ylim(0, 0.5)
                plt.xlim(0, 1.0)
                plt.plot(hist['bins'], hist['values'], label = row[0])

plt.title(sys.argv[2])

plt.legend(loc="upper right")
plt.show()
