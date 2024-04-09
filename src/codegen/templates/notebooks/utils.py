import pandas as pd
import numpy as np
import logging
import os
import sys

# Basic logging configuration
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def convert_latency(df, target_unit):
    """
    Converts the latency values in the DataFrame to the specified unit.

    Parameters:
        df (pandas.DataFrame): The DataFrame with latency values.
        target_unit (str): The target unit for conversion ('ns', 'us', 'ms', 's').

    Returns:
        pandas.DataFrame: The DataFrame with converted latency values.
    """
    # Conversion factors from nanoseconds to the desired units
    conversion_factors = {
        'ns': 1,
        'us': 1e3,  # Nanoseconds to microseconds
        'ms': 1e6,  # Nanoseconds to milliseconds
        's': 1e9,   # Nanoseconds to seconds
    }

    if target_unit in conversion_factors:
        conversion_factor = conversion_factors[target_unit]
        df['latency'] = df['latency'] / conversion_factor
        logging.info(f"Latency values converted to {target_unit}.")
    else:
        logging.warning(f"Unknown target unit '{target_unit}'. No conversion applied.")
    
    return df

def load_data(file_path, config):
    """
    Loads the data from the specified CSV file and adds a configuration column based on the directory name if required.

    Parameters:
        file_path (str): The path to the CSV file.
        config: Dictionary of config variables and flags.

    Returns:
        pandas.DataFrame: The loaded data.
    """
    try:
        # Load the CSV file
        df = pd.read_csv(file_path)
        
        # Convert latency values to the specified unit
        df = convert_latency(df, config["latency_unit"])

        # Add configuration column by extracting the directory name
        config_name = os.path.basename(os.path.dirname(file_path))
        df['configuration'] = config_name
        
        logging.info("Data loaded successfully.")
        return df
    except Exception as e:
        logging.error(f"Failed to load data: {e}")
        sys.exit(1)

def remove_outliers(df, column_name):
    """
    Removes outliers from a specific column in the DataFrame based on the IQR method.

    Parameters:
        df (pandas.DataFrame): The DataFrame to process.
        column_name (str): The name of the column from which to remove outliers.

    Returns:
        pandas.DataFrame: The DataFrame with outliers removed.
    """
    Q1 = df[column_name].quantile(0.25)
    Q3 = df[column_name].quantile(0.75)
    IQR = Q3 - Q1

    # Define the criteria for non-outliers
    criteria = (df[column_name] >= (Q1 - 1.5 * IQR)) & (df[column_name] <= (Q3 + 1.5 * IQR))
    
    # Filter the DataFrame based on the criteria
    filtered_df = df[criteria]
    
    logging.info(f"Outliers removed from column '{column_name}'.")
    return filtered_df

def process_data(file_path, config):
    """
    Main function to process the data based on the provided configuration.

    Parameters:
        file_path (str): The path to the CSV file.
        config (dict): Configuration for processing, including logging and outlier removal.
    """
    df = load_data(file_path, config)
    
    if config.get("remove_outliers"):
        column_name = config["outlier_column"]
        df = remove_outliers(df, column_name)

    return df
