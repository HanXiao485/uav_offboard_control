#!/usr/bin/env python
import rospy
from uav_keyboard_control.msg import DroneData30D, RateThrust  # 导入自定义消息类型
# from mavros_msgs import State
from sensor_msgs.msg import Imu
import numpy as np
import os
import sys
import torch
from tensordict import TensorDict


class DataSubscriber:
    def __init__(self):
        # 初始化ROS节点
        rospy.init_node('drone_data_subscriber_py', anonymous=True)
        
        # 创建订阅者，设置队列大小和回调函数
        self.sub = rospy.Subscriber("/drone_data_30d", 
                                   DroneData30D, 
                                   self.data_callback,
                                   queue_size=10)
        self.pub_control = rospy.Publisher('/drone_control', 
                                          RateThrust, 
                                          queue_size=10)
                                                                         
        rospy.loginfo("Subscriber initialized")

    def data_callback(self, msg):
        """处理无人机数据回调函数"""
        # 解析相对位置
        # rospy.loginfo("\nRelative Position [x:%.2f, y:%.2f, z:%.2f]", 
        #              msg.rpos.x, msg.rpos.y, msg.rpos.z)
            
        orientation = torch.tensor([
            msg.orientation_x,
            msg.orientation_y,
            msg.orientation_z,
            msg.orientation_w
        ])
            
        relative_position = torch.tensor([
            msg.rpos.x, 
            msg.rpos.y, 
            msg.rpos.z
        ])
        
        velocities = torch.tensor([
            msg.linear_velocity.x,  # linear velocity x
            msg.linear_velocity.y,  # linear velocity y
            msg.linear_velocity.z,  # linear velocity z
            msg.angular_velocity.x,  # angular velocity x
            msg.angular_velocity.y,  # angular velocity y
            msg.angular_velocity.z,  # angular velocity z
        ])
        
        heading = torch.tensor([
            msg.heading.x,
            msg.heading.y,
            msg.heading.z
        ])
        
        up = torch.tensor([
            msg.up.x,
            msg.up.y,
            msg.up.z
        ])
        
        rheading = torch.tensor([
            msg.rheading.x,
            msg.rheading.y,
            msg.rheading.z
        ])
        
        throttle = torch.tensor([
            1.0, 0.0, 0.0, 0.0
        ])
        
        time_enconding = torch.tensor([
            0.0, 0.0, 0.0, 0.0
        ])
        
        drone_data_tensor = torch.cat([
            relative_position, 
            orientation, 
            velocities, 
            throttle,
            heading, 
            up,
            rheading,
        ])
        
        # print(drone_data_tensor)
            
        # input_tensor = torch.tensor([0.0] * 30, dtype=torch.float).view(1, 30).to(device)
        
        input_tensor = drone_data_tensor.view(1, 26).to(device)
        
        tensordict = TensorDict({
            # agents 字段下的 observation，需增加一维使形状为 [batch, 1, 36]
            "agents": TensorDict({
                "observation": input_tensor.unsqueeze(1)  # 变为 [1, 1, 36]
            }, batch_size=[1], device=device),
            
            # 添加 feature 键，后续会被 exclude 掉
            "feature": input_tensor,  # [1, 36]
            
            # collector 字段，包含 traj_ids（这里使用0填充）
            "collector": TensorDict({
                "traj_ids": torch.zeros(1, dtype=torch.int64, device=device)
            }, batch_size=[1], device=device),
            
            # done 字段，标记是否结束
            "done": torch.zeros(1, 1, dtype=torch.bool, device=device),
            
            # info 字段，包含 drone_state，示例中 drone_state 的形状为 [batch, 1, 13]
            "info": TensorDict({
                "drone_state": torch.zeros(1, 1, 13, dtype=torch.float, device=device)
            }, batch_size=[1], device=device),
            
            # is_init 标记
            "is_init": torch.zeros(1, 1, dtype=torch.bool, device=device),
            
            # stats 字段，包含一些统计信息（这里均用0填充）
            "stats": TensorDict({
                "action_smoothness": torch.zeros(1, 1, dtype=torch.float, device=device),
                "episode_len": torch.zeros(1, 1, dtype=torch.float, device=device),
                "return": torch.zeros(1, 1, dtype=torch.float, device=device),
                "tracking_error": torch.zeros(1, 1, dtype=torch.float, device=device),
                "tracking_error_ema": torch.zeros(1, 1, dtype=torch.float, device=device)
            }, batch_size=[1], device=device),
            
            # terminated 字段
            "terminated": torch.zeros(1, 1, dtype=torch.bool, device=device),
            
            # truncated 字段
            "truncated": torch.zeros(1, 1, dtype=torch.bool, device=device)
        }, batch_size=[1], device=device)
        

        with torch.no_grad():  # 禁用梯度计算
            output = model(tensordict)  # 输入 TensorDict
            action = output["agents"]["action"]
            target_rate, target_thrust = action.split([3, 1], -1)
            # target_thrust = ((target_thrust + 1) / 2).clip(0.) * 24.0128  # max_thrust
            
            # 创建合并消息对象（核心修改）
            control_msg = RateThrust()
            control_msg.rate_x = target_rate[..., 0].item()
            control_msg.rate_y = target_rate[..., 1].item()
            control_msg.rate_z = target_rate[..., 2].item()
            control_msg.thrust = target_thrust.squeeze().item()
            
            self.pub_control.publish(control_msg)
            
            # rate_x = target_rate[..., 0].item()  # 提取第1个维度的值
            # rate_y = target_rate[..., 1].item()  # 提取第2个维度的值
            # rate_z = target_rate[..., 2].item()  # 提取第3个维度的值
            # thrust = target_thrust.squeeze().item()  # 去除多余维度后提取标量
            
            
            # print(target_rate)
            # print(target_thrust)

if __name__ == '__main__':
    
    # 获取当前脚本所在目录，并定位到包根目录
    current_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(current_dir, '.'))
    # 将 omni_drones 文件夹加入 sys.path，以便 torch.load 时能找到模块定义
    omni_drones_path = os.path.join(project_root)
    if omni_drones_path not in sys.path:
        sys.path.insert(0, omni_drones_path)
        print("Added omni_drones path: " + omni_drones_path)
    
    
    # 导入模型
    device = torch.device("cpu")
    model = torch.load('/home/nv/rl_control_uav/src/uav_keyboard_control/scripts/checkpoint_final_20250416_hover_no-time-encoding.pt', map_location=torch.device('cpu'))
    model.eval()
    
    try:
        ds = DataSubscriber()
        rospy.spin()  # 保持节点持续运行
    except rospy.ROSInterruptException:
        rospy.logerr("Subscriber node terminated abnormally")