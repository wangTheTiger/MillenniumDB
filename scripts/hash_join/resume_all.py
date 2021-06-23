import sys

folder= sys.argv[1]
alg_names = [
    "quad",
    "grace_murmur",
    "new_buffer_farmhash",
    #"grace_clhash",
    #"grace_cityhash",
    "grace_farmhash",
    "memory_murmur",
    #"memory_clhash",
    #"memory_cityhash",
    "memory_farmhash",
    "buffer_murmur",
    #"buffer_clhash",
    #"buffer_cityhash",
    "buffer_farmhash"
    ]

all_tests = {}
for alg_name in alg_names:
    with open(f"{folder}/resumen_{alg_name}.txt") as file:
        all_tests[alg_name] = [test for test in file.readlines()]

# use first alg as reference
first_alg = alg_names[0]
cantidad_tests = len(all_tests[first_alg])

final_lines = ["test_name,"+",".join(alg_names)+",results"]
for test in range(cantidad_tests):
    test_name, time, results = all_tests[first_alg][test].strip().split(",")
    times = [time]
    for alg in alg_names[1:]:
        test_name2, time, results2 = all_tests[alg][test].strip().split(",")
        if test_name2 != test_name:
            test_name = f"{alg} MADE DIFFERENT TESTS"
        times.append(time)
        if results2 != results:
            results = f"{alg} HAVE DIFFERENT RESULTS"
    times = [str(round(float(x), 6)) for x in times]
    test_name = test_name.split("\\")[-1]
    test_name = test_name.split("/")[-1]
    new_line = f"{test_name}," + ",".join(times) + f",{results}"
    final_lines.append(new_line)

for line in final_lines:
    print(line)
