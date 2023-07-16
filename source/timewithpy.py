
import subprocess
import matplotlib.pyplot as plt
import multiprocessing

def calculate_parallel_speedup(serial_tuple, list_of_tuples):
    """
    Calculates the speedup of a serial tuple compared to a list of tuples
    returns a list of tuples with the same format as the input
    """
    serial_time = float(serial_tuple[1])
    return [(int(core), (serial_time / float(time))) for core, time in list_of_tuples]

def run_mpi_dlx(processes, filename, function, granularity):
    cmd = "mpirun -np "+str(processes)+" ./dlx_mpi.out " + filename + " " + str(function) + " " + str(granularity)
    r = subprocess.run(cmd, stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True, encoding="utf-8")
    return r

def calculate_speedup(serial_time, parallel_time):
    return serial_time / parallel_time

filename = "puzzles/puzzledump1.txt"
num_cores = multiprocessing.cpu_count()/2

func0 = []
func1 = []
func0speedup = []
func1speedup = []
serial = 0

proc = 1
prev = 0

for func in range(0,2):
    for gran in [1,2,4,6,8,10]:
        print ("Running with function " + str(func))
        proc = 1
        while proc <= num_cores:
            print ("Running with " + str(proc) + " processes")
            if proc == 1 and func == 0 and gran == 1:
                inner_list = []
                for i in range(0, 10):
                    r = run_mpi_dlx(proc, filename, 2, gran)
                    inner_list.append((proc, r.stdout.split("\n")[0]))
                #average the inner list
                avg = sum([float(i[1]) for i in inner_list]) / len(inner_list)
                serial = avg

            elif proc >= 2:
                inner_list = []
                for i in range(1, 10):
                    r = run_mpi_dlx(proc, filename, func, gran)
                    inner_list.append((proc, r.stdout.split("\n")[0]))

                #average the inner list
                avg = sum([float(i[1]) for i in inner_list]) / len(inner_list)
                if func == 0:
                    func0.append(((proc, avg), gran))
                else:
                    func1.append(((proc, avg), gran))

            if proc == 1:
                proc = 2
            elif proc == int(num_cores-2):
                proc += 2
            else:
                proc += 4

"""
list format is [((proc, time) gran), ...]
"""

for (proc, time), gran in func0:
    func0speedup.append((proc, calculate_speedup(serial, time)), gran)

for (proc, time), gran in func1:
    func1speedup.append((proc, calculate_speedup(serial, time)), gran)


"""
file1 = open("func0speedup.txt", "r")
file2 = open("func1speedup.txt", "r")

func1speedup = []
func0speedup = []

for line in file1:
    func0speedup.append((int(line.split(" ")[0]), float(line.split(" ")[1])))
for line in file2:
    func1speedup.append((int(line.split(" ")[0]), float(line.split(" ")[1])))

func0speedup.sort(key=lambda x: x[0])
func1speedup.sort(key=lambda x: x[0])

func0efficiency = [(proc, speedup / proc) for proc, speedup in func0speedup]
func1efficiency = [(proc, speedup / proc) for proc, speedup in func1speedup]

fileef1 = open("func0efficiency.txt", "w")
fileef2 = open("func1efficiency.txt", "w")

for proc, eff in func0efficiency:
    fileef1.write(str(proc) + " " + str(eff) + "\n")

for proc, eff in func1efficiency:
    fileef2.write(str(proc) + " " + str(eff) + "\n")
"""


file1 = open("func0.txt", "w")
file2 = open("func1.txt", "w")
file3 = open("func0speedup.txt", "w")
file4 = open("func1speedup.txt", "w")

"""
list format is [(proc, time) gran, (proc, time) gran, ...]
"""

for (proc, time), gran in func0:
    file1.write(str(proc) + " " + str(time) + " " + str(gran) + "\n")

for (proc, time), gran in func1:
    file2.write(str(proc) + " " + str(time) + " " + str(gran) + "\n")

for (proc, speedup), gran in func0speedup:
    file3.write(str(proc) + " " + str(speedup) + " " + str(gran) + "\n")

for (proc, speedup), gran in func1speedup:
    file4.write(str(proc) + " " + str(speedup) + " " + str(gran) + "\n")








