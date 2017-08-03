# Simple-Adventure-in-C
Small app showcasing saving and loading from files and mutex usage.

# Compile using:
gcc dheinc.buildrooms.c -o dheinc.buildrooms
gcc -pthread dheinc.adventure.c -o dheinc.adventure

# Preparation
Run dheinc.buildrooms first to create the directory and room files. The rooms will be read back into memory from these files by the adventure app. The rooms are randomly assigned a name and given 3-6 connections to other rooms. Two are randomly designated the start and end rooms.

# Usage
Once room files are generated, adventure app can be run.

The objective is to reach the randomly generated end room. You change rooms by typing the name of a possible connection at the prompt (case sensitive).

You can also check the time by entering 'time' at the prompt.
The 'time' command showcases threads and usage of mutex to write and read to/from the currentTime.txt file.
