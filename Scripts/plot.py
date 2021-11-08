import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys

plt.rcParams.update({'font.size': 28})

if len(sys.argv) < 3:
    print("Usage:\n{} csv-file column-idx".format(sys.argv[0]))
    exit()
colIdx = int(sys.argv[2])
data = pd.read_csv(sys.argv[1], quoting=2)
colsName = list(data)
print("Plotting " + colsName[colIdx])
data = data[colsName[colIdx]]
# data.hist(bins=np.arange(-0.5, 1.5, 0.2))
data.hist(bins=np.arange(0, 1, 0.05))
plt.title("Data")
plt.ylabel("Frequency")
plt.xlabel(colsName[colIdx])
plt.show()
