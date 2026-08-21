# ggRover Cloud CAD, Kinematics, Simulation & 3D Print Automation Workflow

This document specifies the end-to-end automated engineering workflow for **ggRover**: from cloud-native parametric CAD design in **Onshape**, through mathematical kinematics (Rumoca + CasADi), 3D physics simulation (Webots), to headless 3D print slicing and wireless print deployment (**LulzBot Mini + OctoPi**).

---

## 1. Master System Architecture

```mermaid
flowchart TD
    subgraph 1. Parametric CAD & Modular Subframe (Onshape)
        Onshape["Onshape Cloud CAD"]
        Agent["AI Agent (Antigravity)"]
        Onshape <-->|Onshape REST API| Agent
        
        note1["- 100mm Wheel clearance check<br/>- Ackermann steering linkage sweep<br/>- Modular subframe split for 152mm LulzBot bed"]
    end

    subgraph 2. Physics & Kinematics (Rumoca + CasADi)
        MO["Modelica Vehicle Model (.mo)"]
        Rumoca["Rumoca (Rust Modelica Compiler)"]
        CasADi["CasADi (Symbolic Optimizer)"]

        Agent --> MO --> Rumoca --> CasADi
        CasADi -->|Ackermann Ratio & Trajectory Limits| Agent
    end

    subgraph 3. Virtual 3D Simulation & Firmware SIL
        URDF["Exported URDF + Meshes"]
        Sim["Webots 3D Physics Simulator"]
        Firmware["ggRover STM32 Firmware (PlatformIO)"]

        Agent -->|Export URDF| URDF --> Sim
        Firmware <-->|HIL / SIL Telemetry| Sim
    end

    subgraph 4. Headless Slicing & OctoPi Deployment
        Slicer["PrusaSlicer CLI (0.5s Background Slice)"]
        OctoPi["OctoPi (LulzBot Mini REST API)"]

        Agent -->|Export STL| Slicer
        Slicer -->|Read Exact Printed Mass| Agent
        Agent -->|Update URDF Mass| URDF
        Slicer -->|Upload G-code & Auto-Print| OctoPi
    end
```

---

## 2. Onshape Naming & API Conventions

To allow the AI Agent to query and update your CAD assembly cleanly without manual guessing:

### A. Parametric Variables (`FeatureList`)
Define key dimensions using standard `#variable` names:
- `#servo_horn_length` (e.g. `20 mm`)
- `#tie_rod_length` (e.g. `85 mm`)
- `#steering_arm_length` (e.g. `30 mm`)
- `#chassis_track_width` (e.g. `160 mm`)
- `#chassis_wheelbase` (e.g. `220 mm`)

### B. Assembly Mates
Rename mates in your Onshape Assembly tree:
- `Revolute_Servo_Horn` (Driven by agent to test steering sweeps $-45^\circ \to +45^\circ$)
- `Revolute_Kingpin_FL` & `Revolute_Kingpin_FR` (Read back by agent to compute Ackermann steering ratios)

### C. Part & Subframe Assembly Naming
Ensure main components are named for automatic URDF mapping:
- `Subframe_Front_Steering` (Verified $\le 145\text{ mm} \times 145\text{ mm}$ for LulzBot bed)
- `Subframe_Center_Tray`
- `Subframe_Rear_Drive`
- `Wheel_FL`, `Wheel_FR`, `Wheel_RL`, `Wheel_RR` ($100\text{ mm}$ off-the-shelf wheels)

---

## 3. Modular Subframe Design Strategy (LulzBot Mini Fit)

Because the $100\text{ mm}$ wheels require a $200\text{ mm} - 280\text{ mm}$ vehicle wheelbase, the chassis exceeds the LulzBot Mini's **$152\text{ mm} \times 152\text{ mm} \times 158\text{ mm}$** build envelope.

### Design Pattern: Carbon Fiber / Standoff Rail Subframe
1. **Front Module ($\le 145\text{ mm}$):** Steering knuckles, servo mount, kingpin pivots.
2. **Center Module ($\le 145\text{ mm}$):** STM32 Blue Pill, XBee, BME280, battery tray.
3. **Rear Module ($\le 145\text{ mm}$):** Motor mounts and rear axle.
4. **Structural Rails:** All 3 modules are connected using $6\text{ mm}$ carbon fiber tubes or aluminum standoffs running the full length of the chassis.

---

## 4. Headless Slicing & OctoPi Wireless Deployment

### Step 1: Background Slicing via PrusaSlicer CLI
The AI agent runs a 0.5-second headless command to slice the exported Onshape `.stl` using the official LulzBot Mini profile and 20% Gyroid infill:

```bash
prusa-slicer --export-gcode \
  --load ~/.config/PrusaSlicer/printer/LulzBot_Mini_SE_0.5.ini \
  --fill-density 20% \
  --fill-pattern gyroid \
  --output Subframe_Front_Steering.gcode \
  Subframe_Front_Steering.stl
```

### Step 2: Automatic Mass Extraction for Physics Sim
The AI agent reads the sliced G-code header metadata (`Plastic mass: 18.4 g`), updates the mass in `<link name="subframe_front">` in the Webots URDF, ensuring **100% real-world mass match in 3D physics simulation**.

### Step 3: OctoPi REST API Auto-Print
The agent uploads the sliced G-code directly to OctoPi via HTTP REST API to start the physical 3D print:

```bash
curl -H "X-Api-Key: YOUR_OCTOPRINT_API_KEY" \
  -F "file=@Subframe_Front_Steering.gcode" \
  -F "print=true" \
  "http://octopi.local/api/files/local"
```

---

## 5. Summary of Tool Roles

| Layer | Tool / Tech | Responsibility |
| :--- | :--- | :--- |
| **Cloud CAD** | Onshape | Parametric 3D modeling, assembly mates, STEP/STL/URDF export via REST API. |
| **AI Orchestration** | Antigravity AI Agent | Kinematic sweeps, bed fit verification, automated Onshape variable mutation. |
| **Physics Compiler** | Rumoca | Parses Modelica `.mo` vehicle equations to symbolic DAEs. |
| **Symbolic Math** | CasADi | Calculates Jacobians, optimizes trajectory bounds and Ackermann geometry. |
| **3D Simulator** | Webots | Simulates 4WD skid-steer / Ackermann dynamics, sensor feeds, and STM32 firmware SIL. |
| **Headless Slicer** | PrusaSlicer CLI | 0.5-second background STL slicing for exact 20% infill mass calculation. |
| **Print Server** | OctoPi (OctoPrint API) | Wireless G-code upload, auto-print trigger, and webcam print monitoring. |
