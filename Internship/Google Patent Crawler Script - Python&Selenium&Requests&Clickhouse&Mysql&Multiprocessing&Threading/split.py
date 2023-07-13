import os
import shutil

# Arguments
src_dir = "xxx"
dst_dir_a = "xxx"
dst_dir_b = "xxx"

if __name__ == '__main__':
    os.mkdir(dst_dir_a)
    os.mkdir(dst_dir_b)
    # 遍历源文件夹里的所有文件
    for file_name in os.listdir(src_dir):
        file_path = os.path.join(src_dir, file_name)
        if os.path.isfile(file_path):
            # 如果以 a 结尾，则复制到目标文件夹 dst_dir_a
            if file_name.endswith("common.csv"):
                shutil.move(file_path, dst_dir_a)
                print(f"Move {file_name} to {dst_dir_a}")
            # 如果以 b 结尾，则复制到目标文件夹 dst_dir_b
            elif file_name.endswith("cite.csv"):
                shutil.move(file_path, dst_dir_b)
                print(f"Move {file_name} to {dst_dir_b}")