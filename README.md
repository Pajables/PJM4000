# PJM4000
Code for open-source system design of chemistry apparatus, .ino files are Arduino source code, .txt are text copies viewable on any computer.

syringe_pump_rack_arduino is the code for the original system, as discussed in: "Low-budget 3D-printed equipment for continuous flow reactions" by Neumaier et al

system_code_3pumps2valves_updated is the updated system code for 3 pumps and 2 valves
  -added multi-flow valve enabling code allowing pumps to be refilled
  -updated delay calculation to reflect new leadscrew
  -added menu to input volume to be pumped

system_code_3pumps2valves_Final is for the final system code for 3 pumps and 2 valves discussed in the mini-dissertation
  -re-worked delay calculation and stepping to use time difference
  -added homing function
  -added jogging function
  -cleaned menus
  -added new status screen
