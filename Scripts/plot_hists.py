import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import csv
import sys

if len(sys.argv) < 3:
    print("Usage:\n{} csv-file feature-idx".format(sys.argv[0]))
    exit()

classes=['Airplane','Ant','Armadillo','Bearing','Bird','Bust','Chair','Cup','Fish','FourLeg','Glasses','Hand','Human','Mech','Octopus','Plier','Table','Teddy','Vase']

feature = ""

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
                feature = row[int(sys.argv[2])]
                titles = True
            if(className in row[0]):
                hist = {'n_bins':0,'bin_width':0,'bins':[], 'values':[]}
                feat = row[int(sys.argv[2])].split(":")
                hist['n_bins'] = int(feat[0])
                hist['bin_width'] = float(feat[1])
                for i in range(2, hist['n_bins']+2):
                    hist['bins'].append(float(feat[i]))
                for j in range(i+1, i+hist['n_bins']+1):
                    hist['values'].append(float(feat[j]))
                plt.title("Class: {}".format(className),fontdict={'fontsize': 12, 'fontweight': 'medium'})
                # plt.ylim(0, 0.5)
                plt.xlim(0,1.0)
                plt.plot(hist['bins'], hist['values'])

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
