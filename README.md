# Decentralized Split Execution of Heavy Tasks

Check the report file for a much detailed discussion and analysis.

## Problem Statement

Consider a set of $N$ devices $D_1, D_2, \ldots, D_N$ forming a mesh network, randomly deployed over an area of size $A \times A$. Each device has a fixed communication radius $R$; two devices are neighbors if and only if their Manhattan distance is at most $R$.

A heavy computation task is divided into $K$ components $C_1, C_2, \ldots, C_K$. Each component is installed on exactly one device (the assignment is initially uniform, i.e., each component is assigned to exactly $N/K$ devices in a round-robin fashion, then the mapping is randomly shuffled). To complete execution, a task must be processed by all $K$ components *in sequence*:

$$
C_1 \rightarrow C_2 \rightarrow \cdots \rightarrow C_K \rightarrow \text{return to origin}
$$

After being processed by component $C_i$, the task is forwarded through the mesh network (via flooding) to the device hosting component $C_{i+1}$.

Tasks are generated non-uniformly across the network area. Different cells of the $A \times A$ grid generate tasks at different rates, with arrival counts at each cell $(i, j)$ drawn from a Poisson distribution with spatially varying intensity $\lambda_{ij}$.

## System Overview

### Specification

- The deployment area is of size $A$, divided into an $N \times N$ grid of unit cells.

- Events are generated at these grid cells following a Poisson arrival process.

- Since event arrival is **non-uniform**, each cell $(i, j)$ has its own Poisson parameter $\lambda_{ij}$. The number of events at cell $(i,j)$ in a given time slot is drawn from $\text{Poisson}(\lambda_{ij})$, allowing arrival intensity to vary across the area.

- Some of the grid cells host *devices*. There are $M$ devices $D_1, D_2, \dots, D_M$.

- Two devices $D_i$ and $D_j$ can communicate directly if:
  $$
  |x_i - x_j| + |y_i - y_j| \leq R
  $$
  where $R$ is the communication radius.

- Each arriving event generates a *task*.

- Each task is divided into $K$ components $C_1, C_2, \ldots, C_K$, which must be processed in sequence.

- Each device can process only one of the $K$ components.

- Component distribution is **uniform**: each component $C_k$ is assigned to exactly $\lfloor M/K \rfloor$ devices.

### Processing of Tasks

- When an event is generated at cell $(i, j)$, it is picked up by all devices within communication radius $R$.

- Simulation time is divided into fixed intervals, each consisting of two phases:
  - **Computation phase:** Devices process tasks if they host the required component. Processing speed depends on service rate $\mu_k$.
  - **Signal phase:** All communication and message forwarding occurs.

- Once all components are processed:
  $$
  C_1 \to C_2 \to \cdots \to C_K
  $$
  the task returns to its origin cell.

### Commands

- Recommended platform: **Windows**

| Step | Command |
|------|--------|
| Compile | `g++ main.cpp -o program.exe` |
| Run | `./program.exe` |
| Run (live mode) | `./program.exe --live` |

---

### Input Format

- $T$ — number of test cases  
- $N$ — grid size ($N \times N$)  
- $R$ — communication radius  
- $M$ — number of devices (must be multiple of $K$)  
- $K$ — number of components per task  
- `computation_period` — computation phase duration (ms)  
- `signal_period` — signal phase duration (ms)  
- `simulation_time` — total simulation time (seconds)  
- $D[1 \ldots M]$ — device locations $(x, y)$  
- $\mu[1 \ldots K]$ — service rates  
- $\lambda[i][j]$ — arrival rates per grid cell  

### Input File Structure

T  
N  
R  
M  
K  

computation\_period  
signal\_period  
simulation\_time  

$D_{1}.x \;  D_{1}.y $    
$D_{2}.x \;  D_{2}.y $    
$\vdots$    
$D_{M}.x \;  D_{M}.y $    

$\mu_{1}$   
$\mu_{2}$   
$\vdots$    
$\mu_{K}$   

$\lambda_{0,0} \ \lambda_{0,1} \ \dots \ \lambda_{0,N-1}  $    
$\lambda_{1,0} \ \lambda_{1,1} \ \dots \ \lambda_{1,N-1}  $   
$\vdots  $   
$\lambda_{N-1,0} \ \lambda_{N-1,1} \ \dots \ \lambda_{N-1,N-1}$   

### Output

- Components are randomly and uniformly distributed across devices.

- For each test case:
  - Queue lengths are shown live (with `--live`)
  - Final metrics:
    - Average queue length $L$
    - Average time per task $T$ (ms)