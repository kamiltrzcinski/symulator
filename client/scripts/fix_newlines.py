import os

file_path = r"C:\Users\tymon\Desktop\SUSRK\symulator\client\src\thales\thales_browser.cpp"

with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

parts = content.split('"')
for i in range(1, len(parts), 2):
    parts[i] = parts[i].replace('\n', '\\n')

content = '"'.join(parts)

with open(file_path, "w", encoding="utf-8") as f:
    f.write(content)
