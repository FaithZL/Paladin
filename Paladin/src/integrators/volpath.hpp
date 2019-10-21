//
//  volpath.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/21.
//

/**
 * 参考章节
 * http://www.pbr-book.org/3ed-2018/Volume_Scattering/Volume_Scattering_Processes.html
 * 在有参与介质的环境中，有三个主要过程影响辐射的分布
 * 		1.吸收:由于光转换成另一种形式的能量(如热)而引起的亮度降低
 * 		2.辐射:发光粒子对环境产生的辐射
 *   	3.散射:由于与粒子碰撞而散射到其他方向
 *
 * 1.吸收
 * σa为单位距离内光被吸收的概率密度。σa为关于位置与方向的函数，单位为1/m
 * 以下方程描述了光线通过一个微分距离dt，被参与介质吸收的过程
 * 
 * 		Lo(p, ω) − Li(p, −ω) = dLo(p, ω) = −σa(p, ω) Li(p, −ω) dt 
 * 		用人话来说就是入射辐射度减去出射辐射度等于被吸收的辐射度
 * 	
 * 如果需要求解光线传播一段距离l之后的剩余辐射度百分比r，则可以列出以下微分方程
 *
 * 		dr = −σa(p + tω, ω) r dt   
 * 		用人话来说：剩余量的变化量 = 吸收率 * 剩余量 * 位置变化量
 * 		
 * 		求解得：r = e^(−􏰗∫[0,l]σa(p+tω,ω)dt)
 *
 * 2.发射
 * 某些粒子吸收辐射能量之后，由化学，热能，原子能转化为光能
 */

#ifndef volpath_hpp
#define volpath_hpp

#include "core/integrator.hpp"

PALADIN_BEGIN


PALADIN_END

#endif /* volpath_hpp */
