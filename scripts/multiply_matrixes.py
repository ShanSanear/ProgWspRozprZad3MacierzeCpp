import numpy as np
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("matrixes", nargs=2)
parser.add_argument("output", nargs=1)
args = parser.parse_args()
matrixes = args.matrixes
output = args.output[0]
matrix_a = np.genfromtxt(matrixes[0], delimiter=';', skip_header=2)
matrix_b = np.genfromtxt(matrixes[1], delimiter=';', skip_header=2)
out = np.dot(matrix_a, matrix_b)
np.savetxt(output, out, fmt='%.5f', delimiter=';')
with open(output, 'r+') as file:
    content = file.read()
    file.seek(0, 0)
    file.write(f"{out.shape[0]}\n")
    file.write(f"{out.shape[1]}\n")
    file.write(content)
