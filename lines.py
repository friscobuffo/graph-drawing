import os

def count_lines_in_file(file_path):
    with open(file_path, 'r') as file:
        return sum(1 for _ in file)
    
def count_lines_in_file(file_path):
    with open(file_path, 'r') as file:
        return sum(1 for line in file if not line.strip().startswith(('/', '*')))

def count_lines_in_src_folder(src_folder):
    line_counts = {}
    for root, _, files in os.walk(src_folder):
        for file in files:
            file_path = os.path.join(root, file)
            if file_path.endswith('.cpp') or file_path.endswith('.hpp'):
                line_counts[file_path] = count_lines_in_file(file_path)
    return line_counts

if __name__ == "__main__":
    src_folder = 'src'
    line_counts = count_lines_in_src_folder(src_folder)
    total_lines = sum(line_counts.values())
    for file_path, line_count in line_counts.items():
        print(f"{file_path}: {line_count} lines")
    print(f"Total: {total_lines} lines")