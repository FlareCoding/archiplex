#!/bin/python3

from rich.console import Console
from rich.prompt import Confirm
from rich.prompt import Prompt
import json
import os

console = Console()

def main():
    console.print("Welcome to the Archiplex Experiment Code Generator", style="bold green")
    
    experiment_name = Prompt.ask("Enter your experiment name")
    experiment_path = os.path.join("..", "..", "experiments", experiment_name)  # Adjust path as necessary
    
    # Measurement selection through yes/no prompts
    measurements = {
        "Overall Throughput": False,
        "Overall Latency": False,
        "Per-function Latencies": False,
        "Power": False,
        "Perf Statistics": False
    }
    
    for measurement in measurements:
        measurements[measurement] = Confirm.ask(f"Include {measurement}?")
    
    # Custom Measurements
    custom_measurements = Prompt.ask("Enter any other custom metrics to include (space-separated)", default="")
    if custom_measurements.strip():
        measurements["Custom"] = custom_measurements.split()
    
    create_experiment_structure(experiment_path, measurements)

def create_experiment_structure(base_path, measurements):
    paths = ["config", "data/raw", "data/processed", "scripts", "analysis"]
    for path in paths:
        os.makedirs(os.path.join(base_path, path), exist_ok=True)
    
    # Generate initial config.json based on selected measurements
    config = {"measurements": measurements}
    with open(os.path.join(base_path, "config", "config.json"), 'w') as config_file:
        json.dump(config, config_file, indent=4)
    
    console.print("Experiment setup complete!", style="bold blue")

if __name__ == "__main__":
    main()
    
