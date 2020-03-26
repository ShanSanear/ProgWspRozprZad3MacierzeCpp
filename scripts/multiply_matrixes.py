import numpy as np
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("matrixes", nargs=2)
parser.add_argument("output", nargs=1)
args = parser.parse_args()
matrixes = args.matrixes
output = args.output[0]
matrix_a = np.genfromtxt(matrixes[0], delimiter=';')
matrix_b = np.genfromtxt(matrixes[1], delimiter=';')
out = np.dot(matrix_a, matrix_b)
np.savetxt(output, out, fmt='%.5f', delimiter=';')
