#!/bin/bash

# Helper function for output alignment
print_aligned() {
    printf "%-25s %s\n" "$1" "$2"
}

# Helper function to extract library versions
extract_lib_version() {
    lib_name=$1
    shift # Remove the first argument
    lib_version_command="$@"
    
    if version=$($lib_version_command 2>/dev/null | head -n 1); then
        if [ -z "$version" ]; then
            version="Not Available"
        fi
    else
        version="Not Installed"
    fi
    print_aligned "$lib_name Version:" "$version"
}

echo "Hardware Information:"
echo "---------------------"

# CPU information
print_aligned "CPU:" "$(lscpu | grep 'Model name' | sed 's/Model name: *//')"

# CPU Cores
print_aligned "CPU Cores:" "$(lscpu | grep '^CPU(s):' | awk '{print $2}')"

# CPU Cache sizes
print_aligned "CPU Cache (L1):" "$(lscpu | grep 'L1d cache' | sed 's/L1d cache: *//')"
print_aligned "CPU Cache (L2):" "$(lscpu | grep 'L2 cache' | sed 's/L2 cache: *//')"
print_aligned "CPU Cache (L3):" "$(lscpu | grep 'L3 cache' | sed 's/L3 cache: *//')"

# CPU Frequency
print_aligned "CPU Min Frequency:" "$(lscpu | grep 'CPU min MHz' | awk '{print $4 " MHz"}')"
print_aligned "CPU Max Frequency:" "$(lscpu | grep 'CPU max MHz' | awk '{print $4 " MHz"}')"

# Memory information
print_aligned "Memory:" "$(free -h | grep 'Mem' | awk '{print $2 " total"}')"

# Disk information
print_aligned "Disk:" "$(lsblk | grep 'disk' | awk '{print $1, $4}' | tr '\n' ', ' | sed 's/, $//')"

# Turbo Boost
if [ -f /sys/devices/system/cpu/intel_pstate/no_turbo ]; then
    no_turbo=$(cat /sys/devices/system/cpu/intel_pstate/no_turbo)
    if [ "$no_turbo" -eq 0 ]; then
        print_aligned "Turbo Boost:" "Enabled"
    else
        print_aligned "Turbo Boost:" "Disabled"
    fi
elif [ -f /sys/devices/system/cpu/cpufreq/boost ]; then
    boost=$(cat /sys/devices/system/cpu/cpufreq/boost)
    if [ "$boost" -eq 1 ]; then
        print_aligned "Turbo Boost:" "Enabled"
    else
        print_aligned "Turbo Boost:" "Disabled"
    fi
else
    print_aligned "Turbo Boost:" "Not Supported"
fi

# Hyper-Threading
physical_cores=$(lscpu | grep "^Core(s) per socket:" | awk '{print $4}')
logical_processors=$(lscpu | grep "^CPU(s):" | awk '{print $2}')
if [ "$logical_processors" -gt "$physical_cores" ]; then
    print_aligned "Hyper-Threading:" "Enabled"
else
    print_aligned "Hyper-Threading:" "Disabled"
fi

# Network adapters and speeds
echo "Network Interfaces and Speeds:"
for iface in $(ls /sys/class/net/ | grep -v lo); do
   speed=$(cat /sys/class/net/$iface/speed 2>/dev/null)
   if [ -z "$speed" ] || [ "$speed" == "-1" ]; then
       speed="Unknown"
   else
       speed="${speed} Mbps"
   fi
   print_aligned "  $iface:" "$speed"
done

echo ""
echo "Software Information:"
echo "---------------------"

# Kernel version
print_aligned "Kernel:" "$(uname -r)"

# Boot arguments
print_aligned "Boot args:" "$(cat /proc/cmdline)"

# Kernel hash
kernel_version=$(uname -r)
print_aligned "Kernel Hash:" "$(md5sum /boot/vmlinuz-"$kernel_version" | awk '{print $1}')"

# OS version
print_aligned "OS:" "$(lsb_release -d | awk -F: '{ $1=""; sub(/^[ \t]+/, "", $0); print $0}')"

# GLIBC version
print_aligned "GLIBC:" "$(ldd --version | grep 'ldd' | awk '{print $NF}')"

# Linker version
extract_lib_version "Linker" "ld --version | head -n 1"

# GCC version
extract_lib_version "GCC" "gcc --version | head -n 1"

# Commonly used shared libraries versions
extract_lib_version "OpenSSL" "openssl version"

