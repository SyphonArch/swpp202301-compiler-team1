import os
import subprocess
import sys
import shutil
import re

passes_dir = './src/lib/opt'
ll_files_dir = './unit_tests'
temp_dir_path = './tmp'

llvm_path = sys.argv[1]
alive_tv_binary = sys.argv[2]

# get the directory of the current file
dir_path = os.path.dirname(os.path.realpath(__file__))
# change the current working directory to the directory of the current file
os.chdir(dir_path)
shutil.rmtree(temp_dir_path, ignore_errors=True)
os.makedirs(temp_dir_path, exist_ok=True)

with open(f'{ll_files_dir}/entries.csv', 'r') as f:
    entries = [entry.split(',') for entry in f.read().strip().split('\n')[1:]]

failures = False

alive2_pattern = re.compile(r'Summary:\n'
                            r'\s*(\d+) correct transformations\n'
                            r'\s*(\d+) incorrect transformations\n'
                            r'\s*(\d+) failed-to-prove transformations\n'
                            r'\s*(\d+) Alive2 errors')

for entry in entries:
    sourcename, classname, passname = entry

    test_input_dir = os.path.join(ll_files_dir, sourcename)
    test_output_dir = os.path.join(temp_dir_path, sourcename)
    os.makedirs(test_output_dir)

    print(f"== {passname} ==")
    pass_lib = f"lib{classname}.so"
    ll_files = [f for f in os.listdir(test_input_dir) if f.endswith(".ll")]

    for ll_file in ll_files:
        ll_path = f"{test_input_dir}/{ll_file}"

        print(f"Test file: \t{ll_path}")

        if sourcename.endswith('analysis'):
            cmd = f"\t{llvm_path}/bin/opt -load-pass-plugin=./build/{pass_lib} -passes={passname}" \
                  f" {ll_path} -S -disable-output 2>&1 | {llvm_path}/bin/FileCheck {ll_path}"
            print(cmd)
            result = os.system(cmd)
            if result != 0:
                failures = True
            continue

        # Run opt with the pass shared library
        opt_cmd = [f"{llvm_path}/bin/opt", f"-load-pass-plugin=./build/{pass_lib}", f"-passes={passname}",
                   ll_path, "-S", "-o", f"{test_output_dir}/out.{ll_file}"]
        print(f"\t{' '.join(opt_cmd)}")
        result = subprocess.run(opt_cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        with open(f"{test_output_dir}/out.{ll_file}.log", 'w') as f:
            f.write(result.stdout.decode("utf-8"))

        # Run filecheck on the output
        filecheck_cmd = [f"{llvm_path}/bin/FileCheck", ll_path]
        print(f"\t{' '.join(filecheck_cmd)}")
        with open(f'{test_output_dir}/out.{ll_file}', 'r') as f:
            result = subprocess.run(filecheck_cmd, stdin=f, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        if result.returncode != 0:
            print(result.stderr.decode("utf-8"))
            failures = True

        # Run alive2 validation on the output
        alive2_cmd = [alive_tv_binary, ll_path, f"{test_output_dir}/out.{ll_file}"]
        print(f"\t{' '.join(alive2_cmd)}")
        result = subprocess.run(alive2_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        with open(f"{test_output_dir}/out.{ll_file}.alive", 'w') as f:
            f.write(result.stdout.decode("utf-8"))

        match_result = alive2_pattern.findall(result.stdout.decode('utf-8'))
        assert len(match_result) == 1
        alive_correct, alive_incorrect, alive_failed_to_prove, alive_error = map(int, match_result[0])

        if alive_incorrect:
            print(f'Alive2 incorrect: {alive_incorrect}')

        if alive_failed_to_prove:
            print(f'Alive2 failed to prove: {alive_failed_to_prove}')

        if alive_error:
            print(f'Alive2 error: {alive_error}')

        if result.returncode != 0:
            print(result.stderr.decode("utf-8"))

    print()

# Exit with the appropriate exit code
if failures:
    print('Unit Test Failure.')
    sys.exit(1)
else:
    print('Unit Tests Successful!')
    sys.exit(0)
