# *The HoPE model*
*Homing Pigeons Escape*

A agent-based model of flocking under predation, adjusted to the collective motion and collective escape of pigeons. Individuals are moving around in a 2-dimensional periodic space.


## Prerequisites
* Operating system: Windows.
* Graphic card with support for OpenGL 4.4 or later.

# The model

## _The framework_ 

This model is based on self-organization. Agent interact with their surrounding neighbors based on the rules of attraction, alignment and avoidance. Pigeon-agents flock together and avoid a mobile predator. Predator-agents chase and attack pigeon-agents. Cathes of prey (pigeon-agents) are not modelled.

## _Parameters_
All user-defined parameters are parsed by combining a series of .json files: *config.json* (simulation parameters),  *pigeon_dev.json* (prey parameters, here adjusted to pigeons) and *predator.json* (predators parameters). Distance is measured in meters [m], time in seconds [s] and angles in degrees [deg].ment...

## _Individual Actions_

Actions are the basic elements controlling the movement of each agent in the simulations. Each action represents a steering (pseudo) force that acts on each agent. The weighted sum of all actions change the direction of motion (heading) and the speed of individuals. Each action has each own user-defined parameters. Multiple actions are combined to create states. The majority of actions control the interactions between agents (coordination between pigeon-agents, escape actions of pigeons-agents from the predator-agents, and hunting actions of the predator-agents towards pigeon-agents). The model is based mainly on **topological** interactions.

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
    * __wiggle__: the individuals turn by a random angle controlled by the weight of this perpedicular to the agent' heading steering force, sampled from the range defined by -_w_ and _w_. Parameters: _w_.

Default predator-agents actions:
* Avoid actions: 
    * __avoid_closest_prey*: the predator turns away from the position of its closest prey. Used in states where the predator should not hunt the prey. Parameters: _w_.
    * __set_retreat__: the predator is repositioned at a given distance away from the flock and given a new speed. Parameters: _distAway_, _speed_.
   
* Non-interacting actions:
    * __wiggle__: the individuals turn by a random angle controlled by the weight of this perpedicular to the agent' heading steering force, sampled from the range defined by -_w_ and _w_. Parameters: _w_.
    * __hold_current__: the agents tries to hold a constant position. Parameters: _w_.
    
* Hunting actions:
    * __select_flock__: the predator chooses a flock as its target. Selection can be made based on the flock's size or proximity. Parameters: _selection_.
    * __shadowing__: the predator follows (or tries to follow) its target flock from a given angle and distance, keeping a contant speed that scales from the speed of its target. Parameters: _bearing_ (angle starting from the flock's heading), _distance_, _placement_ (whether to automatically reposition the predator to the given shadowing position), _prey_speed_scale_, _w_.
    *  __chase_closest_prey__: the predator turns towards the closest pigeon-agent at every time point (target) and moves with a speed that scales from this agent's speed. Parameters: _prey_speed_scale_, _w_. 
    *  __lock_on_closest_prey__: the predator turns towards its target pigeon-agents, selected at the beginning of the attack (state-switch) as the closest prey. The predator moves with a speed that scales from this target's speed. Parameters: _prey_speed_scale_, _w_. 
    
    
_Note_: More actions are included in the code (__actions__ folder) but are not active in the present state of the model. The model's architecture is very flexible and allows for quick switch between the actions of the agents. Contact us for any questions concerning the specifics of actions switching. 

## _States_

States in the model are defined as combinations of actions. *Persistent* states have a user-defined duration, whereas *Transient* states can change after a time-step. The transition between states is controlled by a user-defined transition matrix. In the default version of the model, pigoen-agents move based on a single state. The hunting strategy of the predator is built on a chain of persistent states.

The user can easily add and remove states in the model. Feel free to contact us for details on how to do so. 

## _Initialization_

The initial conditions of the agents can be controlled from the config file. Agents can be initiated in a flock formation (default for pigeon-agents), in a random position (default for predator) or based on an input csv file with individual positions and headings for each agent.


## _The simulation_

Agents move in a periodic space and have a personal 'state' as mentioned above. This state (through its actions) defines how each agent will update its position and heading.

### __Application keys:__

1. PgUp: speed-up simulation
2. PgDown: slow-down simulation
3. Space: stop/continue simulation
4. Right Arrow: run 1 simulation step

## _Data Collection_

Under development.

## Authors
* **Marina Papadopoulou** - PhD student - For any problem email at: <m.papadopoulou@rug.nl>
* **Dr. Hanno Hildenbrandt** - PhD supervisor
