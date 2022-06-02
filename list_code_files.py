import os
import sys

base_dir = "./src/"
code_file_extensions = (".cpp", ".h", ".hpp", ".cc", ".c")

for root, subdirs, files in os.walk(base_dir):
	for filename in files:
		file_path = os.path.join(root, filename)
		
		if (file_path.lower().endswith(code_file_extensions)):
			print("\"" + file_path.replace(base_dir, "").replace('\\', '/') + "\"")