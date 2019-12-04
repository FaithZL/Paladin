# Paladin
Paladin是本人的第一个离线渲染器，代码基本来自于pbrt，当然，并不是无脑抄

	目前是看懂pbrt的代码，再拿过来使用并做一些修改，写清楚中文注释，几乎每个表达式都推导过一遍写在注释中。
	所以才有底气说这是自己的东西，除了参照pbrt，也会借鉴一些其他的渲染器，比如mitsuba，tungsten等。
	完成大体功能之后开始做性能优化，代码优化，以及pbrt不完善的部分，还有pbrbook书上的课后练习。


	so，如果不嫌弃的话，尽管来看，看不懂算我输😂。
	考虑到也许这些代码以及注释可能会对比我还要新手的新手有一些微不足道的帮助，做了跨平台。
	或者如果发现有什么不对的地方，希望各位大侠不吝赐教

	基本的框架已经搭好，场景是用json文件来描述，目前实现的渲染算法也只有路径追踪，其他的部分待后续完善，先贴上经典效果图connellbox
	![](https://github.com/FaithZL/Paladin/blob/master/gallery/connellbox-pt-16spp-halton.png)


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
	CMake 3.9.0-rc6
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

