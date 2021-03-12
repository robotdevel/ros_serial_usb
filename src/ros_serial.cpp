
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "serialport.h"
#include "error-log.h"
#include <nav_msgs/Odometry.h>
#include <ros/ros.h>

ros::Publisher pub_robot_pose;

pthread_t read_tid;  /**< read thread */
pthread_t exit_tid;  /**< exit thread */

/** The data we write to the port. */
//char *buf = "Are you going to die?\r\n";
char *buf = "Are you going to die? To be or not to be, that is the question.\r\n";
/** data we receive */
char tmp[512];

/**
 * write_port_thread - A thread that writes data to the port
 *
 * @param argc : Here means the port(specified by the fd).
 *
 * @note
 * This is only a test, not the @e real one.
 */

void *read_port_thread(void *argc)
{
    int num;
    int fd;

    //fd = (int)argc;
    fd = 3;
    while (1)
    {
      ROS_INFO("start", tmp[0]);
        while ((num = read(fd, tmp, 512)) > 0)
        {
            debug_msg("read num: %d\n", num);
            tmp[num+1] = '\0';
            printf("%s\n", tmp);
            ROS_INFO("while", tmp[0]);
        }
        sleep(0.1);
        if (num < 0){
          ROS_INFO("pthread_exit", tmp[0]);
            pthread_exit(NULL);
        }

        int tmp_robot_x = atoi(&tmp[6]);
        int tmp_robot_y = atoi(&tmp[12]);

        double robot_x = (double)tmp_robot_x/(double)100;
        double robot_y = (double)(tmp_robot_y/(double)100);
        //ROS_INFO("robot id %d", robot_id);
        ROS_INFO("start char %c", tmp[0]);
        ROS_INFO("robot x %lf", robot_x);
        ROS_INFO("robot y %lf", robot_y);

        nav_msgs::Odometry laserOdometryROS;
        laserOdometryROS.header.frame_id = "/map";
        laserOdometryROS.header.stamp = ros::Time::now();
        laserOdometryROS.pose.pose.position.x = robot_x;
        laserOdometryROS.pose.pose.position.y = robot_y;

         pub_robot_pose.publish(laserOdometryROS);

  }
    pthread_exit(NULL);
}

void* exit_thread(void* argc)
{
    while (1)
    {
        int c = getchar();
        if ('q' == c)
        {
            printf("You exit, not myself.\n");
            exit(0);
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "ros_serial");
    ros::NodeHandle nh;
    pub_robot_pose = nh.advertise<nav_msgs::Odometry> ("/uwd_pose", 1);

    int fd;
    int ret;
    char dev_name[32] = {0};

    strcpy(dev_name, "/dev/ttyS10");
    if (argc == 2)
    {
      sprintf(dev_name,"%s",argv[1]);
    }

    fd = open_port(dev_name);          /* open the port */
    if (fd < 0)
    {
      printf("open %s err\n",dev_name);
      exit(0);
    }

    ret = setup_port(fd, 115200, 8, 'N', 1);  /* setup the port */
    if (ret<0)
      exit(0);

    ret = pthread_create(&read_tid, NULL, read_port_thread, (void*)fd);
    if (ret < 0)
      unix_error_exit("Create read thread error.");

    ret = pthread_create(&exit_tid, NULL, exit_thread, NULL);
    if (ret < 0)
        unix_error_exit("Create exit thread error.");


    ros::spin();

    pthread_join(read_tid, NULL);
    pthread_join(exit_tid, NULL);
    close_port(fd);
    //ros::spin();

    return 0;
}
