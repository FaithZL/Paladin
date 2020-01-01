# Paladin


Paladin是本人的第一个离线渲染器，代码基本来自于pbrt，当然，并不是无脑抄

	目前是看懂pbrt的代码，再拿过来使用并做一些修改，写清楚中文注释，几乎每个表达式都推导过一遍写在注释中。
	btw:搞渲染如果不推导表达式，那跟咸鱼有什么区别？

	只有推导过表达式才有底气说这是自己的东西，除了参照pbrt，也会借鉴一些其他的渲染器，比如mitsuba，tungsten等。
	完成大体功能之后开始做性能优化，代码优化，以及pbrt不完善的部分，还有pbrbook书上的课后练习。

	so，如果不嫌弃的话，尽管来看，看不懂算我输😂。
	考虑到也许这些代码以及注释可能会对比我还要新手的新手有一些微不足道的帮助，做了跨平台。
	或者如果发现有什么不对的地方，希望各位大侠不吝赐教

	基本的框架已经搭好，场景是用json文件来描述，目前实现的渲染算法也只有路径追踪，
	其他的部分待后续完善，各种效果图陆续奉上(可以在gallery目录下查看)

# Feature

- 渲染算法，效果
  - [x] 路径追踪(PT,path tracing)
  - [x] 体路径追踪(VPT,volume path tracing)
  - [ ] 双向路径追踪(BDPT,bidirectional path tracing)
  - [ ] 梅特波利斯光照传输(MLT,metropolis light transport)
  - [ ] 随机渐进光子映射(SPPM,stochastic progress photon mapping)
  - [ ] 光子映射与双向路径追踪结合(VCM,vertex connection and merging)
  - [ ] 光的色散

- 场景模型解析
  - [x] 自定义json格式场景，场景解析
  - [x] 简单反射机制 
  - [ ] 加载obj模型

- 几何相关
  - [x] watertight三角形与ray求交算法
  - [ ] 经典三角形与ray求交算法
  - [x] 球体，圆柱，圆锥
  - [x] 三角形网格，quad，cube
  - [ ] 曲线
  - [ ] 双曲面

- BSDF，材质相关
  - [ ] 次表面散射BSSRDF
  - [ ] 傅里叶BSDF
  - [ ] 迪士尼材质(Disney material)
  - [x] microfacet BRDF
  - [x] lambertian 反射透射
  - [x] Specular 反射透射
  - [x] fresnel 高光
  - [x] Oren-Nayer
  - [x] Fresnel Blend(用于模拟漫反射物体表面有一层光泽反射层)

- 相机
  - [x] 透视相机
  - [ ] 正交相机
  - [ ] 真实相机

- 数学
  - [x] 四元数，矩阵，transform，animated transform
  - [x] 各种几何表面采样(待完善)
  - [x] 离散一维分布，二维分布
  
- 采样器
  - [x] 随机采样器(random sampler)
  - [x] halton采样器(halton sampler)
  - [x] 分层采样器(stratified sampler)
  - [ ] sobol采样器(sobol sampler)
  - [ ] MaxMinDistSampler
  - [ ] ZeroTwoSequenceSampler

- 内存
  - [x] 内存池
  - [x] 针对cache line优化(内存重拍)

- 滤波器(filter)
  - [x] 高斯(gaussian filter)
  - [x] 盒式滤波器(box filter)
  - [x] sinc filter
  - [x] mitchell filter
  - [x] 三角滤波器(triangle filter)

- 参与介质
  - [x] 均匀介质(homogeneous)
  - [ ] 非均匀介质(heterogeneous)

- 光源
  - [x] 点光源(point light)
  - [x] 面光源(diffuse area light)
  - [x] 聚光灯(spot light)
  - [x] 环境光(environment)
  - [x] 平行光(distant light))
  - [ ] 投影仪(projection light)
  - [ ] 纹理面光源(texture area light)


 * connell box 16spp halton
   ![](https://github.com/FaithZL/Paladin/blob/master/gallery/connellbox-pt-16spp-halton.png)
 * connell box fog 128spp halton
   ![](https://github.com/FaithZL/Paladin/blob/master/gallery/connellbox-fog-halton-128spp.png)

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

