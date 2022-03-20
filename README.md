# ShadowMapping
使用OpenGL实现的PCSS实时软阴影

# Percentage-closer Soft Shadow
PCSS通过三个步骤进行计算：  
1. 计算当前像素点一定范围内的遮挡物平均深度。
2. 利用平均深度计算PCF滤波范围，这也与光源大小呈正相关。
3. 使用PCF计算可视值visibility

![image](PCSS.png)
