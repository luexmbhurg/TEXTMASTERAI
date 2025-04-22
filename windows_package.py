
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Windows packaging script for TextMaster application.
This script is used by PyInstaller to create a standalone Windows executable.
"""

import os
import sys
import subprocess
import atexit
import time

def main():
    print("Starting TextMaster application...")
    
    # Get the directory containing the executable
    basedir = os.path.dirname(sys.executable)
    if getattr(sys, 'frozen', False):
        # Running as compiled executable
        basedir = sys._MEIPASS
    
    # Set up resource paths
    resource_dir = os.path.join(basedir, 'resources')
    if not os.path.exists(resource_dir):
        os.makedirs(resource_dir)
    
    # Install required Python packages if they're not already installed
    try:
        import spacy
        import numpy
        import transformers
        import torch
    except ImportError:
        print("Installing required Python packages...")
        subprocess.check_call([sys.executable, "-m", "pip", "install", 
                               "spacy", "numpy", "transformers", "torch"])
        
        # Download spaCy model
        subprocess.check_call([sys.executable, "-m", "spacy", "download", "en_core_web_sm"])
    
    # Launch the Qt application
    textmaster_exe = os.path.join(basedir, "TextMaster.exe")
    if os.path.exists(textmaster_exe):
        subprocess.Popen([textmaster_exe])
    else:
        print(f"Error: TextMaster executable not found at {textmaster_exe}")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
