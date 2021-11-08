import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import csv
import sys

if len(sys.argv) < 3:
    print("Usage:\n{} csv-file feature-name".format(sys.argv[0]))
    exit()

classes=['Airplane','Ant','Armadillo','Bearing','Bird','Bust','Chair','Cup','Fish','FourLeg','Glasses','Hand','Human','Mech','Octopus','Plier','Table','Teddy','Vase']
features=["Path","3D_Area","3D_MVolume","3D_BBVolume","3D_Diameter","3D_Compactness","3D_Eccentricity","3D_A3","3D_D1","3D_D2","3D_D3","3D_D4"]

if not (sys.argv[2] in features):
    print("Invalid feature name, use one of the following: \n{}".format(features))
    exit()

fig = plt.figure()
data = pd.read_csv(sys.argv[1], quoting=2)
a = 1
for className in classes:
    fig.add_subplot(4, 5, a)
    a = a+1
    classRows = data[data["Path"].str.contains(className)]
    classRows[sys.argv[2]].hist(bins=80, range=(data[sys.argv[2]].min(),data[sys.argv[2]].max()))
    plt.title(f"Class: {className}")
    plt.xlim(data[sys.argv[2]].min(),data[sys.argv[2]].max())
    plt.ylim(0,20)

fig.suptitle(sys.argv[2])
fig.subplots_adjust(
    top=0.937,
    bottom=0.035,
    left=0.024,
    right=0.991,
    hspace=0.296,
    wspace=0.175
)


plt.show()
