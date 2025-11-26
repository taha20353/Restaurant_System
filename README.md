# Restaurant Simulation in C++

This project is a standalone, simple restaurant simulation implemented in C++.

## Overview

This program simulates the process of orders arriving and being prepared by chefs of different types: Normal, Vegan, and VIP. Orders have characteristics like arrival time, size, and priority (for VIP customers). The simulation assigns orders to chefs based on availability, speeding, and priority rules, while auto-promoting long-waiting normal orders to VIP status.

## Features

- Event-driven simulation of restaurant orders and chefs.
- Supports three types of orders and chefs: Normal, Vegan, VIP.
- Auto-promotion of normal orders to VIP after a waiting threshold.
- Priority queue for VIP orders, standard queues for others.
- Calculates and outputs order finish time, waiting time, and service time.
- Provides summary statistics at the end, including average waiting and service times and percentage of auto-promoted orders.

## How to Build

No specific build scripts; you can compile the program using a standard C++ compiler.

Example using g++:

```
g++ -std=c++17 -o restaurant_sim main.cpp
```

## How to Run

Input file format:

- First line: Number of chefs for Normal (N), Vegan (G), VIP (V) respectively.
- Second line: Speed of each chef type.
- Third line: Number of orders (M).
- Next M lines: Each order with format `<TYPE_CHAR> <RT> <ID> <SIZE> <MONEY>` where:
  - `TYPE_CHAR`: 'G' for Vegan, 'V' for VIP, and any other for Normal.
  - `RT`: Arrival time.
  - `ID`: Unique order ID.
  - `SIZE`: Work units required.
  - `MONEY`: Order money value (used for VIP priority).

Run the executable and ensure `input.txt` is present in the working directory:

```
./restaurant_sim
```

Simulation output is written to `output.txt` with order details and summary statistics.

## Output Format

The output file includes a header line, one line per finished order with finish time, ID, arrival time, waiting time, and service time. At the end, a summary reports:

- Total orders by type.
- Number of chefs by type.
- Average waiting and service times.
- Percentage of auto-promoted orders.

## Notes

- The code uses C++17 features like smart pointers and STL containers.
- The simulation advances time based on next event (arrival or chef availability).
- Orders assigned to chefs according to priority and type matching.
- Designed for easy extension and modification.

## License

This project is released under the MIT License.
