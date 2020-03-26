import numpy as np
import argparse


parser = argparse.ArgumentParser()
parser.add_argument('-n', help="N dimension of the output matrix", type=int)
parser.add_argument('-m', help="M dimension of the output matrix", type=int)
parser.add_argument('--output', help="Output file path where the matrix will be placed", default="out.csv")
parser.add_argument('--lower', help='Low limit of the values possible in matrix', type=float, default=0.0)
parser.add_argument('--higher', help='High limit of the values possible in matrix', type=float, default=150.0)
args = parser.parse_args()

n = args.n
m = args.m
lower = args.lower
higher = args.higher
output = args.output

int_random = np.random.randint(lower, higher, (n, m))
double_random = np.random.rand(n, m)
finished_matrix = int_random * double_random
finished_matrix.tofile(output, sep=';')
