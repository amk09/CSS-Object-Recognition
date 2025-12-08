#!/usr/bin/env python3
"""
Create GIF from animation frames using Python PIL/Pillow
"""
import sys
import os
from pathlib import Path
try:
    from PIL import Image
except ImportError:
    print("Error: PIL/Pillow not installed. Install with: pip install Pillow")
    sys.exit(1)

def create_gif(frames_dir, output_path, duration=150):
    """
    Create GIF from PNG frames in a directory
    
    Args:
        frames_dir: Directory containing frame_*.png files
        output_path: Output GIF path
        duration: Duration per frame in milliseconds
    """
    frames_path = Path(frames_dir)
    
    # Get all frame files sorted by number
    frame_files = sorted(frames_path.glob("frame_*.png"))
    
    if not frame_files:
        print(f"Error: No frame files found in {frames_dir}")
        return False
    
    print(f"Found {len(frame_files)} frames")
    
    # Load all frames
    frames = []
    for frame_file in frame_files:
        img = Image.open(frame_file)
        frames.append(img)
    
    # Save as GIF
    if frames:
        frames[0].save(
            output_path,
            save_all=True,
            append_images=frames[1:],
            duration=duration,
            loop=0
        )
        print(f"GIF saved to: {output_path}")
        return True
    
    return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 create_gif.py <frames_directory> [output.gif] [duration_ms]")
        sys.exit(1)
    
    frames_dir = sys.argv[1]
    output_gif = sys.argv[2] if len(sys.argv) > 2 else frames_dir.rstrip('/') + '.gif'
    duration = int(sys.argv[3]) if len(sys.argv) > 3 else 150
    
    if create_gif(frames_dir, output_gif, duration):
        print("Success!")
    else:
        print("Failed to create GIF")
        sys.exit(1)
