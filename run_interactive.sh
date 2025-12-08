#!/bin/bash

# Helper script to run CSS Interactive Visualization with proper environment

# Load required modules
module load opencv

# Set library paths
export LD_LIBRARY_PATH=/oscar/runtime/software/external/opencv/4.6.0/lib64:$LD_LIBRARY_PATH

# Run the interactive tool with all arguments
./bin/css_interactive "$@"
