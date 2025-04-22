import sys
import os
import json

# Add the src/resources directory to Python path
sys.path.append(os.path.join(os.path.dirname(__file__), 'src', 'resources'))

from nlp_processor import process_text

def test_long_text():
    # Test text - a longer article about AI
    test_text = """
    Artificial Intelligence: A Comprehensive Overview

    Artificial Intelligence (AI) has become one of the most transformative technologies of our time. 
    From self-driving cars to virtual assistants, AI is reshaping how we live and work. This article 
    explores the current state of AI, its applications, and future implications.

    Understanding AI
    AI refers to the simulation of human intelligence in machines that are programmed to think and 
    learn like humans. The term may also be applied to any machine that exhibits traits associated 
    with a human mind, such as learning and problem-solving.

    Key Components of AI
    1. Machine Learning: A subset of AI that enables systems to learn and improve from experience 
    without being explicitly programmed.
    2. Deep Learning: A more complex form of machine learning that uses neural networks with many 
    layers to analyze various factors of data.
    3. Natural Language Processing: The ability of computers to understand, interpret, and generate 
    human language.
    4. Computer Vision: The field of AI that trains computers to interpret and understand visual 
    information from the world.

    Applications of AI
    AI is being applied across numerous industries:
    - Healthcare: AI is used for disease detection, drug discovery, and personalized medicine.
    - Finance: AI powers fraud detection, algorithmic trading, and risk assessment.
    - Transportation: Self-driving cars and traffic management systems rely on AI.
    - Education: AI enables personalized learning and automated grading systems.
    - Manufacturing: AI optimizes production processes and predictive maintenance.

    Challenges and Considerations
    While AI offers tremendous potential, it also presents several challenges:
    1. Ethical Concerns: Issues around privacy, bias, and decision-making accountability.
    2. Job Displacement: The potential impact on employment and workforce transformation.
    3. Security Risks: Vulnerabilities in AI systems and potential misuse.
    4. Technical Limitations: Current constraints in AI capabilities and understanding.

    Future of AI
    The future of AI holds both promise and uncertainty. Key areas of development include:
    - General AI: Systems that can perform any intellectual task that a human can.
    - Quantum Computing: Potential to revolutionize AI processing capabilities.
    - AI Ethics: Development of frameworks for responsible AI deployment.
    - Human-AI Collaboration: New ways for humans and AI to work together effectively.

    Conclusion
    AI continues to evolve at a rapid pace, offering both opportunities and challenges. As we 
    navigate this technological revolution, it's crucial to balance innovation with ethical 
    considerations and ensure that AI development benefits humanity as a whole.
    """
    
    # Test different modes
    modes = ["brief", "detailed", "bullet_points"]
    
    for mode in modes:
        print(f"\nTesting mode: {mode}")
        result = process_text(test_text, mode)
        print("Result:", json.loads(result))

if __name__ == "__main__":
    test_long_text() 