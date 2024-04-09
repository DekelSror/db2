


import datetime
import subprocess
import csv
import sys
import time


def prepare_input(file_path: str) -> dict:
    data_dict = {}
    with open(file_path, 'r') as file:
        csv_reader = csv.reader(file)
        for row in csv_reader:
            key = row[0]
            value = ''.join(row[1:])
            data_dict[key] = value
    return data_dict

stonks = prepare_input('./abc.us.txt')

db_bin = sys.argv[1]
print(db_bin)

start = time.time()
for k, v in stonks.items():
    subprocess.run([db_bin, 'set', k, v], executable=db_bin, capture_output=True, text=True)
end = time.time()

print(f'{len(stonks)} were set and get in {end - start}')