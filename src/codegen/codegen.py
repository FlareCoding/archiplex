#!/usr/bin/env python3

from configparser import ConfigParser
from rich.console import Console
from rich.prompt import Confirm, Prompt
import os
from pathlib import Path
import shutil
import sys
import signal

def signal_handler(sig, frame):
    print('\nCancelled experiment creation, exiting...')
    sys.exit(0)

console = Console()

def copy_templates_to_experiment(base_path):
    script_dir = Path(__file__).parent.absolute()
    templates_path = script_dir.joinpath("templates").resolve()

    target_src_path = Path(base_path).joinpath("src")
    target_makefile_path = Path(base_path)  # Makefile is directly under the experiment dir

    # Paths of the template files
    benchmark_template = templates_path.joinpath("benchmark.c")
    makefile_template = templates_path.joinpath("Makefile")

    # Ensure target directories exist
    target_src_path.mkdir(parents=True, exist_ok=True)
    target_makefile_path.mkdir(parents=True, exist_ok=True)

    # Destination paths
    benchmark_destination = target_src_path.joinpath("benchmark.c")
    makefile_destination = target_makefile_path.joinpath("Makefile")

    # Copying the template files to the target directory
    shutil.copy(benchmark_template, benchmark_destination)
    shutil.copy(makefile_template, makefile_destination)

def create_run_script(base_path, configuration):
    run_script_path = os.path.join(base_path, "scripts", f"run_{configuration}.sh")
    with open(run_script_path, 'w') as run_script:
        run_script.write("""#!/bin/bash

# Navigate to the experiment's root directory
cd "$(dirname "$0")/.."

make clean
make EXPCONFIG=-DCONFIG_MEASURE_LATENCIES
      
# Run the compiled experiment binary
cd bin
./benchmark
""")
    # Make the script executable
    os.chmod(run_script_path, 0o755)
    
def create_experiment_structure(base_path, measurements, configurations = ['baseline']):
    paths = ["config", "data/raw", "data/processed", "scripts", "analysis", "src"]
    for path in paths:
        os.makedirs(os.path.join(base_path, path), exist_ok=True)

    # Initialize the configuration with default settings and selected measurements
    config = ConfigParser()
    config['Settings'] = {
        'EXPERIMENT_VERSION': '1.0.0',
        'EXPERIMENT_LOOP_COUNT': '1000000',
        'EXPERIMENT_ITERATIONS': '30',
        'EXPERIMENT_CONFIGURATIONS': 'baseline',
        'EXPERIMENT_RUN_ID': '0',
        'EXPERIMENT_RUN_CONFIGURATION': ','.join(configurations),
    }
    config['Settings']['EXPERIMENT_CONFIGURATIONS'] = ','.join(configurations)
    config['Measurements'] = {k: str(v) for k, v in measurements.items()}

    with open(os.path.join(base_path, "config", "config.ini"), 'w') as config_file:
        config.write(config_file)

    # Copy the source template files to the experiment directory 
    copy_templates_to_experiment(base_path);

    # Create run scripts for all specified configurations
    for configuration in configurations:
        create_run_script(base_path, configuration)
    
    console.print("Experiment setup complete!", style="bold blue")
    
def main():
    # Set up signal handler for SIGINT
    signal.signal(signal.SIGINT, signal_handler)

    console.print("Welcome to the Archiplex Experiment Code Generator", style="bold green")

    experiment_name = Prompt.ask("Enter your experiment name")

    script_dir = Path(__file__).parent.absolute()
    experiments_dir = script_dir.joinpath("..", "..", "experiments").resolve()
    experiment_path = experiments_dir.joinpath(experiment_name)
    
    # Measurement selection through yes/no prompts
    measurements = {
        "Overall_Throughput": False,
        "Overall_Latency": False,
        "Per_function_Latencies": False,
        "Power": False,
        "Perf_Statistics": False
    }

    for measurement in measurements:
        measurements[measurement] = Confirm.ask(f"Include {measurement}?", default="y")

    # Ask for a comma-separated list of configurations
    configurations_input = Prompt.ask("Enter a comma-separated list of configurations the experiment will support", default="baseline")
    configurations = [config.strip() for config in configurations_input.split(',')]

    create_experiment_structure(experiment_path, measurement, configurations)

if __name__ == "__main__":
    main()

