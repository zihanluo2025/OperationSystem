import os
import csv
import subprocess

# 参数设置
traces = ["bzip.trace", "gcc.trace", "sixpack.trace", "swim.trace"]
frames = [4, 8, 16, 32, 64, 128, 256, 512, 1024]

# rand, 输出 CSV 文件
# algorithm = "rand"
# output_file = "rand_results.csv"


# LRU
# algorithm = "lru"
# output_file = "lru_results.csv"


# CLOCK
algorithm = "clock"
output_file = "clock_results.csv"



with open(output_file, mode="w", newline="") as file:
    writer = csv.writer(file)
    # CSV 表头
    writer.writerow(["trace", "frames", "algorithm", "page_faults", "disk_reads", "disk_writes"])

    for trace in traces:
        for f in frames:
            cmd = ["python3", "memsim.py", trace, str(f), algorithm, "quiet"]
            print(f"Running: {' '.join(cmd)}")

            # 捕获命令行输出
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

            # 解析 memsim.py 的输出
            output = result.stdout.strip().split("\n")
            # 假设输出最后三行是 faults / reads / writes
            page_faults = output[-3].split()[-1]
            disk_reads = output[-2].split()[-1]
            disk_writes = output[-1].split()[-1]

            # 写入 CSV
            writer.writerow([trace, f, algorithm, page_faults, disk_reads, disk_writes])

print(f"✅ All results saved into {output_file}")
