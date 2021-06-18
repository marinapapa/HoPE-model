# *The HoPE (Homing Pigeons Escape) model*

A two-dimensional agent-based model of flocking under predation, adjusted to the collective motion and collective escape of pigeons. First presented in the manuscript:
_Papadopoulou, M., Hildenbrandt H., Sankey, D.W.E., Portugal S.J.,and Hemelrijk, C.K. "Self-organization of collective escape in pigeon flocks". Submitted._

## Prerequisites

### Windows
* Operating system: Windows 10.
* Graphic card with support for OpenGL 4.4 or later.

### Linux
To build the software under Linux (Debian packet system):
```bash
~$ sudo apt install libtbb-dev
~$ sudo apt install libglm-dev
~$ cd HoPe
~/HoPe$ make
~/HoPe$ make install   # creates the excecutable ./bin/Release/pigeons
```

# The model

<div style="text-align:center"><img src="simulation_screenshot.png" /></div>

## _Framework_ 

This model is based on self-organization and includes pigeons-like and predator-like agents. Agents interact with their surrounding neighbors based on the rules of attraction, alignment and avoidance. Pigeon-agents (prey) flock together and avoid the predator. Predator-agents chase and attack pigeon-agents. Cathes of prey are not modelled.

## _Parameters_
All user-defined parameters are parsed by combining a series of .json files: *config.json* (simulation parameters),  *pigeon.json* (prey parameters, here adjusted to pigeons) and *predator.json* (predator parameters). Distance is measured in meters [m], time in seconds [s] and angles in degrees [deg].

## _Individual Actions_

Actions are the basic elements controlling the movement of each agent in the simulations. Each action represents a steering (pseudo) force that acts on each agent. The weighted sum of all actions change the direction of motion (heading) and the speed of individuals. Each action has each own user-defined parameters. Multiple actions are combined to create *states*. The majority of actions control the interactions between agents (coordination between pigeon-agents, escape actions of pigeons-agents from the predator-agents, and hunting actions of the predator-agents towards pigeon-agents). The model is based mainly on **topological** interactions.

Default pigeon-agents actions:
* Avoid actions: 
    * __avoid_n_position__: the individual turns away from the position of its _topo_ closest neighbors if they are in distance smaller than _minsep_. Parameters: _topo_ (number of neighbors to take interact with), _fov_ (field of view), _maxdist_, _minsep_, _w_.
  
    * __avoid_p_direction__: the individual turns away from the heading of the predator if it is within the minimum separation distance (_minsep_). Parameters: _w_, _minsep_.

* Align actions:
    * __align_n__: the individual turns towards the average heading of its _topo_ closest neighbors. Parameters: _topo_ (number of neighbors to take interact with), _fov_ (field of view), _maxdist_, _w_.

* Cohere actions:
    * __cohere_turn_n_all__: the individual turns towards the center of the positions of its _topo_ closest neighbors. Parameters: _topo_ (number of neighbors to take interact with), _fov_ (field of view), _maxdist_, _w_.

    * __cohere_accel_n_front__: the individual changes its speed to get closer to its _topo_ closest neighbors. If there are other agents within its frontal field of view is accelerates. If no neighbors are in front, the agent decelerates. Parameters: _topo_ (number of neighbors to take interact with), _fov_ (field of view), _ffov_ (front field of view), _maxdist_, _min_accel_dist_, _max_accel_dist_, _decel_w, _w_.


* Non-interacting actions:
    * __wiggle__: the individuals turn by a random angle controlled by the weight of this perpendicular to the agent' heading steering force, sampled from the range [-w,w]_. Parameters: _w_.

Default predator-agents actions:
* Avoid actions: 
    * __avoid_closest_prey__: the predator turns away from the position of its closest prey. Used in states where the predator should not hunt the prey. Parameters: _w_.
    * __set_retreat__: the predator is repositioned at a given distance away from the flock and given a new speed. Parameters: _distAway_, _speed_.
   
* Non-interacting actions:
    * __wiggle__: the individuals turn by a random angle controlled by the weight of this perpendicular to the agent' heading steering force, sampled from the range [-w,w]_. Parameters: _w_.
    * __hold_current__: the agents tries to hold a constant position. Parameters: _w_.
    
* Hunting actions:
    * __select_flock__: the predator chooses a flock as its target. Selection can be made based on the flock's size or proximity. Parameters: _selection_.
    * __shadowing__: the predator follows (or tries to follow) its target flock from a given angle and distance, keeping a constant speed that scales from the speed of its target. Parameters: _bearing_ (angle starting from the flock's heading), _distance_, _placement_ (whether to automatically reposition the predator to the given shadowing position), _prey_speed_scale_, _w_.
    *  __chase_closest_prey__: the predator turns towards the closest pigeon-agent at every time point (target) and moves with a speed that scales from this agent's speed. Parameters: _prey_speed_scale_, _w_. 
    *  __lock_on_closest_prey__: the predator turns towards its target pigeon-agents, selected at the beginning of the attack (state-switch) as the closest prey. The predator moves with a speed that scales from this target's speed. Parameters: _prey_speed_scale_, _w_. 
    
    
_Note_: More actions are included in the code (__actions__ folder) but are not active in the present state of the model. The model's architecture is very flexible and allows for quick switch between the actions of the agents. Contact us for any questions concerning the specifics of actions switching. 

## _States_

States in the model are defined as combinations of actions. *Persistent* states have a user-defined duration, whereas *Transient* states can change after a time-step. The transition between states is controlled by a user-defined transition matrix. In the default version of the model, pigeon-agents move based on a single state. The hunting strategy of the predator is built on a chain of persistent states.

The user can easily add and remove states in the model. Feel free to contact us for details on how to do so. 

## _Initialization_

The initial conditions of the agents can be controlled from the config file. Agents can be initiated in a flock formation (default for pigeon-agents), in a random position (default for predator) or based on an input csv file with individual positions and headings for each agent.


## _The simulation_

Agents move in a periodic space and have a personal 'state' as mentioned above. This state (through its actions) defines how each agent will update its position and velocity. Time-steps in the model are referred to as 'ticks'. 

### __Application keys:__

1. PgUp: speed-up simulation
2. PgDown: slow-down simulation
3. Space: pause/continue simulation
4. Right Arrow: run 1 simulation step
5. A: darkens background
6. T: shows/hides the position trail of each pigeon-agent
7. Shift+T: shows/hides the position trail of each predator-agent
8. K: kills or revives predator-agents
9. 1: applies colormap of id of pigeon-agents
10. 2: applies colormap of speed of pigeon-agents
11. 4: applies colormap of state of pigeon-agents
12. 5: applies colormap of flock id of pigeon-agents (which flock each individual belongs to)
13. 6: colors the pigeon-agent that is the target of a predator
14. Shift+1: applies colormap of id of predator-agents
15. Shift+2: applies colormap of state of predator-agents

## _Data Collection_

The model exports data in _.csv_ format. It creates a unique folder within the user-defined data_folder (in the config.json), in which it saves a single .csv file for each Observer, as defined in the config file. Sampling frequency and output name of each files are also controled by the config.
The config is also copied to the saving directory, along with an empty txt file that has its initial name. All details concerning data collection are defined in its _Analysis_ section, organized in different observers.

In its current state, the model exports (1) timeseries of positions, heading, speed etc for each agent, (2) timeseries on information of the neighbors of each agent (id, distance to, bearing angle etc), (3) information about the flock(s) that form during the simulation, (4) timeseries of the effect of coorindation forces acting on each agent. More observers are present in the model and can be used by including them in the config file.

## Authors
* **Marina Papadopoulou** - PhD student - For any problem email at: <m.papadopoulou.rug@gmail.com>
* **Dr. Hanno Hildenbrandt** - PhD supervisor
