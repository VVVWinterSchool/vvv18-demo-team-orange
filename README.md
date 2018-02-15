# VVV18 ORANGE TEAM

- 📚 [**Project description**](https://github.com/vvv-school/vvv18/wiki/Team-contest)
- 👋 [**Meet the team**](https://github.com/orgs/vvv-school/teams/vvv18-team-orange/members)
- :feet: [**Project management**](https://github.com/vvv-school/vvv18-demo-team-orange/projects/1)
- 🗣 [**Issues**](https://github.com/vvv-school/vvv18-demo-team-orange/issues)

# Documentation

Ciao a tutti and welcome to the official documentation of the **VVV18 Orange Team** repository for the final project of this amazing school!

Let's start with a brief overview of the problem itself and then move on how we decided to tackle it.

## Overview

We were asked to make iCub able to:
 - look at two objects shown by the human in front of him;
 - classify them correctly and point at the one asked by the human;
 - wait for an acknowledgement from the human to understand if he succeeded or not and behave consequently, showing happiness or sadness.

The approach we decided to follow has been inspired by the YARP modularity, leading us to a State-Machine architecture. 

![application](misc/framework.png)

The central role is played by a **manager application**, which is in charge of communicating with the modules available using a rpc protocol. The information exchanged can be both data (for instance, the 3D position of the object to point at) and triggering signals (just to make a module run). The manager is also, of course, in charge of defining the temporal sequence of actions, waiting for an ack after each single operation. The temporal sequence can be defined as follows:

//here we put obvs the single modules

The other modules are divided according with the topics subdivision followed during the school and provide the function implementations needed from the manager to carry out the demo. Only the Kinematics module includes also the Gaze control, in order to avoid "empty modules" (modules which just one function or few lines of code inside). This finds support also in the YARP implementation of the two interfaces, which share the same basic idea of control and implementation. Let's have a look at each module in details:

### Vision
The vision module performs the image acquisition and processing needed to obtain information from the robot's cameras about the physical world with which it interacts. In particular, it is used to locate objects placed on the table in front of it and to obtain their exact position with respect to it's own body coordinates.

![application](misc/vision.png)

This component makes use of two YARP modules: lbpExtractor and SFM. The former acquires the left robot camera and performs texture filtering and object segmentation, outputting the bounding boxes coordinates of the detected blobs. These are used by the vision module to compute, for each of them, their center point in (x,y) image coordinates. Subsequently, SFM collects both the left and right robot camera images, computes the disparity map and uses it, together with the an (x,y) image coordinate point, to compute the (x,y,z) world coordinate of the latter with reference to the robot's torso. Finally, the vision module returns to the manager the list of bounding boxes and their centers in world coordinates.

### Classification (Deep Learning)

### Kinematics
In the kinematics lecture, we programmed iCub to reach a position in the cartesian space with his hand, and this point was retrieved from the triangulation of two points retrieved from images issued from the iCub cameras. In this case, we are not interested in reach the point, but to point to it using the index finger. We found that this functionality is already implemented in the Actions Rendering Engine module, a module combining multiple libraries and modules from the iCub repository that allows to execute some basic and complex actions. A fast test of this module starts by connecting to the module:

- `$ yarp rpc /actionsRenderingEngine/cmd:io`

to ask iCub to point to a far point, we use the function pfar and the coordinates of the point. This point is defined in the global frame of iCub:

- `>>pfar (-1.0 0.0 0.0)`

iCub will try to point to the point and then comes back to the home position, when it finish, it sends an acknowledgment.

![make-it-point](/misc/iCubpoint.gif)

### Dynamics
As we learnt from the *Robot Dynamics* lecture, iCub mounts on the body sensors able to perceive generalized forces applied to the end effector. This has been exploited starting from the consideration that having the robot hand in *high five* or *low five* position means that our end-effector frame has the axes almost collinear with the root frame (even if the frames are somehow rotated). This means that, once we established the gestures we intend to use to confirm/reject the classification, we just need to read the force applied along the axis of interest to discriminate the two cases. For instance, in the high-five configuration the axis of interest is the *x_root*; while in the low-five configuration we'll be interested in the *z_root* component. The magnitude of the force, if higher than a certain threshold, will tell us that a contact happened; the direction of the force will tell us if it's a positive or negative ack.
To do so, it is mandatory to reset the sensor once we reach the high/low five hand configuration with the kinematic control. In this way, we will read always 0 (more or less) and a higher value only in case of contact! Useful instructions to test the module are:

- `$ yarp rpc /wholeBodyDynamics/rpc:i`

to reset the sensor to 0, giving this value as input to the rpc port;

- `$ yarp read ... /wholeBodyDynamics/right_arm/cartesianEndEffectorWrench:o`

to read forces and wrenches perceived by the sensor and projected to the end-effector;

- `$ yarpscope --remote "/wholeBodyDynamics/right_arm/cartesianEndEffectorWrench:o`

to open a yarp scope and link it to the sensor readings, in order to understand the direction of the applied forces and to tune the threshold to detect the contact.

### Temporal chart

![application](misc/temporal.png)

#### Dependencies
- [robotology/segmentation](https://github.com/robotology/segmentation)
- [robotology/stereo-vision](https://github.com/robotology/stereo-vision)



