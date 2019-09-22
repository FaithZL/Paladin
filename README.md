# Paladin
Paladin is a pbrt style renderer

代码内附带详细注释以及各种表达式推导过程，尽管来看，看不懂算我输😂

# 获取仓库
	git clone --recursive https://github.com/FaithZL/Paladin.git

# 跨平台编译相关说明
Windows

	Win7 64bit
	CMake 3.15.0
	VS2015(对应编译器cl版本 Microsoft (R) C/C++ 优化编译器 19.00.24215.1 版)

	操作步骤:
	1、命令行进入项目根目录，cd build(没有则新建)
	2、cmake ../Paladin
	3、在build中找到VS工程文件Paladin.sln打开


Linux:

	CentOS 6.5
	CMake 2.8.12.2
	GNU Make 3.81
	GCC 4.8.0

	操作步骤:
	1、命令行进入项目根目录，cd build(没有则新建)
	2、cmake ../Paladin
	3、make


MACOS:

	Sierra 10.12.5
	CMake 3.9.3
	GNU Make 3.81
	Clang Apple LLVM version 8.1.0 (clang-802.0.42)
	Xcode 8.3.3

	操作步骤:
	1、命令行进入项目根目录，cd build(没有则新建)
	2、cmake ../Paladin -G "Xcode"
	3、在build中找到Xcode工程文件Paladin.xcodeproj打开

