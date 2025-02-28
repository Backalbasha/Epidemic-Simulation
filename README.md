# Epidemic-Simulation

This problem focuses on working with POSIX threads (pthreads), utilizing barriers and mutex locks to synchronize and manage parallel execution.

The simulation models the spread of an infectious disease in a rectangular area where individuals move based on predefined patterns. Each person has a direction (N, S, E, W) and an amplitude that dictates how far they move at each time step. If a person reaches a border, they reverse direction.

Individuals can be in one of three states: infected, immune, or susceptible. Infection occurs when a susceptible person remains in the same location as an infected individual. The disease lasts for a fixed INFECTED_DURATION, after which the person recovers and becomes immune for IMMUNE_DURATION before becoming susceptible again.

The simulation runs in discrete time steps, updating each person's status based on their interactions. Parallelization with pthreads ensures efficient execution by dividing computations among multiple threads while using barriers and mutex locks for synchronization.

For a detailed analysis of time efficiency and a comparison between parallel and serial execution times, please refer to the documentation.
