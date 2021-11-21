## Low Cost Quadruped Development

![Quad_V1](/Images/V1/Quadruped_V1_big.jpg)

<p>Three versions of my quadruped design currently exist. The first was made with the goal of creating a walking dog robot with minimal controls and autonomy. The second improved on the design by reorganizing the electronics and actuators and adding a microprocessor. The third version was redesigned to reduce weight as well as house more sensors and a cheaper microprocessor.</p>

<p>
  The first version of this quadruped was designed with the exercise of inverse kinematics in mind. At a surface level, a quadruped seems like a difficult robot to derive inverse kinematics for. The problem turns out to be very intuitive and simple to solve. First, we derive the inverse kinematics for a single leg, which is a three degree of freedom system. We want to control the position of each leg's end effector in 3D space. Since the leg is comprised of only three actuators, we are unable to control the orientation at which the end effector - the foot - contacts the ground.
  Once the IK for each leg has been derived, we can coordinate the movements of each foot to move the entire body however we like.
  The first step in computing the inverse kinematics for a single leg is to define a frame of reference to use when positioning the foot. When computing foot positions to move the entire robot, we see that the feet always move in the opposite direction of body motion. For example, in order to shift the body forward, the legs must shift backward an equal distance relative to the body's reference frame. In order to move the body up, the legs must move down an equal distance relative to the body's reference frame. The same is true for lateral motions and a similar thought process can be used to compute foot poses that make the body roll, pitch, and yaw. To make abstraction easier later, it makes sense to choose a foot reference frame that's opposite to the body reference frame. However, this results in confusion if, in the future, a user might want to control only one foot and set it to a posiiton with respect to the body's reference frame. Thus, I chose to use the same coordinate frame for my feet and robot body. Inverting the foot positions requires only inverting the sign of the value computed by the whole-body IK. In my implementation x is forward, y is left, and z is up.
  We separate the IK into two parts, one for the singular yaw servo and one for the hip and knee servos. We then derive equations to derive the joint angles from the given end effector positions. The hip and knee servos form a triangle. We define the length of one side of this triangle using the desired z value. The other two side lengths are known, since they are rigid bodies. We then solve for the two joint angles using the law of cosines. A computational improvement can be made if the hip and shin leg segments are the same length. This makes the triangle an isosceles, which can be fully defined with only one law of cosines computation.
</p>

![IK side nom](/Images/Others/side_nominal.PNG)
![IK side up](/Images/Others/side_up.PNG)
![IK front nom](/Images/Others/front_nom.PNG)
![IK front up](/Images/Others/front_up.PNG)

![VIO front](/Images/Dog/IMG_6395.jpg)
![VIO back](/Images/Dog/IMG_6393.jpg)

![v2 v3 comparison](/Images/Dog/v2v3.jpg)
![v3 leg open](/Images/Dog/IMG_6225.jpg)
![v3 leg closed](/Images/Dog/IMG_6227.jpg)
![v3 on table](/Images/Dog/IMG_6240.jpg)
![v3 no shell](/Images/Dog/IMG_6386.jpg)

