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
    target_analysis_path = Path(base_path).joinpath("analysis")
    target_makefile_path = Path(base_path)  # Makefile is directly under the experiment dir

    # Paths of the template files
    benchmark_template = templates_path.joinpath("benchmark.c")
    makefile_template = templates_path.joinpath("Makefile")
    notebooks_template_path = templates_path.joinpath("notebooks")

    # Ensure target directories exist
    target_src_path.mkdir(parents=True, exist_ok=True)
    target_analysis_path.mkdir(parents=True, exist_ok=True)
    target_makefile_path.mkdir(parents=True, exist_ok=True)

    # Destination paths
    benchmark_destination = target_src_path.joinpath("benchmark.c")
    makefile_destination = target_makefile_path.joinpath("Makefile")

    # Copying the template files to the target directory
    shutil.copy(benchmark_template, benchmark_destination)
    shutil.copy(makefile_template, makefile_destination)
    
    # Copying all notebooks from the template notebooks directory to the target analysis directory
    if notebooks_template_path.exists() and notebooks_template_path.is_dir():
        for notebook_file in notebooks_template_path.iterdir():
            if notebook_file.is_file():  # Make sure it's a file
                shutil.copy(notebook_file, target_analysis_path.joinpath(notebook_file.name))

def create_run_script(base_path, configuration, measurements):
    run_script_path = os.path.join(base_path, "scripts", f"run_{configuration}.sh")
    EXPCONFIG = f"-DCONFIG_{configuration.upper()} "
    if measurements['Throughput']:
        EXPCONFIG += "-DCONFIG_MEASURE_THROUGHPUT "
    if measurements['Latency']:
        EXPCONFIG += "-DCONFIG_MEASURE_LATENCY "

    with open(run_script_path, 'w') as run_script:
        run_script.write(f"""#!/bin/bash

# Navigate to the experiment's root directory
cd "$(dirname "$0")/.."

make clean 1>&2
make EXPCONFIG="{EXPCONFIG}" 1>&2
      
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
        'EXPERIMENT_LOOP_COUNT': '100000',
        'EXPERIMENT_WORK_MIN_SIZE': '1',
        'EXPERIMENT_WORK_MAX_SIZE': '1',
        'EXPERIMENT_WORK_SIZE_STEP': '1',
        'EXPERIMENT_ITERATIONS': '30',
        'EXPERIMENT_CONFIGURATIONS': 'baseline',
        'EXPERIMENT_RUN_ID': '0',
        'EXPERIMENT_RUN_CONFIGURATION': configurations[0],
    }
    config['Settings']['EXPERIMENT_CONFIGURATIONS'] = ', '.join(configurations)

    with open(os.path.join(base_path, "config", "config.ini"), 'w') as config_file:
        config.write(config_file)

    # Copy the source template files to the experiment directory 
    copy_templates_to_experiment(base_path)

    # Create run scripts for all specified configurations
    for configuration in configurations:
        create_run_script(base_path, configuration, measurements)
    
    console.print("Experiment setup complete!", style="bold blue")
    
def main():
    # Set up signal handler for SIGINT
    signal.signal(signal.SIGINT, signal_handler)

    console.print("Welcome to the Archiplex Experiment Code Generator", style="bold green")

    experiment_name = Prompt.ask("Enter your experiment name")

    script_dir = Path(__file__).parent.absolute()

    if str(script_dir).startswith("/usr/local/share"):
        experiments_dir = Path(os.path.expanduser("~")).joinpath("experiments")
    else:
        experiments_dir = script_dir.joinpath("..", "..", "experiments").resolve()

    experiment_path = experiments_dir.joinpath(experiment_name)
    
    # Measurement selection through yes/no prompts
    measurements = {
        "Throughput": False,
        "Latency": False,
        "Power": False,
        "Perf_Statistics": False
    }

    for measurement in measurements:
        measurements[measurement] = Confirm.ask(f"Include {measurement}?", default="y")

    # Ask for a comma-separated list of configurations
    configurations_input = Prompt.ask("Enter a comma-separated list of configurations the experiment will support", default="baseline")
    configurations = [config.strip() for config in configurations_input.split(',')]

    create_experiment_structure(experiment_path, measurements, configurations)

if __name__ == "__main__":
    main()

