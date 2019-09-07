#!/usr/bin/python
# -*- coding:utf-8 -*-

import os

count = 0

for root,dirs,files in os.walk(os.getcwd()):
	for file in files:
		fn = os.path.join(root,file)
		if fn.endswith(".py"):
			continue
		f = open(fn, "r")
		num = len(f.readlines())
		f.close()
		count += num

print count

