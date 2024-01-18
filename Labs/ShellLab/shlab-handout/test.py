import pty
import os
import subprocess
import re
import tempfile

# 测试文件列表
trace_files = [f"trace{str(i).zfill(2)}.txt" for i in range(1, 17)]

# 执行 make clean && make
os.system("make clean && make")


# 解析输出，忽略 PID 和 /bin/ps 命令的输出
def parse_output(output):
    lines = output.split("\n")
    parsed_lines = []
    ignore = False
    for line in lines:
        if line.startswith("tsh> /bin/ps a"):
            ignore = True
        elif line.startswith("tsh>"):
            ignore = False
        if not ignore:
            parsed_lines.append(re.sub(r"\(\d+\)", "()", line))
    return "\n".join(parsed_lines)


for trace_file in trace_files:
    # 清屏并打印正在运行的测试文件
    print("\033c", end="")
    print(f"Running on {trace_file}...")

    # 运行参考 Shell 并获取输出
    ref_output = subprocess.check_output(
        f"./sdriver.pl -t {trace_file} -s ./tshref -a '-p'", shell=True
    ).decode()

    # 运行待测试 Shell 并获取输出
    test_output = subprocess.check_output(
        f"./sdriver.pl -t {trace_file} -s ./tsh -a '-p'", shell=True
    ).decode()

    ref_output_parsed = parse_output(ref_output)
    test_output_parsed = parse_output(test_output)

    # 比较解析后的输出
    if ref_output_parsed != test_output_parsed:
        # 将输出写入到临时文件中
        with tempfile.NamedTemporaryFile(
            delete=False
        ) as ref_file, tempfile.NamedTemporaryFile(delete=False) as test_file:
            ref_file.write(ref_output_parsed.encode())
            test_file.write(test_output_parsed.encode())

        # 使用 diff 命令来对比两个文件
        master_fd, slave_fd = pty.openpty()  # 打开一个新的伪终端对
        diff_process = subprocess.Popen(
            f"diff --color=always {ref_file.name} {test_file.name}",
            shell=True,
            stdout=slave_fd,
        )

        os.close(slave_fd)  # 关闭从设备，使得可以读取主设备
        diff_output = os.read(master_fd, 1024).decode()  # 从主设备读取输出
        print(diff_output)

        # 从 diff 命令的输出中解析行号和差异类型
        matches = re.findall(r"(\d+)([acd])(\d+)", diff_output)
        for match in matches:
            line_number1, change_type, line_number2 = match
            print(
                f"Line {line_number1} in the first file and line {line_number2} in the second file have a {change_type} change."
            )

        break
    elif ref_output_parsed == test_output_parsed and trace_file == trace_files[-1]:
        print("All tests passed!")
