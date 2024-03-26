#!/usr/bin/env python3

from configparser import ConfigParser
from rich.console import Console
from rich.prompt import Confirm, Prompt
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
    custom_metrics_list = custom_measurements.split() if custom_measurements.strip() else []

    create_experiment_structure(experiment_path, measurements, custom_metrics_list)

def create_experiment_structure(base_path, measurements, custom_metrics):
    paths = ["config", "data/raw", "data/processed", "scripts", "analysis"]
    for path in paths:
        os.makedirs(os.path.join(base_path, path), exist_ok=True)

    # Generate initial config.ini based on selected measurements
    config = ConfigParser()
    config['Measurements'] = {k: str(v) for k, v in measurements.items()}
    if custom_metrics:
        config['Custom Metrics'] = {metric: 'True' for metric in custom_metrics}

    with open(os.path.join(base_path, "config", "config.ini"), 'w') as config_file:
        config.write(config_file)

    console.print("Experiment setup complete!", style="bold blue")

if __name__ == "__main__":
    main()

