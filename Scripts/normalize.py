import os
import fnmatch
import sys
from multiprocessing import Pool, TimeoutError
import subprocess

if len(sys.argv) < 3:
    print("Usage:\n{} binary-dir db-dir vertices".format(sys.argv[0]))
    exit()

files= list()
print("Looking for files in", sys.argv[2])
for dirName, subdirList, fileList in os.walk(sys.argv[2]):
    print('Found directory: %s', dirName)
    for fname in fileList:
        if fnmatch.fnmatch(fname,"*.off") or fnmatch.fnmatch(fname, "*.ply"):
            files.append(os.path.abspath(os.path.join(dirName, fname)))

def normalize(mesh):
    print("Normalizing", mesh)
    subprocess.call([sys.argv[1], mesh, sys.argv[3]])
    print("Done with", mesh)

if __name__ == '__main__': 
    with Pool(processes=8) as pool:
        pool.map(normalize, files)
