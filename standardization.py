import numpy as np
import pandas as pd

data = pd.read_csv('feats_ordered_old.csv', quoting=2)
data = data.drop(columns=['Path', '3D_A3', '3D_D1', '3D_D2','3D_D3','3D_D4'])
mean = data.mean()
std = data.std()
f_data = (data-mean)/std
print(f_data.to_csv())
