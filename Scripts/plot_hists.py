import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import csv
import sys

if len(sys.argv) < 3:
    print("Usage:\n{} csv-file feature-idx".format(sys.argv[0]))
    exit()

classes=['Airplane','Ant','Armadillo','Bearing','Bird','Bust','Chair','Cup','Fish','FourLeg','Glasses','Hand','Human','Mech','Octopus','Plier','Table','Teddy','Vase']
features=["Path","3D_Area","3D_MVolume","3D_BBVolume","3D_Diameter","3D_Compactness","3D_Eccentricity","3D_A3","3D_D1","3D_D2","3D_D3","3D_D4"]

feature = ""
featureIdx = 0
bins = np.arange(0.0,1.0,0.1)

for i in range(0, len(features)):
    if features[i] == sys.argv[2]:
        featureIdx = i
        feature = features[i]
        break
if i == -1:
    print("Invalid feature name, use one of the following: \n{}".format(features))
    exit()

with open(sys.argv[1]) as csvfile:
    reader = csv.reader(csvfile, delimiter=',')
    fig = plt.figure()
    titles = False
    a = 1
    for className in classes:
        print("{} Plotting for class: {}".format(a, className))
        fig.add_subplot(4, 5, a)
        a = a+1
        csvfile.seek(0)
        for row in reader:
            if not titles:
                titles = True
            if(className in row[0]):
                values = []
                feat = row[featureIdx].split(":")
                for j in range(0, 10):
                    values.append(float(feat[j]))
                plt.title("Class: {}".format(className),fontdict={'fontsize': 12, 'fontweight': 'medium'})
                # plt.ylim(0, 0.5)
                plt.xlim(0,1.0)
                plt.plot(bins, values)

fig.suptitle(feature)
fig.subplots_adjust(
    top=0.937,
    bottom=0.035,
    left=0.024,
    right=0.991,
    hspace=0.296,
    wspace=0.175
)


plt.show()
