# Branch Prediction Simulator

This repository contains a C++ simulator for evaluating branch prediction strategies, developed as part of **ECE 563: Computer Architecture** at North Carolina State University, Fall 2023. The simulator supports **bimodal**, **gshare**, and **hybrid** predictors, and provides analysis of misprediction behavior across benchmarks.

## Project Summary

Branch prediction is key to modern processor performance. This project implements and compares different branch predictors by simulating their behavior using trace data. The simulator collects misprediction rates and allows configuration of prediction table sizes and history lengths.

- **Course**: ECE 563 – Microprocessor Architecture
- **Project**: Branch Prediction 
- **Author**: Padmanabha Nikhil Bhimavarapu
- **Language**: C++
- **Prediction Models**:
  - Bimodal (2-bit saturating counters)
  - Gshare (global history + PC indexing)
  - Hybrid (chooser table to select between gshare and bimodal)

## Predictors Supported

### Bimodal Predictor
- Uses only PC bits for indexing.
- Simple table of 2-bit saturating counters.
- No history tracking.

### Gshare Predictor
- XORs PC index bits with a global branch history register.
- Adjustable PC bit width (`m`) and global history length (`n`).
- Offers better specialization across dynamic branch behavior.

### Hybrid Predictor (ECE 563 only)
- Combines gshare and bimodal predictors.
- Uses a chooser table (2-bit counters) to dynamically select the better predictor.

## How to Run

### Command Line Syntax:

```bash
# Bimodal
./sim bimodal <M2> <tracefile>

# Gshare
./sim gshare <M1> <N> <tracefile>

# Hybrid
./sim hybrid <K> <M1> <N> <M2> <tracefile>
