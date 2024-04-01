#!/bin/bash

# Function to read a configuration value for a given key
read_config_value() {
    local key=$1
    local value=$(awk -F ' = ' -v key="$key" '$1 == key {print $2}' "$CONFIG_FILE_PATH")
    echo $value
}

# Function to write a configuration value for a given key
write_config_value() {
    local key=$1
    local value=$2
    sed -i "/^$key/c\\$key = $value" "$CONFIG_FILE_PATH"
}

# Function to update experiment_run_configuration in the config file
update_experiment_run_configuration() {
    local config_name="$1"
    write_config_value "experiment_run_configuration" "$config_name"
}

# Check if at least one argument is provided
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <EXPERIMENT_DIR> [CONFIG_LIST]"
    exit 1
fi

EXPERIMENT_DIR=$1  # The first argument is the experiment directory

# Now dynamically set CONFIG_FILE_PATH
CONFIG_FILE_PATH="$EXPERIMENT_DIR/config/config.ini"

# Check if the experiment directory is provided and valid
if [ ! -d "$EXPERIMENT_DIR" ]; then
    echo "Error: '$EXPERIMENT_DIR' is not a valid directory."
    exit 2
fi

# Optional comma-separated list of configurations to run, provided as the second argument
CONFIG_LIST=$2

# Check if the config file exists
if [ ! -f "$CONFIG_FILE_PATH" ]; then
    echo "Error: Configuration file does not exist at '$CONFIG_FILE_PATH'"
    exit 2
fi

if [ -n "$CONFIG_LIST" ]; then
    IFS=',' read -r -a CONFIGURATIONS <<< "$CONFIG_LIST"
    echo "Running provided list of configurations: ${CONFIGURATIONS[*]}"
else
    # Read the experiment configurations into an array from the config file
    readarray -t CONFIGURATIONS < <(awk -F ' = ' '/experiment_configurations/ {gsub(/, /, "\n", $2); print $2}' "$CONFIG_FILE_PATH")
fi

VERBOSE_LEVEL=0
if [ "$3" == "-v" ]; then
    VERBOSE_LEVEL=1
elif [ "$3" == "-vv" ]; then
    VERBOSE_LEVEL=2
fi

# Save the old config run value
OLD_CONFIG_RUN=$(read_config_value "experiment_run_configuration")

for config in "${CONFIGURATIONS[@]}"; do
    update_experiment_run_configuration "$config"
    RUN_SCRIPT_NAME=run_$config.sh
    CONFIG_RUN_SCRIPT="$EXPERIMENT_DIR/scripts/$RUN_SCRIPT_NAME"

    if [ -x "$CONFIG_RUN_SCRIPT" ]; then
        echo "Executing $RUN_SCRIPT_NAME"
        case $VERBOSE_LEVEL in
            0) "$CONFIG_RUN_SCRIPT" &>/dev/null ;;
            1) "$CONFIG_RUN_SCRIPT" 2>/dev/null ;;
            2) "$CONFIG_RUN_SCRIPT" ;;
            *) echo "Error: Invalid verbosity level" ;;
        esac

        if [ $? -ne 0 ]; then
            echo "Error: Script $RUN_SCRIPT_NAME failed to execute successfully."
            exit 3
        fi
    else
        echo "Error: Configuration script $RUN_SCRIPT_NAME does not exist or is not executable."
    fi
done

# Restore the old config run value
write_config_value "experiment_run_configuration" "$OLD_CONFIG_RUN"
