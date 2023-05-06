import os
import subprocess
import sys

passes_dir = './src/lib/opt'
ll_files_dir = './unit_tests'
llvm_path = sys.argv[1]
alive_tv_binary = sys.argv[2]

# get the directory of the current file
dir_path = os.path.dirname(os.path.realpath(__file__))
# change the current working directory to the directory of the current file
os.chdir(dir_path)
os.makedirs('./tmp', exist_ok=True)


with open(f'{ll_files_dir}/entries.csv', 'r') as f:
    entries = [entry.split(',') for entry in f.read().strip().split('\n')[1:]]


failures = False

for entry in entries:
    sourcename, classname, passname, testname = entry
    print(f"== {passname} ==")
    pass_lib = f"lib{classname}.so"
    ll_files = [f for f in os.listdir(ll_files_dir) if f.startswith(testname) and f.endswith(".ll")]

    for ll_file in ll_files:
        ll_path = f"{ll_files_dir}/{ll_file}"

        print(f"Test file: \t{ll_path}")
        # Run opt with the pass shared library
        opt_cmd = [f"{llvm_path}/bin/opt", f"-load-pass-plugin=./build/{pass_lib}", f"-passes={passname}",
                   ll_path, "-S", "-o", f"./tmp/out.{ll_file}"]
        print(f"\t{' '.join(opt_cmd)}")
        subprocess.run(opt_cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Run filecheck on the output
        filecheck_cmd = [f"{llvm_path}/bin/FileCheck", ll_path]
        print(f"\t{' '.join(filecheck_cmd)}")
        with open(f'./tmp/out.{ll_file}', 'r') as f:
            result = subprocess.run(filecheck_cmd, stdin=f, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        if result.returncode != 0:
            print(result.stderr.decode("utf-8"))
            failures = True
        
        # Run alive2 validation on the output
        alive2_cmd = [alive_tv_binary, ll_path, f"./tmp/out.{ll_file}"]
        print(f"\t{' '.join(alive2_cmd)}")
        result = subprocess.run(alive2_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        if result.returncode != 0:
            print(result.stderr.decode("utf-8"))
            failures = True

# Exit with the appropriate exit code
if failures:
    print('Unit Test Failure.')
    sys.exit(1)
else:
    print('Unit Tests Successful!')
    sys.exit(0)
