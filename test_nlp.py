import sys
import os
import json

# Add the src/resources directory to Python path
sys.path.append(os.path.join(os.path.dirname(__file__), 'src', 'resources'))

from nlp_processor import process_text

def test_nlp():
    # Test text
    test_text = """
    The quick brown fox jumps over the lazy dog. This is a test sentence to verify that 
    the NLP processing is working correctly. The application should be able to process 
    this text and generate a summary.
    """
    
    # Test different modes
    modes = ["brief", "detailed", "bullet_points"]
    
    for mode in modes:
        print(f"\nTesting mode: {mode}")
        result = process_text(test_text, mode)
        print("Result:", json.loads(result))

if __name__ == "__main__":
    test_nlp() 