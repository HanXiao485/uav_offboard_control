#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <mavros_msgs/ESCStatus.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <uav_keyboard_control/DroneData30D.h>  // 自定义消息类型

class DataPublisher {
private:
    ros::NodeHandle nh;
    ros::Subscriber odom_sub;
    ros::Subscriber esc_sub;
    ros::Publisher data_pub;
    
    geometry_msgs::Point target_pos;
    int rotor_num;
    uav_keyboard_control::DroneData30D msg;

public:
    DataPublisher() : nh("~") {
        // 参数加载
        nh.param<double>("target_x", target_pos.x, 0.0);
        nh.param<double>("target_y", target_pos.y, 0.0); 
        nh.param<double>("target_z", target_pos.z, 2.0);
        nh.param<int>("rotor_num", rotor_num, 4);

        // 订阅初始化
        odom_sub = nh.subscribe<nav_msgs::Odometry>(
            "/mavros/local_position/odom", 10, &DataPublisher::odomCb, this);
        esc_sub = nh.subscribe<mavros_msgs::ESCStatus>(
            "/mavros/esc_status", 10, &DataPublisher::escCb, this);

        // 发布器初始化
        data_pub = nh.advertise<uav_keyboard_control::DroneData30D>("/drone_data_30d", 10);
    }

    void odomCb(const nav_msgs::Odometry::ConstPtr& odom) {
        // 相对位置
        msg.rpos.x = target_pos.x - odom->pose.pose.position.x;
        msg.rpos.y = target_pos.y - odom->pose.pose.position.y;
        msg.rpos.z = target_pos.z - odom->pose.pose.position.z;

        // 航向角计算
        geometry_msgs::Quaternion q_msg = odom->pose.pose.orientation;
        // double yaw = tf2::getYaw(q_msg);  // 使用正确的函数

        // 线速度和角速度（假设drone_state是double[]）
        msg.drone_state.clear();
        msg.drone_state.push_back(odom->twist.twist.linear.x);
        msg.drone_state.push_back(odom->twist.twist.linear.y);
        msg.drone_state.push_back(odom->twist.twist.linear.z);
        msg.drone_state.push_back(odom->twist.twist.angular.x);
        msg.drone_state.push_back(odom->twist.twist.angular.y);
        msg.drone_state.push_back(odom->twist.twist.angular.z);

        data_pub.publish(msg);  // 在escCb中填充其他字段后统一发布
    }

    void escCb(const mavros_msgs::ESCStatus::ConstPtr& esc) {
        // 填充油门信息
        for(int i=0; i<rotor_num && i<esc->esc_status.size(); ++i){
            msg.drone_state.push_back(esc->esc_status[i].rpm);  // 使用正确字段
        }
        data_pub.publish(msg);  // 若需实时更新，可在此处发布
    }

};

int main(int argc, char** argv) {
    ros::init(argc, argv, "drone_data_publisher");
    DataPublisher dp;
    ros::spin();
    return 0;
}