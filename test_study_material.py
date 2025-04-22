import sys
import os
import json

# Add the src/resources directory to Python path
sys.path.append(os.path.join(os.path.dirname(__file__), 'src', 'resources'))

from nlp_processor import process_text

def test_study_material():
    # Sample study material about Newton's Laws of Motion
    test_text = """
    Newton's Laws of Motion: A Comprehensive Study Guide

    First Law (Law of Inertia)
    An object at rest stays at rest, and an object in motion stays in motion unless acted upon by an external force.
    This is defined as the law of inertia. Inertia is the tendency of an object to resist changes in its state of motion.

    Second Law (Force and Acceleration)
    The acceleration of an object is directly proportional to the net force acting on it and inversely proportional to its mass.
    This can be expressed by the formula: F = ma, where F is force, m is mass, and a is acceleration.
    The unit of force is the Newton (N), which is defined as kg·m/s².

    Third Law (Action-Reaction)
    For every action, there is an equal and opposite reaction.
    This means that forces always occur in pairs. When object A exerts a force on object B, object B exerts an equal and opposite force on object A.

    Key Concepts:
    1. Force is defined as a push or pull on an object.
    2. Mass is the amount of matter in an object.
    3. Acceleration is the rate of change of velocity.
    4. Inertia is the resistance of an object to changes in its motion.

    Practice Questions:
    What is the relationship between force, mass, and acceleration?
    How does the law of inertia apply to everyday situations?
    Why do action and reaction forces not cancel each other out?
    Calculate the force needed to accelerate a 5 kg object at 2 m/s².
    """
    
    # Test different modes
    modes = ["brief", "detailed", "bullet_points"]
    
    for mode in modes:
        print(f"\nTesting mode: {mode}")
        result = process_text(test_text, mode)
        print("Result:", json.loads(result))

if __name__ == "__main__":
    test_study_material() 