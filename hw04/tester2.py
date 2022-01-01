#!/usr/bin/env python
import sys
import os
import string
import random
import re

LETTERS = string.ascii_letters

TERM = ''.join(random.choice(LETTERS) for _ in range(random.randrange(1, 4)))


def randname():
	if random.randrange(2) == 0:
		name = TERM
	else:
		name = ''
	
	name += ''.join(random.choice(LETTERS) for _ in range(random.randrange(2)))
	name = name + ''.join(random.choice(LETTERS) for _ in range(random.randrange(2)))
	
	return name

def create(cur, depth):
	os.system(f'mkdir {cur}')
	cur += '/'
	
	ok = 0
	err = 0
	counter = 0
	
	if depth == 0:
		return 0, 0
	
	for i in range(random.randrange(3)):
		counter += 1
		sub = f'{cur}{counter}{randname()}'
		sub_ok, sub_err = create(sub, depth - 1)
		
		if range(random.randrange(2)) == 0:
			os.system(f'chmod 000 {sub}')
			err += 1
		else:
			ok += sub_ok
			err += sub_err
	
	for i in range(random.randrange(3)):
		counter += 1
		sub = f'{cur}{counter}{randname()}'
		os.system(f'touch {sub}')
		
		if TERM in sub:
			ok += 1
	
	return ok, err

	
	

PERM_DENIED_PATTERN = re.compile(r'^Directory (.+): Permission denied\.$')
DONE_SEARCHING_PATTERN = re.compile(r'^Done searching, found (\d+) files$')

if os.path.exists('tree'):
	os.system('sudo rm -rf tree')
OK, ERR = create('tree', depth=4)

os.system('gcc-5.3.0 -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 -pthread pfind.c')
print('\n=================================================================\n')


print(f"search term: {TERM}")
print(f"expecting: {OK} matches, {ERR} errors")
print(f"")

for i in range(1, 50):
	ok = 0
	err = 0
	pipe = os.popen(f'./a.out tree {TERM} {i} 2>/dev/null')
	
	for line in pipe:
		line = line[:-1]
		if PERM_DENIED_PATTERN.search(line):
			err += 1
		elif DONE_SEARCHING_PATTERN.search(line):
			pass
		elif os.path.exists(line):
			ok += 1
		else:
			raise Exception("unknown output to stdout: " + line)
	
	pipe.close()
	
	if ok != OK or err != ERR:
		print(f"\nERROR: got {ok} oks and {err} errors")
		break
else:
	print("\nSUCCESS!")

	