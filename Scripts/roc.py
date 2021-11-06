import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from scipy.interpolate import splrep, splev
from scipy.integrate import simps
import sys

if len(sys.argv) < 2:
    print("Usage:\n{} roc1.csv [roc2.csv roc3.csv...]".format(sys.argv[0]))
    exit()

rocs = sys.argv[1:]

fig = plt.figure()
ax = fig.add_subplot(111)
ax.set_aspect('equal', adjustable='box')

for roc in rocs:
    data = pd.read_csv(roc, quoting = 2)
    # poly = np.polyfit(data["Recall"],data["Specificity"],5)
    # poly_y = np.poly1d(poly)(data["Recall"])
    # plt.plot(data["Recall"], poly_y, label = roc.split("/")[-1])
    bspl = splrep(data["Recall"],data["Specificity"],s=5)
    bspl_y = splev(data["Recall"], bspl)
    plt.plot(data["Recall"], bspl_y, label = roc.split("/")[-1])
    plt.xlabel("Recall")
    plt.ylabel("Specificity")
    print(f'AUROC for {roc.split("/")[-1]}: {simps(data["Specificity"])}')
    plt.legend()

plt.plot([0,1],[1,0], 'y--')

plt.show()
