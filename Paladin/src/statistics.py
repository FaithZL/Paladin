#!/usr/bin/python
# -*- coding:utf-8 -*-

import os

count = 0

com = 0

inCom = False

for root,dirs,files in os.walk(os.getcwd()):
	for file in files:
		if "alltest" in root or "ext" in root:
			continue
		fn = os.path.join(root,file)
		if fn.endswith(".py"):
			continue
		f = open(fn, "r")
		lines = f.readlines()

		for line in lines:
			if "//" in line or inCom:
				com += 1
			if "/*" in line:
				com += 1
				inCom = True
			if "*/" in line:
				com += 1
				inCom = False

		num = len(lines)
		f.close()
		count += num

print (count, com)

