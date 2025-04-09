#!/usr/bin/env python
import rospy
from uav_keyboard_control.msg import DroneData30D  # 导入自定义消息类型

class DataSubscriber:
    def __init__(self):
        # 初始化ROS节点
        rospy.init_node('drone_data_subscriber_py', anonymous=True)
        
        # 创建订阅者，设置队列大小和回调函数
        self.sub = rospy.Subscriber("/drone_data_30d", 
                                   DroneData30D, 
                                   self.data_callback,
                                   queue_size=10)
        rospy.loginfo("Subscriber initialized")

    def data_callback(self, msg):
        """处理无人机数据回调函数"""
        # 解析相对位置
        rospy.loginfo("\nRelative Position [x:%.2f, y:%.2f, z:%.2f]", 
                     msg.rpos.x, msg.rpos.y, msg.rpos.z)
        
        # 解析drone_state数组
        if len(msg.drone_state) >= 6:
            rospy.loginfo("Linear Velocity [x:%.2f, y:%.2f, z:%.2f]\n"
                         "Angular Velocity [x:%.2f, y:%.2f, z:%.2f]", 
                         msg.drone_state[0], msg.drone_state[1], msg.drone_state[2],
                         msg.drone_state[3], msg.drone_state[4], msg.drone_state[5])
        else:
            rospy.logwarn("Incomplete drone_state data received (size=%d)", 
                         len(msg.drone_state))

if __name__ == '__main__':
    try:
        ds = DataSubscriber()
        rospy.spin()  # 保持节点持续运行
    except rospy.ROSInterruptException:
        rospy.logerr("Subscriber node terminated abnormally")