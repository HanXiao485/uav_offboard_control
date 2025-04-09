#include <ros/ros.h>
#include <uav_keyboard_control/DroneData30D.h>  // 包含自定义消息头文件

class DataSubscriber {
private:
    ros::NodeHandle nh;
    ros::Subscriber data_sub;

public:
    DataSubscriber() {
        // 初始化订阅器，话题名与发布者保持一致
        data_sub = nh.subscribe<uav_keyboard_control::DroneData30D>(
            "/drone_data_30d", 10, &DataSubscriber::dataCallback, this);
    }

    void dataCallback(const uav_keyboard_control::DroneData30D::ConstPtr& msg) {
        // 解析相对位置
        ROS_INFO("Relative Position [x:%.2f, y:%.2f, z:%.2f]", 
                msg->rpos.x, msg->rpos.y, msg->rpos.z);

        // 解析drone_state数组（需确保数组长度正确）
        if(msg->drone_state.size() >= 6) {
            ROS_INFO("Linear Velocity [x:%.2f, y:%.2f, z:%.2f]", 
                    msg->drone_state[0], msg->drone_state[1], msg->drone_state[2]);
            ROS_INFO("Angular Velocity [x:%.2f, y:%.2f, z:%.2f]",
                    msg->drone_state[3], msg->drone_state[4], msg->drone_state[5]);
        } else {
            ROS_WARN("Incomplete drone_state data received");
        }
    }
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "drone_data_subscriber");
    DataSubscriber ds;
    ros::spin();
    return 0;
}