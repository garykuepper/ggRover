# CAD, AI Agent (MCP), Rumoca & CasADi Architecture for ggRover

This document details the complete end-to-end workflow architecture for **ggRover**: from parametric Onshape/FreeCAD CAD optimization via AI Agents (MCP), through **Rumoca** and **CasADi** physics modeling, to virtual 3D simulation in **Webots** / **Gazebo**.

---

## 1. System Architecture & Workflow Pipeline

```mermaid
flowchart TD
    subgraph Phase 1: CAD Kinematics & Linkage Optimization
        Onshape["Onshape (Cloud CAD / REST API)"]
        FreeCAD["FreeCAD (Headless / Python API)"]
        MCP["FreeCAD / Onshape MCP Server"]
        Agent["AI Agent (Antigravity)"]

        Onshape -->|Import STEP / Parasolid| FreeCAD
        Agent <-->|MCP Tools (Query/Mutate Geometry & Sweep Angles)| MCP
        MCP <--> FreeCAD
        FreeCAD -->|Optimized Ackermann & Linkage Dimensions| Agent
    end

    subgraph Phase 2: Equation Compilation & Symbolic Optimization
        MO["Modelica Vehicle Model (.mo)<br/>(Chassis, Suspension, Tire Scrub)"]
        Rumoca["Rumoca (Rust Modelica Frontend)"]
        CasADi["CasADi (Symbolic Math Engine)"]
        Solver["IPOPT / Non-Linear Optimization"]

        Agent -->|Writes Vehicle DAE Equations| MO
        MO -->|Parses & Flattens| Rumoca
        Rumoca -->|Exports Symbolic C/Python Functions| CasADi
        CasADi -->|Calculates Jacobians & Hessians| Solver
        Solver -->|Optimal Steer & Trajectory Limits| Agent
    end

    subgraph Phase 3: 3D Simulation & Firmware Testing
        URDF["URDF / Robot Description"]
        Simulator["Webots / Gazebo Simulator (gz-mcp / ROS 2)"]
        Firmware["ggRover STM32 Firmware (Drivetrain & Odometry)"]

        FreeCAD -->|Export URDF + Meshes| URDF
        URDF --> Simulator
        Rumoca -->|C++ Header Export| Simulator
        Firmware <-->|HIL / SIL Telemetry (Serial / Sockets)| Simulator
    end
```

---

## 2. Deep Dive: Rumoca vs. CasADi

A common point of confusion is how **Rumoca** and **CasADi** differ and why both are needed:

| Aspect | **Rumoca** | **CasADi** |
| :--- | :--- | :--- |
| **Role** | **Modelica Compiler / Frontend** (written in Rust) | **Symbolic Math & Algorithmic Differentiation Engine** |
| **Primary Job** | Parses human-readable multi-domain physical models (`.mo`) and flattens them into Differential-Algebraic Equations (DAEs). | Computes exact Jacobians/Hessians and interfaces with non-linear optimization solvers (IPOPT). |
| **Code Level** | High-level declarative physical component descriptions. | Lower-level mathematical matrix expressions & symbolic graphs. |
| **Solvers** | Does **not** include built-in non-linear optimization solvers. | Built specifically to feed solvers for MPC and trajectory optimization. |

### Why They Work Together
1. **Readable Physics in Modelica:** You write clean physical models for `ggRover` in Modelica (4WD chassis, suspension linkage, motor curves).
2. **Parsing via Rumoca:** Rumoca compiles the `.mo` files into flattened mathematical state-space equations $\dot{x} = f(x, u)$.
3. **Symbolic Optimization in CasADi:** Rumoca exports these equations into **CasADi symbolic functions**, which calculate exact derivatives for trajectory optimization and Model Predictive Control (MPC).

---

## 3. AI Agent CAD Control via Onshape & FreeCAD MCP

### Onshape REST API Capabilities
Because Onshape is cloud-native, its REST API allows an AI Agent to:
- Mutate FeatureStudio parametric variables (`#steering_horn_length`, `#tie_rod_length`).
- Trigger assembly recomputations and query interference/collision status.
- Extract mass properties ($CoG$, inertia tensor $I_{xx}, I_{yy}, I_{zz}$) and export STEP/URDF files.

### FreeCAD MCP Integration
Running a local FreeCAD MCP server allows the agent to:
- Sweep steering linkage joints through their full rotation range ($0^\circ \to 35^\circ$).
- Calculate inner vs. outer wheel Ackermann steering angles dynamically.
- Check for mechanical binding or suspension frame collisions during sweeps.

---

## 4. 3D Simulator Integration (Webots & Gazebo MCP)

- **Webots:** Uses a Python `Supervisor` API (easily wrapped in an MCP server) for lightweight desktop 3D simulation of 4WD skid-steer friction and sensor telemetry.
- **Gazebo Harmonic:** Leverages `gazebo-mcp` or `ros2_mcp` to allow the AI agent to spawn models, control simulation time, and inspect ROS 2 sensor topics (`/cmd_vel`, `/odom`, IMU).
