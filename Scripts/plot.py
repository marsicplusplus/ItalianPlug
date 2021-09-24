import matplotlib.pyplot as plt
import pandas as pd
import sys

if len(sys.argv) < 3:
    print("Usage:\n{} csv-file column-idx".format(sys.argv[0]))
    exit()
colIdx = int(sys.argv[2])
data = pd.read_csv(sys.argv[1], quoting=2)
colsName = list(data)
print(colsName[colIdx])
data = data[colsName[colIdx]]
data.hist(bins=200)
plt.title("Data")
plt.ylabel("Frequency")
plt.xlabel(colsName[colIdx])
plt.show()
