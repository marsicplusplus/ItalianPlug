import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from scipy.interpolate import splrep, splev
from numpy import trapz
import sys

data = pd.read_csv(sys.argv[1], quoting = 2)[['Class', 'MAP']].sort_values(by=['MAP'])
data.plot.barh(y='MAP', x='Class')

plt.show()
