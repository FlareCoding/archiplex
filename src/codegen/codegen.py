#!/usr/bin/env python3

from configparser import ConfigParser
from rich.console import Console
from rich.prompt import Confirm, Prompt
import os
import shutil

console = Console()

def copy_templates_to_experiment(base_path):
    templates_path = os.path.join("templates")  # Adjust as necessary
    target_src_path = os.path.join(base_path, "src")
    target_makefile_path = base_path  # Makefile is directly under the experiment dir

    # Paths of the template files
    benchmark_template = os.path.join(templates_path, "benchmark.c")
    makefile_template = os.path.join(templates_path, "Makefile")

    # Destination paths
    benchmark_destination = os.path.join(target_src_path, "benchmark.c")
    makefile_destination = os.path.join(target_makefile_path, "Makefile")

    # Copying the template files to the target directory
    shutil.copy(benchmark_template, benchmark_destination)
    shutil.copy(makefile_template, makefile_destination)

def create_run_script(base_path):
    run_script_path = os.path.join(base_path, "scripts", "run.sh")
    with open(run_script_path, 'w') as run_script:
        run_script.write("""#!/bin/bash

# Navigate to the experiment's root directory
cd "$(dirname "$0")/../bin"
        
# Run the compiled experiment binary
./benchmark
""")
    # Make the script executable
    os.chmod(run_script_path, 0o755)
    
def create_experiment_structure(base_path, measurements, custom_metrics):
    paths = ["config", "data/raw", "data/processed", "scripts", "analysis", "src"]
    for path in paths:
        os.makedirs(os.path.join(base_path, path), exist_ok=True)

    # Initialize the configuration with default settings and selected measurements
    config = ConfigParser()
    config['Settings'] = {
        'EXPERIMENT_VERSION': '1.0.0',      # Default version
        'EXPERIMENT_LOOP_COUNT': '100000',  # Default iteration count for the benchmark loop
        'EXPERIMENT_ITERATIONS': '30',
    }
    config['Measurements'] = {k: str(v) for k, v in measurements.items()}
    if custom_metrics:
        config['Custom Metrics'] = {metric: 'True' for metric in custom_metrics}

    with open(os.path.join(base_path, "config", "config.ini"), 'w') as config_file:
        config.write(config_file)

    # Copy the source template files to the experiment directory 
    copy_templates_to_experiment(base_path);

    # Create the run.sh script
    create_run_script(base_path)
    
    console.print("Experiment setup complete!", style="bold blue")
    
def main():
    console.print("Welcome to the Archiplex Experiment Code Generator", style="bold green")

    experiment_name = Prompt.ask("Enter your experiment name")
    # Assuming the script is run from the archiplex/src/codegen directory,
    # adjust the path to correctly point to the experiments directory
    experiment_path = os.path.join("..", "..", "experiments", experiment_name)

    # Measurement selection through yes/no prompts
    measurements = {
        "Overall_Throughput": False,
        "Overall_Latency": False,
        "Per_function_Latencies": False,
        "Power": False,
        "Perf_Statistics": False
    }

    for measurement in measurements:
        measurements[measurement] = Confirm.ask(f"Include {measurement}?")

    # Custom Measurements
    custom_measurements = Prompt.ask("Enter any other custom metrics to include (space-separated)", default="")
    custom_metrics_list = custom_measurements.split() if custom_measurements.strip() else []

    create_experiment_structure(experiment_path, measurements, custom_metrics_list)

if __name__ == "__main__":
    main()

