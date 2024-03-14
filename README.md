## Physical Design Automation

### Homework 1: P&R Tool
Utilizing a P&R tool (Cadence Innovus) to do APR to generate a layout.  
Goal：Optimizing timing, total area of chip, and total wire length without violating any timing or DRC constraints.
### Homework 2: Two-way Min-cut Partitioning
Let 𝑪 be a set of cells and 𝑵 be a set of nets. Each net connects a subset of cells. The two-way min-cut partitioning problem is to partition the cell set into two disjoint groups 𝑨 and 𝑩, where each group of cells is put in a different die. The 
cost of a two-way partitioning is measured by the cut size, which is the number of nets having cells in both groups.  
Goal：Implement an existing algorithm or develop your own algorithm to solve the problem of two-way min-cut partitioning. 
### Homework 3: Fixed-outline Floorplanning with Fixed and Soft Modules 
Given a fixed outline, a set of rectangular hard modules each with a fixed position, 
a set of rectangular soft modules each with a minimum area, and a set of 2-pin nets 
along with their weights, you are asked to implement an existing algorithm or 
develop your own algorithm to determine the shape and position of each soft 
module. All modules must be placed within the outline without overlapping, with 
the objective of minimizing the total weighted wirelength between modules. 
### Homework 4: Global Placement 

